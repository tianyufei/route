/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     logtool.h                                                       *
*  @brief    ���ļ���װ����־������ 										 *
*  Details.                                                                  *
*            ��װ�û����ݿ�,ʹ��ѯ������									 *
*  @author   ZhengDajun                                                      *
*  @email    zhengdajun@vip.sina.com.                                        *
*  @version  1.0.0.1(�汾��)                                                 *
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
*@brief ��ʼ����־��¼���ߵĹ����ڴ�
*/
extern void logtool_init();

/**
*@brief ��¼��־
*
*@param [in] info ����¼����־���ݡ�
*/
extern void logtool_write(char* info);


/**
*@brief ��������ڴ�
*
*/
extern void logtool_deinit();

#endif

