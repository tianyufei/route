/* 
 * Copyright (c) 2014 YAYA Software Team.
 * All Rights Reserved.
 *
 * Draft:  zhengdajun 
 */
#include <stdlib.h> 
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <asm/unistd.h>
#include <sys/stat.h>
//#include <linux/netlink.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
//#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
//#include <linux/if_arp.h>
#include <linux/if.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/shm.h>
#include <sys/syslog.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <stdarg.h>
#include "tools.h"
#include "config.h"
#include <openssl/md5.h>

#ifdef TARGET_TYPE_target
#include <uci.h>
#endif

typedef unsigned char       BYTE;


static int g_dout_flag = false;
static FILE* g_out_fp = NULL;
static string g_out_term;
#define DEBUG_CONTENT_LEN 1024

#define POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL               -1
#define POLARSSL_ERR_BASE64_INVALID_CHARACTER              -2

static char g_dev_sn[64] = {0};

static const unsigned char base64_dec_map[128] =
{
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127,  62, 127, 127, 127,  63,  52,  53,
     54,  55,  56,  57,  58,  59,  60,  61, 127, 127,
    127,  64, 127, 127, 127,   0,   1,   2,   3,   4,
      5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
     25, 127, 127, 127, 127, 127, 127,  26,  27,  28,
     29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
     39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
     49,  50,  51, 127, 127, 127, 127, 127
};
static const unsigned char base64_enc_map[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'
};


int encode_base64( unsigned char *dst, int *dlen,
                   unsigned char *src, int  slen )
{
    int i, n;
    int C1, C2, C3;
    unsigned char *p;

    if( slen == 0 )
        return( 0 );

    n = (slen << 3) / 6;

    switch( (slen << 3) - (n * 6) )
    {
        case  2: n += 3; break;
        case  4: n += 2; break;
        default: break;
    }

    if( *dlen < n + 1 )
    {
        *dlen = n + 1;
        return( POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL );
    }

    n = (slen / 3) * 3;

    for( i = 0, p = dst; i < n; i += 3 )
    {
        C1 = *src++;
        C2 = *src++;
        C3 = *src++;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 &  3) << 4) + (C2 >> 4)) & 0x3F];
        *p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
        *p++ = base64_enc_map[C3 & 0x3F];
    }

    if( i < slen )
    {
        C1 = *src++;
        C2 = ((i + 1) < slen) ? *src++ : 0;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

        if( (i + 1) < slen )
             *p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
        else *p++ = '=';

        *p++ = '=';
    }

    *dlen = p - dst;
    *p = 0;

    return( 0 );
}

int decode_base64( unsigned char *dst, int *dlen,
                   unsigned char *src, int  slen )
{
    int i, j, n;
    unsigned long x;
    unsigned char *p;

    for( i = j = n = 0; i < slen; i++ )
    {
        if( ( slen - i ) >= 2 &&
            src[i] == '\r' && src[i + 1] == '\n' )
            continue;

        if( src[i] == '\n' )
            continue;

        if( src[i] == '=' && ++j > 2 )
        	{
            return( POLARSSL_ERR_BASE64_INVALID_CHARACTER );
        	}
        if( src[i] > 127 || base64_dec_map[src[i]] == 127 )
        	{
            return( POLARSSL_ERR_BASE64_INVALID_CHARACTER );
        	}

        if( base64_dec_map[src[i]] < 64 && j != 0 )
        	{
            return( POLARSSL_ERR_BASE64_INVALID_CHARACTER );
        	}

        n++;
    }

    if( n == 0 )
        return( 0 );
	

    n = ((n * 6) + 7) >> 3;
	

    if( *dlen < n )
    {
        *dlen = n;
        return( POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL );
    }

   for( j = 3, n = x = 0, p = dst; i > 0; i--, src++ )
   {
        if( *src == '\r' || *src == '\n' )
            continue;

        j -= ( base64_dec_map[*src] == 64 );
        x  = (x << 6) | ( base64_dec_map[*src] & 0x3F );

        if( ++n == 4 )
        {
            n = 0;
            if( j > 0 ) *p++ = (unsigned char)( x >> 16 );
            if( j > 1 ) *p++ = (unsigned char)( x >>  8 );
            if( j > 2 ) *p++ = (unsigned char)( x       );
        }
    }

    *dlen = p - dst;

    return( 0 );
}

char Encode_GetChar(BYTE num)
{
    return 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "+/="[num];
}

size_t Base64_Encode_GB2312(char *pDest, const char *pSrc, size_t srclen)
{
    BYTE input[3], output[4];
    size_t i, index_src = 0, index_dest = 0;
    for(i = 0; i < srclen; i += 3)
    {
        //char [0]
        input[0] = pSrc[index_src++];
        output[0] = (BYTE)(input[0] >> 2);
        pDest[index_dest++] = Encode_GetChar(output[0]);

        //char [1]
        if(index_src < srclen)
        {
            input[1] = pSrc[index_src++];
            output[1] = (BYTE)(((input[0] & 0x03) << 4) + (input[1] >> 4));
            pDest[index_dest++] = Encode_GetChar(output[1]);
        }
        else
        {
            output[1] = (BYTE)((input[0] & 0x03) << 4);
            pDest[index_dest++] = Encode_GetChar(output[1]);
            pDest[index_dest++] = '=';
            pDest[index_dest++] = '=';
            break;
        }
        
        //char [2]
        if(index_src < srclen)
        {
            input[2] = pSrc[index_src++];
            output[2] = (BYTE)(((input[1] & 0x0f) << 2) + (input[2] >> 6));
            pDest[index_dest++] = Encode_GetChar(output[2]);
        }
        else
        {
            output[2] = (BYTE)((input[1] & 0x0f) << 2);
            pDest[index_dest++] = Encode_GetChar(output[2]);
            pDest[index_dest++] = '=';
            break;
        }

        //char [3]
        output[3] = (BYTE)(input[2] & 0x3f);
        pDest[index_dest++] = Encode_GetChar(output[3]);
    }
    //null-terminator
    pDest[index_dest] = 0;
    return index_dest;
}


inline char GetCharIndex(char c) //内联函数可以省去函数调用过程，提速  
{   if((c >= 'A') && (c <= 'Z'))  
    {   return c - 'A';  
    }else if((c >= 'a') && (c <= 'z'))  
    {   return c - 'a' + 26;  
    }else if((c >= '0') && (c <= '9'))  
    {   return c - '0' + 52;  
    }else if(c == '+')  
    {   return 62;  
    }else if(c == '/')  
    {   return 63;  
    }else if(c == '=')  
    {   return 0;  
    }  
    return 0;  
}


int Base64_Decode_GB2312(char *lpString, char *lpSrc, int sLen)   //解码函数  
{   static char lpCode[4];  
    register int vLen = 0;  
    if(sLen % 4)        //Base64编码长度必定是4的倍数，包括'='  
    {   lpString[0] = '\0';  
        return -1;  
    }  
    while(sLen > 2)      //不足三个字符，忽略  
    {   lpCode[0] = GetCharIndex(lpSrc[0]);  
        lpCode[1] = GetCharIndex(lpSrc[1]);  
        lpCode[2] = GetCharIndex(lpSrc[2]);  
        lpCode[3] = GetCharIndex(lpSrc[3]);  
  
        *lpString++ = (lpCode[0] << 2) | (lpCode[1] >> 4);  
        *lpString++ = (lpCode[1] << 4) | (lpCode[2] >> 2);  
        *lpString++ = (lpCode[2] << 6) | (lpCode[3]);  
  
        lpSrc += 4;  
        sLen -= 4;  
        vLen += 3;  
    }  
    return vLen;  
}  

extern void get_sys_env(char *name, char* buf, int buf_size);

string getOwnDevSn(){
	char buf[256] = {0};

	if(strstr(Y_PRODUCT_NAME,"CIG")){
		unsigned char mac[6] = {0};
		getBaseIntfMac(mac,sizeof(mac));
		if(strstr(Y_PRODUCT_NAME,"122")){
			snprintf(buf,sizeof(buf)-1,"%02x%02x%02x%02x%02x%02x",
				mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		}else if(strstr(Y_PRODUCT_NAME,"180")){
			snprintf(buf,sizeof(buf)-1,"%02x%02x%02x%02x%02x%02x",
				mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		}
		return buf;
	}else{
		get_sys_env("yaya_device_sn",buf,sizeof(buf)-1);
		string sn = termString(buf);
		if(!sn.empty()){
			return sn;
		}else{
			unsigned char mac[6] = {0};
			getBaseIntfMac(mac,sizeof(mac));
			snprintf(buf,sizeof(buf)-1,"%02x%02x%02x%02x%02x%02x",
				mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			return buf;
		}
	}
}

int h2d(char hex){
	if (hex >= '0'&&hex<='9') return hex-'0';
	if (hex >= 'a'&&hex<='f') return hex-'a'+10;
	if (hex >= 'A'&&hex<='F') return hex-'A'+10;
	return 0;
}

void s2u(char *s, char *u ){
	while(*s != '\0'){
		if (*s=='\\'&&(*(s+1))=='x'){
			*u=(h2d(*(s+2)))*16+(h2d(*(s+3)));
			s+=4;
		}
		else {
			*u=*s;
			s++;
		}
		u++;
	}
	*u=*s;
}

extern "C" {

	void debugDoutSwitch(boolean flag){
		g_dout_flag = flag;
		if(g_dout_flag){
			snprintf(g_dev_sn,sizeof(g_dev_sn)-1,"%s",s2c(getOwnDevSn()));
			openlog("yaya", LOG_CONS | LOG_PID,  LOG_USER);
		}else{
			closelog();
		}
	}

	void debugDout(char* pname, const char* file, const char* func, int line, char* fmt, ...){
		if(g_dout_flag){
			static char buf[DEBUG_CONTENT_LEN] = {0};

			struct timeval tmr;
			struct timezone tz;
			memset(&tmr,0,sizeof(tmr));
			gettimeofday (&tmr ,&tz);
			
			if(pname){
				snprintf(buf,sizeof(buf)-1,"info(%s %d-%d %s %s %d):",
					pname,tmr.tv_sec,tmr.tv_usec,file,func,line);
			}else{
	//			snprintf(buf,sizeof(buf)-1,"info(%d %s %s %d):",
	//				getpid(),file,func,line);
				snprintf(buf,sizeof(buf)-1,"info(%s %d-%d %s %s %d):",
					g_dev_sn,tmr.tv_sec,tmr.tv_usec,file,func,line);

			}
			if((1+strlen(buf)) < DEBUG_CONTENT_LEN){
				va_list argp;
				va_start( argp, fmt );
				vsnprintf((buf+strlen(buf)),(DEBUG_CONTENT_LEN-1-strlen(buf)),fmt,argp);
				va_end( argp ); 
			}
			if(g_out_term.empty()){
				printf("%s\n",buf);
				#ifndef YAYA_TARGET_TYPE_host
			//		syslog(LOG_INFO,"%s\n",buf + strlen("info"));
				#endif
			}else{
				printf("%s\n",buf);
				if(g_out_fp == NULL){
					g_out_fp = fopen(g_out_term.c_str(),"w");
				}
				if(g_out_fp != NULL){
					fprintf(g_out_fp,"%s\n",buf);
					fflush(g_out_fp);
				}
				#ifndef YAYA_TARGET_TYPE_host
			//		syslog(LOG_INFO,"%s\n",buf + strlen("info"));
				#endif
			}
		}
	}

	void dumpOut(char* fmt, ...){
			static char buf[DEBUG_CONTENT_LEN] = {0};
			va_list argp;
			va_start( argp, fmt );
			vsnprintf(buf,(DEBUG_CONTENT_LEN-1),fmt,argp);
			va_end( argp ); 
			if(g_out_term.empty()){
				printf("%s",buf);
			}else{
				if(g_out_fp == NULL){
					g_out_fp = fopen(g_out_term.c_str(),"w");
				}
				if(g_out_fp != NULL){
					fprintf(g_out_fp,"%s",buf);
					fflush(g_out_fp);
				}
			}
	}


	void debugEout(char* pname, const char* file, const char* func, int line, char* fmt, ...){
		static char buf[DEBUG_CONTENT_LEN] = {0};
		struct timeval tmr;
		struct timezone tz;
		memset(&tmr,0,sizeof(tmr));
		gettimeofday (&tmr ,&tz);

		if(pname){
			snprintf(buf,sizeof(buf)-1,"info(%s %d-%d %s %s %d):",
				pname,tmr.tv_sec,tmr.tv_usec,file,func,line);
		}else{
//			snprintf(buf,sizeof(buf)-1,"error(%d %s %s %d):",
//				getpid(),file,func,line);
			snprintf(buf,sizeof(buf)-1,"info(%s %d-%d %s %s %d):",
				g_dev_sn,tmr.tv_sec,tmr.tv_usec,file,func,line);
		}
		if((1+strlen(buf)) < DEBUG_CONTENT_LEN){
			va_list argp;
			va_start( argp, fmt );
			vsnprintf((buf+strlen(buf)),(DEBUG_CONTENT_LEN-1-strlen(buf)),fmt,argp);
			va_end( argp ); 
		}
		if(g_out_term.empty()){
			printf("%s\n",buf);
			#ifndef YAYA_TARGET_TYPE_host
				syslog(LOG_ERR,"%s\n",buf + strlen("error"));
			#endif
		}else{
			if(g_out_fp == NULL){
				g_out_fp = fopen(g_out_term.c_str(),"w");
			}
			if(g_out_fp != NULL){
				fprintf(g_out_fp,"%s\n",buf);
				fflush(g_out_fp);
			}
			#ifndef YAYA_TARGET_TYPE_host
				syslog(LOG_ERR,"%s\n",buf + strlen("error"));
			#endif
		}
	}

	void debugChangeTerm(char* term){
		if(term){
			if(g_out_term != term){
				if(g_out_fp != NULL){
					fclose(g_out_fp);
					g_out_fp = NULL;
				}
				g_out_term = term;
			}
		}
	}

}


/** 分割字符串*/
void splitString(string src, string flag, vector<string>& result){

	unsigned int b = 0;
	result.clear();
	for(unsigned int i = 0; i <= src.size(); i++){
		char tmp[2] = {0};
		tmp[0] = src[i];
		if(flag.find(tmp) != string::npos){
			result.push_back(src.substr(b,i-b));
			b = i + 1;
		}
	}
	if(b <= src.size()){
		result.push_back(src.substr(b, src.size()-b));
	}

}
int getIntfMac(char* intf, unsigned char *buf, int size){
	int inet_sock;
	char *value= NULL;
	struct ifreq ifr;
	memset(&ifr.ifr_hwaddr, 0, sizeof(struct sockaddr));
	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

	if(!buf) {
		goto failed;
	}
	memset(buf, 0, size);
	
	strcpy(ifr.ifr_name, intf);
	
	if (ioctl(inet_sock, SIOCGIFHWADDR, &ifr)< 0) {
		perror("ioctl");
		goto failed;
	}
	if(size < ETH_ALEN) {
		goto failed;
	}
	value = &ifr.ifr_ifru.ifru_hwaddr.sa_data[0];
	memcpy(buf, value, ETH_ALEN);
	close(inet_sock);
	return 0;
failed:
	close(inet_sock);
	return -1;
}

int getBaseIntfMac(unsigned char *buf, int size){
	//9C:03:9E:01:62:C8
//	buf[0] = 0x9c;
//	buf[1] = 0x03;
//	buf[2] = 0x9e;
//	buf[3] = 0x00;
//	buf[4] = 0x09;
//	buf[5] = 0xa8;
//	return 0;
	return getIntfMac("eth0",buf,size);
}


string getBaseMac(){
	unsigned char buf[6] = {0};
	getBaseIntfMac(buf,sizeof(buf));
	char out[32] = {0};
	snprintf(out,sizeof(out)-1,"%02x:%02x:%02x:%02x:%02x:%02x",
		buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
	return out;
}


/**  字符串替换*/
string replaceInString(string src, string org, string dest){
	int e = 0;
	int b = 0;
	string out;
	e = src.find(org,b);
	while(e != string::npos){
		out = out + src.substr(b,e-b);
		out += dest;
		b = e + org.length();
		e = src.find(org,b);
	}
	out += src.substr(b,e-b);
	return out;
}

string termString(string src,string arg){
	//工具。去除字符串两端的空格或其它指定字符
	if(src.empty())
		return src;
	int b = src.find_first_not_of(arg);
	if(b == string::npos)
		return "";
	int e = src.find_last_not_of(arg + "\n\0");
	return src.substr(b,e-b+1);
}

extern boolean checkHDD(){
	FILE* fp = popen("df","r");
	if(fp){
		char line[256] = {0};
		while(fgets(line,sizeof(line)-1,fp)){
			if(strstr(line,"/mnt/usb")){
				pclose(fp);
				return true;
			}
		}
		pclose(fp);
	}
	return false;
}

/** 字符转换*/
string c2s(char* v){
	if(v)
		return v;
	else
		return "";
}

string c2s(const char* v){
	if(v)
		return v;
	else
		return "";
}

char* s2c(string v){
	return (char*)v.c_str();
}

status ipToNet(string ip, string mask, string &out){
	//check mask if ok.
	vector<string> mask_list;
	int bits = 0;
	unsigned char _comp[9] = {0x00,0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF};
	splitString(mask,".",mask_list);
	if(mask_list.size() != 4){
		ASSERT_STATUS(s_err_param);
	}
	
	//不判断mask中的内容是否合法了。
	for(unsigned int i = 0; i < mask_list.size(); i++){
		unsigned int tmp_num = atoi(s2c(mask_list[i]));
		unsigned int j = 0;
		for(j = 0; j < sizeof(_comp)/sizeof(_comp[0]); j++){
			if(tmp_num == _comp[j]){
				bits += j;
				break;
			}
		}
	}


	struct sockaddr_in _ip;
	struct sockaddr_in _mask;
	_ip.sin_addr.s_addr = inet_addr(s2c(ip));
	_mask.sin_addr.s_addr = inet_addr(s2c(mask));
	_ip.sin_addr.s_addr = (_ip.sin_addr.s_addr & _mask.sin_addr.s_addr);

	
	char bufout[64] = {0};
	snprintf(bufout,sizeof(bufout)-1,"%s/%u",inet_ntoa(_ip.sin_addr),bits);
	out = bufout;
	return s_ok;
}

/** 比较2个表，将减少的和增加的找出来*/
void compareList(vector<string>& src, vector<string>& dst, vector<string>& same, vector<string>& less, vector<string>& more){
	unsigned int i = 0;
	unsigned int j = 0;

	for(i = 0; i < src.size(); i++){
		for(j = 0; j < dst.size(); j++){
			if(src[i] == dst[j]){
				same.push_back(src[i]);
				break;
			}
		}
		if(j == dst.size()){
			less.push_back(src[i]);
		}
	}

	for(i = 0; i < dst.size(); i++){
		for(j =0; j < src.size(); j++){
			if(dst[i] == src[j]){
				break;
			}
		}
		if(j == src.size()){
			more.push_back(dst[i]);
		}
	}
}

long getSysTicks() {
	struct sysinfo sys_info;
	sysinfo(&sys_info);
	return sys_info.uptime;
}

long getSysTime(){
	time_t tmr;
	return (long)time(&tmr);
}


string generateSession() {
    string session;
    const char ch[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01234567899";
    srand (time(NULL));
    for(int i=0;i<32;i++) {
        int j = rand() % 62;
        session.push_back(ch[j]);
    }
    return session;
}

string calMd5(unsigned char *src, unsigned int len){
	char md5[32] = {0};
	unsigned char md[16] = {0};
	MD5_CTX c;
	MD5_Init(&c);
	MD5_Update(&c,src,(unsigned long)len);
	MD5_Final(&(md[0]),&c);
	for (int i = 0; i < 16; i++){
		sprintf((md5 + i*2),"%02x",md[i]);
	}
	return md5;
}

///////////////////////////////////////////////////////////////////////////////


UciWrapper::UciWrapper(string filename){
	mFileName = filename;
}	

status UciWrapper::loadSection(string section_class, string section_name){
	mSection = section_class;
	mSectionName = section_name;
	mForce = true;
	return loadFromFile();
}
	

status UciWrapper::loadSection(string section_class, string token_key, string token_val){
	mSection = section_class;
	mToken = token_key;
	mTokenKey = token_val;
	mForce = true;
	return loadFromFile();
}


UciWrapper::~UciWrapper(){
	clearListAll();
}

status UciWrapper::setOption(string opt, string val){
	map<string,string>::iterator it;
	for(it = mOptionList.begin(); it != mOptionList.end(); it++){
		map<string,string>::value_type pair = *it;
		string f = pair.first;
		string s = pair.second;
		if(f == opt){
			if(val != s){
				mOptionList[f] = val;
				mChanged = true;
			}
			return s_ok;
		}
	}
	mOptionList[opt] = val;
	mChanged = true;
	return s_ok;
}

status UciWrapper::setOption(string opt, int val){
	char tmp[64] = {0};
	snprintf(tmp,sizeof(tmp)-1,"%d",val);
	return setOption(opt,tmp);
}

status UciWrapper::setList(string lst, string val){
	map<string,ListContent*>::iterator it;
	for(it = mMutiList.begin(); it != mMutiList.end(); it++){
		map<string,ListContent*>::value_type pair = *it;
		string f = pair.first;
		ListContent* s = pair.second;
		if(f == lst && s){
			boolean find = false;
			for(unsigned int i = 0; i < s->mList.size(); i++){
				if(val == s->mList[i]){
					find = true;
					break;
				}
			}
			if(!find){
				mChanged = true;
				s->mList.push_back(val);
			}
			return s_ok;
		}
	}
	ListContent* tmp = new ListContent();
	if(tmp){
		tmp->mList.push_back(val);
		mMutiList[lst] = tmp;
		mChanged = true;
	}
	return s_ok;
}

status UciWrapper::rmList(string lst,string val){
	map<string,ListContent*>::iterator it;
	for(it = mMutiList.begin(); it != mMutiList.end(); it++){
		map<string,ListContent*>::value_type pair = *it;
		string f = pair.first;
		ListContent* s = pair.second;
		if(f == lst && s){
			vector<string>::iterator itv;
			for(itv == s->mList.begin(); itv != s->mList.end(); itv++){
				string tmp = *itv;
				if(tmp == val){
					s->mList.erase(itv);
					mChanged = true;
					return s_ok;
				}
			}
			return s_ok;
		}
	}
	return s_ok;
}

status UciWrapper::clearList(string lst){
	map<string,ListContent*>::iterator it;
	for(it = mMutiList.begin(); it != mMutiList.end(); it++){
		map<string,ListContent*>::value_type pair = *it;
		string f = pair.first;
		ListContent* s = pair.second;
		if(f == lst){
			if(s){
				delete s;
			}
			mMutiList.erase(lst);
			mChanged = true;
			return s_ok;
		}
	}
	return s_ok;
}
status UciWrapper::clearListAll(){
	map<string,ListContent*>::iterator it;
	for(it = mMutiList.begin(); it != mMutiList.end(); it++){
		map<string,ListContent*>::value_type pair = *it;
		string f = pair.first;
		ListContent* s = pair.second;
		if(s){
			delete s;
		}
	}
	mMutiList.clear();
	mChanged = true;
	return s_ok;
}

status UciWrapper::getList(string lst, vector<string>& result){
	map<string,ListContent*>::iterator it;
	for(it = mMutiList.begin(); it != mMutiList.end(); it++){
		map<string,ListContent*>::value_type pair = *it;
		string f = pair.first;
		ListContent* s = pair.second;
		if(f == lst){
			for(unsigned int i = 0; i < s->mList.size(); i++){
				result.push_back(s->mList[i]);
			}
			return s_ok;
		}
	}
	return s_ok;
}

string UciWrapper::getOption(string opt){
	map<string,string>::iterator it;
	for(it = mOptionList.begin(); it != mOptionList.end(); it++){
		map<string,string>::value_type pair = *it;
		string f = pair.first;
		string s = pair.second;
		if(f == opt){
			return s;
		}
	}
	return "";
}

status UciWrapper::loadFromFile(){
#ifdef TARGET_TYPE_target
	int file_exist = 0;
	struct stat c_st;
	memset(&c_st,0,sizeof(c_st));
	if(stat(s2c(string("/etc/config/") + mFileName),&c_st) != 0){
		FILE* fp = fopen(s2c(string("/etc/config/") + mFileName),"w+");
		if(fp){
			fprintf(fp,"# this file created by uci wrapper!\n");
			fclose(fp);
			file_exist = 1;
		}
	}else{
		file_exist = 1;
	}
	if(!file_exist){
		return s_err_syscall;
	}


	//call uci_load
	struct uci_context * ctx = NULL; //定义一个UCI上下文的静态变量.  
	struct uci_package * pkg = NULL;  
 	struct uci_element *e = NULL;  
	struct uci_option *o = NULL;
	struct uci_section *s = NULL;
	int error_id = UCI_OK;
	
 	ctx = uci_alloc_context(); // 申请一个UCI上下文.  
	error_id = uci_load(ctx, s2c(mFileName), &pkg);
   	 if (UCI_OK != error_id) {
		uci_free_context(ctx);	
		ctx = NULL;  
		ASSERT_STATUS(s_err_param);
    	}
 
  
   	 /*遍历UCI的每一个节*/  
	boolean bFind = false;
	uci_foreach_element(&pkg->sections, e)	
	{  
		s = uci_to_section(e);	
		if(s && (mSection == s->type)){
			//只要是section符合,即认为找到!
			if(mSectionName.empty() && mToken.empty() && mTokenKey.empty()){
				bFind = true;
				break;
			}
				
			if(!mSectionName.empty()){
				//section name符合,则认为找到!
				DOUT("---mSectionMame=%s,s->e.name=%s",s2c(mSectionName),s->e.name);
				if(mSectionName == s->e.name){
					bFind = true;
					break;
				}
			}else if((!mToken.empty()) && (!mTokenKey.empty())){
				//token符合,则认为找到.
				const char* val = uci_lookup_option_string(ctx, s, s2c(mToken));
				if(val && (mTokenKey == val)){
					//找到了
					//TO DO.
					bFind = true;
					break;
				}
			}
		}
	}	

	if(bFind){
		uci_foreach_element(&s->options, e){
        		o = uci_to_option(e);  
			string n = e->name;
			if(o){
				if( o->type == UCI_TYPE_STRING){
					//options.
					setOption(n,o->v.string);
				}else if( o->type == UCI_TYPE_LIST){
					//list.
					struct uci_element *l = NULL;	
					 uci_foreach_element(&o->v.list,l)  
					 {	
						 //这里会循环遍历 list	
						setList(n,l->name);
					 }	
				}
			}
		}
	}
    uci_unload(ctx, pkg); // 释放 pkg   
    uci_free_context(ctx);  
    ctx = NULL;  
#endif
	return s_ok;
}

status UciWrapper::commit(){
#ifdef TARGET_TYPE_target
	int file_exist = 0;
	struct stat c_st;
	memset(&c_st,0,sizeof(c_st));
	if(stat(s2c(string("/etc/config/") + mFileName),&c_st) != 0){
		FILE* fp = fopen(s2c(string("/etc/config/") + mFileName),"w+");
		if(fp){
			fprintf(fp,"# this file created by uci wrapper!\n");
			fclose(fp);
			file_exist = 1;
		}
	}else{
		file_exist = 1;
	}
	if(!file_exist){
		return s_err_syscall;
	}

	struct uci_context * ctx = NULL; //定义一个UCI上下文的静态变量.  
	struct uci_package * pkg = NULL;  
 	struct uci_element *e = NULL;  
	struct uci_option *o = NULL;
	struct uci_section *s = NULL;

	//创建上下文
 	ctx = uci_alloc_context(); // 申请一个UCI上下文.  
    	if (UCI_OK != uci_load(ctx, s2c(mFileName), &pkg)) {
		DOUT("%s not exist!",s2c(mFileName));
    	}
   
    	/*遍历UCI的每一个SECTION */  
	boolean bFind = false;
	if(pkg){
		uci_foreach_element(&pkg->sections, e)	
		{  
			s = uci_to_section(e);	
			if(s && (mSection == s->type)){
				//只要是section符合,即认为找到!
				if(mSectionName.empty() && mToken.empty() && mTokenKey.empty()){
					bFind = true;
					break;
				}
					
				if(!mSectionName.empty()){
					//section name符合,则认为找到!
					if(mSectionName == s->e.name){
						bFind = true;
						break;
					}
				}else if((!mToken.empty()) && (!mTokenKey.empty())){
					//token符合,则认为找到.
					const char* val = uci_lookup_option_string(ctx, s, s2c(mToken));
					if(val && (mTokenKey == val)){
						//找到了
						//TO DO.
						bFind = true;
						break;
					}
				}
			}
		}		
	}

	//add or new sections.
	struct uci_ptr ptr;
	memset(&ptr,0,sizeof(ptr));
	ptr.p = pkg;
	if(bFind){
		ptr.s = s;
	}else{
		//extern int uci_add_section(struct uci_context *ctx, struct uci_package *p, const char *type, struct uci_section **res);
		uci_add_section(ctx,ptr.p,s2c(mSection),&ptr.s);
	}
	if(!mSectionName.empty())
		ptr.section = s2c(mSectionName);
	if(!mSection.empty())
		ptr.value = s2c(mSection);
	uci_set(ctx,&ptr);	

	//delete old options.
	{
		map<string,struct uci_option*> tmp_list;	
		uci_foreach_element(&ptr.s->options, e){
			struct uci_option *o = uci_to_option(e);  
			string n = e->name;
			tmp_list[n] = o;
		}
		map<string,struct uci_option*> ::iterator ito;	
		for(ito = tmp_list.begin(); ito != tmp_list.end(); ito++){
			map<string,struct uci_option*>::value_type pair = *ito;
			string n = pair.first;
			struct uci_option *o = pair.second;	
			struct uci_ptr tmp_ptr;
			memset(&tmp_ptr,0,sizeof(tmp_ptr));
			tmp_ptr.p = pkg;
			tmp_ptr.s = ptr.s;
			tmp_ptr.o = o;
			tmp_ptr.option = s2c(n);
			tmp_ptr.value = "";
			uci_set(ctx,&tmp_ptr);
		}
	}

	//add options.
	map<string,string>::iterator it;
	for( it = mOptionList.begin(); it != mOptionList.end(); it++){
		map<string,string>::value_type pair = *it;
		string o = pair.first;
		string v = pair.second;
		ptr.o = NULL;
		if(o.empty())
			ptr.option = NULL;
		else
			ptr.option = s2c(o);
		if(v.empty())
			ptr.value = NULL;
		else 
			ptr.value = s2c(v);
		uci_set(ctx,&ptr); //写入配置	
	}

	//add list.
	map<string,ListContent*>::iterator itl;
	for(itl = mMutiList.begin(); itl != mMutiList.end(); itl++){
		map<string,ListContent*>::value_type pair = *itl;
		string f = pair.first;
		ListContent* l = pair.second;
		if(l){
			ptr.o = NULL;
			for(unsigned int i = 0; i < l->mList.size(); i++){
				string v = l->mList[i];
//				ptr.o = NULL;
				if(f.empty())
					ptr.option = NULL;
				else
					ptr.option = s2c(f);
				if(v.empty())
					ptr.value = NULL;
				else 
					ptr.value = s2c(v);
				uci_add_list(ctx,&ptr); //写入配置	
			}
		}
	}
	uci_commit(ctx, &ptr.p, false); //提交保存更改  
	uci_unload(ctx,ptr.p); //卸载包  
	uci_free_context(ctx); //释放上下文  
	char intent_buf[256] = {0};
	snprintf(intent_buf,sizeof(intent_buf),"intent "E_YOS_CFG_CHANGED" %s %s %s",
		s2c(mFileName),s2c(mSection),s2c(mSectionName));
	system(intent_buf);
#endif
	return s_ok;
}
boolean UciWrapper::checkChanged(){
	return mChanged;
}

void UciWrapper::copyFile(string src, string dst){
	char buf[512] = {0};
	snprintf(buf,sizeof(buf)-1,"cp -rf %s %s",s2c(src),s2c(dst));
	system(buf);
	int id = dst.find_last_of("/");
	if(id != string::npos)
		dst = dst.substr(id+1, dst.length());
	snprintf(buf,sizeof(buf)-1,"intent "E_YOS_CFG_CHANGED" %s",
		s2c(dst));
	system(buf);
}

boolean UciWrapper::same(UciWrapper* uci){
	if(NULL == uci)
		return false;

	if(mSection != uci->mSection)
		return false;
	if(mSectionName != uci->mSectionName)
		return false;

	{
	
		map<string,string>::iterator it;
		vector<string> ls;
		vector<string> ld;
		
		for(it = mOptionList.begin(); it != mOptionList.end(); it++){
			map<string,string>::value_type pair = *it;
			ls.push_back(pair.first);
		}

		for(it = uci->mOptionList.begin(); it != uci->mOptionList.end(); it++){
			map<string,string>::value_type pair = *it;
			ld.push_back(pair.first);
		}
		
		vector<string> less;
		vector<string> more;
		vector<string> same;
		compareList(ls, ld, same, less, more);
		if(less.size() || more.size())
			return false;

		for(it = mOptionList.begin(); it != mOptionList.end(); it++){
			map<string,string>::value_type pair = *it;
			string k = pair.first;
			string sc = pair.second;
			string dc = uci->mOptionList[k];
			if(sc != dc)
				return false;
		}
	}



	{
	
		map<string,ListContent*>::iterator it;
		vector<string> ls;
		vector<string> ld;
		
		for(it = mMutiList.begin(); it != mMutiList.end(); it++){
			map<string,ListContent*>::value_type pair = *it;
			ls.push_back(pair.first);
		}

		for(it = uci->mMutiList.begin(); it != uci->mMutiList.end(); it++){
			map<string,ListContent*>::value_type pair = *it;
			ld.push_back(pair.first);
		}
		
		vector<string> less;
		vector<string> more;
		vector<string> same;
		compareList(ls, ld, same, less, more);
		if(less.size() || more.size())
			return false;

		for(it = mMutiList.begin(); it != mMutiList.end(); it++){
			map<string,ListContent*>::value_type pair = *it;
			string k = pair.first;
			ListContent* sc = pair.second;
			ListContent* dc = uci->mMutiList[k];
			if(sc == dc)
				continue;
			if(NULL == sc || NULL == dc)
				return false;
		
			more.clear();
			same.clear();
			less.clear();
			compareList(sc->mList, dc->mList, same, less, more);
			if(less.size() || more.size())
				return false;
		}
	}
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////


string getMacFromIp(string ips){

	char cmd[256]= {0};
	string out;
	snprintf(cmd,sizeof(cmd)-1,"cat /proc/net/arp | grep '%s '",s2c(ips));
	FILE* fp = popen(cmd,"r");
	if(fp){
		char line[256] = {0};
		if(fgets(line,sizeof(line)-1,fp)){
			while(fgets(line,sizeof(line)-1,fp)){
				if(strstr(line,s2c(ips))){
					char ip[32] = {0};
					char hw[16] = {0};
					char flag[16] = {0};
					char mac[18] = {0};
					sscanf(line,"%s %s %s %s",ip,hw,flag,mac);
					out = mac;
					break;
				}
			}
		}
		pclose(fp);
	}

	return out;
}



boolean getIntfInfo(char* devname/*eth0,eth0.2,ppp0...*/,IntfInfoStru* info) {
	if(NULL == info || NULL == devname)
		return false;

	char cmd[256] = {0};
	snprintf(cmd,sizeof(cmd)-1,"ifconfig %s 2>&1",devname);
	FILE* fp = popen(cmd,"r");
	if(fp){
		char line[256] = {0};
		strncpy(info->name,devname,sizeof(info->name)-1);
		while(fgets(line,sizeof(line)-1,fp)){
			char *p = strstr(line,"\n");
			if(p) p[0]= '\0';
			p = strstr(line,"\r");
			if(p) p[0]= '\0';
//			p = strstr(line,"\t");
//			if(p) p[0]= '\0';
			if(NULL != (p = strstr(line,"error fetching interface"))){
				pclose(fp);
				return false;
			}
			if(NULL != (p = strstr(line,"HWaddr "))){
				sscanf(p + strlen("HWaddr "),"%s",info->mac);
			}
			if(NULL != (p = strstr(line,"inet addr:"))){
				sscanf(p + strlen("inet addr:"),"%s",info->ip);
			}
			if(NULL != (p = strstr(line,"Mask:"))){
				sscanf(p + strlen("Mask:"),"%s",info->mask);
			}
			if(NULL != (p = strstr(line,"RX packets:"))){
				sscanf(p + strlen("RX packets:"),"%s",info->rxPackages);
				if(NULL != (p = strstr(line,"errors:"))){
					sscanf(p + strlen("errors:"),"%s",info->rxErrors);
				}
				if(NULL != (p = strstr(line,"dropped:"))){
					sscanf(p + strlen("dropped:"),"%s",info->rxDropped);
				}
			}
			if(NULL != (p = strstr(line,"TX packets:"))){
				sscanf(p + strlen("TX packets:"),"%s",info->txPackages);
				if(NULL != (p = strstr(line,"errors:"))){
					sscanf(p + strlen("errors:"),"%s",info->txErrors);
				}
				if(NULL != (p = strstr(line,"dropped:"))){
					sscanf(p + strlen("dropped:"),"%s",info->txDropped);
				}
			}
			if(NULL != (p = strstr(line,"RX bytes:"))){
				sscanf(p + strlen("RX bytes:"),"%s",info->rxBytes);
			}
			if(NULL != (p = strstr(line,"TX bytes:"))){
				sscanf(p + strlen("TX bytes:"),"%s",info->txBytes);
			}
		}	
		pclose(fp);	
		return true;
	}
	return false;
} 



void getCurrentRouteIntf(string &route, string &gateway){
	FILE* fp = popen("route -n","r");
	if(fp){
		char line[256] = {0};
		if(fgets(line,sizeof(line)-1,fp)){
			while(fgets(line,sizeof(line)-1,fp)){
				char dest[32] = {0};
				char gw[32] = {0};
				char mask[32] = {0};
				char flags[32] = {0};
				char metric[32] = {0};
				char ref[32] = {0};
				char use[32] = {0};
				char intf[32] = {0};
				sscanf(line,"%s %s %s %s %s %s %s %s",
					dest,gw,mask,flags,metric,ref,use,intf);
				if(strcmp(mask,"0.0.0.0") == 0 && strstr(flags,"G")){
					route = intf;
					gateway = gw;
					break;
				}
			}
		}
		pclose(fp);
	}
}


