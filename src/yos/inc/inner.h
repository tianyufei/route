/* 
 * Copyright (c) 2014 Yaya Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Draft:  zhengdajun 
 */

 #ifndef _YOS_INNER_H
 #define _YOS_INNER_H


enum RpcInterfaceType{
	enRpcTypeServer = 0,
	enRpcTypeClient,
	enRpcTypeEvent,
};
typedef void* (*rpcInterface)(int type);

//EVENT Define
#define RPC_EVENT_NAME "event"
#define RPC_EVENT_BROADCAST "_event_broadcast"


/**
*���������Ϣ�ӿ�ÿ�ε���ʱ�������󳤶ȡ�
*/
#define DEBUG_CONTENT_LEN	1024
/**
*����SHELL������󳤶ȡ�
*/
#define SHELL_CMD_MAX_LEN	1024

 #endif
 

