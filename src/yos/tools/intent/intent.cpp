/* 
 * Copyright (c) 2014 Yaya Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Draft:  zhengdajun 
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include "common.h"
#define SYSTEMEVENT_LOCK_FILE 	"/tmp/intent_lock_file"

int main (int argc, char* argv[]){

	int fd = open(SYSTEMEVENT_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0)
	{
		flock(fd, LOCK_EX); 
		IContext* ctx = getContext();
		if(ctx){
			ctx->init(argc,argv);
			IEvent* event = ctx->eventFactory();
			if(event && argc > 2){
				string event_key = argv[1];
				vector<string> param;
				for(unsigned int i = 2; i < argc; i++){
					param.push_back(argv[i]);
				}
				for(unsigned int i = 0; i < (6 - argc); i++){
					param.push_back("");
				}
				event->notify(event_key,param[0],param[1],param[2],param[3]);
			}
			releaseContext(ctx);
		}
		flock(fd, LOCK_UN);
		close(fd);
	}
	return 1;
}
