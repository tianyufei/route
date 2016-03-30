/* 
 * Copyright (c) 2014 Yaya Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Draft:  zhengdajun 
 */

#include "irpc_dbus.h"

RPCHandler::RPCHandler(IContext* ctx):IRpc(),DBusCB(){
	mContext = ctx;
	mInSyncMethod = 0;
	if(mContext){
		mDBus = new DBus(mContext->getName(),mContext->loopFactory());
		if(mDBus){
			mDBus->registCB(this);
			listenIntf(mDBus->mIntf);
			listenObj(mDBus->mObj);
		}
	}
}

RPCHandler::~RPCHandler(){
	list<RpcParam*>::iterator it;

	if(mDBus){
		mDBus->unregistCB(this);
		delete mDBus;
	}
	mDBus = NULL;

}

status RPCHandler::listenObj(string obj){
	if(NULL == mDBus){
		ASSERT_STATUS(s_err_init);
	}
	return mDBus->listenObj(obj);
}

status RPCHandler::listenIntf(string intf){
	if(NULL == mDBus){
		ASSERT_STATUS(s_err_init);
	}
	return mDBus->listenIntf(intf);
}


status RPCHandler::cbAsync(string intf, string method_name, IRpcParameter* param){
	IRpcMethod* method = mMethodAsyncTable[method_name];
	if(method)
		return method->methodCalled(method_name,param,NULL);
	EOUT("method_name=%s,not find!,name=%s",s2c(method_name),s2c(mContext->getName()));
	ASSERT_STATUS(s_err_notfound);
}
	
status RPCHandler::cbSync(string intf, string method_name, IRpcParameter* param,IRpcParameter* reply){
	IRpcMethod* method = mMethodSyncTable[method_name];
	if(method){
		mInSyncMethod++;
		int rtn = method->methodCalled(method_name,param,reply);
		mInSyncMethod--;
		return rtn;
	}
	DOUT("not find! intf=%s, method_name=%s \n",intf.c_str(), method_name.c_str());
	return s_err_notfound;
}

status RPCHandler::asyncCall(string obj, string intf, string method, IRpcParameter* param){
	if(NULL == mDBus){
		ASSERT_STATUS(s_err_init);
	}
//	if(string(DEFAULT_OBJ(mContext->getName())) == obj){
//		return cbAsync(intf,method,param);
//	}else{
		return mDBus->sendSignal(obj,intf,method,param);
//	}
}

status RPCHandler::syncCall(string server, string obj, string intf, string method, IRpcParameter* param, IRpcParameter* reply,int timeout){
	if(NULL == mDBus){
		ASSERT_STATUS(s_err_init);
	}
	
	if(string(DEFAULT_NAME(mContext->getName())) == server){
		int	result = cbSync(intf,method,param,reply);
		RpcParam* temp = (RpcParam*)reply;
		if(temp)
			temp->push_int_to_first(result);
		return result;
	}else{
		if(mInSyncMethod){
			ASSERT_STATUS(s_err_call_in_sync);
		}
		return mDBus->sendMethod(server,obj,intf,method,param,reply,timeout);
	}
}

status RPCHandler::registMethod(string method_name,boolean sync, IRpcMethod *rm){
	if(!sync){
		mMethodAsyncTable[method_name] = rm;
	}else{
		mMethodSyncTable[method_name] = rm;
	}
	return s_ok;
}	

status RPCHandler::unRegistMethod(string method_name,boolean sync, IRpcMethod *rm) {
	if(!sync){
		IRpcMethod* tmp = NULL;
		tmp = mMethodAsyncTable[method_name];
		if((tmp == rm) || (tmp == NULL)){
			mMethodAsyncTable.erase(method_name);
		}
	}else{
		IRpcMethod* tmp = NULL;
		tmp = mMethodSyncTable[method_name];
		if((tmp == rm) || (tmp == NULL)){
			mMethodSyncTable.erase(method_name);
		}
	}
	return s_ok;
}


IRpcParameter* RPCHandler::initParameter(){
	if(NULL == mDBus){
		DOUT("%s",status_errinfo(s_err_init));
		return NULL;
	}
	return mDBus->initParameter();
}

void RPCHandler::deinitParameter(IRpcParameter* param){
	if(mDBus)
		mDBus->deinitParameter((RpcParam*)param);
}


//for dlopen.
extern "C" {

void* getInstance(void* ctx){
	return (void*)(new RPCHandler((IContext*)ctx));
}


void relInstance(void* rpc){
	RPCHandler* handl = (RPCHandler*)rpc;
	if(handl)
		delete handl;
}

}

