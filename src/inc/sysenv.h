/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     sysenv.h                                                        *
*  @brief    该文件封装了对环境变量的操作									 *
*  Details.                                                                  *
*            封装了对环境变量的操作						 					 *
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

#ifndef _SYS_ENV_H
#define _SYS_ENV_H

#define SYSENV_MODE_SET 	1
#define SYSENV_MODE_GET 	2
#define SYSENV_MODE_MEMBER 	3

//#ifdef __cplusplus
//	extern "C" {
//#endif
/**
*@brief 设置环境变量
*
*@param [in] name 环境变量的名称
*@param [in] value 环境变量的值
*/
extern void set_sys_env(char *name, char* value);

/**
*@brief 获取环境变量的值
*
*@param [in] name 		环境变量的名称
*@param [out] buf  		存放环境变量的buffer.
*@param [out] buf_size  存放环境变量的buffer的大小.
*/
extern void get_sys_env(char *name, char* buf, int buf_size);

//#ifdef __cplusplus
//}
//#endif

#endif

