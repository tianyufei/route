/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     type.h                                                          *
*  @brief    该文件定义通用的数据类型和进程之间的公共消息类型。				 *
*  Details.                                                                  *
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

#ifndef _TYPE_H 
#define _TYPE_H 

#include <stdlib.h>
#include <string.h>
#include "config.h"

#ifndef boolean
#define boolean int /**< 定义布尔型类型 */
#endif

#ifndef true
#define true (!0) /**< 定义布尔真值 */
#endif

#ifndef false
#define false 0 /**< 定义布尔假值 */
#endif


typedef int status;

/**
*@brief 函数返回类型定义
*/
enum S_STATUS{
	s_err_base = -1024,
	s_err_call_in_sync,/**< 调用其它进程的服务失败。*/
	s_err_param, /**< 函数参数错误 */	
	s_err_init, /**< 函数初始化错误 */	
	s_err_syscall,	/**< 系统调用错误 */
	s_err_tmout, /**< 超时错误 */
	s_err_notfound, /**< 没有找到资源 */
	s_err_memoryout, /**< 内存分配失败*/
	s_err_reconfig, /**< 重复配置 */
	s_err_userdata,
	s_err_entry, /**< 功能无入口 */
	s_err_dl_auth,
	s_err_dl_url,
	s_err_install,
	s_err_type,
	/**
	*函数成功返回。
	*/
	s_ok = 0,
};


/**
*@将错误ID转换成字符串提示。
*
*@param err_id 错误ID,见S_STATUS的定义。
*@return 错误ID的字符串提示.
*/
static inline const char* status_errinfo(int err_id){
	switch(err_id){
		case s_err_param:
			return "param error!";
		case s_err_init:
			return "not init!";
		case s_err_syscall:
			return "call system function failed!";
		case s_err_tmout:
			return "time out!";
		case s_err_notfound:
			return "not find!";
		case s_err_memoryout:
			return "memory out!";
		case s_err_reconfig:
			return "redundancy config!";
		case s_err_userdata:
			return "userdata is null?";
		case s_ok:
			return "is ok!";
		case s_err_call_in_sync:
			return "call in service";
		case s_err_entry:
			return "no entry found";
		case s_err_dl_auth:
			return "dowload file failed, auth error!";
		case s_err_dl_url:
			return "download file failed, url not found!";
		case s_err_install:
			return "install file failed!";
		default:
			return "unknow error!";

	}
}

#define ASSERT_STATUS(x)do{ \
	if(x != s_ok){ \
		EOUT("%s",status_errinfo(x)); \
	} \
	return x; \
}while(0)


/*
*打印DEBUG信息。
*/

#ifdef __cplusplus
	extern "C" {
#endif
	
	extern void debugDout(char* pname, const char* file, const char* func, int line, char* fmt, ...);
	extern void debugEout(char* pname, const char* file, const char* func, int line, char* fmt, ...);
	extern void dumpOut(char* fmt, ...);
	extern void debugChangeTerm(char* term);
#ifdef __cplusplus
	}
#endif
	
/**
*@brief 调试信息输出接口。
*/
#define DOUT(fmt,args...) do{ \
	debugDout(NULL,__FILE__,__FUNCTION__,__LINE__,(char*)fmt,##args); \
}while(0)
	
/**
*@brief 错误信息输出接口。
*/
#define EOUT(fmt,args...)  do{ \
	debugEout(NULL,__FILE__,__FUNCTION__,__LINE__,(char*)fmt,##args); \
}while(0)
	
/**
*@brief dump信息。
*
*dump信息，dump信息与调试信息的区别在于它不受调试开关控制，适合用于帮助信息和统计信息的展示。
*/
#define DUMP(fmt,args...)  do{ \
	dumpOut(fmt,##args); \
}while(0)
	
/**
*@brief 指定日志输出的文件或终端，
*
*缺省为屏幕。 fmt为文件或终端的名字。
*/
#define DFILE(fmt) debugChangeTerm(fmt)




/*
*系统事件。
*/

#define E_YOS_CFG_CHANGED "e_yos_cfg_changed" /**< 广播/etc/config/下的配置文件有更改消息 intent e_yos_cfg_changed <config_file_name> <section> <section_name>*/
#define E_YOS_GDB_OPEN	"e_yos_dbg_open" /**< 收到DEBUG打开事件*/
#define E_YOS_GDB_CLOSE	"e_yos_dbg_close" /**< 收到DEBUG关闭事件*/ 
#define E_YOS_GDB_TRMC	"e_yos_dbg_trmc" /**< 切换输出中断*/

#endif
