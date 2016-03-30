//#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	struct stat sa,sb;
	int ra = 0, rb = 0;

	if(argc != 3){
		printf("error: argc = %d !\n",argc);	
		return 0;
	}

	ra = stat(argv[1],&sa); //source
	rb = stat(argv[2],&sb); //dest
	if(ra != 0){
		printf("error: not find %s!\n",argv[1]);
		return 0;
	}
	if(rb != 0){
		char buf[1024] = {0};
		snprintf(buf,sizeof(buf)-1,"ln -s %s %s",argv[1],argv[2]);
		system(buf);
		printf("%s\n",buf);
	}else{
		if(sa.st_mtime != sb.st_mtime){
			char buf[1024] = {0};
			snprintf(buf,sizeof(buf)-1,"rm %s; ln -s %s %s",argv[2],argv[1],argv[2]);
			system(buf);
			printf("%s\n",buf);
		}	
	}
	return 0;
}
