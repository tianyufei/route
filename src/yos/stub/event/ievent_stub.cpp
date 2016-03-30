/* 
 * Copyright (c) 2014 Yaya Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Draft:  zhengdajun 
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <list>
#include <map>
#include "common.h"
#include "inner.h"


class EventHandle : public IEvent,IRpcMethod{
	private:
		IContext* mContext;
		IRpc* mRpc;
		class EventObserverList{
			private:
				class EventHandle* mHandle;
				list<IEventObserver*> mList;
			public:
				EventObserverList(EventHandle* server){
					mHandle = server;
				}
				~EventObserverList(){};
				status addOne(IEventObserver* obs){
					list<IEventObserver*>::iterator it;
					for(it = mList.begin(); it != mList.end(); it++){
						IEventObserver* tmp = *it;
						if(tmp == obs){
							return s_ok;
						}
					}
					mList.push_back(obs);
					return s_ok;
				}
				status delOne(IEventObserver* obs){
					list<IEventObserver*>::iterator it;
					for(it = mList.begin(); it != mList.end(); it++){
						IEventObserver* tmp = *it;
						if(tmp == obs){
							mList.erase(it);
							break;
						}
					}
					return s_ok;
				}
				status notify(string owner,string event, string data,string data2, string data3, string data4){
					list<IEventObserver*>::iterator it;
					for(it = mList.begin(); it != mList.end(); it++){
						IEventObserver* tmp = *it;
						if(NULL != tmp){
							tmp->update(owner,event,data,data2,data3,data4);
						}
					}
					return s_ok;
				}
		};
		
		map<string,EventObserverList*> mEventObserverList;
	
	public:
		EventHandle(IContext* ctx){
			mContext = ctx;
			mRpc = NULL;
			if(mContext)
				mRpc = mContext->rpcFactory();
			if(mRpc){
				mRpc->listenObj(DEFAULT_OBJ(RPC_EVENT_NAME));
				mRpc->listenIntf(DEFAULT_INTF(RPC_EVENT_NAME));
				mRpc->registMethod(RPC_EVENT_BROADCAST,false,this);
			}
		}
		~EventHandle(){
			map<string,EventObserverList*>::iterator it;
			for(it = mEventObserverList.begin(); it != mEventObserverList.end(); it++){
				map<string,EventObserverList*>::value_type pair = *it;
				EventObserverList* tmp = pair.second;
				if(tmp)
					delete tmp;
			}
			mEventObserverList.clear();
		};
		status registObserver(string event,IEventObserver* obs){
			EventObserverList *tmp = mEventObserverList[event];
			int result = s_ok;
			if(NULL == tmp){
				tmp = new EventObserverList(this);
				mEventObserverList[event] = tmp;
			}
			if(tmp)
				result = tmp->addOne(obs);
			else
				result = s_err_memoryout;
			
			return result;

		}
		status unregistObserver(string event,IEventObserver* obs){
			EventObserverList *tmp = mEventObserverList[event];
			if(tmp)
				return tmp->delOne(obs);
			return s_ok;
		}			

		status notify(string event, string data1, string data2,string data3, string data4){
			if(NULL == mContext || mRpc == NULL){
				DOUT("not inited!");
				return s_err_init;
			}
			IRpcParameter *param = mRpc->initParameter();
			if(param){
				param->setStr(mContext->getName());
				param->setStr(event);
				param->setStr(data1);
				param->setStr(data2);
				param->setStr(data3);
				param->setStr(data4);
				mRpc->asyncCall(DEFAULT_OBJ(RPC_EVENT_NAME),DEFAULT_INTF(RPC_EVENT_NAME),RPC_EVENT_BROADCAST,param);
			}
			mRpc->deinitParameter(param);
			return s_ok;
		}

		status methodCalled(string method_name,IRpcParameter* param, IRpcParameter* reply){
			if(NULL == param)
				ASSERT_STATUS(s_err_param);

			string owner = param->getStr();
			param->next();
			string event = param->getStr(); //event name
			param->next();
			string data = param->getStr(); //event data.
			string data2;
			string data3;
			string data4;
			if(param->next())
				data2 = param->getStr(); //event data.
			if(param->next())
				data3 = param->getStr(); //event data.
			if(param->next())
				data4 = param->getStr(); //event data.
			param->next();
			EventObserverList *tmp = mEventObserverList[event];
			if(tmp)
				return tmp->notify(owner,event, data,data2,data3,data4);
			return s_ok;
		}
};

//for dlopen.
extern "C" {

void* getInstance(void* ctx){
	return (void*)(new EventHandle((IContext*)ctx));
}


void relInstance(void* rpc){
	EventHandle* handl = (EventHandle*)rpc;
	if(handl)
		delete handl;
}

}

