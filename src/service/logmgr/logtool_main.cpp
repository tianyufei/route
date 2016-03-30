#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>
#include "type.h"


void help(){
	 printf("\n logtool -s                          start logtool server ");
	 printf("\n waltool -d                     		display and remove cached info");
	 printf("\n waltool -i                     		display queue info");
	 printf("\n");
}

extern void logtool_init();
extern void logtool_write(char* info);
extern boolean logtool_read(char* info, int info_size);
extern void logtool_dump();
extern void logtool_deinit();
extern void logtool_queueinfo();


void server_start(){
	logtool_init();
	openlog("usrinfo", LOG_CONS | LOG_PID,	LOG_NEWS);
	while(1){
		char buf[256] = {0};
		while(logtool_read(buf,256)){
			syslog(LOG_INFO,"%s",buf);
		}
		sleep(1);
	}
	closelog();
	logtool_deinit();
}

int main(int argc, char* argv[]){
	int opt = 0;
	char mode = 0;
	while( (opt = getopt(argc, argv,"sdi")) > 0){
		 switch(opt){
			 case 's':
				 mode = 's';
				 break;
			 case 'd':
			 	 mode = 'd';
			 	 break;
			 case 'i':
			 	 mode = 'i';
				 break;
			default:
				help();
				return 0;
		 }		 
	 }
	 switch(mode){
		case 's':
			server_start();
			return 0;
		case 'i':
			logtool_init();
			logtool_queueinfo();
			logtool_deinit();
			return 0;
		case 'd':
			logtool_init();
			logtool_dump();
			logtool_deinit();
			return 0;
		default:
			help();
			return 0;
	}
}

