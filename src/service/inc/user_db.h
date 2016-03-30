/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     user_db.h                                                       *
*  @brief    ���ļ���װ���û����ݿ�											 *
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

#ifndef _USR_DB_H
#define _USR_DB_H

typedef void (*read_proc)(char* mac, char* cls, long exire) ;

extern void usrdb_init();
extern void usrdb_deinit();
extern void usrdb_insert(char* mac, char* cls, long expire);
extern void usrdb_remove(char* mac);
extern int usrdb_lkup(char* mac, char* cls, int outsize);
extern void usrdb_display();
extern void usrdb_dbg_store();
extern void usrdb_readAll(read_proc proc);


#endif

