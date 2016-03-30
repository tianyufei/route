/*
	本文件用于跨进程读写用户表
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
#include "user_db.h"

#define USRMGR_CLSDB_LOCK_FILE 	"/tmp/usrmgr_db_lockfile"

struct cl_usr_node {
	int hash_prev_id; 
	int hash_next_id;
	int store_prev_id;
	int store_next_id;
	long expire;
	char mac[20];	//mac地址
	char cls[20];	//用户级别
};

struct cl_usr_cb{
	struct cl_usr_node tab[1024];
	struct cl_usr_node hash[256];
	struct cl_usr_node used;
	struct cl_usr_node freed;
//	int magic;
};



static struct cl_usr_cb* g_cb = NULL;


static int _char2hex(char c){
	if((c >= '0') && (c <= '9'))
		return c - '0';
	if((c >= 'a') && ( c <= 'f'))
		return c - 'a' + 10;
	if((c >= 'A') && ( c <= 'F'))
		return c - 'A' + 10;
	return 0;
}


#define USRMGR_CLSDB_KEY	1234

extern void usrdb_dbg_store(){
	int count = 0;
	int id = g_cb->freed.store_next_id;
	while(id){
		count++;
		id = g_cb->tab[id].store_next_id;
	}
	printf("free store: %d\n",count);
	count = 0;
	id = g_cb->used.store_next_id;
	while(id){
		count++;
		id = g_cb->tab[id].store_next_id;
	}
	printf("used store: %d\n",count);
	for(unsigned int i = 0; i < 256; i++){
		id = g_cb->hash[i].hash_next_id;
		count = 0;
		while(id){
			count++;
			id = g_cb->tab[id].hash_next_id;
		}
		if(count){
			printf("hash[index=%03d,count=%02d]: ",i,count);
			id = g_cb->hash[i].hash_next_id;
			count = 0;
			while(id){
				printf("(%s,%s) ",g_cb->tab[id].mac,g_cb->tab[id].cls);
				id = g_cb->tab[id].hash_next_id;
			}
			printf("\n");
		}
	}

}

extern void usrdb_init(){
	if(g_cb == NULL){
		int inited = false;
		struct stat clstat;
		if(stat(USRMGR_CLSDB_LOCK_FILE,&clstat) == 0)
			inited = true;
		int fd = open(USRMGR_CLSDB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
		if(fd >= 0){
			flock(fd, LOCK_EX); 
//			int shmid = shmget(USRMGR_CLSDB_KEY, sizeof(struct cl_usr_cb), S_IRUSH|S_IWUSR|IPC_CREAT|0666);
			int shared_size =  ((sizeof(struct cl_usr_cb) / 4096) + 1) * 4096;
			int shmid = shmget(USRMGR_CLSDB_KEY, shared_size, IPC_CREAT|0600);
			if (-1 == shmid) {
				printf("usrdb_init: shmget error:%s", strerror(errno));
				flock(fd, LOCK_UN);
				close(fd);
				return;
			}
			void* share_memory_head = shmat(shmid, NULL, 0) ;
			if ((int)share_memory_head == -1) {
				printf("usrdb_init: share_memory_head error:%s", strerror(errno));
				flock(fd, LOCK_UN);
				close(fd);
				return;
			}
			g_cb = (struct cl_usr_cb*)share_memory_head;
			if(!inited){
		//	if(g_cb->magic != 0x87654321){
				memset(g_cb,0,sizeof(struct cl_usr_cb));
		//		g_cb->magic = 0x87654321;
				for(unsigned int i = 1; i < 1024; i++){
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

extern void usrdb_insert(char* mac, char* cls, long expire){
	int idx = _char2hex(mac[15]) * 16 + _char2hex(mac[16]);
	int fd = open(USRMGR_CLSDB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 
		int id = g_cb->hash[idx].hash_next_id;
		while(id){
			if(strcmp(g_cb->tab[id].mac,mac) == 0){
				strncpy(g_cb->tab[id].cls,cls,sizeof(g_cb->tab[id].cls)-1);
				g_cb->tab[id].expire = expire;
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
				strncpy(g_cb->tab[id].mac,mac,sizeof(g_cb->tab[id].mac)-1);
				strncpy(g_cb->tab[id].cls,cls,sizeof(g_cb->tab[id].cls)-1);
				g_cb->tab[id].expire = expire;
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
		flock(fd, LOCK_UN);
		close(fd);
	}

}

extern void usrdb_remove(char* mac){
	int idx = _char2hex(mac[15]) * 16 + _char2hex(mac[16]);
	int fd = open(USRMGR_CLSDB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 
		if(g_cb){
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
		flock(fd, LOCK_UN);
		close(fd);
	}
}


extern int usrdb_lkup(char* mac, char* cls, int outsize){
	int idx = _char2hex(mac[15]) * 16 + _char2hex(mac[16]);
	int out = 0;
//	printf("%s: idx=%d\n",__FUNCTION__,idx);
	if(cls && outsize){
		cls[0] = '\0';
		int fd = open(USRMGR_CLSDB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
		if(fd >= 0){
//			printf("%s: fd=%d\n",__FUNCTION__,fd);
			flock(fd, LOCK_EX); 
			if(g_cb){
				int id = g_cb->hash[idx].hash_next_id;
				while(id){
//					printf("%s: ==>id=%d,idnext=%d,idprev=%d,want=%s,mac=%s,cls=%s\n",__FUNCTION__,
//						id,g_cb->tab[id].hash_next_id,g_cb->tab[id].hash_prev_id,
//						mac,
//						g_cb->tab[id].mac,
//						g_cb->tab[id].cls
//					);
					if(strcmp(g_cb->tab[id].mac,mac) == 0){
						strncpy(cls,g_cb->tab[id].cls,outsize);
						out = 1;
						break;
					}
					id = g_cb->tab[id].hash_next_id;
				}
			}
			flock(fd, LOCK_UN);
			close(fd);
		}
	}
	return out;
}

extern void usrdb_display(){
	int fd = open(USRMGR_CLSDB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 
		if(g_cb){
			int count = 0;
			for(unsigned int i = 0; i < 256; i++){
				int id = g_cb->hash[i].hash_next_id;
				while(id){
					printf("\n %d: %s %s %u",++count,g_cb->tab[id].mac,g_cb->tab[id].cls,g_cb->tab[id].expire);
					id = g_cb->tab[id].hash_next_id;
				}
			}
			printf("\n");
		}
		flock(fd, LOCK_UN);
		close(fd);
	}
}

extern void usrdb_readAll(read_proc proc){
	int fd = open(USRMGR_CLSDB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 
		if(g_cb){
			int count = 0;
			for(unsigned int i = 0; i < 256; i++){
				int id = g_cb->hash[i].hash_next_id;
				while(id){
					if(proc)
						proc(g_cb->tab[id].mac,g_cb->tab[id].cls,g_cb->tab[id].expire);
					id = g_cb->tab[id].hash_next_id;
				}
			}
		}
		flock(fd, LOCK_UN);
		close(fd);
	}
}

extern void usrdb_deinit(){
	int fd = open(USRMGR_CLSDB_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 
		shmdt((void*)g_cb);
		g_cb = NULL;
		flock(fd, LOCK_UN);
		close(fd);
	}
}

