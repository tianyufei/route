/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     ievent.h                                                          *
*  @brief    该文件定义了进程间的广播事件机制.									 *
*  Details.                                                                  *
*            Yaya每个进程都可以向其它进程广播消息，本文件定义了广播的收发接口*
*  @author   ZhengDajun                                                      *
*  @email    zhengdajun@vip.sina.com.                                        *
*  @version  1.0.0.1(版本号)                                                 *
*  @date     zhengdajun@vip.sina.com.                                        *
*  @license  GNU Lesser General Public License (LGPL)                        *
*                                                                            *
*----------------------------------------------------------------------------*
*  Remark         : Description                                              *
*----------------------------------------------------------------------------*
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2016/03/05 | 1.0.0.1   | ZhengDajun     | Create file                     *
*----------------------------------------------------------------------------*
*                                                                            *
*****************************************************************************/
#ifndef _IEVENT_H 
#define _IEVENT_H 
#include <string>
using namespace std;
#include "type.h"


/** 
*@brief 广播事件观察者接口。
*
*该类用于观察特殊事件，比如设备启动完成，配置更改后保存，网络连接状态变化等。 
*各进程创建该接口的实例，然后通过IEvent接口的registObserver函数进行注册，这样有事件产生的时候，
*对该事件感兴趣的模块或进程就可以监听到变化。\n
*/

class IEventObserver{
protected:
	IEventObserver(){};
public:
	virtual ~IEventObserver(){};
	/** 
	*@brief 该方法用于用户对监听到的事件进行后续处理。
	*
	*@param event_owner 广播事件的进程名。
	*@param event 事件类型。
	*@param data* 广播的具体信息。
	*@return 如果返回值为s_ok表示成功，其它值表示用户操作失败。
	*/
	virtual status update(string event_owner, string event, string data1, string data2, string data3, string data4) = 0;
};

/**
*@brief 广播事件发送和注册接口。
*
*该接口用于向其它进程广播事件或监听其它进程的广播消息。
*该类中包含纯虚函数，因此不能直接实例化。 只能通过IContext上下文接口得到它的实例。
*/
class IEvent{
protected:
	IEvent(){};
public:
	virtual ~IEvent(){};
	/**
	*@brief 事件观察者注册函数
	*
	*该函数注册对制定事件感兴趣的监听者。
	*@param event 监听的事件。
	*@param obs	监听者。见IEventObserver接口的相关描述。
	*@return s_ok表示成功，其它值表示失败。
	*/
	virtual status registObserver(string event,IEventObserver* obs) = 0;
	/**
	*@brief 事件观察者注销函数
	*
	*该函数注销对制定事件感兴趣的监听者。
	*@param event 监听的事件。
	*@param obs	监听者。见IEventObserver接口的相关描述。
	*@return s_ok表示成功，其它值表示失败。
	*/
	virtual status unregistObserver(string event,IEventObserver* obs) = 0;
	/**
	*@brief 发送事件。
	*
	*该函数发出事件。
	*@param event 广播的事件
	*@param data* 事件的具体参数
	*@return s_ok表示成功，其它值表示失败。
	*/
	virtual status notify(string event, string data1, string data2 = "", string data3 = "", string data4 = "") = 0;
	
};


#endif

