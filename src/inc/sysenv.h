/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     sysenv.h                                                        *
*  @brief    ���ļ���װ�˶Ի��������Ĳ���									 *
*  Details.                                                                  *
*            ��װ�˶Ի��������Ĳ���						 					 *
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

#ifndef _SYS_ENV_H
#define _SYS_ENV_H

#define SYSENV_MODE_SET 	1
#define SYSENV_MODE_GET 	2
#define SYSENV_MODE_MEMBER 	3

//#ifdef __cplusplus
//	extern "C" {
//#endif
/**
*@brief ���û�������
*
*@param [in] name ��������������
*@param [in] value ����������ֵ
*/
extern void set_sys_env(char *name, char* value);

/**
*@brief ��ȡ����������ֵ
*
*@param [in] name 		��������������
*@param [out] buf  		��Ż���������buffer.
*@param [out] buf_size  ��Ż���������buffer�Ĵ�С.
*/
extern void get_sys_env(char *name, char* buf, int buf_size);

//#ifdef __cplusplus
//}
//#endif

#endif

