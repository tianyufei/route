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
*定义调试信息接口每次调用时输出的最大长度。
*/
#define DEBUG_CONTENT_LEN	1024
/**
*定义SHELL命令最大长度。
*/
#define SHELL_CMD_MAX_LEN	1024

 #endif
 

