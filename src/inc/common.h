/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     common.h                                                        *
*  @brief    该文件定义了加载Yaya类型的动态库的接口.                         *
*  Details.                                                                  *
*            Yaya类型的动态库,包括Instance, RInstance两个固定接口. 通过      *
*            这两个接口,可以获取动态库的实例.								 *
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



#ifndef _YAYA_COMMON_H 
#define _YAYA_COMMON_H 
#include <stdio.h>
#include <string>
using namespace std;
#include "config.h"
#include "icontext.h"
#include "tools.h"

/**@brief 获取上下文环境(IContext).
* 
*Yaya平台的每个进程都生存子一个Yaya的上下文环境中. 本函数在进程刚创建时使用.
*同一个进程中,不论调用多少次getContext, 只有一个上下文实例.
*
*@return 本进程的上下文实例. 
*/
extern IContext* getContext(); 

/**@brief 销毁由getContext函数分配的上下文环境实例.
*
*销毁IContext实例。该函数应该在进程退出的时候调用，调用前应确保进程
*内没有其它角色再使用该IContext实例。 
* 
*/
extern void releaseContext(IContext* ctx);

/**@brief Yaya动态库接口定义
*
*用户编译的动态库，都应该包含一个如下定义的函数接口，函数名为getInstance. 
*YAYA系统通过调用动态库的getInstance函数，获取实例。
*@param ctx  上下文环境接口
*@return 返回的实例指针。
*/
typedef void* (*Instance)(void* ctx) ;

/**@brief Yaya动态库接口定义
*
*用户编译的动态库，都应该包含一个如下定义的函数接口，函数名为relInstance. 
*YAYA系统通过调用动态库的relInstance函数，释放由getInstance函数分配的实例。
*@param 实例指针
*/
typedef void (*RInstance)(void* );


/**
*@brief  动态库加载管理类. 
*
*该类封装了操作系统加载动态库的细节，为用户提供了一个简单易用的接口。
*能被DllMgr加载的动态库,必须包含Instance接口.
*/
class DllMgr {
	private:
		void* mInstance;
		void* mHandle;
		string mPath;
	public:
		/**
		*该函数加载参数中指定的动态库。
		* @param path 动态库所在的路径和文件名. 例如 "/lib/librt.so".	
		*/
		DllMgr(string path);

		~DllMgr();
		/**
		*@brief 获取动态库中的实例。
		*
		*该函数调用动态库的Instance接口，获取实例。一个DllMgr类的生命周期内，该实例只会被创建一次。 
		*@param ctx 上下文环境句柄
		*@return 动态库实例
		*/
		void* getInstance(IContext* ctx);

		/**
		*@brief 释放动态库中的实例。
		*
		*该函数释放getInstance函数中获取的实例。 一旦释放后，通过getInstance创建的实例,不能再使用
		*/
		void relInstance();
};



#endif

