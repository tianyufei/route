/* 
 * Copyright (c) 2014 Yaya Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Draft:  zhengdajun 
 */
#include <dbus/dbus.h>
#include <sys/poll.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>

#include "irpc_dbus.h"
#include "icontext.h"

ParamNode::ParamNode(){};
ParamNode::~ParamNode(){};

ParamNode* ParamNode::dup(){
	ParamNode* out = new ParamNode();
	if(out){
		out->mType = mType;
		out->v_str = v_str;
		out->v_int = v_int;
		out->v_bool = v_bool;
		out->v_byte = v_byte;
		out->v_uint = v_uint;
		out->v_object = v_object;
	}
	return out;
}

RpcParam::RpcParam():IRpcParameter(){
	mParamNodeList.empty();
	init();
};

RpcParam::~RpcParam(){
	list<ParamNode*>::iterator it;
	for(it = mParamNodeList.begin(); it != mParamNodeList.end(); it++){
		ParamNode* node = *it;
		if(node)
			delete node;
	}
	mParamNodeList.empty();
}

void RpcParam::init(){
	mIt = mParamNodeList.begin();
}

void RpcParam::reset(){
	init();
}

void RpcParam::clone(IRpcParameter* dest){
	RpcParam* out = (RpcParam*)dest;
	if(out){
		list<ParamNode*>::iterator it;
		for(it = mParamNodeList.begin(); it != mParamNodeList.end(); it++){
			ParamNode* tmp = *it;
			if(tmp)
				out->mParamNodeList.push_back(tmp->dup());
		}
		out->mIt = out->mParamNodeList.begin();
	}
}

RpcParam* RpcParam::dup(){
	RpcParam* out = new RpcParam();
	if(out){
		list<ParamNode*>::iterator it;
		for(it = mParamNodeList.begin(); it != mParamNodeList.end(); it++){
			ParamNode* tmp = *it;
			if(tmp)
				out->mParamNodeList.push_back(tmp->dup());
		}
		out->mIt = out->mParamNodeList.begin();
	}
	return out;
}

//for read.
RT_TYPE RpcParam::getType(){
	ParamNode* node = *mIt;
	if(node && (mIt != mParamNodeList.end()))
		return node->mType;
	return enRTUnkown;
}

char RpcParam::getByte(){
	ParamNode* node = *mIt;
	if(node){
		if(node->mType != enRTByte){
			DOUT("ERROR MSG TYPE CONTENT!, type=%d\n",node->mType);
		}
		return node->v_byte;
	}
	return enRTUnkown;
}

string RpcParam::getStr(){
	ParamNode* node = *mIt;
	if(node){
		if(node->mType != enRTStr){
			DOUT("ERROR MSG TYPE CONTENT!, type=%d\n",node->mType);
		}
		return node->v_str;
	}
	return "";
}
int RpcParam::getInt(){
	ParamNode* node = *mIt;
	if(node){
		if(node->mType != enRTInt){
			DOUT("ERROR MSG TYPE CONTENT!, type=%d\n",node->mType);
		}
		return node->v_int;
	}
	return 0;
}

unsigned int RpcParam::getUint(){
	ParamNode* node = *mIt;
	if(node){
		if(node->mType != enRTUint){
			DOUT("ERROR MSG TYPE CONTENT!, type=%d\n",node->mType);
		}
		return node->v_uint;
	}
	return 0;
}

int RpcParam::getBool(){
	ParamNode* node = *mIt;
	if(node){
		if(node->mType != enRTBool){
			DOUT("ERROR MSG TYPE CONTENT!, type=%d\n",node->mType);
		}
		return node->v_bool;
	}
	return false;
}

void* RpcParam::getObject(){
	ParamNode* node = *mIt;
	if(node){
		if(node->mType != enRTObject){
			DOUT("ERROR MSG TYPE CONTENT!, type=%d\n",node->mType);
		}
		return node->v_object;
	}
	return false;
}

boolean RpcParam::next() {
	if(mIt == mParamNodeList.end())
		return false;
	else{
		mIt++;
		if(mIt == mParamNodeList.end())
			return false;
		return true;
	}
}
	//for write.
void RpcParam::setByte(char v){
	ParamNode* node = new ParamNode();
	if(node){
		 node->mType = enRTByte;
		 node->v_byte = v;
		 mParamNodeList.push_back(node);
		 init();
	}
}
void RpcParam::setStr(string v){
	ParamNode* node = new ParamNode();
	if(node){
		 node->mType = enRTStr;
		 node->v_str = v;
		 mParamNodeList.push_back(node);
		 init();
	}
}
void RpcParam::setInt(int v) {
	ParamNode* node = new ParamNode();
	if(node){
		 node->mType = enRTInt;
		 node->v_int = v;
		 mParamNodeList.push_back(node);
		 init();
	}
}

void RpcParam::setUint(unsigned int v) {
	ParamNode* node = new ParamNode();
	if(node){
		 node->mType = enRTInt;
		 node->v_uint = v;
		 mParamNodeList.push_back(node);
		 init();
	}
}

void RpcParam::setBool(boolean v){
	ParamNode* node = new ParamNode();
	if(node){
		 node->mType = enRTBool;
		 node->v_bool = v;
		 mParamNodeList.push_back(node);
		 init();
	}
}

void RpcParam::setObject(void* v){
	ParamNode* node = new ParamNode();
	if(node){
		 node->mType = enRTObject;
		 node->v_object = v;
		 mParamNodeList.push_back(node);
		 init();
	}
}



//inner
void RpcParam::push_int_to_first(int v){
	ParamNode* node = new ParamNode();
	if(node){
		 node->mType = enRTInt;
		 node->v_int= v;
		 mParamNodeList.push_front(node);
		 init();
	}
}




/**
*将消息内容从dbus 中解析出来，并存放到out结构中。
*/
static status parserMessage(DBusMessage* msg, IRpcParameter* out){
	if(NULL == msg || NULL == out){
		DOUT("not inited! error!!");
		return s_err_param;
	}	

    DBusMessageIter iter;
	boolean stat = false;
	unsigned int ui = 0;
	char* str = NULL;
	char byte;
	void* object;
	unsigned int intdata = 0;
	dbus_message_iter_init(msg, &iter);

	int msg_type = dbus_message_iter_get_arg_type(&iter);

	while(msg_type != DBUS_TYPE_INVALID){
		switch(msg_type){
			case DBUS_TYPE_BOOLEAN:
				dbus_message_iter_get_basic(&iter, &stat);
				out->setBool(stat);
			//	DOUT("parser msg, v_bool=%d",stat);
				break;
			case DBUS_TYPE_INT32:
				dbus_message_iter_get_basic(&iter, &intdata);
				out->setInt(intdata);
			//	DOUT("parser msg, v_int=%d",intdata);
				break;
			case DBUS_TYPE_UINT32:
				dbus_message_iter_get_basic(&iter, &ui);
				out->setUint(ui);
			//	DOUT("parser msg, v_uint=%d",ui);
				break;
			case DBUS_TYPE_STRING:
				dbus_message_iter_get_basic(&iter, &str);
				out->setStr(str);
			//	DOUT("parser msg, v_str=%s",str);
				break;
			case DBUS_TYPE_BYTE:
				dbus_message_iter_get_basic(&iter, &byte);
				out->setByte(byte);
			//	DOUT("parser msg, v_byte=%d",byte);
				break;
			case DBUS_TYPE_UINT64:
				long long object64;
				dbus_message_iter_get_basic(&iter, &object64);
				object = (void*)object64;
				out->setObject(object);
			//	DOUT("parser msg, v_object=%d",object);
				break;
			default:
				DOUT("parser msg,type=%d, not support now!",msg_type);
				break;
		}
		if(dbus_message_iter_next (&iter)){
			msg_type = dbus_message_iter_get_arg_type(&iter);
		}else{
			break;
		}
	}	
	
	 //dbus_message_unref(msg);
	 return s_ok;
}

static status appendMessage(DBusMessage* msg, IRpcParameter* param){
	if(param && msg){
		DBusMessageIter iter;
		int result = 0;
		char v_byte = 0;
		const char* v_str = NULL;
		int v_int = 0;
		bool v_bool = false;
		void* v_object = NULL;
		unsigned int v_uint = 0;
		long long object64 = 0;

		dbus_message_iter_init_append(msg,&iter);
		while(1){
			switch(param->getType()){
				case enRTByte:
					v_byte = param->getByte();
				//	DOUT("append msg, v_byte=%d",v_byte);
					result = dbus_message_iter_append_basic(&iter,DBUS_TYPE_BYTE,&v_byte);
					break;
				case enRTStr:
					v_str = param->getStr().c_str();
				//	DOUT("append msg, v_str=%s",v_str);
					result = dbus_message_iter_append_basic(&iter,DBUS_TYPE_STRING,&v_str);
					break;
				case enRTInt:
					v_int = param->getInt();
				//	DOUT("append msg, v_int=%d",v_int);
					result = dbus_message_iter_append_basic(&iter,DBUS_TYPE_INT32,&v_int);
					break;
				case enRTUint:
					v_uint = param->getUint();
				//	DOUT("append msg, v_uint=%d",v_uint);
					result = dbus_message_iter_append_basic(&iter,DBUS_TYPE_INT32,&v_uint);
					break;
				case enRTBool:
					v_bool = param->getBool();
				//	DOUT("append msg, v_bool=%d",v_bool);
					result = dbus_message_iter_append_basic(&iter,DBUS_TYPE_BOOLEAN,&v_bool);
					break;
				case enRTObject:
					v_object = param->getObject();
					object64 = (long long)v_object;
					result = dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT64,&object64);
				//	DOUT("append msg, v_object=%d",v_object);
					break;
				default:
					DOUT("Paramer type not support !");
					break;
			}
			if(!result){
				DOUT("Failed to append input parameters result=%d",result );
				return s_err_syscall;
			}
			if(!param->next())
				break;
		}
/*		result = dbus_message_iter_append_basic(&iter,DBUS_TYPE_INVALID,&v_invalid);
		if(!result){
			DOUT("Failed to append input parameters");
			return s_err_syscall;
		}
*/		return s_ok;
	}
	
	DOUT("parameter error!!");
	return s_err_param;

}


static dbus_bool_t _add_watch(DBusWatch *watch, void *data)
{
    int fd = dbus_watch_get_unix_fd(watch);
	DBus* rpc = (DBus*)data;
	if(rpc && rpc->mLoop){
		DBus::DBusWatchHandle* pwatch = (DBus::DBusWatchHandle*)rpc->mLoop->getFD(fd);
		if(!pwatch){
			pwatch =  new DBus::DBusWatchHandle(rpc);
		}
		if(pwatch){
			rpc->mDBusWatchHandleList[pwatch] = true;
			pwatch->addWatch(watch);
			rpc->mLoop->addFD(fd,ILoop::enFlagRead,pwatch);
		}else{
			DOUT(" no memory!!!, error!");
			return s_err_memoryout;
		}
	}
   	return true;
}


static void _del_watch(DBusWatch *watch, void __attribute__ ((unused)) *data)
{
    int fd = dbus_watch_get_unix_fd(watch);
    /* unregister fd hander out of main loop */
	DBus* rpc = (DBus*)data;
	if(rpc && rpc->mLoop){
		DBus::DBusWatchHandle* pwatch = (DBus::DBusWatchHandle*)rpc->mLoop->getFD(fd);
		if(pwatch){
			rpc->mDBusWatchHandleList.erase(pwatch);
			delete pwatch;
		}

		rpc->mLoop->delFD(fd);
	}
}


static DBusHandlerResult _dbus_msg_handler(DBusConnection *conn, DBusMessage *message, void *user_data){
	DBus* srv = (DBus*)user_data;
	if(srv){
		srv->recv(message);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


DBusObjectPathVTable dbus_vtable = {NULL, &_dbus_msg_handler,	NULL, NULL, NULL, NULL};


/*
*
*/
status DBus::init(string name, ILoop* loop){
		if(name.empty() || name.length() == 0){
			DOUT("parameter error!!");
			return s_err_param;
		}
		mName = DEFAULT_NAME(name);
		mObj = DEFAULT_OBJ(name);
		mIntf = DEFAULT_INTF(name);
		mLoop = loop;
		return s_ok;
}
/*
*/

status DBus::create(){
		DBusError err;
		int ret;
		int result = s_ok;
		
		// connect to the DBUS system bus, and check for errors
		dbus_error_init(&err);

		mConn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
		if (dbus_error_is_set(&err)) { 
			EOUT("Connection Error (%s)\n", err.message); 
			result = s_err_syscall;
			goto error_1;
		}
		
		if (NULL == mConn) { 
			EOUT("Connection is NULL\n"); 
			result = s_err_syscall;
			goto error_1;
		}
		
		dbus_connection_set_exit_on_disconnect(mConn, FALSE);
		dbus_connection_set_watch_functions(
			mConn, 
			_add_watch,
			_del_watch, 
			NULL,
			this, 
			NULL);
		
		// register our name on the bus, and check for errors
		ret = dbus_bus_request_name(mConn, mName.c_str(), DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER , &err);
		if (dbus_error_is_set(&err)) { 
			EOUT( "Name Error (%s)\n", err.message); 
			result = s_err_syscall;
			goto error_1;
		}
		
		if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) { 
			EOUT("Not Primary Owner (%d),errName=%s \n", ret,mName.c_str());
			result = s_err_syscall;
			goto error_1;
		}
	error_1:
		dbus_error_free(&err); 
		return result;
			
}
	
/*
*/
DBus::DBus(string name,ILoop* loop){
		mConn = NULL;
		init(name,loop);
		if(s_ok == create()){
		}
}

/*
*/

DBus::~DBus(){
		if(mConn){
			DBusError error;
			dbus_error_init(&error);
			dbus_bus_release_name(mConn, mName.c_str(), &error);
			if (dbus_error_is_set(&error)) {
				/* request well-known bus name failed */
				DOUT("Failed to release well-known bus name: %s due to %s",mName.c_str(), error.message);
			}
			/* close connection */
			dbus_connection_unref(mConn);
			dbus_error_free(&error);
		}

		if(mDBusWatchHandleList.size() > 0){
			map<DBusWatchHandle*,boolean>::iterator it;
			for(it = mDBusWatchHandleList.begin(); it != mDBusWatchHandleList.end(); it++){
				map<DBusWatchHandle*,boolean>::value_type pair = *it;
				DBusWatchHandle* tmp = pair.first;
				if(tmp){
					delete tmp;
				}
			}
			mDBusWatchHandleList.clear();
		}
}
/*
*/

status DBus::sendSignal(string obj, string intf, string method, IRpcParameter* param){

		status result = s_ok;
		dbus_uint32_t serial = 0;
		DBusError _error;
		
		if(NULL == mConn){
		
			EOUT("not inited");
			return s_err_init;
		}
		
		if(obj.empty() || intf.empty() || method.empty()){
		
			EOUT("method_name error!");
			return s_err_param;
		}
		
		DBusMessage* _message = dbus_message_new_signal(
			obj.c_str(),				// object name of the signal
			intf.c_str(),				// interface name of the signal
			method.c_str());		// name of the signal
		
		if (!_message) {
			EOUT("Failed to create method_call message,OBJ=%s,intf=%s,method=%s",s2c(obj),s2c(intf),s2c(method));
			return s_err_syscall;
		}
		if(param)
			appendMessage(_message,param);
		dbus_error_init(&_error);
		 // send the message and flush the connection
		if (!dbus_connection_send(mConn, _message, &serial)) {
			EOUT("send signal failed!"); 
			result = s_err_syscall;
		}
/*		DOUT("OUT: OBJ=%s, INTF=%s, method=%s,serial=%d ",obj.c_str(),intf.c_str(),method.c_str(),serial);
*/		
		dbus_connection_flush(mConn);
		dbus_message_unref(_message);
		dbus_error_free(&_error);

		return result;
}

/*
*/

status DBus::sendMethod(string server, string obj, string intf, string method, IRpcParameter* param, IRpcParameter* reply,int timeout){
		status result = s_ok;
		dbus_uint32_t serial = 0;
		DBusError _error;
		
		if(NULL == mConn){
		
			DOUT("not inited");
			return s_err_init;
		}
		
		if(server.empty() || obj.empty() || intf.empty() || method.empty()){
		
			DOUT("method_name error!");
			return s_err_param;
		}

/*		if(	method != string("_rpc_inner_async_call")){	
			DOUT("OUT: SERVER=%s, OBJ=%s, INTF=%s, method=%s ", server.c_str(), obj.c_str(),intf.c_str(),method.c_str());
		}
*/	
		DBusMessage* _message = dbus_message_new_method_call(
			server.c_str(),
			obj.c_str(),
			intf.c_str(),
			method.c_str());
		
		if (!_message) {
			DOUT("Failed to create method_call message");
			return s_err_syscall;
		}
		if(param)
			appendMessage(_message,param);
		dbus_error_init(&_error);

		DBusMessage* _reply = dbus_connection_send_with_reply_and_block(mConn, _message, timeout, &_error);
		dbus_message_unref(_message);
		dbus_connection_flush(mConn);

		if (dbus_error_is_set (&_error)) {
			DOUT("dbus send method call failed error %s: %s",_error.name, _error.message);
			result = s_err_syscall;
		}
		dbus_error_free(&_error);
		if(s_ok == result){
			if (!_reply) {
				DOUT("failed to get reply message");
				result = s_err_tmout;
			}else{	
				result = parserMessage(_reply,reply);
				dbus_message_unref(_reply);
			}
		}
		return result;
}


/*
*/
status DBus::replyToMethodCall(DBusMessage* msg, IRpcParameter* param)
{
	if(NULL == mConn){
	
		DOUT("not inited");
		return s_err_init;
	}

	DBusMessage* reply = NULL;
   	dbus_uint32_t serial = 0;
	int result = s_ok;
   	// create a reply from the message
   	reply = dbus_message_new_method_return(msg);
	if(reply == NULL){
		DOUT(" call failed!");
		return s_err_syscall;
	}
		
   	appendMessage(reply,param);
   	if (!dbus_connection_send(mConn, reply, &serial)) {
		DOUT("send reply  failed!");
		result = s_err_syscall;
	}

   	dbus_connection_flush(mConn);
	dbus_message_unref(reply);
	return result;
}

RpcParam* DBus::initParameter(){
	return new RpcParam();
}
void DBus::deinitParameter(RpcParam* p){
	if(p)
		delete p;
}

status DBus::registCB(DBusCB *CB){
	mCB = CB;
	return s_ok;
}
status DBus::unregistCB(DBusCB *CB){
	mCB = NULL;
	return s_ok;
}

status DBus::recv(DBusMessage* msg){
	int result = s_ok;
	if(msg){

		string member = dbus_message_get_member (msg);
			
		string if_name = dbus_message_get_interface (msg);


/*		if(member != string("_rpc_inner_async_call")){
			DOUT (" IN: serial=%u path=%s; interface=%s; member=%s\n",
				 dbus_message_get_serial (msg),
				 dbus_message_get_path (msg),
				 string2char(if_name),
				 string2char(member));
		}
*/
		RpcParam* param = initParameter();

		RpcParam* reply = initParameter();

		if(mCB && param && reply){
			parserMessage(msg,param);
			if(dbus_message_is_method_call(msg, if_name.c_str(), member.c_str())){
				result = mCB->cbSync(if_name,member,param,reply);
				RpcParam* temp = (RpcParam*)reply;
				temp->push_int_to_first(result);
				replyToMethodCall(msg,reply);
			}else if(dbus_message_is_signal(msg, if_name.c_str(), member.c_str())){
				result =  mCB->cbAsync(if_name,member,param);
			}
		}else{
			DOUT("no memory! error!");
			result = s_err_memoryout;
			
		}
		
		deinitParameter(param);
		deinitParameter(reply);
	}else{
		DOUT("message param error!");
		result = s_err_param;
	}
	return result;
}

/*
*/
status DBus::listenObj(string obj){
		if(NULL == mConn){
		
			DOUT("not inited");
			return s_err_init;
		}

		if(obj.empty()){
		
			DOUT("method_name error!");
			return s_err_param;
		}

//		DOUT("listen obj=%s",obj.c_str());
		if (!dbus_connection_register_object_path(mConn, obj.c_str(), &dbus_vtable, this)) {
			DOUT("Failed to register object path: %s", obj.c_str());
			return s_err_syscall;
		}
		return s_ok;
}


status DBus::listenIntf(string intf){

		char buff[512] = {0};
		int result = s_ok;
		DBusError _error;
		
		if(NULL == mConn){
		
			DOUT("not inited");
			return s_err_init;
		}

		if(intf.empty()){
		
			DOUT("method_name error!");
			return s_err_param;
		}
		
		// see signals from the given interface
		snprintf(buff,sizeof(buff)-1,"type='signal',interface='%s'",intf.c_str());

		dbus_error_init(&_error);
		dbus_bus_add_match(mConn,buff,&_error);
		if (dbus_error_is_set(&_error)) { 
			DOUT( "Name Error (%s)\n", _error.message); 
			result = s_err_syscall;
		}
		dbus_error_free(&_error);

		dbus_connection_flush(mConn);
		return result;
}


/**
*
*/
DBus::DBusWatchHandle::DBusWatchHandle(DBus* server):ILoopData(){
		mWatch = NULL;
		mServer = server;
}
	
DBus::DBusWatchHandle::~DBusWatchHandle(){
		mWatch = NULL;
}
	
status DBus::DBusWatchHandle::addWatch(DBusWatch* watch){
		mWatch = watch;
		return s_ok;
}
	
status DBus::DBusWatchHandle::handler(short events,int fd){
		unsigned int flags = 0;
		if (events & POLLIN)  
			flags |= DBUS_WATCH_READABLE;
		if (events & POLLOUT) 
			flags |= DBUS_WATCH_WRITABLE;
		if (events & POLLHUP) 
			flags |= DBUS_WATCH_HANGUP;
		if (events & POLLERR) 
			flags |= DBUS_WATCH_ERROR;
	
		while (!dbus_watch_handle(mWatch, flags)) {
			DOUT("dbus_watch_handle needs more memory\n");
			sleep(1);
		}
		dbus_connection_ref(mServer->mConn);
		while (dbus_connection_dispatch(mServer->mConn) == DBUS_DISPATCH_DATA_REMAINS);
		dbus_connection_unref(mServer->mConn);
		return true;
}


