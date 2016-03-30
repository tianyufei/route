/* 
 * Copyright (c) 2014 Yaya Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Draft:  zhengdajun 
 */
#include <list>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/timerfd.h>
#include <map>
#include "common.h"
#include "inner.h"


#define EVT_PATH "libyaya_event.so"
#define LOOP_PATH "libyaya_loop.so"
#define RPC_PATH "libyaya_rpc.so"
extern "C" {

extern void debugDoutSwitch(boolean flag);
extern void debugDout(char* pname, const char* file, const char* func, int line, char* fmt, ...);
extern void debugEout(char* pname, const char* file, const char* func, int line, char* fmt, ...);
extern void debugChangeTerm(char* term);

}

class Context : public IContext,public IEventObserver,public ITimer, public ILoopData, public IRpcMethod {
private:
	DllMgr* mLoopMgr;
	DllMgr* mRpcMgr;
	DllMgr* mEventMgr;
	string mName;
	clock_t mTimeHead;

	
	class TimerHandler {
		public:
			TimerHandler(IContext* ctx){
				mFd = -1;
				mTm = NULL;
				mContext = ctx;
				mOnce = 1;
				mActive = true;
			};
			~TimerHandler(){
				if(NULL == mContext)
					return;
				ILoop* loop = mContext->loopFactory();
				if(loop)
					loop->delFD(mFd);
				close(mFd);
			};
			int mFd;	
			ITimer* mTm;	
			IContext* mContext;
			boolean mActive;
			boolean mOnce;
	};
	map<int,TimerHandler*> mTimerList;
	
public:
	Context():IContext(),IEventObserver(),ITimer(),ILoopData(){
		mLoopMgr =  NULL;
		mRpcMgr =  NULL;
		mEventMgr =  NULL;
	}
	~Context(){
		if(mLoopMgr)
			delete mLoopMgr;
		if(mEventMgr)
			delete mEventMgr;
		if(mRpcMgr)
			delete mRpcMgr;

		mLoopMgr = NULL;
		mEventMgr = NULL;
		mRpcMgr = NULL;
		
		map<int,TimerHandler*>::iterator it;
		for(it = mTimerList.begin(); it != mTimerList.end(); it++){
			map<int,TimerHandler*>::value_type pair = *it;
			TimerHandler* tmp = pair.second;
			if(tmp)
				delete tmp;
		}
		mTimerList.clear();
	}

	//ILoopData
	status handler(short event,int fd){
		TimerHandler *tmr = mTimerList[fd];
		if(tmr){
			if(!tmr->mActive){
				delete tmr;
				mTimerList.erase(fd);
				return s_ok;
			}
			long long time_count;
			if(read(tmr->mFd, &time_count, sizeof(time_count)) > 0) {
				if(tmr->mTm){
					tmr->mTm->timeArrive(tmr->mFd);
				}
			}
			if(tmr->mOnce ){
				mTimerList.erase(fd);
				delete tmr;
			}
		}
		return s_ok;
	}
	//
	status init(int argc, char* argv[]){
		string name = argv[0];
		int n = name.find_last_of("/");
		if(n == string::npos)
			n = -1;
		mName = name.substr(n+1,name.length()-n-1);

		IEvent* event = eventFactory();
		if(event){
			event->registObserver(E_YOS_GDB_OPEN,this);
			event->registObserver(E_YOS_GDB_CLOSE,this);
			event->registObserver(E_YOS_GDB_TRMC,this);
			return s_ok;
		}
		return s_err_syscall;
	}

	string getName(){
		return mName;
	}

	status methodCalled(string method_name,IRpcParameter* param, IRpcParameter* reply){
		return s_ok;
	}


	status update(string event_owner, string event, string data,string data2, string data3, string data4){
		data = termString(data);
		if(event == string(E_YOS_GDB_OPEN)){
			if(string::npos != data.find(getName())){
				debugDoutSwitch(true);			
			}
			else if(string::npos != data.find("all")){
				debugDoutSwitch(true);			
			}
		}else if(event == string(E_YOS_GDB_CLOSE)){
			if(string::npos != data.find(getName())){
				debugDoutSwitch(false);			
			}
			else if(string::npos != data.find("all")){
				debugDoutSwitch(false);			
			}
		}else if(event == E_YOS_GDB_TRMC){
			//切换显示终端
			debugChangeTerm(s2c(data));
		}
		return s_ok;
	}


	void debugSwitch(boolean open,string term_name){
		debugDoutSwitch(open);
		if(!term_name.empty())
			debugChangeTerm(s2c(term_name));
	}
	
	IEvent* eventFactory(){
		if(mEventMgr == NULL)
			mEventMgr = new DllMgr(EVT_PATH);
		if(mEventMgr)
			return (IEvent*)mEventMgr->getInstance(this);
		DOUT("event Instance create failed!");
		return NULL;
	}

	ILoop* loopFactory(){
		if(mLoopMgr == NULL)
			mLoopMgr = new DllMgr(LOOP_PATH);
		if(mLoopMgr)
			return (ILoop*)mLoopMgr->getInstance(this);
		DOUT("loop Instance create failed!");
		return NULL;
	}

	IRpc* rpcFactory(){
		if(mRpcMgr == NULL)
			mRpcMgr = new DllMgr(RPC_PATH);
		if(mRpcMgr)
			return (IRpc*)mRpcMgr->getInstance(this);
		DOUT("rpc Instance create failed!");
		return NULL;
	}

	/**
	*系统调用
	*/
	int systemCall(const char* fmt, ...){
		char buf[SHELL_CMD_MAX_LEN] = {0};
	//	ILog *log = logFactory();
		int rtn = -1;

		va_list argp;
		va_start( argp, fmt );
		vsnprintf(buf,SHELL_CMD_MAX_LEN-1,fmt,argp);
		va_end( argp ); 

		rtn = system(buf);

	//	if(log){
			if (-1 == rtn){
				DOUT("system cmd(%s) failed,errorcode=-1",buf);
		//		log->write(getName(),enLevDebug,"system cmd(%s) failed,errorcode=-1",buf);
			}else{	
				if ((WIFEXITED(rtn)) && (0 == WEXITSTATUS(rtn)) ){
					DOUT("shell cmd(%s) successful!",buf);
		//			log->write(getName(),enLevDebug,"shell cmd(%s) successful!",buf);
					return s_ok;
				}else{
					DOUT("shell cmd(%s) failed!, err=%d",buf,rtn);
		//			log->write(getName(),enLevDebug,"shell cmd(%s) failed,errorcode=%d",buf,WEXITSTATUS(rtn));
				}
			}	
	//	}
		return s_err_syscall;
	}

	/**
	*系统调用
	*/
	FILE* popenCall(const char* fmt, ...){
		char buf[SHELL_CMD_MAX_LEN] = {0};
		va_list argp;
		va_start( argp, fmt );
		vsnprintf(buf,SHELL_CMD_MAX_LEN-1,fmt,argp);
		va_end( argp ); 
		DOUT("popen cmd: %s",buf);
//		ILog *log = logFactory();
//		if(log)
//			log->write(getName(),enLevDebug,buf);
		return popen(buf,"r");		
	}

	void timeArrive(int id){
	}

	void stopTimer(int fd){
		TimerHandler* tmr = mTimerList[fd];
		if(tmr){
			tmr->mActive = false;
		}else{
			mTimerList.erase(fd);
		}
	}

	int startTimer(int sec, int usec, boolean once, ITimer* cb){
		return setTimer(sec,usec,sec,usec,once,cb);
	}


	int setTimer(int sec, int usec, int left_sec, int left_usec, bool once, ITimer* cb){
		struct itimerspec new_tv;
		int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
		memset(&new_tv, 0, sizeof(struct itimerspec));
		if(!once){
			new_tv.it_interval.tv_sec = sec;
			new_tv.it_interval.tv_nsec = usec;
		}
		
		new_tv.it_value.tv_sec = left_sec;
		new_tv.it_value.tv_nsec = left_usec;
		if (timerfd_settime(timer_fd, 0, &new_tv, NULL) < 0) {
			DOUT("Failed to set timer");
			ASSERT_STATUS(s_err_syscall);
		}

		ILoop* loop = loopFactory();
		if(loop){
			TimerHandler* tmr = new TimerHandler(this);
			if(tmr){
				tmr->mFd = timer_fd;
				tmr->mTm = cb;
				tmr->mOnce = once;
				mTimerList[timer_fd] = tmr;
				loop->addFD(timer_fd,ILoop::enFlagRead,this);
				return timer_fd;
			}
		}
		
		ASSERT_STATUS(s_err_syscall);
	}
};


static Context* g_context = NULL;

IContext* getContext(){
	if(!g_context)
		g_context = new Context();
	return g_context;
}

void releaseContext(IContext* ctx){
	if(g_context){
		delete g_context;
	}
	g_context = NULL;
}

