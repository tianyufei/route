#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include "user_db.h"



void help(){
	 printf("\n usrdb -s                          	display mac and userclass ");
	 printf("\n usrdb -a <mac> -v <cls> -e <expire>	add mac and userclass");
	 printf("\n usrdb -d <mac>                    	remove mac");
	 printf("\n usrdb -i                          	clsdb detail info");
	 printf("\n");
}


int main(int argc, char* argv[]){
	int opt = 0;
	int mode = 0;
	long expire = 0;
	char mac[32] = {0};
	char cls[32] = {0};
	usrdb_init();
	while( (opt = getopt(argc, argv,"isa:d:v:e:")) > 0){
		 switch(opt){
			 case 's':
				 usrdb_display();
				 return 0;
			 case 'i':
			 	 usrdb_dbg_store();
			 	 return 0;
			 case 'a':
				 mode = 1;
				 if(NULL == (optarg)){
					 help();
					 return -1;
				 }
				 strncpy(mac,(char*)(optarg),sizeof(mac)-1);
				 break;
			 case 'e':
				 mode = 1;
				 if(NULL == (optarg)){
					 help();
					 return -1;
				 }
				 expire = atol(optarg);
				 break;
			 case 'd':
				 mode = 2;
				 if(NULL == (optarg)){
					 help();
					 return -1;
				 }
				 strncpy(mac,(char*)(optarg),sizeof(mac)-1);
				 break;
			 case 'v':
				 if(NULL == (optarg)){
					 help();
					 return -1;
				 }
				 strncpy(cls,(char*)(optarg),sizeof(cls)-1);
				 break;
			default:
				help();
				return 0;
		 }		 
	 }
	
	switch(mode){
		case 1:
			usrdb_insert(mac,cls,expire);
			break;
		case 2:
			usrdb_remove(mac);
			break;
		default:
			help();
			break;
	}
	usrdb_deinit();
	return 0;
}

