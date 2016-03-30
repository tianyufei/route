/* 
 * Copyright (c) 2014 Yaya Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Draft:  zhengdajun 
 */

#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include "common.h"
#include "inner.h"

#define RPC_CONTEXT_QUIT "RPC_CONTEXT_QUIT"

class Loop : public ILoop{
private:
	map<int,ILoopData*> mLoopDataTable;
	IContext *mContext;
	int mPollFD;
	boolean mQuit;
public:
	Loop(IContext* ctx);
	~Loop();
	status addFD(int fd, ILoop::LoopFlag flag, ILoopData* data);
	status delFD(int fd);
	status run();
	void quit();
	ILoopData* getFD(int fd);
};



#ifndef EPOLLRDHUP
/*
 * EPOLLRDHUP is supported since linux 2.6.17,
 * but some libc may not include it into header file
 * Openwrt backfire uses linux version 2.6.32, but without
 * this support in ucLibc header files
 */
#define EPOLLRDHUP 0x2000
#endif /* EPOLLRDHUP */
#define LOOP_MAX_EVENTS 100


Loop::Loop(IContext* ctx){
	mLoopDataTable.empty();
	mContext = ctx;
	mQuit = false;
	mPollFD = epoll_create(32);
	if(mPollFD < 0){
		DOUT("epoll create error!\n");
	}
}

Loop::~Loop(){
	mLoopDataTable.empty();
	if(mPollFD >= 0)
		close(mPollFD);
}

status Loop::addFD(int fd,ILoop::LoopFlag flag, ILoopData * data){
	if(fd == -1 || mPollFD < 0){
		DOUT("param error!\n");
		return false;
	}
	
	int op = EPOLL_CTL_ADD;
	if(mLoopDataTable[fd] != NULL)
		op = EPOLL_CTL_MOD;
	mLoopDataTable[fd] = data;

	struct epoll_event ev;
	
	memset(&ev, 0, sizeof(struct epoll_event));

	if (flag & ILoop::enFlagRead)
		ev.events |= EPOLLIN | EPOLLRDHUP;

	if (flag & ILoop::enFlagWrite)
		ev.events |= EPOLLOUT;

	if (flag & ILoop::enFlagEpollEt)
		ev.events |= EPOLLET;

	ev.data.fd = fd;
	//ev.data.ptr = (void*)data;

	if(!epoll_ctl(mPollFD, op, fd, &ev)){
		return true;
	}
	//ERROR
	mLoopDataTable[fd] = NULL;
	DOUT("add fd to Loop failed, err=%d !\n",errno);
	return false;
}

status Loop::delFD(int fd){
	ILoopData* data = mLoopDataTable[fd];
	if(NULL == data){
		return true;
	}

	if(!epoll_ctl(mPollFD, EPOLL_CTL_DEL, fd, 0)){
		mLoopDataTable[fd] = NULL;
		return true;
	}
	//ERROR
	DOUT("delete fd(%d) from Loop failed, err=%d !\n",fd,errno);
	return false;
}

ILoopData* Loop::getFD(int fd){
	return mLoopDataTable[fd];
}

void Loop::quit(){
	mQuit = true;
	if(mContext){
		IRpc* rpc = mContext->rpcFactory();
		if(rpc){
			IRpcParameter* param = rpc->initParameter();
			if(param){
				rpc->asyncCall(DEFAULT_OBJ(mContext->getName()),DEFAULT_INTF(mContext->getName()),RPC_CONTEXT_QUIT,param);
			}
			rpc->deinitParameter(param);
		}
	}
}


status Loop::run(){
	if(mPollFD < 0){
		DOUT("Loop fd error! \n");
		return s_err_init;
	}
	
	struct epoll_event events[LOOP_MAX_EVENTS];

	while(1){
		memset(events,0,sizeof(events));
		
		int nfds = epoll_wait(mPollFD, events, LOOP_MAX_EVENTS, -1);
		for (int i = 0; i < nfds; i++) {
			//ILoopData *data = (ILoopData*)(events[i].data.ptr);
			ILoopData* data = mLoopDataTable[events[i].data.fd];
			if (data) {
				if(mQuit){
					mQuit = false;
					return s_ok;
				}
				data->handler(events[i].events,events[i].data.fd);
			}
		}

	}
	return s_ok;
}


//for dlopen.

extern "C" {

void* getInstance(void* ctx){
	return (void*)(new Loop((IContext*)ctx));
}


void relInstance(void* rpc){
	Loop* handl = (Loop*)rpc;
	if(handl)
		delete handl;
}

}

