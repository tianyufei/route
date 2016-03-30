/* 
 * Copyright (c) 2014 Yaya Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Draft:  zhengdajun 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include "config.h"
#include "sysenv.h"
#include "tools.h"

extern void get_sys_env(char *name, char* buf, int buf_size);
extern void set_sys_env(char *name, char* value);

void help(){
	 printf("\n sysenv -s -n <name> -v <value> ");
	 printf("\n sysenv -g -n <name>");
	 printf("\n sysenv -g");
	 printf("\n -g			 get system env");
	 printf("\n -s			 set system env");
	 printf("\n -n <name>	 env name");
	 printf("\n -v <value>	 env value");
	 printf("\n -m           sn member");	
 }

 int main(int argc, char* argv[]){
	 int opt = 0;
	 int mode = SYSENV_MODE_GET;
	 char name[128] = {0};
	 char value[128] = {0};

	while( (opt = getopt(argc, argv,"hsgmn:v:")) > 0){
		 switch(opt){
			 case 's':
				 mode = SYSENV_MODE_SET;
				 break;
			 case 'g':
				 mode = SYSENV_MODE_GET;
				 break;
			case 'm':
				 mode = SYSENV_MODE_MEMBER;
				 break;
			 case 'n':
				 if(NULL == (optarg)){
					 help();
					 return -1;
				 }
				 strncpy(name,(char*)(optarg),sizeof(name)-1);
				 break;
			 case 'v':
				 if(NULL == (optarg)){
					 help();
					 return -1;
				 }
				 strncpy(value,(char*)(optarg),sizeof(name)-1);
				 break;
			case 'h':
			default:
				help();
				break;
		 }		 
	 }
 
	 if(mode == SYSENV_MODE_SET){
	 	if(strlen(name) == 0){
			help();
			return -1;
		}
		set_sys_env(name,value);
 
	 }else if(mode == SYSENV_MODE_MEMBER){
		printf("%s",s2c(getOwnDevSn()));
	 }else if(mode == SYSENV_MODE_GET){
#ifndef SOFT_DEVROLE_SERVER
//#ifndef SOFT_DEVROLE_GATEWAY
		 if(strlen(name) == 0){
			 system("fw_printenv");
			 return 0;
		 }
//#endif		 
#endif
		 get_sys_env(name,value,sizeof(value));
		 printf("%s\n",value);
	 }
	 return 0;
 }
 

 
 

