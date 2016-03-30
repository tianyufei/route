#ifndef _SRV_EVENT_ID_H
#define _SRV_EVENT_ID_H

//COMMON DEFINE.
#define E_SRV_ALL_START 		"srv_all_start"
#define E_SRV_ALL_STOP 			"srv_all_stop"
#define E_SRV_NETWORK_RESTART	"srv_network_restart"

//USER MGR
#define E_SRV_URM_CLS_CHANGE	"srv_urm_cls_change" //�����ն�,�û�����. intent srv_urm_cls_change <mac> <cls>
#define E_SRV_URM_CLS_REMOVE	"srv_urm_cls_remove" //�����ն�,�û�����. intent srv_urm_cls_remove <mac> <cls>
#define E_SRV_URM_ILLEGAL_AP	"srv_urm_illegal_ap" //���ַǷ�AP,Ҫ����. intent srv_urm_illegal_ap <ap_mac>	

//TERMINAL INFO
#define E_SRV_TERM_INFO			"srv_term_info" 	//�ն���Ϣ,��ʽΪ intent srv_term_info <mac> <cls> <expire> <gwip>
#define E_SRV_TERM_TEMPINFO		"srv_term_temp_info" //�ն���ʱ��Ϣ,��ʽΪ intent srv_term_info <mac> <cls> <expire> <gwip>
//TERMINAL association
#define E_SRV_TERM_ASSCO		"srv_term_assco" //�ն˹�����ĳ��ap, ap��ϢΪ��ap��ip��ַ��mac��ַ. ��ʽΪ intent srv_term_assco <mac> <ap_mac> <ap_ip>
#define E_SRV_TERM_DEASSCO		"srv_term_deassco" //�ն���ĳ��ap�����, ap��ϢΪ��ap��ip��ַ��mac��ַ. ��ʽΪ intent srv_term_deassco <mac> <ap_mac> <ap_ip>
#define E_SRV_TERM_IPALLOC		"srv_term_ipalloc"	//�ն˻�ȡip��ַ, ����ϢӦ����dnsmasq����. ��ʽΪ intent srv_term_ipalloc <mac> <ip> <expire>

//HEARTBEAT INFO
#define E_SRV_HB_CFG_INFO		"srv_hb_cfg_info"		//����ģ��֪ͨ���ù���ģ�飬�������õ���Ϣ��

#define E_SRV_CFM_UPDATE_START  "srv_cfm_update_start"	//ͬ�����ÿ�ʼ
#define E_SRV_CFM_UPDATE_STOP 	"srv_cfm_update_stop"	//ͬ�����ý���



#endif
