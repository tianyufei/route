/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     icontext.h                                                        *
*  @brief    该文件定义了上下文接口的功能.									 *
*  Details.                                                                  *
*            Yaya每个进程都有一个上下文环境, 通过上下文接口可以使用这些环境, *
*            以及功能.														 *
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


#ifndef _ICONTEXT_H 
#define _ICONTEXT_H 
#include <stdio.h>
#include <string>
using namespace std;
#include "ievent.h"
#include "iloop.h"
#include "irpc.h"


/**
*@brief  时钟回调接口. 
*
*用户创建一个时钟时, 将该接口的实例注册到时钟上. 当时钟到期时,该接口被调用.
*时钟运行在进程的主线程中.
*/
class ITimer {
protected:
	ITimer(){};
public:
	virtual ~ITimer(){};
	/**
	*@brief 获取动态库中的实例。
	*
	*时钟到期后,调用的回调方法。 
	*@param id 时钟id. 是IContext::startTimer方法的返回值.
	*/
	virtual void timeArrive(int id) = 0;
};


/**
*@brief  进程上下文接口. 
*
*上下文包括获取IEvent, ILoop, IRpc接口的方法; 还包括时钟创建和消除方法,
*日志开关, shell调用方法. 该接口必须通过getContext()获取实例.
*/

class IContext{
protected:
	IContext(){};
public:
	virtual ~IContext(){};
	/**
	*@brief 初始化上下文参数.
	*
	*初始化上下文接口，将进程的启动参数传递给环境变量.
	*@param argc 进程启动参数中的argc
	*@param argv 进程启动参数中的argv
	*/
	virtual status init(int argc, char* argv[]) = 0;

	/**
	*@brief 获取上下文名称.
	*
	*上下文的名称,一般情况下就是进程的名称.
	*/
	virtual string getName() = 0;

	/**
	*@brief 获取IEvent接口实例.
	*
	*IEvent接口用于向其它进程广播事件, 或监听从其它进程以及shell命令上广播过来的消息.
	*该工厂方法获取IEvent接口实例. 无论该方法调用多少次,返回同一个实例。
	*@return IEvent接口实例。
	*/
	virtual IEvent* eventFactory() = 0;

	/**
	*@brief 获取ILoop接口实例.
	*
	*ILoop接口用于管理进程中的悬挂点, 它可以将各种阻塞机制悬挂到进程的同一个阻塞点上.
	*该接口的重大作用在于进程在单线程模式下,就可以处理来源于其它进程,socket,
	*时钟等发送过来的消息.\n
	*该工厂方法获取ILoop接口实例. 无论该方法调用多少次,返回同一个实例。
	*@return ILoop接口实例。
	*/
	virtual ILoop* loopFactory() = 0;

	/**
	*@brief 获取IRpc接口实例.
	*
	*IRpc是进程间调度接口. 通过该接口, 一个进程可以同步或异步调用其它进程中的方法,并得到运行结果.
	*该工厂方法获取IRpc接口实例. 无论该方法调用多少次,返回同一个实例。
	*@return IRpc接口实例。
	*/	
	virtual IRpc* rpcFactory() = 0;

	/**
	*@brief 调试信息输出开关.
	*
	*打开或关闭调试信息.
	*@param open true-打开调试开关; false-关闭调试开关。
	*/	
	virtual void debugSwitch(boolean open, string term_name) = 0;

	/**
	*@brief shell命令调用方法.
	*
	*调用shell脚本.
	*@param  fmt 变参, 为要调用的shell命令。
	*@return 执行结果, 参考linux中的system系统调用的返回.
	*/	
	virtual int systemCall(const char* fmt, ...) = 0;

	/**
	*@brief shell命令调用方法,并得到执行输出.
	*
	*调用shell脚本.
	*@param  fmt 变参, 为要调用的shell命令。
	*@return shell脚本输出的信息, 参考linux的popen系统调用的返回.
	*/	
	virtual FILE* popenCall(const char* fmt, ...) = 0;

	/**
	*@brief 创建时钟(定时器).
	*
	*创建进程中使用的一次性时钟或周期性时钟. 
	*@param sec 	定时器超时的秒数
	*@param usec 	定时器超时的微秒数
	*@param once 	true-一次性的时钟; false-周期性时钟。
	*@param cb		时钟到达后的回调接口
	*@return 		时钟FD.
	*/
	virtual int startTimer(int sec, int usec, boolean once, ITimer* cb) = 0;

	/**
	*@brief 终止时钟(定时器).
	*
	*如果时钟还没有到期, 通过该方法可以终止时钟. 如果时钟是一次性的,并已经到达,
	*则不需要调用该函数.
	*@param fd		要终止的时钟FD
	*/
 	virtual void stopTimer(int fd) =  0;

};

#endif

