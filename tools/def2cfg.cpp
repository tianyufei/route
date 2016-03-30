#include <stdlib.h>
#include <string>
using namespace std;
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]){
	if(argc < 2)
		return 0;
	FILE* fp = fopen(argv[1],"r");
	FILE* fpsys = fopen("image/sysenv","w");
	char buf[256];
	if(fp){
		while(fgets(buf,sizeof(buf),fp)){
			//ignore hint.
			char* hint = strstr(buf,"#");
			if(hint)
				hint[0] = '\0';
			//
			int m = string(buf).find_first_of("=");
			int b = string(buf).find("export");
			if(b == string::npos)
				continue;

			b += strlen("export");

			int e = string(buf).find_last_not_of(" \n\t\r");

			if(e - string(buf).find("=n") == 1){
				printf("#undef %s\n",string(buf).substr(b,m-b).c_str());
				continue;
			}

			if(string(buf).find(" DEVICE_VENDOR=") != string::npos){
				printf("#define DEVICE_VENDOR_%s 1 \n",string(buf).substr(m+1,e-m).c_str());
				printf("#define DEVICE_VENDOR \"%s\"\n",string(buf).substr(m+1,e-m).c_str());
				continue;
			}
			if(string(buf).find(" DEVICE_CATEGORY=") != string::npos){
				printf("#define DEVICE_CATEGORY_%s 1 \n",string(buf).substr(m+1,e-m).c_str());
			}
			if(string(buf).find(" DEVICE_TYPE=") != string::npos){
				printf("#define DEVICE_TYPE_%s 1 \n",string(buf).substr(m+1,e-m).c_str());
			}
			if(string(buf).find(" DEVICE_USER=") != string::npos){
				printf("#define DEVICE_USER_%s 1 \n",string(buf).substr(m+1,e-m).c_str());
			}
			if(fpsys && string(buf).find(" SYS_ENV=") != string::npos){
				string env = string(buf).substr(m+1,e-m);
				int _b = env.find_first_of("\"");
				int _e = env.find_last_of("\"");
				env = env.substr(_b+1,_e-_b-1);
				char c = '\0';

				for(_b = 0; _b < env.length(); _b++){
					char c = env[_b];
					if(c == ','){
						c = '\0';
					}
					fwrite(&c,1,1,fpsys);
				}
				c = '\0';
				fwrite(&c,1,1,fpsys);
				fwrite(&c,1,1,fpsys);
			}
			


			if(e - string(buf).find("=y") == 1){
				printf("#define %s 1\n",string(buf).substr(b,m-b).c_str());
			}else{
				string env = string(buf).substr(m+1,e-m);
				int _b = env.find_first_of("\"");
				int _e = env.find_last_of("\"");
				if(_b != _e){
					env = env.substr(_b+1,_e-_b-1);
				}
				printf("#define %s \"%s\"\n",string(buf).substr(b,m-b).c_str(),env.c_str());
			}
		}
		fclose(fp);
	}
	if(fpsys)
		fclose(fpsys);
}
