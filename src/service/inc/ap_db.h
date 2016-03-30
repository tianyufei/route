/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     ap_db.h                                                         *
*  @brief    ���ļ���װ��AP���ݿ�											 *
*  Details.                                                                  *
*            ��װAP���ݿ�,ʹ��ѯ������										 *
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

#ifndef _AP_DB_H
#define _AP_DB_H


#define AP_TYPE_THIN	1	/**< ��AP */
#define AP_TYPE_FAT		2	/**< ��AP */
#define WLAN_TYPE_2G	4	/**< 2.4G����MAC */
#define WLAN_TYPE_5G	8	/**< 5.8G����MAC */



/**
*@brief ����AP�����ڴ��
*
*AP�����ڴ���������н��̲�ѯAP��WAN��MAC������MAC�Ķ�Ӧ��ϵ.
*/
extern void apdb_init();

/**
*@brief �ͷ�AP�����ڴ��
*
*/
extern void apdb_deinit();


/**
*@brief ��AP�����ڴ���в����¼
*
*@param [in] ap_wan_mac AP��WAN��MAC��ַ
*@param [in] mac_2g		AP��2G���ߵĻ���ַ�MAC��ַ
*@param [in] mac_5g		AP��2G���ߵĻ���ַ�MAC��ַ
*@param [in] type 		AP������AP_TYPE_THIN-��AP, AP_TYPE_FAT-��AP

*/
extern void apdb_insert(char* ap_wan_mac, char* mac_2g, char* mac_5g, int type);

/**
*@brief ɾ��һ��AP�ļ�¼
*
*@param [in] ap_wan_mac Ҫɾ����AP��MAC��ַ.
*/
extern void apdb_remove(char* ap_wan_mac);

/**
*@brief ���һ��MAC��ַ�Ƿ�Ϊ�Ϸ���AP��mac��ַ.
*
*@param [in] ap_wan_mac Ҫɾ����AP��MAC��ַ.
*@return 0-�Ƿ���ַ, ����ֵΪ�Ϸ���ַ.
*/
extern int apdb_check(char* ap_wan_mac);

/**
*@brief ����AP��WAN��MAC��ַ��ȡ��2.4G��5.8G��MAC��ַ.
*
*@param [in] ap_wan_mac Ҫ���ҵ�AP��WAN MAC��ַ.
*@param [in] type 		Ҫ���ҵ�MAC����,WLAN_TYPE_2G-2.4G, WLAN_TYPE_5G-5.8G.
*@return �մ���ʾû�鵽, ����ֵ��ʾ�鵽.
*/
extern char* apdb_get_wlan(char* ap_wan_mac, int type);

/**
*@brief ����AP��WLAN��MAC��ַ��ȡ��WAN�ڵ�MAC��ַ.
*
*@param [in] wlan_mac Ҫ���ҵ�AP��WLAN MAC��ַ.
*@return �մ���ʾû�鵽, ����ֵ��ʾ�鵽.
*/
extern char* apdb_get_wan(char* wlan_mac);


#endif

