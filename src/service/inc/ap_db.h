/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     ap_db.h                                                         *
*  @brief    该文件封装了AP数据库											 *
*  Details.                                                                  *
*            封装AP数据库,使查询更方便										 *
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

#ifndef _AP_DB_H
#define _AP_DB_H


#define AP_TYPE_THIN	1	/**< 瘦AP */
#define AP_TYPE_FAT		2	/**< 胖AP */
#define WLAN_TYPE_2G	4	/**< 2.4G无线MAC */
#define WLAN_TYPE_5G	8	/**< 5.8G无线MAC */



/**
*@brief 创建AP共享内存表
*
*AP共享内存表用于所有进程查询AP的WAN口MAC和无线MAC的对应关系.
*/
extern void apdb_init();

/**
*@brief 释放AP共享内存表
*
*/
extern void apdb_deinit();


/**
*@brief 向AP共享内存表中插入记录
*
*@param [in] ap_wan_mac AP的WAN口MAC地址
*@param [in] mac_2g		AP的2G无线的基地址贛AC地址
*@param [in] mac_5g		AP的2G无线的基地址贛AC地址
*@param [in] type 		AP的类型AP_TYPE_THIN-瘦AP, AP_TYPE_FAT-胖AP

*/
extern void apdb_insert(char* ap_wan_mac, char* mac_2g, char* mac_5g, int type);

/**
*@brief 删除一个AP的记录
*
*@param [in] ap_wan_mac 要删除的AP的MAC地址.
*/
extern void apdb_remove(char* ap_wan_mac);

/**
*@brief 检查一个MAC地址是否为合法的AP的mac地址.
*
*@param [in] ap_wan_mac 要删除的AP的MAC地址.
*@return 0-非法地址, 其它值为合法地址.
*/
extern int apdb_check(char* ap_wan_mac);

/**
*@brief 根据AP的WAN口MAC地址获取它2.4G或5.8G的MAC地址.
*
*@param [in] ap_wan_mac 要查找的AP的WAN MAC地址.
*@param [in] type 		要查找的MAC类型,WLAN_TYPE_2G-2.4G, WLAN_TYPE_5G-5.8G.
*@return 空串表示没查到, 其它值表示查到.
*/
extern char* apdb_get_wlan(char* ap_wan_mac, int type);

/**
*@brief 根据AP的WLAN口MAC地址获取它WAN口的MAC地址.
*
*@param [in] wlan_mac 要查找的AP的WLAN MAC地址.
*@return 空串表示没查到, 其它值表示查到.
*/
extern char* apdb_get_wan(char* wlan_mac);


#endif

