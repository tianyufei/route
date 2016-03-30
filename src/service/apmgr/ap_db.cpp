/*
	本文件用于保存ap的信息
*/
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
#include "ap_db.h"

#define APMGR_DB_LOCK_FILE 		"/tmp/apmgr_db_lockfile"
#define AP_COUNT_DEFAULT 		64
#define AP_COUNT_LICENCE_FILE 	"/etc/config/apcount_licence" //定义许可文件
#define STR_COUNT_TOKEN 		"count"
#define APMGR_CLSDB_KEY			2234



struct st_ap_node{
	int hash_prev_id; 
	int hash_next_id;
	int store_prev_id;
	int store_next_id;
	int type;			// 1 is thin ap mac, 2 is fat ap mac,  4 is 2g mac, 8 is 5g mac.
	char mac[20];
	char mac_wan[20];	//mac地址
	char mac_2g[20];	//2g
	char mac_5g[20];	//2g
};

struct st_ap_cb{
	struct st_ap_node hash[256];
	struct st_ap_node used;
	struct st_ap_node freed;
	struct st_ap_node tab[0];
//	int magic;
};

static struct st_ap_cb* g_cb = NULL;

static int _char2hex(char c){
	if((c >= '0') && (c <= '9'))
		return c - '0';
	if((c >= 'a') && ( c <= 'f'))
		return c - 'a' + 10;
	if((c >= 'A') && ( c <= 'F'))
		return c - 'A' + 10;
	return 0;
}

extern void apdb_init(){
	if(g_cb == NULL){
		int inited = false;
		struct stat clstat;
		//判断是否初始化了.
		if(stat(APMGR_DB_LOCK_FILE,&clstat) == 0)
			inited = true;

		int fd = open(APMGR_DB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
		if(fd >= 0){
			flock(fd, LOCK_EX); 
			//获取能支持的AP数量许可
			struct stat ct;
			int count = AP_COUNT_DEFAULT;
			if(stat(AP_COUNT_LICENCE_FILE,&ct) == 0){
				FILE* fp = fopen(AP_COUNT_LICENCE_FILE,"r");
				if(fp){
					char line[256];
					int num = 0;
					char token[32];
					while(fgets(line,sizeof(line)-1,fp)){
						if(strstr(line,STR_COUNT_TOKEN)){
							sscanf(line,"%s %d",token,&num);
							if(!strcmp(token,STR_COUNT_TOKEN)){
								count = num;
								break;
							}
						}
					}
					fclose(fp);
				}
			}
			//分配共享内存
			int shared_size = ((sizeof(st_ap_cb) + 3*count*sizeof(st_ap_node))/4096+1) * 4096;
			int shmid = shmget(APMGR_CLSDB_KEY, shared_size, IPC_CREAT|0600);
			if (-1 == shmid) {
				printf("apdb_init: shmget error:%s", strerror(errno));
				flock(fd, LOCK_UN);
				close(fd);
				return;
			}
			void* share_memory_head = shmat(shmid, NULL, 0) ;
			if ((int)share_memory_head == -1) {
				printf("apdb_init: share_memory_head error:%s", strerror(errno));
				flock(fd, LOCK_UN);
				close(fd);
				return;
			}
			g_cb = (struct st_ap_cb*)share_memory_head;
			if(!inited){
				memset(g_cb,0,shared_size);
				for(int i = 1; i < (3*count); i++){
					g_cb->tab[i].store_next_id = i+1;
					g_cb->tab[i].store_prev_id = i-1;
				}
				g_cb->freed.store_next_id = 1;
			}


			flock(fd, LOCK_UN);
			close(fd);
		}
	}
}


extern void apdb_deinit(){
	int fd = open(APMGR_DB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 
		shmdt((void*)g_cb);
		g_cb = NULL;
		flock(fd, LOCK_UN);
		close(fd);
	}
}

int _apdb_insert(char* mac){
	int idx = _char2hex(mac[15]) * 16 + _char2hex(mac[16]);
	int id = g_cb->hash[idx].hash_next_id;
	while(id){
		if(strcmp(g_cb->tab[id].mac,mac) == 0){
			break;
		}
		id = g_cb->tab[id].hash_next_id;
	}
	if(0 == id){
		id = g_cb->freed.store_next_id;
		if(id){
			//从空闲表中取出一个node
			int idnext = g_cb->tab[id].store_next_id;
			if(idnext){
				 g_cb->freed.store_next_id = idnext;
				 g_cb->tab[idnext].store_prev_id = 0;
			}
			//为该node填写值
			strncpy(g_cb->tab[id].mac,mac,sizeof(g_cb->tab[id].mac_wan)-1);
			//将该node加入到使用表中
			g_cb->tab[id].store_next_id = g_cb->used.store_next_id;
			if(g_cb->used.store_next_id){
				g_cb->tab[g_cb->used.store_next_id].store_prev_id = id;
			}
			g_cb->tab[id].store_prev_id = 0;
			g_cb->used.store_next_id = id;
			//将该node加入到hash表中
			g_cb->tab[id].hash_next_id = g_cb->hash[idx].hash_next_id;
			if(g_cb->hash[idx].hash_next_id){
				g_cb->tab[g_cb->hash[idx].hash_next_id].hash_prev_id = id;
			}
			g_cb->hash[idx].hash_next_id = id;
			g_cb->tab[id].hash_prev_id = 0;
		}
	}
	return id;
}

extern void apdb_insert(char* ap_wan_mac, char* mac_2g, char* mac_5g, int type/* AP_TYPE_FIT OR AP_TYPE_FAT */){
	if((NULL == g_cb) || (NULL == ap_wan_mac) || (0 == strlen(ap_wan_mac)))
		return;
	
	int fd = open(APMGR_DB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 
		int id_wan = _apdb_insert(ap_wan_mac);
		g_cb->tab[id_wan].type = type;
		if(mac_2g){
			int id_mac_2g = _apdb_insert(mac_2g);
			strncpy(g_cb->tab[id_wan].mac_2g,mac_2g,sizeof(g_cb->tab[id_wan].mac_2g)-1);
			g_cb->tab[id_mac_2g].type = WLAN_TYPE_2G;
			strncpy(g_cb->tab[id_mac_2g].mac_wan,ap_wan_mac,sizeof(g_cb->tab[id_mac_2g].mac_wan)-1);
		}
		if(mac_5g){
			int id_mac_5g = _apdb_insert(mac_5g);
			strncpy(g_cb->tab[id_wan].mac_5g,mac_5g,sizeof(g_cb->tab[id_wan].mac_5g)-1);
			g_cb->tab[id_mac_5g].type = WLAN_TYPE_5G;
			strncpy(g_cb->tab[id_mac_5g].mac_wan,ap_wan_mac,sizeof(g_cb->tab[id_mac_5g].mac_wan)-1);
		}
		flock(fd, LOCK_UN);
		close(fd);
	}
}



extern int apdb_check(char* ap_wan_mac){
	if((NULL == g_cb) || (NULL == ap_wan_mac) || (0 == strlen(ap_wan_mac)))
		return 0;
	int find = 0;
	int fd = open(APMGR_DB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 
		int idx = _char2hex(ap_wan_mac[15]) * 16 + _char2hex(ap_wan_mac[16]);
		int id = g_cb->hash[idx].hash_next_id;
		while(id){
			if(strcmp(g_cb->tab[id].mac,ap_wan_mac) == 0){
				find = id;
				break;
			}
			id = g_cb->tab[id].hash_next_id;
		}

		flock(fd, LOCK_UN);
		close(fd);
	}
	return find;
}

extern char* apdb_get_wlan(char* ap_wan_mac, int type){
	char* out = "";
	if((NULL == g_cb) || (NULL == ap_wan_mac) || (0 == strlen(ap_wan_mac)))
		return out;
	int fd = open(APMGR_DB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 

		int id = apdb_check(ap_wan_mac);
		if(id){
			if(type == WLAN_TYPE_2G)
				out = g_cb->tab[id].mac_2g;
			if(type == WLAN_TYPE_5G)
				out = g_cb->tab[id].mac_5g;
		}
		flock(fd, LOCK_UN);
		close(fd);
	}
	return out;	
}

extern char* apdb_get_wan(char* wlan_mac){
	char* out = "";
	if((NULL == g_cb) || (NULL == wlan_mac) || (0 == strlen(wlan_mac)))
		return out;
	int fd = open(APMGR_DB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 

		int id = apdb_check(wlan_mac);
		if(id){
			out = g_cb->tab[id].mac_wan;
		}
		flock(fd, LOCK_UN);
		close(fd);
	}
	return out;	
}


static void _apdb_remove(char* mac){
	if((NULL == mac) || (0 == strlen(mac)))
		return ;

	int idx = _char2hex(mac[15]) * 16 + _char2hex(mac[16]);
	int id = g_cb->hash[idx].hash_next_id;
	while(id){
		if(strcmp(g_cb->tab[id].mac,mac) == 0){
			//从hash表中摘除
			if(g_cb->tab[id].hash_prev_id){
				g_cb->tab[g_cb->tab[id].hash_prev_id].hash_next_id = g_cb->tab[id].hash_next_id;
			}else{
				g_cb->hash[idx].hash_next_id = g_cb->tab[id].hash_next_id;
			}
			
			if(g_cb->tab[id].hash_next_id){
				g_cb->tab[g_cb->tab[id].hash_next_id].hash_prev_id = g_cb->tab[id].hash_prev_id;
			}
			g_cb->tab[id].hash_prev_id = g_cb->tab[id].hash_next_id = 0;
			
			//从使用表中摘除
			if(g_cb->tab[id].store_prev_id){
				g_cb->tab[g_cb->tab[id].store_prev_id].store_next_id = g_cb->tab[id].store_next_id;
			}else{
				g_cb->used.store_next_id = g_cb->tab[id].store_next_id;
			}
			if(g_cb->tab[id].store_next_id){
				g_cb->tab[g_cb->tab[id].store_next_id].store_prev_id = g_cb->tab[id].store_prev_id;
			}
			g_cb->tab[id].store_prev_id = g_cb->tab[id].store_next_id = 0;

			//添加到空闲表中
			g_cb->tab[id].store_next_id = g_cb->freed.store_next_id;
			if( g_cb->freed.store_next_id)
				g_cb->tab[ g_cb->freed.store_next_id].store_prev_id = id;
			g_cb->freed.store_next_id = id;
			break;
		}
		id = g_cb->tab[id].hash_next_id;
	}
}

extern void apdb_remove(char* ap_wan_mac){
	if(NULL == g_cb) {
		int fd = open(APMGR_DB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
		if(fd >= 0){
			flock(fd, LOCK_EX); 
			int id = apdb_check(ap_wan_mac);
			if(id){
				_apdb_remove(g_cb->tab[id].mac_2g);
				_apdb_remove(g_cb->tab[id].mac_5g);
			}
			_apdb_remove(ap_wan_mac);
			flock(fd, LOCK_UN);
			close(fd);
		}
	}
}

