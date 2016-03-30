/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     irpc.h                                                          *
*  @brief    该文件定义了进程间同异步调度接口.								 *
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

#ifndef _IRPC_H 
#define _IRPC_H 
#include <string>
using namespace std;
#include "type.h"
#include "iloop.h"

#define DBUS_CONFIG_PATH_PREFEX			"com.yaya."
#define DBUS_CONFIG_OBJPATH_PREFEX		"/com/yaya/"
#define DBUS_ASYNC_NOTIFY				"dbus_async_notify"


/**
*
*根据参数生成IRpc::syncCall,IRpc:asyncCall中的obj参数
*@param x 一般为目标进程的名字
*/
#define DEFAULT_OBJ(x)	( string(DBUS_CONFIG_OBJPATH_PREFEX) +  string(x) )

/**
*
*根据参数生成IRpc::syncCall,IRpc:asyncCall中的intf参数
*@param x 一般为目标进程的名字
*/
#define DEFAULT_INTF(x) ( string(DBUS_CONFIG_PATH_PREFEX) +  string(x) + string(".intf") )

/**
*
*根据参数生成IRpc::syncCall中的name参数
*@param x 一般为目标进程的名字
*/
#define DEFAULT_NAME(x)	( string(DBUS_CONFIG_PATH_PREFEX) + string(x) )




/**
*@brief 进程间传递的参数的类型定义
*
*进程间调用时,可以同时传递很多参数. 每个参数的类型可以不同. 
*本枚举定义这些参数的类型.
*
*/
enum RT_TYPE{

 	enRTUnkown,		/**< 数据类型未知 */
	enRTByte,		/**< BYTE型数据 */
	enRTStr,		/**< 字符串型数据 */
	enRTInt,		/**< 整形数据 */
	enRTUint,		/**< 无符号整形数据 */
	enRTBool,		/**< 布尔型数据 */
	enRTObject,		/**< void* 类型指针 */
};


/**
*@brief 进程间传递的参数表容器
*
*该容器用于存放将要发送给远程模块的数据，或用于接收从远程发送过来的数据。
*远程发送过来的数据是串行存入该类中的，该类保持了串行存入的顺序。也就是远程发送
*的第一个数据在该类中第一个被读到。远程发送的最后一个数据在该类中最后被读到。
*该类不能直接实例化。 只能通过IRpc::initParameter方法获取实例。
*/
class IRpcParameter{
protected:
	IRpcParameter(){};
public:
	virtual ~IRpcParameter(){};
	virtual void clone(IRpcParameter* dest) = 0;
	virtual void reset() = 0;
	/**
	*@brief 得到容器中当前数据的类型
	*
	*容器中的众多数据,是通过迭代器的方式一个一个获取的.
	*本函数判断迭代器的当前数据的类型。
	*@return 数据类型，见RT_TYPE中的定义。
	*/
	virtual RT_TYPE getType() = 0;
	/**
	*如果当前类型是BYTE,通过该接口能得到当前数据的值。
	*@return 当前数据
	*/
	virtual char getByte() = 0;
	/**
	*@brief 获取当前数据.
	*
	*如果当前类型是字符串,通过该接口能得到当前数据的值。
	*@return 当前数据
	*/
	virtual string getStr() = 0;
	/**
	*@brief 获取当前数据.
	*
	*如果当前类型是整形,通过该接口能得到当前数据的值。
	*@return 当前数据
	*/
	virtual int getInt() = 0;
	/**
	*@brief 获取当前数据.
	*
	*如果当前类型是指针,通过该接口能得到当前数据的值。
	*@return 当前数据
	*/
	virtual void* getObject() = 0;
	/**
	*@brief 获取当前数据.
	*
	*如果当前类型是无符号整形,通过该接口能得到当前数据的值。
	*@return 当前数据
	*/
	virtual unsigned int getUint() = 0;
	/**
	*@brief 获取当前数据.
	*
	*如果当前类型是布尔,通过该接口能得到当前数据的值。
	*@return 当前数据
	*/
	virtual boolean getBool() = 0;
	/**
	*@brief 指向下一个数据.
	*
	*判断是否还有其它数据，如果有，则指向下一条数据。
	*@return true-容器中还有下一条数据, false-容器中没有下一条数据了. 
	*/
	virtual boolean next() = 0;
	/**
	*@brief 向容器添加数据.
	*
	*写入一个BYTE型数据
	*/
	virtual void setByte(char v) = 0;
	/**
	*@brief 向容器添加数据.
	*
	*写入一个字符串型数据
	*/
	virtual void setStr(string v) = 0;
	/**
	*@brief 向容器添加数据.
	*
	*写入一个整型数据
	*/
	virtual void setInt(int v) = 0;
	/**
	*@brief 向容器添加数据.
	*
	*写入一个无符号整型数据
	*/
	virtual void setUint(unsigned int v) = 0;
	/**
	*@brief 向容器添加数据.
	*
	*写入一个布尔型数据
	*/
	virtual void setBool(boolean v) = 0;
	/**
	*@brief 向容器添加数据.
	*
	*写入一个 object指针
	*/
	virtual void setObject(void* v) = 0;

};


/**
*@brief  远程服务供给接口. 
*
*一个进程要向其它进程提供服务,供其它进程调用时, 必须要实现该接口实例. 
*其它进程会调用到该实例的methodCalled方法.
*/
class IRpcMethod {
protected:
	IRpcMethod(){};
public:
	virtual ~IRpcMethod(){};
	/** 
	*@brief 远程服务调用方法。
	* 
	*假设A进程提供了该方法， 则B,C,D等进程通过IRpc::syncCall或IRpc::asyncCall可调用到该方法。
	*假设进程B,C,D等进程采用IRpc::syncCall调用进程B中的方法，则进程B中的方法不能再使用
	*IRpc::syncCall同步调用其它进程的方法，以免死锁。 
	*@param param 调用者提供的参数容器，包含了调用进程传递给服务提供进程的参数。
	*@param reply 调用者提供的参数容器，用于服务提供进程回传接过给调用进程。\n
	*			  如果调用进程采用IRpc::asyncCall调用，则reply为NULL. 
	*@return s_ok-成功； 其它值表示失败。
	*/
	virtual status methodCalled(string method_name,IRpcParameter* param, IRpcParameter* reply) = 0;
};


/**
*@brief 远程调用接口。
* 
*该接口封装了远程调用的诸多细节，给用户提供了一个简单实用的接口。
*该类中包含纯虚函数，因此不能直接实例化。只能通过IContext接口获取实例。
*/
class IRpc {

public:
	IRpc(){};
	virtual ~IRpc(){};
	
	/**
	*@brief 监听服务类别
	*
	*对于服务提供进程， 需要注册(监听)服务类别， 
	*当调用进程发出的服务类别，服务接口与监听的一致时，IRpcMethod中的方法才会被调用到。
	*@param obj 目标全名。参考DEFAULT_OBJ
	*@return s_ok-成功； 其它值表示失败。
	*/
	virtual status listenObj(string obj) = 0;
	/**
	*@brief 监听服务接口
	*
	*对于服务提供进程， 需要注册(监听)服务接口， 
	*当调用进程发出的服务类别，服务接口与监听的一致时，IRpcMethod中的方法才会被调用到。
	*@param obj 目标全名。参考DEFAULT_INTF
	*@return s_ok-成功； 其它值表示失败。
	*/
	virtual status listenIntf(string intf) = 0;

	/**
	*@brief 异步调用接口
	*
	*异步调用其它进程的函数。该函数被调用后，立即返回。
	*@param obj 远程的OBJ PATH,参考DEFAULT_OBJ
	*@param intf 远程的Interface Name, 参考DEFAULT_INTF
	*@param method 远程被调用的方法.
	*@param param 要传递给远程函数的参数
	*@return s_ok-成功； 其它值表示失败。
	*/
	virtual status asyncCall(string obj, string intf, string method, IRpcParameter* param) = 0;
	/**
	*@brief 同步调用接口
	*
	*同步调用其它进程的函数。该函数被调用后，会被阻塞，直到远程函数执行完毕。
	*@param server 提供服务的进程的名字.
	*@param obj 远程的OBJ PATH,参考DEFAULT_OBJ
	*@param intf 远程的Interface Name, 参考DEFAULT_INTF
	*@param method 远程被调用的方法.
	*@param param 要传递给远程函数的参数
	*@param param 要传递给远程函数的参数
	*@param reply 远程方法执行完成后，返回给调用者的参数。
	*@return s_ok-成功； 其它值表示失败。
	*/
	virtual status syncCall(string server, string obj, string intf,
		string method, IRpcParameter* param, IRpcParameter* reply,int timeout) = 0;
	
	/** 
	*@brief 服务注册方法
	*
	*注册方法供远程调用。 
	*@param method_name 方法名，远程调用者通过方法名能关联到此方法。
	*@param sync true-同步服务； false-异步服务.
	*@param rm 服务供给接口. 
	*@return s_ok-成功； 其它值表示失败。
	*/
	virtual status registMethod(string method_name,boolean sync, IRpcMethod *rm) = 0;

	/** 
	*@brief 服务注销方法
	*
	*注册方法供远程调用。 
	*@param method_name 方法名，远程调用者通过方法名能关联到此方法。
	*@param sync true-同步服务； false-异步服务.
	*@param rm 服务供给接口. 
	*@return s_ok-成功； 其它值表示失败。
	*/	
	virtual status unRegistMethod(string method_name,boolean sync, IRpcMethod *rm) = 0;
	/** 
	*@brief 参数容器获取方法
	*
	*该方法用于初始化一个参数容器。
	*@return 参数容器。
	*/
	virtual IRpcParameter* initParameter() = 0;
	/** 
	*@brief 参数容器释放方法
	*
	*该方法用于释放一个参数容器。
	*@return 参数容器。
	*/
	virtual void deinitParameter(IRpcParameter* param) = 0;


};

#endif

