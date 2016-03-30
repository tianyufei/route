/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     logtool.h                                                       *
*  @brief    该文件封装了日志处理工具 										 *
*  Details.                                                                  *
*            封装用户数据库,使查询更方便									 *
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

#ifndef _LOG_TOOL_H
#define _LOG_TOOL_H

/**
*@brief 初始化日志记录工具的共享内存
*/
extern void logtool_init();

/**
*@brief 记录日志
*
*@param [in] info 被记录的日志内容。
*/
extern void logtool_write(char* info);


/**
*@brief 清除共享内存
*
*/
extern void logtool_deinit();

#endif

