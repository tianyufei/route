#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <json.h>
#include "config.h"
#include "tools.h"
#include "srv_env_id.h"
#include "sysenv.h"
#include "cfm_define.h"


/******************************************************************************************************
*VersionListFile
*/

void VersionListFile::parser(string filename){
	mFileMD5List.clear();
	struct stat c_st;
	if(!stat(s2c(filename),&c_st)){
		json_object *root_obj = json_object_from_file(s2c(filename));
		if(root_obj){
			mCfgVer = json_object_get_int(json_object_object_get(root_obj,STR_CFGVER));
			mAddress = c2s(json_object_get_string(json_object_object_get(root_obj,STR_ADDRESS)));
			mSn = c2s(json_object_get_string(json_object_object_get(root_obj,STR_SN)));
			json_object* obj_file_list = json_object_object_get(root_obj, STR_LIST);
			if(obj_file_list){
				int len = json_object_array_length(obj_file_list);
				for(int i = 0; i < len; i++){
					json_object* obj_i = json_object_array_get_idx(obj_file_list, i);
					string file = c2s(json_object_get_string(json_object_object_get(obj_i, STR_FILE)));
					string md5 = c2s(json_object_get_string(json_object_object_get(obj_i, STR_MD5)));
					if(!file.empty())
						mFileMD5List[file] = md5;
				}
			}
			json_object_put(root_obj);
		}
	}
}

void VersionListFile::writeFile(string filename){
	json_object *root_obj = json_object_new_object();
	if(root_obj){
		json_object_object_add(root_obj, STR_CFGVER, json_object_new_int(mCfgVer));
		char addrbuf[256] = {0};
		get_sys_env((char*)Y_SYS_ADDRESS_CODE,(char*)addrbuf,sizeof(addrbuf)-1);
		json_object_object_add(root_obj, STR_ADDRESS,json_object_new_string(addrbuf));
		//CREAE LIST.
		json_object *array_obj = json_object_new_array();
		if(array_obj){
			json_object_object_add(root_obj, STR_LIST,array_obj);
			map<string,string>::iterator it;
			for(it = mFileMD5List.begin(); it != mFileMD5List.end(); it++){
				json_object *item_obj = json_object_new_object();
				if(item_obj){
					map<string,string>::value_type pair = *it;
					string fname = pair.first;
					string md5 = pair.second;
					json_object_object_add(item_obj, STR_FILE, json_object_new_string(s2c(fname)));
					json_object_object_add(item_obj, STR_MD5, json_object_new_string(s2c(md5)));
					json_object_array_add (array_obj,item_obj);
				}
			}
			
		}
		FILE* fp = fopen(s2c(filename),"w+");
		if(fp){
			fprintf(fp,"%s",json_object_to_json_string(root_obj));
			fclose(fp);
		}						
		json_object_put(root_obj);
	}
}

string VersionListFile::getMD5(string filename){
	map<string,string>::iterator it = mFileMD5List.find(filename);
	if(it != mFileMD5List.end())
		return mFileMD5List[filename];
	return "";
}
		



/******************************************************************************************************
DR_VERLIST_REQ
*/
void contents_DR_VERLIST_REQ::writeFile(string filename,unsigned int req){
	json_object *root_obj = json_object_new_object();
	if(root_obj){
		json_object_object_add(root_obj, STR_SN, json_object_new_string(s2c(getOwnDevSn())));
		char addrbuf[256] = {0};
		get_sys_env((char*)Y_SYS_ADDRESS_CODE,(char*)addrbuf,sizeof(addrbuf)-1);
		json_object_object_add(root_obj, STR_ADDRESS,json_object_new_string(addrbuf));
		json_object_object_add(root_obj, STR_REQ, json_object_new_int(req));
		json_object_object_add(root_obj, STR_MSG, json_object_new_string("DR_VERLIST_REQ"));
		FILE* fp = fopen(s2c(filename),"w+");
		if(fp){
			fprintf(fp,"%s",json_object_to_json_string(root_obj));
			fclose(fp);
		}
		json_object_put(root_obj);
	}
}


/******************************************************************************************************
RD_VERLIST_RSP
*/
boolean contents_RD_VERLIST_RSP::parser(string filename){
	struct stat c_st;
	boolean rtn = false;
	if(!stat(s2c(filename),&c_st)){
		json_object *root_obj = json_object_from_file(s2c(filename));
		if(root_obj){
			mCfgVer = json_object_get_int(json_object_object_get(root_obj,STR_CFGVER));
			mUrl = c2s(json_object_get_string(json_object_object_get(root_obj,STR_URL)));
			mUser = c2s(json_object_get_string(json_object_object_get(root_obj,STR_USR))); 
			mPwd = c2s(json_object_get_string(json_object_object_get(root_obj,STR_PWD))); 
			mMsg = c2s(json_object_get_string(json_object_object_get(root_obj,STR_MSG))); 
			json_object* obj_file_list = json_object_object_get(root_obj,STR_LIST);
			if(obj_file_list){
				int len = json_object_array_length(obj_file_list);
				for(int i = 0; i < len; i++){
					json_object* obj_i = json_object_array_get_idx(obj_file_list, i);
					string file = c2s(json_object_get_string(json_object_object_get(obj_i, STR_FILE)));
					string md5 = c2s(json_object_get_string(json_object_object_get(obj_i, STR_MD5)));
					if(!file.empty()){
						mFileMD5List[file] = md5;
					}
				}
			}
			json_object_put(root_obj);
		}
	}else{
		return rtn;
	}
	if(mMsg != "RD_VERLIST_RSP"){
		return rtn;
	}
	return true;
}

/******************************************************************************************************
*
*/
void contents_DR_SYNC_CON::writeFile(string filename, int cfgver){
	json_object *root_obj = json_object_new_object();
	if(root_obj){
		json_object_object_add(root_obj, STR_SN, json_object_new_string(s2c(getOwnDevSn())));
		char addrbuf[256] = {0};
		get_sys_env((char*)Y_SYS_ADDRESS_CODE,(char*)addrbuf,sizeof(addrbuf)-1);
		json_object_object_add(root_obj, STR_ADDRESS,json_object_new_string(addrbuf));
		json_object_object_add(root_obj, STR_MSG, json_object_new_string("DR_SYNC_CON"));
		json_object_object_add(root_obj, STR_CFGVER, json_object_new_int(cfgver));
		FILE* fp = fopen(s2c(filename),"w+");
		if(fp){
			fprintf(fp,"%s",json_object_to_json_string(root_obj));
			fclose(fp);
		}
		json_object_put(root_obj);
	}
}


/***************************************************************************
	DR_UPLOAD_RSP
	{
		"sn":"xxxx",			#该配置是针对哪个设备的。
		"address":"01087600",	#地址码
		"msg":"DR_UPLOAD_RSP",  #消息类型
		"file":"advert", 		#传输的文件名
		"rst":"ok|failed"		#传输结果
	}
*/
boolean contents_DR_UPLOAD_RSP::parser(string filename){
	struct stat c_st;
	if(!stat(s2c(filename),&c_st)){
		json_object *root_obj = json_object_from_file(s2c(filename));
		if(root_obj){
			mSn = c2s(json_object_get_string(json_object_object_get(root_obj,STR_SN)));
			mAddress = c2s(json_object_get_string(json_object_object_get(root_obj,STR_ADDRESS)));
			mMsg = c2s(json_object_get_string(json_object_object_get(root_obj,STR_MSG)));
			mFile = c2s(json_object_get_string(json_object_object_get(root_obj,STR_FILE)));
			mResult = c2s(json_object_get_string(json_object_object_get(root_obj,STR_RST)));
			json_object_put(root_obj);
			return true;				
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
*@brief 计算一个小文件的MD5值
*
*@param file 文件名
*@return MD5 值
*/
extern string getFileMd5(string file){
	struct stat c_stat;
	string out;
	if(stat(s2c(file),&c_stat) == 0){
		char* tmp = (char*)malloc(c_stat.st_size + 1);
		if(tmp){
			memset(tmp,0,c_stat.st_size);
			FILE* fp = fopen(s2c(file),"r");
			if(fp){
				fread(tmp,c_stat.st_size,1,fp);
				fclose(fp);
				out = calMd5((unsigned char*)tmp,c_stat.st_size);
			}
			free(tmp);
		}
	}
	return out;
}


#define CFM_FILES_LOCK_FILE 	"/tmp/cfm_files_lockfile"

int lockCfm(){
	int fd = open(CFM_FILES_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 
	}
	return fd;
}


void unlockCfm(int fd){
	if(fd >= 0){
		flock(fd, LOCK_UN);
		close(fd);
	}
}


