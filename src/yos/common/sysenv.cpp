/* 
 * Copyright (c) 2014 Yaya Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Draft:  zhengdajun 
 */
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <map>
#include <string>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include <stdlib.h>
#include "config.h"
using namespace std;

/* 因为在X86上不存在mtd分区,
	没有地方保存环境变量,
   	 因此采用MSATA和硬盘双备份的方式保存.
*/
#define ENV_HDD_FILE_NAME	"/mnt/usb/hhsysenv"


#ifndef TARGET_TYPE_host
#define ENV_FILE_NAME		"/etc/sysenv"
#else
#define ENV_FILE_NAME 		"/tmp/sysenv"
#endif

#ifndef O_BINARY
#define O_BINARY 0x8000
#endif

class EnvFileContent{
private:
	map<string,string> mEnvList;
	char buf[64*1024];

public:
	EnvFileContent(){
		memset(buf,0,sizeof(buf));
	}
	~EnvFileContent(){
	}

	void dumpFromFile(){
		/*  存在文件中的格式
			name=zhengdajun\0age=36\0home=beijing\0\0
		*/

		if(strstr(Y_PRODUCT_NAME,"E-")){
			struct stat cl_stat;
			if(stat(ENV_FILE_NAME,&cl_stat) != 0){
				if(stat(ENV_HDD_FILE_NAME,&cl_stat) == 0){
					char tmpbuf[512] = {0};
					snprintf(tmpbuf,sizeof(tmpbuf)-1,"cp %s %s -rf",ENV_HDD_FILE_NAME,ENV_FILE_NAME);
					system(tmpbuf);
					system("sync");
				}
			}
		}

		int fd = open(ENV_FILE_NAME,O_RDWR,0777);

		memset(buf,0,sizeof(buf));
		if(fd>=0){
			int readsize = read(fd,buf,sizeof(buf)-2);
			buf[readsize] = '\0';
			buf[readsize+1] = '\0';
			close(fd);
		}

		int i = 0;
		int len = -1;
		while((len = strlen(&buf[i])) > 0){
			string nameval = &(buf[i]);
			int j = nameval.find_first_of("=");
			string name = nameval.substr(0,j);
			string value = nameval.substr(j+1,nameval.length());
			mEnvList[name] = value;
			i += len + 1;
		}
	}

	void saveToFile(){
		int fd = 0;
		if(strstr(Y_PRODUCT_NAME,"B-") || strstr(Y_PRODUCT_NAME,"E-")){
		//file system
			fd = open(ENV_FILE_NAME,O_CREAT|O_WRONLY,0777);
		}else{		//mtd
			fd = open(ENV_FILE_NAME,O_RDWR,0777);
		}
		if(fd < 0){
			printf("saveToFile, fd=%d\n",fd);
			return;
		}

		int result = -1;
		map<string,string>::iterator it;
		for(it = mEnvList.begin(); it != mEnvList.end(); it++){
			map<string,string>::value_type pair = *it;
			string name = pair.first;
			string value = pair.second;
		
			snprintf(buf,sizeof(buf),"%s=%s",name.c_str(),value.c_str());
			result = write(fd,buf,strlen(buf)+1);
		}
		buf[0] = '\0';
		result = write(fd,buf,1);
		close(fd);
		if(strstr(Y_PRODUCT_NAME,"E-")){
			char tmpbuf[512] = {0};
			snprintf(tmpbuf,sizeof(tmpbuf)-1,"cp %s %s -rf",ENV_FILE_NAME,ENV_HDD_FILE_NAME);
			system(tmpbuf);
			system("sync");
		}
	}

	void setValue(string name, string value){
		if(name.empty())
			return;
		if(value.empty())
			mEnvList.erase(name);
		else
			mEnvList[name] = value;
	}

	string getValue(string name){
		if(name.empty())
			return "";
		string value = mEnvList[name];
		if(value.empty())
			mEnvList.erase(name);
		return value;
	}

	void dumpAll(){
		map<string,string>::iterator it;
		for(it = mEnvList.begin(); it != mEnvList.end(); it++){
			map<string,string>::value_type pair = *it;
			string name = pair.first;
			string value = pair.second;
			printf("\n%s=%s ",name.c_str(),value.c_str());
		}
		printf("\n");
	}

};

//extern "C" {

	
void set_sys_env(char *name, char* value){
	if(strstr(Y_PRODUCT_NAME,"B-") || strstr(Y_PRODUCT_NAME,"E-")){
		if(NULL != name && strlen(name) > 0){
			EnvFileContent env;
			env.dumpFromFile();
			if(NULL != value){
				env.setValue(name,value);
			}
			env.saveToFile();
		}
	}else{
		char cmd[256] = {0};
		sprintf(cmd,"fw_setenv %s %s 2>/dev/null",name,value);
		system(cmd);
	}
}

void get_sys_env(char *name, char* buf, int buf_size){
	if(strstr(Y_PRODUCT_NAME,"B-") || strstr(Y_PRODUCT_NAME,"E-")){
		EnvFileContent env;
		env.dumpFromFile();
		if((NULL == name) ||(0 == strlen(name)) ){
			env.dumpAll();
		}else{
			if(buf && buf_size){
				snprintf(buf,buf_size-1,"%s",env.getValue(name).c_str());
			}
		}

	}else{
		char cmd[256] = {0};
		char* p = NULL;
		int i = 0;
		sprintf(cmd,"fw_printenv %s 2>/dev/null",name);
		FILE* fp = popen(cmd,"r");
		if(fp){
			while(1) {
				if(fgets(cmd,120,fp) != NULL) {
					p = strstr(cmd,"Bad CRC");
					if(p) {
						break;
					}
					p = strstr(cmd,"=");
					if(p) {
						p++;
						i = strlen(p);
						strncpy(buf,p,i-1);
						break;
					}
				} else {
					break;
				}
			}
			pclose(fp);
		}
	}
}

//}
