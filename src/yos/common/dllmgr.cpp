/* 
 * Copyright (c) 2014 Yaya Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Draft:  zhengdajun 
 */

#include <stdio.h>
#include <dlfcn.h>
#include "common.h"

#define GET_INSTANCE "getInstance"
#define REL_INSTANCE "relInstance"


/**
* DLLMgr define.
*/
extern "C" {
static void* _dll_open(char* name, int flag){
	return (void*)dlopen(name,flag);
}

static void* _dll_close(void* handle){
	dlclose(handle);
}

static void* _dll_sym(void* handle, const char* func){
	return (void*)dlsym(handle,func);
}

}

DllMgr::DllMgr(string path){
	mPath = path;
	mHandle = _dll_open(s2c(path),RTLD_GLOBAL|RTLD_LAZY);
	mInstance = NULL;
	if(mHandle == NULL){
//		printf("DllMgr:: get handle error! lib=%s, errno=%s ",mPath.c_str(),dlerror());
	}else{
//		printf("DllMgr:: dlopen %s successful!\n",path.c_str());
	}
}


DllMgr::~DllMgr(){
	relInstance();
	if(mHandle){
		_dll_close(mHandle);
	}
	mHandle = NULL;
}



void* DllMgr::getInstance(IContext* ctx){
	if(mHandle && (!mInstance) ){
		Instance get = (Instance)_dll_sym(mHandle,GET_INSTANCE);
		if(get){
			mInstance = get((void*)ctx);
		}else{
//			printf("DllMgr:: get function %s error! lib=%s, errno=%s ",GET_INSTANCE,mPath.c_str(),dlerror());
		}
	}
	return mInstance;
}

void DllMgr::relInstance(){
	char* error = NULL;
	if(mHandle){
		RInstance rel = (RInstance)_dll_sym(mHandle,REL_INSTANCE);
		if(rel){
			rel(mInstance);
			if ((error = dlerror()) != NULL){
				fprintf(stderr, "dlsym: %s\n", error);
			}

			mInstance = NULL;
		}
		else{
//			printf("DllMgr:: get function %s error! lib=%s, errno=%s ",REL_INSTANCE,mPath.c_str(),dlerror());
		}
	}
}


