/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     iloop.h                                                         *
*  @brief    该文件定义了系统事件监听机制.		 							 *
*  Details.                                                                  *
*            Yaya每个进程可以将不同的系统事件接收者阻塞到同一个悬挂点，	     *
*            可以使所有逻辑在同一个进程中运行。								 * 		
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
#ifndef _ILOOP_H 
#define _ILOOP_H 
#include <string>
using namespace std;
#include "type.h"

/**
*@brief 悬挂点事件监听接口。
*
*当socket等接收到数据后，通过该接口可以接收知道有悬挂点事件到来。
*/
class ILoopData{
protected:
	ILoopData(){};
public:
	virtual ~ILoopData(){};
	/**
	*@brief 悬挂点判断方法。 
	*
	*通过该方法知道是哪个系统事件到来。
	*@param event 用户一般不用关心。
	*@param fd 接收事件的FD.
	*@return s_ok表示成功，其它值表示失败。
	*/
	virtual status handler(short event, int fd = -1) = 0;
};

/**
*@brief 消息阻塞监听和调用接口。
*
*Loop是一个消息阻塞监听和调用机制。可以将socket,file,messageq,dbus等设备的fd
*悬挂在Loop上，当相应fd接收到事件时，该fd对应的消息处理函数被调用。
*这套机制保证了所有消息处理函数在同一个线程下处理，减少了线程间交互
*的数据保护机制。
*/
class ILoop{
protected:
	ILoop(){};
public:
	virtual ~ILoop(){};
	/**
	*@brief 监听的事件类型。
	*/
	enum LoopFlag{
		enFlagNone = 0, /**< 不监听任何事件 */
		enFlagRead = 1, /**< 监听FD中是否收到数据 */
		enFlagWrite = 2, /**< 监听FD中是否有数据要发送 */
		enFlagEpollEt = 4,
	};

	/**
	*@brief 加入监听FD。
	*
	*给Loop中加入要监听的FD
	*@param fd 要加入的FD
	*@param flag 要监听的事件类型
	*@param data 监听到时间后，调用的处理函数句柄。
	*@return s_ok-成功; 其它值失败。
	*/
	virtual status addFD(int fd, LoopFlag flag, ILoopData* data) = 0;
	/**
	*@brief 注销FD。
	*
	*注销Loop中监听的FD
	*@param fd 要注销的FD.
	*@return s_ok表示成功，其它值表示失败。
	*/
	virtual status delFD(int fd) = 0;
	/**
	*@brief 获取事件处理句柄。
	*
	*获取FD对应的处理函数句柄
	*@param fd 
	*@return fd对应的消息处理函数句柄。
	*/
	virtual ILoopData* getFD(int fd) = 0;
	/**
	*@brief 主线程运行。
	*
	*该方法执行后，主线程进入无限循环，监听FD上是否有事件发生。
	*@return s_ok-表示成功，其它值表示失败。
	*/
	virtual status run() = 0;

	/**
	*@brief 退出主线程。
	*
	*退出主线程，从而退出进程。
	*/
	virtual void quit() = 0;
};


#endif

