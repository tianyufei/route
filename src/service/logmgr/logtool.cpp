/*
	本文件用于跨进程读写日志
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
#include <syslog.h>
#include "type.h"


#define LOGTOOL_LOCK_FILE 	"/tmp/logtool_lock_file"
#define LOGTOOL_KEY			3234
#define LOG_QUEUE_SIZE 		2048


struct cl_log_node {
	char buf[256];
};


struct cl_log_cb{
	int head;
	int tail;
//	int magic;
	unsigned int write_count;
	unsigned int read_count;
	struct cl_log_node node[LOG_QUEUE_SIZE];
};

static struct cl_log_cb* g_cb = NULL;


extern void logtool_init(){
	if(g_cb == NULL){
		int inited = false;
		struct stat clstat;
		if(stat(LOGTOOL_LOCK_FILE,&clstat) == 0)
			inited = true;
		int fd = open(LOGTOOL_LOCK_FILE,O_CREAT|O_WRONLY,0777);
		if(fd >= 0){
			flock(fd, LOCK_EX); 
			int shared_size =  ((sizeof(struct cl_log_cb) / 4096) + 1) * 4096;
			int shmid = shmget(LOGTOOL_KEY, shared_size, IPC_CREAT|0600);
			if (-1 == shmid) {
				printf("logtool_init: shmget error:%s", strerror(errno));
				flock(fd, LOCK_UN);
				close(fd);
				return;
			}
			void* share_memory_head = shmat(shmid, NULL, 0) ;
			if ((int)share_memory_head == -1) {
				printf("logtool_init: share_memory_head error:%s", strerror(errno));
				flock(fd, LOCK_UN);
				close(fd);
				return;
			}
			g_cb = (struct cl_log_cb*)share_memory_head;
			if(!inited){
			//if(g_cb->magic != 0x12345678){
				memset(g_cb,0,sizeof(struct cl_log_cb));
			//	g_cb->magic = 0x12345678;
				g_cb->head = 0;
				g_cb->tail = -1;
			}
			flock(fd, LOCK_UN);
			close(fd);
		}
	}
}

extern void logtool_write(char* info){
	if(g_cb != NULL){
		int fd = open(LOGTOOL_LOCK_FILE,O_CREAT|O_WRONLY,0777);
		if(fd >= 0){
			flock(fd, LOCK_EX); 
			strncpy(g_cb->node[g_cb->head].buf,info,255);
			g_cb->write_count++;
			g_cb->head = (g_cb->head + 1)%LOG_QUEUE_SIZE;
			if(g_cb->tail == g_cb->head){
				g_cb->tail = (g_cb->tail + 1)%LOG_QUEUE_SIZE;
			}
			flock(fd, LOCK_UN);
			close(fd);
		}
	}
}


extern boolean logtool_read(char* info, int info_size){
	boolean out = false;
	if(info && info_size && (g_cb != NULL)){
		int fd = open(LOGTOOL_LOCK_FILE,O_CREAT|O_WRONLY,0777);
		if(fd >= 0){
			flock(fd, LOCK_EX);
			int rst = g_cb->head - g_cb->tail;
			if(rst < 0)
				rst += LOG_QUEUE_SIZE;
			if(rst > 1){
				out = true;
				g_cb->tail = (g_cb->tail + 1)%LOG_QUEUE_SIZE;
				strncpy(info,g_cb->node[g_cb->tail].buf,info_size-1);
				g_cb->read_count++;
			}
			flock(fd, LOCK_UN);
			close(fd);
		}
	}
	return out;
}

extern void logtool_queueinfo(){
	if(g_cb != NULL){
		int fd = open(LOGTOOL_LOCK_FILE,O_CREAT|O_WRONLY,0777);
		if(fd >= 0){
			flock(fd, LOCK_EX);

			int cached = g_cb->head - g_cb->tail - 1;
			if(cached < 0)
				cached += LOG_QUEUE_SIZE;
			
			printf("wrotten count: %d\n",g_cb->write_count);
			printf("read    count: %d\n",g_cb->read_count);
			printf("cached  count: %d\n",cached);
			
			flock(fd, LOCK_UN);
			close(fd);
		}
	}

}

extern void logtool_dump(){
	char buf[256] = {0};
	while(logtool_read(buf,256)){
		printf(buf);
		printf("\n");
	}
}

extern void logtool_deinit(){
	int fd = open(LOGTOOL_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 
		shmdt((void*)g_cb);
		g_cb = NULL;
		flock(fd, LOCK_UN);
		close(fd);
	}
}

