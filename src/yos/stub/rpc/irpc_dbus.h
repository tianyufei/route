/* 
 * Copyright (c) 2014 Yaya Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Draft:  zhengdajun 
 */

#ifndef _IRPC_DBUS_H 
#define _IRPC_DBUS_H 
#include <dbus/dbus.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <list>
#include "common.h"
#include "iloop.h"
#include "irpc.h"
#include "icontext.h"



/******************************************************************************
IRPC中IRpcParameter接口的实例
*******************************************************************************/
class ParamNode{
	public:
		RT_TYPE mType;
		string v_str;
		int v_int;
		boolean v_bool;
		char v_byte;
		unsigned int v_uint;
		void* v_object;
		ParamNode();
		~ParamNode();
		ParamNode* dup();
};

class  RpcParam : public IRpcParameter{
private:

	list<ParamNode*> mParamNodeList;
	list<ParamNode*>::iterator mIt;
		
public:
	RpcParam();
	~RpcParam();
	//for read.
	RT_TYPE getType();
	char getByte();
	string getStr();
	int getInt();
	unsigned int getUint();
	int getBool();
	void* getObject();
	boolean next();
	//for write.
	void setByte(char v);
	void setStr(string v);
	void setInt(int v);
	void setUint(unsigned int v);
	void setBool(boolean v);
	 void setObject(void* v);
	//inner
	string mDestServer;
	string mDestObj;
	string mDestIntf;
	string mMethod;
	void push_int_to_first(int v);
	void init();
	RpcParam* dup();
	void reset();
	void clone(IRpcParameter* dest);
};



/******************************************************************************
DBUS使用到的接口
*******************************************************************************/
class DBusCB {
	public:
		DBusCB(){};
		virtual ~DBusCB(){};
		virtual status cbSync(string intf, string method, IRpcParameter* param,IRpcParameter* reply) = 0;
		virtual status cbAsync(string intf, string method, IRpcParameter* param) = 0;
};
/******************************************************************************
对DBUS进行封装
*******************************************************************************/
class DBus {
private:
	DBusConnection* mConn;
	string mName;
	string mObj;
	string mIntf;
	friend class DBusWatchHandle;
	friend class RPCHandler;
	DBusCB *mCB;
private:
	status init(string name, ILoop* loop);
	status create();
public:
	ILoop* mLoop;

	DBus(string name,ILoop*);
	~DBus();
	status registCB(DBusCB *);
	status unregistCB(DBusCB *);
	status sendSignal(string obj, string intf, string method, IRpcParameter* param);
	status sendMethod(string server, string obj, string intf, string method, IRpcParameter* param, IRpcParameter* reply,int timeout);
	status replyToMethodCall(DBusMessage* msg, IRpcParameter* param);

	/**
	*监听目标
	*@param obj: 目标全名。比如"com.ittim.event"
	*/
	status listenObj(string obj);
	/**
	*监听接口，接口名必须与obj名相匹配，否则监听不到。 比配规则为obj名字作为intf名字的前缀。
	*@param intf: 接口的全名. 比如"com.ittim.event.intf" 
	*/
	status listenIntf(string intf);

	class DBusWatchHandle : public ILoopData {
		private:
			DBus* mServer;
			DBusWatch* mWatch;
		public:
			//ILoopData
			DBusWatchHandle(DBus* server);
			~DBusWatchHandle();
			status handler(short event,int fd);
			status addWatch(DBusWatch* watch);
	};
	RpcParam* initParameter();
	void deinitParameter(RpcParam* p);
	status recv(DBusMessage* msg);
	map<DBusWatchHandle*,boolean> mDBusWatchHandleList;

};

/******************************************************************************
	关于IRPC 子类的定义
*******************************************************************************/
class RPCHandler : public IRpc,public DBusCB {
private:
	IContext* mContext;
	DBus* mDBus;
	
	map<string,IRpcMethod*> mMethodAsyncTable;
	map<string,IRpcMethod*> mMethodSyncTable;
	int mInSyncMethod;

public:
	RPCHandler(IContext*) ;
	~RPCHandler();
	//DBusCB functions
	status cbSync(string intf, string method, IRpcParameter* param,IRpcParameter* reply);
	status cbAsync(string intf, string method, IRpcParameter* param);

	//IRpc functions
	status listenObj(string obj);
	status listenIntf(string intf);
	status asyncCall(string obj, string intf, string method, IRpcParameter* param);
	status syncCall(string server, string obj, string intf, string method, 
		IRpcParameter* param, IRpcParameter* reply,int timeout);
//	status delayCall(IAsyncCall*);
//	status oneshotCall(string obj, string intf, string method, IRpcParameter* param);
	status registMethod(string method_name,boolean need_reply, IRpcMethod *rm);
	status unRegistMethod(string method_name,boolean sync, IRpcMethod *rm);
	IRpcParameter* initParameter();
	void deinitParameter(IRpcParameter*);

};

#endif

