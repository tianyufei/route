#ifndef _SRV_EVENT_ID_H
#define _SRV_EVENT_ID_H

//COMMON DEFINE.
#define E_SRV_ALL_START 		"srv_all_start"
#define E_SRV_ALL_STOP 			"srv_all_stop"
#define E_SRV_NETWORK_RESTART	"srv_network_restart"

//USER MGR
#define E_SRV_URM_CLS_CHANGE	"srv_urm_cls_change" //发现终端,用户级别. intent srv_urm_cls_change <mac> <cls>
#define E_SRV_URM_CLS_REMOVE	"srv_urm_cls_remove" //发现终端,用户级别. intent srv_urm_cls_remove <mac> <cls>
#define E_SRV_URM_ILLEGAL_AP	"srv_urm_illegal_ap" //发现非法AP,要禁用. intent srv_urm_illegal_ap <ap_mac>	

//TERMINAL INFO
#define E_SRV_TERM_INFO			"srv_term_info" 	//终端信息,格式为 intent srv_term_info <mac> <cls> <expire> <gwip>
#define E_SRV_TERM_TEMPINFO		"srv_term_temp_info" //终端临时信息,格式为 intent srv_term_info <mac> <cls> <expire> <gwip>
//TERMINAL association
#define E_SRV_TERM_ASSCO		"srv_term_assco" //终端关联到某个ap, ap信息为该ap的ip地址和mac地址. 格式为 intent srv_term_assco <mac> <ap_mac> <ap_ip>
#define E_SRV_TERM_DEASSCO		"srv_term_deassco" //终端与某个ap解关联, ap信息为该ap的ip地址和mac地址. 格式为 intent srv_term_deassco <mac> <ap_mac> <ap_ip>
#define E_SRV_TERM_IPALLOC		"srv_term_ipalloc"	//终端获取ip地址, 该消息应该由dnsmasq发出. 格式为 intent srv_term_ipalloc <mac> <ip> <expire>

//HEARTBEAT INFO
#define E_SRV_HB_CFG_INFO		"srv_hb_cfg_info"		//心跳模块通知配置管理模块，关于配置的信息。

#define E_SRV_CFM_UPDATE_START  "srv_cfm_update_start"	//同步配置开始
#define E_SRV_CFM_UPDATE_STOP 	"srv_cfm_update_stop"	//同步配置结束



#endif
