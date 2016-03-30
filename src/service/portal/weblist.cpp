#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <map>
#include <pthread.h>  
#include <stdarg.h>
#include <stdio.h>
#include <string>
using namespace std;
#include "tools.h"
#include "user_db.h"
#include "logtool.h"


#define STR_PORTAL 			"portal"
#define STR_DEFAULT			"default"
#define STR_ORDER			"order"
#define STR_PORTAL4G		"portal_4g"
#define REDIRECT_LOCK_FILE "/tmp/dns_redirect_lock_file"
#define USER_ROUTEFILE 		"/tmp/user_rootfile.txt"

extern "C" {


/*****************************************************************
DEBUG接口, 向syslog中输出重定向日志信息
******************************************************************/
static void (*debug_out)(int level, char* content) = 0;				
static int g_syslog_open_flag = 0;

static void __write_syslog(const char* fmt, ...){
	if(!g_syslog_open_flag){
		g_syslog_open_flag = 1;
		logtool_init(); //"dnsinfo"
	}
	char buf[1024] = {0};
	va_list argp;
	va_start( argp, fmt );
	vsnprintf((buf+strlen(buf)),(1023-strlen(buf)),fmt,argp);
	va_end( argp ); 
	logtool_write(buf);
}

#if 1
static void __debug_native(const char* fmt, ...){
	//if(debug_out){
	//	char buf[1024] = {0};
	//	va_list argp;
	//	va_start( argp, fmt );
	//	vsnprintf((buf+strlen(buf)),(1023-strlen(buf)),fmt,argp);
	//	va_end( argp ); 
		//debug_out(1,buf);
	//}
}
#else
static void __debug_native(const char* fmt, ...){
	char buf[1024] = {0};
	va_list argp;
	va_start( argp, fmt );
	vsnprintf((buf+strlen(buf)),(1023-strlen(buf)),fmt,argp);
	va_end( argp ); 
	char cache_cmd[1124] = {0};
	snprintf(cache_cmd,sizeof(cache_cmd)-1,"echo '%s' >> /tmp/dns_debug.txt",buf);
	system(cache_cmd);
	system("sync");
}

#endif

void wal_set_debug_func(void (*debug_func)(int level, char* content)){
//	debug_out = debug_func;
}


/*****************************************************************
查表操作
******************************************************************/

#define TN_ROOT 4 //根节点
#define TN_LEEF 1 //子节点
#define TN_OBJ	2 //Object节点

#define TL_WHITE 1
#define TL_BLACK 2

#define CFG_FILE_PATH(x) (string("/etc/config/") + x)

class Dot {
	private:
		//子节点
		map<char, Dot*> mSons;
		Dot* mParent;
		Dot* matchtree(string url, int i){
			Dot* node = FindSon(url[i]);
			//没有相应子节点
			if(NULL == node){
				return node;
			}
			//找到叶子节点,已经匹配,直接返回
			if(node->mType & TN_LEEF){
				return node;
			}
			//没有找到叶子节点,就已经结束匹配
			if(url.length() <= i){
				return NULL;
			}
			//继续匹配
			return node->matchtree(url,i+1);
		}

		void delSons(){
			map<char, Dot*>::iterator it;
			for(it = mSons.begin(); it != mSons.end(); it++){
				map<char, Dot*>::value_type pair = *it;
				Dot* dot = pair.second;
				if(dot)
					delete dot;
			}
			mSons.clear();
			mType = TN_LEEF;
		}
		
		Dot* FindSon(char c){
			map<char, Dot*>::iterator it = mSons.find(c);
			if(it == mSons.end())
				return NULL;
			map<char, Dot*>::value_type pair = *it;
			Dot* dot = pair.second;
			return dot; 		
		}
	

	public:
		//本节点的值
		char mVal; 
		//记录节点类型
		int mType;
		//记录名单类型
		int mTabType;
		//给黑名单使用
		string mRedirectAddr;
		Dot(char v, int type, int tab_type){
			mVal = v;
			mType = type;
			mTabType = tab_type;
			mParent = NULL;
		}

		~Dot(){
			delSons();
		}

		//添加一个子节点, 并返回子节点的Dot
		Dot* addSon(char v, int type){
			Dot* dot = NULL;
			map<char, Dot*>::iterator it = mSons.find(v);
			if(it == mSons.end()){
				dot = new Dot(v,type,mTabType);
				if(dot){
					mSons[v] = dot;
					dot->mParent = this;
				}
			}else{
				map<char, Dot*>::value_type pair = *it;
				dot = pair.second;
			}
			if(dot){
				dot->mType |= type;
				mType |= TN_OBJ;
			}
//			__debug_native("parent=(%x,v=%c,t=%d), son=(%x,v=%c,t=%d)\n",this,mVal,mType,dot,dot->mVal,dot->mType);
			return dot;
		}

		Dot* addTree(string url){
			string tree = termString(url);
			string ip;
			if(mTabType == TL_BLACK){ 
				int i = tree.find_first_of(" ");
				if(i == string::npos){
					IntfInfoStru ifinfo;
					memset(&ifinfo,0,sizeof(ifinfo));
					getIntfInfo("br-lan",&ifinfo);
					ip = ifinfo.ip;
				}else{
					ip = termString(tree.substr(0,i));
					if(ip == "self"){
						IntfInfoStru ifinfo;
						memset(&ifinfo,0,sizeof(ifinfo));
						getIntfInfo("br-lan",&ifinfo);
						ip = ifinfo.ip;
					}else if(ip == "gateway"){
						string gw;
						string gw_device;
						getCurrentRouteIntf(gw_device,gw);
						ip = gw;
					}
					tree = termString(tree.substr(i,tree.length()));
				}
			}
			Dot* son = NULL;
			unsigned int len = tree.length();
			for(unsigned int i = 0; i < len; i++){
				int type;
				if(i < (len - 1)){
					type = TN_OBJ;
				}else{
					type = TN_LEEF;
				}
				if(NULL == son){
					son = addSon(tree[i],type);
				}else{
					son = son->addSon(tree[i],type);
				}
			}
			if(son){
				son->mRedirectAddr = ip;
			}
			return son;
		}

		Dot* lookupTree(string url){
			unsigned int len = url.length();
			for(unsigned int i = 0; i < len; i++){
//				__debug_native("look from: %s\n",url.c_str() + i);
				Dot* dot = matchtree(url,i);
				if(dot){
//					__debug_native("matched!\n");
					return dot;
				}
			}
//			__debug_native("\n");
			return NULL;
		}


		void dump(string prev){
			char tp[32] = {0};
			sprintf(tp,"%c",mVal);
			prev += (char*)tp;
			if(mType & TN_LEEF){
//				__debug_native("%s\n",prev.c_str());
			}	
			map<char,Dot*>::iterator it;
			for(it = mSons.begin(); it != mSons.end(); it++){
				map<char,Dot*>::value_type pair = *it;
				Dot* dot = pair.second;
				if(dot)
					dot->dump(prev);
			}
		}

		void dumpPath(){
			if(mParent){
				mParent->dumpPath();
			}
//			__debug_native("%c",mVal);
		}

		void dumpRedirectAddr(){
//			__debug_native("\naddr=%s\n",mRedirectAddr.c_str());
		}

		
		
};

class UserNode{
	private:
		string mUsr;
		string mCfgFile;
		string mSection;
		map<string, Dot*> mListTabs;
		vector<string> mOrderTabs;
		time_t mCfgLastChangeTmr;
		
		void clearCfg(){
			map<string, Dot*>::iterator it;
			for(it = mListTabs.begin(); it != mListTabs.end(); it++){
				map<string,Dot*>::value_type pair = *it;
				Dot* dot = pair.second;
				if(dot)
					delete dot;
			}
			mListTabs.clear();
			mOrderTabs.clear();
		}

		void readCfgFile(){
			int fd = open(REDIRECT_LOCK_FILE,O_CREAT|O_WRONLY,0777);
			if(fd >= 0){
//				__debug_native("in readcfgfile:, section=%s\n",s2c(mSection));
				flock(fd, LOCK_EX); 
				UciWrapper wrap(STR_PORTAL);
				wrap.loadSection(mSection);
				flock(fd, LOCK_UN);
				close(fd);

				string default_policy = wrap.getOption(STR_DEFAULT);
				if(default_policy == "deny"){
					mRegWal = true;
				}

				string order =	termString(wrap.getOption(STR_ORDER));
				vector<string> order_list;
//				__debug_native("read order, order is %s",s2c(order));
				if(order.empty()){
					mOrderTabs.push_back("white");
					mOrderTabs.push_back("black");
				}else{
					splitString(order,",",order_list);
					for(unsigned int i = 0; i < order_list.size(); i++){
						mOrderTabs.push_back(termString(order_list[i]));
					}
				}
				for(unsigned int i = 0; i < mOrderTabs.size(); i++){
					vector<string> ord_list;
					string tab = mOrderTabs[i];
					wrap.getList(tab,ord_list);
//					__debug_native("feed %s table, size=%d\n",s2c(padTabName(tab)),ord_list.size());
					if(ord_list.size()){
						int tab_type = TL_WHITE;
						if(tab.find("black") != string::npos){
							tab_type = TL_BLACK;
						}
						Dot* root = new Dot(' ',TN_ROOT,tab_type);
						if(root){
							mListTabs[tab] = root;
							for(unsigned int j = 0; j < ord_list.size(); j++){
								string node_value = termString(ord_list[j]);
								if(!node_value.empty()){
									root->addTree(node_value);
								}
							}
							root->dump("");
						}
					}
				}
			}
		}
		
	public:
		boolean mRegWal;
		UserNode(string cfg_file, string sec){
			mCfgFile = cfg_file;
			mSection = sec;
			mCfgLastChangeTmr = 0;
			mRegWal = false;
		}
		
		~UserNode(){
			clearCfg();
		}
		//重新读取配置
		void freshCfg(){
			struct stat cl_stat;
			memset(&cl_stat,0,sizeof(cl_stat));	
			if(stat(s2c(CFG_FILE_PATH(mCfgFile)),&cl_stat) != 0)
				return;
			if(mCfgLastChangeTmr != cl_stat.st_mtime){
				mCfgLastChangeTmr = cl_stat.st_mtime;
				
//				__debug_native("in UserNode->freshCfg, mCfgFile=%s\n",s2c(mCfgFile));
				clearCfg();
				readCfgFile();
			}
		}
		//检查是否匹配到, 返回值1 匹配到, 0 没匹配到
		int match(string url, int& type, string& redirect_ip){
			for(unsigned int i = 0; i < mOrderTabs.size(); i++){
				Dot* root = mListTabs[mOrderTabs[i]];
				if(root){
					Dot* dot = root->lookupTree(url);
					if(dot){
						type = dot->mTabType;
						if(!dot->mRedirectAddr.empty()){
							struct sockaddr_in server_sockaddr_in;
							vector<string> server_ip_list;
							bzero(&server_sockaddr_in, sizeof(server_sockaddr_in));
							server_sockaddr_in.sin_family = AF_INET;  
							if(!inet_aton(s2c(dot->mRedirectAddr), &server_sockaddr_in.sin_addr)){
								struct hostent *ht = gethostbyname(s2c(dot->mRedirectAddr));
								if(ht){
									struct in_addr **addr_list;
									addr_list = (struct in_addr**)ht->h_addr_list;
									for(int i = 0; addr_list[i] != NULL; i++){
										redirect_ip = inet_ntoa(*addr_list[i]);
										__debug_native("redirect url=%s, ip=%s",s2c(dot->mRedirectAddr),s2c(redirect_ip));
										break;
									}
								}
							}else{
								redirect_ip = dot->mRedirectAddr;
							}
						}
//						__debug_native("matched, type=%d, redirectip=%s\n",type,s2c(redirect_ip));
						return 1;
					}
				}
			}
			return 0;
		}
};


class RedirectMgr {
	private:
		time_t mUsrClsFileLastMdf;
		map<string,string> mUserClsMap;
		map<string,UserNode*> mUserCfgTab;
		string mCfgFile;
		int mInLock;
		void lock(){
			while(mInLock){
//				__debug_native(" IN LOCK!");
				usleep(10000);
			}
			mInLock = 1;
		}

		void unlock(){
			mInLock = 0;
		}
		
	public:
		RedirectMgr(string cfg_file){
			mUsrClsFileLastMdf = 0;
			mInLock = 0;
			mCfgFile = cfg_file;
		}

		~RedirectMgr(){
			map<string,UserNode*>::iterator it;
			for(it = mUserCfgTab.begin(); it != mUserCfgTab.end(); it++){
				map<string,UserNode*>::value_type pair = *it;
				UserNode* node = pair.second;
				if(node)
					delete node;
			}
			mUserCfgTab.clear();
		}

		//根据用户级别查找匹配到的配置section.
		int getMatchedTable(string usr_class, string url, int& type, string& redirect_ip){
			UserNode* node = NULL;
			lock();
			int out = 0;
			map<string,UserNode*>::iterator it = mUserCfgTab.find(usr_class);
			if(it == mUserCfgTab.end()){
				node = new UserNode(mCfgFile, usr_class);
				if(node){
					mUserCfgTab[usr_class] = node;
				}
			}else{
				map<string,UserNode*>::value_type pair = *it;
				node = pair.second;
			}
			if(node){
				node->freshCfg();
//				__debug_native("user_class=%s, url=%s\n",s2c(usr_class),s2c(url));
				out = node->match(url,type,redirect_ip);
			}
			unlock();
			return out;
		}

		boolean getRegFlag(string user_class){
			map<string,UserNode*>::iterator it = mUserCfgTab.find(user_class);
			if(it == mUserCfgTab.end())
				return false;
			UserNode *node = mUserCfgTab[user_class];
			if(node)
				return node->mRegWal;
			return false;
		}
};


RedirectMgr* g_portal_normal = NULL;
RedirectMgr* g_portal_4g = NULL;
RedirectMgr* g_portal = NULL;

time_t m_routefile_last_changed = 0;


//指定用哪个配置表
void lookupRoute(){
	int fd = open(REDIRECT_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0){
		flock(fd, LOCK_EX); 
		struct stat c_stat;
		if(stat(USER_ROUTEFILE,&c_stat) == 0){
			if(m_routefile_last_changed != c_stat.st_mtime){
				m_routefile_last_changed = c_stat.st_mtime;
				g_portal = g_portal_normal;
				char line[32] = {0};
				FILE* fp = fopen(USER_ROUTEFILE,"r");
				if(fp){
					while(fgets(line,sizeof(line)-1,fp)){
						if(strstr(line,"3g-ppp0")){
							g_portal = g_portal_4g;
							break;
						}
					}
					fclose(fp);
				}
			}
		}

		flock(fd, LOCK_UN);
		close(fd);
	}
}




/*****************************************************************
黑白名单查询接口, 返回值1表示dnsserver不用拦截报文, 0 表示需要拦截报文
******************************************************************/

/*
static void __test_timer(char* info){
	struct timeval tmr;
	struct timezone tz;
	memset(&tmr,0,sizeof(tmr));
	gettimeofday (&tmr ,&tz);
	char buf[256] = {0};
	snprintf(buf,sizeof(buf)-1,"echo '---TMRVAL: %d-%d %s' >> /tmp/dns_tmr.log",
		tmr.tv_sec,tmr.tv_usec,info);
	system(buf);
}
*/

int wal_dns_access_status(char *user_mac, char *dst_domain, char *redir_ip, int size){
	if((dst_domain == NULL) ||strstr(dst_domain,"in-addr.arpa")){
//		char dbgbuf[256] = {0};
//		snprintf(dbgbuf,sizeof(dbgbuf)-1," 1 mac=%s domain=%s",user_mac,dst_domain);
//		__test_timer(dbgbuf);
		return 0;
	}
	if((redir_ip == NULL) || (dst_domain == NULL) || (user_mac == NULL))
		return 1;
	if(size == 0)
		return 1;

//	__debug_native("\n\n\nin wal_dns_access_status, mac=%s, dst-url=%s\n",user_mac,dst_domain);

	//判断取哪个配置文件
	if((NULL == g_portal_normal) && (NULL == g_portal_4g)){
		usrdb_init();
//		__debug_native("in first call, g_portal=0x%x\n",g_portal);
		g_portal_normal = new RedirectMgr(STR_PORTAL);
		g_portal_4g = new RedirectMgr(STR_PORTAL4G);
		g_portal = g_portal_normal;
	}

//	__test_timer(" 2 begin wal_notify_domain_ip");

	lookupRoute();
//	__test_timer(" 3 begin wal_notify_domain_ip");

//	__debug_native("g_portal=0x%x,g_portal_normal=0x%x,g_portal_4g=0x%x\n",
//		g_portal,g_portal_normal,g_portal_4g);

	if(NULL == g_portal)
		return 0;

	//获取匹配结果
	string direct_ip;
	int type;
	string user_class;
	{
		char clstype[64] = {0};
		__debug_native("lkup0 %s",user_mac);
		if(usrdb_lkup(user_mac,clstype,63)){
			user_class = clstype;
			__debug_native("result0=%s",clstype);
		}else{
			user_class = "visitor";
		}
	}
	snprintf(redir_ip,size,"192.168.2.1");
//	__test_timer(" 4 begin wal_notify_domain_ip");
	
//	string user_class = g_portal->getUserClassFromMac(user_mac);
//	__debug_native("in getUserClassFromMac, mac=%s, get user class=%s\n",s2c(user_mac),s2c(user_class));
	
	int matched = g_portal->getMatchedTable(user_class,dst_domain,type,direct_ip);
	if(!direct_ip.empty())
		strncpy(redir_ip,s2c(direct_ip),size);
//	__test_timer(" 5 begin wal_notify_domain_ip");

	//输出到日志文件
	if(matched){
		if(type == TL_WHITE){
			//匹配到白名单
			__write_syslog("%u QUERY_URL mac=%s cls=%s redirect=no url=%s, match=white!",
				time(NULL),user_mac,s2c(user_class),dst_domain);
//			__debug_native("%u QUERY_URL mac=%s cls=%s redirect=no url=%s, match=white!",
//				time(NULL),user_mac,s2c(user_class),dst_domain);
			return 1;
		}else if(type == TL_BLACK){
			//匹配到黑名单
			__write_syslog("%u QUERY_URL mac=%s cls=%s redirect=%s url=%s, match=black!",
				time(NULL),user_mac,s2c(user_class),redir_ip,dst_domain);
//			__debug_native("%u QUERY_URL mac=%s cls=%s redirect=%s url=%s, match=black!",
//				time(NULL),user_mac,s2c(user_class),redir_ip,dst_domain);
			return 0;
		}
	}
//	__test_timer(" 6 begin wal_notify_domain_ip");
	
	//不在白名单和黑名单里,直接放行
	__write_syslog("%u QUERY_URL mac=%s cls=%s redirect=no url=%s, match=none!",
		time(NULL),user_mac,s2c(user_class),dst_domain);
//	__debug_native("%u QUERY_URL mac=%s cls=%s redirect=no url=%s, match=none!",
//		time(NULL),user_mac,s2c(user_class),dst_domain);

//	__test_timer(" 7 begin wal_notify_domain_ip");

	return 1;
}





#if 0
map<string, UciWrapper*> m_cfg_list_normal;
map<string, UciWrapper*> m_cfg_list_3g;
map<string, UciWrapper*> *m_cfg_list = NULL;


static int g_multex_lock_inited = 0;
pthread_mutex_t g_mutex ; 





#if 0
class WeblistTreeNode{
	private:
		void clearSons(){
			map<char,WeblistTreeNode*>::iterator it;
			for(it = mSonList.begin(); it != mSonList.end(); it++){
					map<char,WeblistTreeNode*>::value_type pair = *it;
					WeblistTreeNode* n = pair.second;
					if(n){
						delete n;
					}
			}
			mSonList.clear();
		}
	public:
		map<char,WeblistTreeNode*> mSonList;
		boolean mLeef;
		WeblistTreeNode(){
			mLeef = false;
		}
		~WeblistTreeNode(){
			clearSons();
		}
		
		void addBranch(char* branch){
			if(mLeef)
				return;
			char* p = branch;
			if(p && p[0] == '\0'){
				mLeef = true;
				clearSons();
				return;
			}
			
			if(p){
				WeblistTreeNode* n = mSonList[p[0]];
				if(n){
					n->addBranch(&p[1]);
				}else{
					n = new WeblistTreeNode();
					if(n){
						mSonList[p[0]] = n;
						n->addBranch(&p[1]);
					}
				}
			}
		}
		boolean matchBranch(char* src){
			char* p = src;
			if(!p)
				return false;
			if(mLeef)
				return true;
			WeblistTreeNode* n = mSonList[p[0]];
			if(!n){
				mSonList.erase(p[0]);
				return false;
			}else{
				return n->matchBranch(&p[1]);
			}
		}
};

class UserNode {
	public:
		map<string,WeblistTreeNode*> mTreeList;
		UserNode(string file, string type){
			UciWrapper wpr(type,STR_null);
			wpr.dump2mem(file);
			string order =	wpr.getOption(STR_ORDER);
			if(order.empty()){
				mOrderList.push_back(STR_white);
				mOrderList.push_back(STR_black);
			}else{
				splitString(order,",",mOrderList);
			}

			mTreeList[STR_web_white_list] = NULL;
			mTreeList[STR_web_black_list] = NULL;
			mTreeList[STR_web_prewhite_list] = NULL;
			mTreeList[STR_web_preblack_list] = NULL;
			map<string,WeblistTreeNode*>::iterator it;
			for(it = mTreeList.begin(); it != mTreeList.end(); it++){
				map<string,WeblistTreeNode*>::value_type pair = *it;
				vector<string> tree_list;
				wpr.getList(pair.first,tree_list);
				if(tree_list.size()){
					WeblistTreeNode* node = new WeblistTreeNode();
					if(node){
						for(unsigned int i = 0; i < tree_list.size(); i++)
							node->addBranch(s2c(tree_list[i]));
					}
					mTreeList[pair.first] = node;
				}
			}
		}
		~UserNode(){
			map<string,WeblistTreeNode*>::iterator it;
			for(it = mTreeList.begin(); it != mTreeList.end(); it++){
				map<string,WeblistTreeNode*>::value_type pair = *it;
				WeblistTreeNode* n = pair.second;
				if(n)
					delete n;
			}			
			mTreeList.clear();
		}
};

class WalCfgNode {
	public:
		map<string,WeblistTreeNode*> mPreWhiteTree;
		map<string,WeblistTreeNode*> mPreBlackTree;
		map<string,WeblistTreeNode*> mWhiteTree;
		map<string,WeblistTreeNode*> mBlackTree;
		map<string,vector<string>*> mOrderList;
		string mFile;
		WalCfgNode(string file){
			mFile = file;
		}

		status load(string type){

		}
};

#endif


/*
  获取路由信息，如果是3G,则使用3G的策略。
  如果不通过3G上网，则使用普通策略。
*/

static void __wal_check_3groute(int &flg_change, int &flg_3g){
	struct stat c_stat;
	if(stat(USER_ROUTEFILE,&c_stat) == 0){
		if(m_routefile_last_changed != c_stat.st_mtime){
			m_routefile_last_changed = c_stat.st_mtime;
			flg_change = 1;
			char line[32] = {0};
			FILE* fp = fopen(USER_ROUTEFILE,"r");
			if(fp){
				while(fgets(line,sizeof(line)-1,fp)){
					if(strstr(line,"3g-ppp0")){
						fclose(fp);
						flg_3g = 1;
						return;
					}
				}
				fclose(fp);
			}
		}
	}
	return;
}

static void __wal_get_type(string user_mac, string &type){
	FILE* fp = fopen(USER_CLASS_FILE,"r");
	if(fp){
		char line[256] = {0};
		while(fgets(line,sizeof(line)-1,fp)){
			char* p = strstr(line,"\n");
			if(p)
				p[0] = '\0';
			p =  strstr(line,"\r");
			if(p)
				p[0] = '\0';
	
			if(strlen(line) > 20){
				char mac[20] = {0};
				char cls[20] = {0};
				sscanf(line,"%s %s",mac,cls);
				if(!strcmp(mac,s2c(user_mac))){
					type = cls;
					break;
				}
			}
		}
		fclose(fp);
	}
}

static void __wal_get_cfg(string mac, int &flg_change, int &flg_3g, string &type){
	flg_change = 0;
	flg_3g = 0;

	int fd = open(REDIRECT_LOCK_FILE,O_CREAT|O_WRONLY,0777);
	if(fd >= 0)
	{
		flock(fd, LOCK_EX);	
		__wal_check_3groute(flg_change,flg_3g);
		__wal_get_type(mac,type);
//		__write_syslog("ZDJTEST MAC=%s, type=%s, flg_3g=%d, flg_change=%d",s2c(mac),s2c(type),flg_3g,flg_change);
		flock(fd, LOCK_UN);
		close(fd);
	}
}

static UciWrapper* wal_get_cfg(string mac,string &type){
	UciWrapper* wpr = NULL;
	int flg_change = 0;
	int flg_3g = 0;
	__wal_get_cfg(mac,flg_change,flg_3g,type);
	if(flg_change){
		if(flg_3g){
			m_cfg_list = &m_cfg_list_3g;
//			__write_syslog("ZDJTEST0, cfg form 3g");
		}else{
			m_cfg_list = &m_cfg_list_normal;
//			__write_syslog("ZDJTEST1 cfg form normal");
		}
	}else{
		if(NULL == m_cfg_list){
			m_cfg_list = &m_cfg_list_normal;
//			__write_syslog("ZDJTEST2, cfg form normal2");
		}
	}
	map<string,UciWrapper*>::iterator it;
	for(it = m_cfg_list->begin(); it != m_cfg_list->end(); it++){
		map<string,UciWrapper*>::value_type pair = *it;
		if(pair.first == type){
			wpr = pair.second;
			break;
		}
	}

	if(!wpr){
		wpr = new UciWrapper(type,STR_null);
		if(wpr){

			if(m_cfg_list == &m_cfg_list_normal){
				m_cfg_list_normal[type] = wpr;	
				wpr->dump2mem(STR_PORTAL);
//				__write_syslog("ZDJTEST3, dump form normal");
			}else{
				m_cfg_list_3g[type] = wpr;	
				wpr->dump2mem(STR_PORTAL4G);
//				__write_syslog("ZDJTEST4, dump form 3g");
			}
		}
	}
	return wpr;
}

int wal_dns_access_status(char *user_mac, char *dst_domain, char *redir_ip, int size){

	if((redir_ip == NULL) || (dst_domain == NULL) || (user_mac == NULL))
		return 1;
	if(size == 0)
		return 1;

	snprintf(redir_ip,size,"192.168.2.1");

	if(g_multex_lock_inited == 0){
		g_multex_lock_inited = 1;
		pthread_mutex_init(&g_mutex,NULL);	
	}

	pthread_mutex_lock(&g_mutex);  
		

	string type = STR_visitor;
	time_t tmr = time(NULL);
	UciWrapper* wpr = wal_get_cfg(user_mac,type);
	 if(wpr){
		string order =	wpr->getOption(STR_ORDER);
		vector<string> order_list;
//		__write_syslog("ZDJTEST5, order is %s",s2c(order));
		
		if(order.empty()){
			order_list.push_back(STR_white);
			order_list.push_back(STR_black);
		}else{
			splitString(order,",",order_list);
		}
		for(unsigned int i = 0; i < order_list.size(); i++){
			string tmp = order_list[i];
			if((tmp == STR_white)||(tmp == STR_prewhite)){
				vector<string> white_list;
				if(tmp == STR_white) {
					wpr->getList(STR_web_white_list,white_list);
				}else{
					wpr->getList(STR_web_prewhite_list,white_list);
				}

				for(unsigned int j = 0; j < white_list.size(); j++){
					string url = termString(white_list[j]);
					string domain = dst_domain;

					if(domain.find(url) != string::npos){
		//				__debug_native("huahua:dns_query:url mac=%s cls=%s nodirect url=%s",user_mac,s2c(type),dst_domain);
						__write_syslog("%u QUERY_URL mac=%s cls=%s redirect=no url=%s, match=%s",tmr,user_mac,s2c(type),dst_domain,s2c(url));
						pthread_mutex_unlock(&g_mutex);  
						return 1;
					}
				}

			}else if((tmp == STR_black) ||(tmp == STR_preblack)){
				vector<string> black_list;
				if(tmp == STR_black) {
					wpr->getList(STR_web_black_list,black_list);
				}else{
					wpr->getList(STR_web_preblack_list,black_list);
				}

				for(unsigned int j = 0; j < black_list.size(); j++){
					string tmp = termString(black_list[j]);
					unsigned int s = tmp.find_first_of(" ");
					string url,ip;
					if(s != string::npos){
						ip = termString(tmp.substr(0,s));
						url = termString(tmp.substr(s,tmp.length() - s));
					}else{
						ip = "192.168.2.1";
						url = tmp;
					}
			
					string domain = dst_domain;
					if(domain.find(url) != string::npos){
						strncpy(redir_ip,s2c(ip),size);
		//				__debug_native("huahua:dns_query:url mac=%s cls=%s redirect=%s url=%s",user_mac,s2c(type),redir_ip,dst_domain);
						__write_syslog("%u QUERY_URL mac=%s cls=%s redirect=%s url=%s, match=%s",tmr,user_mac,s2c(type),redir_ip,dst_domain,s2c(url));
						pthread_mutex_unlock(&g_mutex);  
						return 0;
					}
				}
			}
		}
	}
//	 __debug_native("huahua:dns_query:url mac=%s cls=%s nodirect url=%s",user_mac,s2c(type),dst_domain);
	 __write_syslog("%u QUERY_URL mac=%s cls=%s redirect=no url=%s",tmr,user_mac,s2c(type),dst_domain);
	pthread_mutex_unlock(&g_mutex);  
	return 1;
}

#endif

//////////////////////////////////////////////////////////////////////////////////

class TypeDnsIpReplyNode{
	public:
		map<string,long> mIpExpireList;
		TypeDnsIpReplyNode(){}
		~TypeDnsIpReplyNode(){}
};

map<string,TypeDnsIpReplyNode*> g_type_ip_reply_tab;
static boolean wal_clear_expires(string type, string ip, long ttl){
	long now = getSysTicks();
	map<string,TypeDnsIpReplyNode*>::iterator it = g_type_ip_reply_tab.find(type);
	map<string,long>::iterator itl;
	TypeDnsIpReplyNode* node = NULL;
	if(it == g_type_ip_reply_tab.end()){
		node = new TypeDnsIpReplyNode();
		g_type_ip_reply_tab[type] = node;
	}else{
		node = g_type_ip_reply_tab[type];
	}
	__debug_native("find node=%x",node);
	if(node){
		itl = node->mIpExpireList.find(ip);
		if(itl == node->mIpExpireList.end()){
			char cmd[256] = {0};
			snprintf(cmd,sizeof(cmd)-1,"/sbin/%s_dns.sh add %s &",s2c(type),s2c(ip));
			system(cmd);
			__debug_native("cmd=%s",cmd);
		}
		node->mIpExpireList[ip] = now + ttl + 60;
	}

	for(it = g_type_ip_reply_tab.begin(); it != g_type_ip_reply_tab.end(); it++){
		map<string,TypeDnsIpReplyNode*>::value_type pair = *it;
		node = pair.second;
		if(node){
			vector<string> cls_list;
			for(itl = node->mIpExpireList.begin(); itl != node->mIpExpireList.end(); itl++){
				map<string,long>::value_type pairl = *itl;
				string f = pairl.first;
				long s = pairl.second;
				if(now > s){
					char cmd[256] = {0};
					snprintf(cmd,sizeof(cmd)-1,"/sbin/%s_dns.sh del %s &",s2c(type),s2c(f));
					cls_list.push_back(f);
					system(cmd);
					__debug_native("cmd=%s",cmd);
				}
			}
			for(unsigned int i = 0; i < cls_list.size(); i++){
				node->mIpExpireList.erase(cls_list[i]);
			}
		}
	}
	return 0;
}

void wal_notify_domain_ip(char *mac, char *domain,  char *domain_ip, long ttl){
	vector<string> ip_dot;
	splitString(domain_ip,".",ip_dot);
	int ip_dot0 = atoi(s2c(ip_dot[0]));
	string ipnet;

//	struct stat cl_stat;

//	if(stat("/tmp/regwal_enable",&cl_stat) != 0)
//		return;

#if 1
	if(ip_dot0 < 127){
		ipnet = ip_dot[0] + ".0.0.0/8";
	}else if(ip_dot0 < 192){
		ipnet = ip_dot[0] + string(".") + ip_dot[1] + ".0.0/16";
	}else if(ip_dot0 < 224){
		ipnet = ip_dot[0] + string(".") + ip_dot[1] + string(".") + ip_dot[2] + ".0/24";
	}else{
		ipnet = domain_ip;
	}
#else
	if(ip_dot0 < 224){
		ipnet = ip_dot[0] + string(".") + ip_dot[1] + string(".") + ip_dot[2] + ".0/24";
	}else{
		ipnet = domain_ip;
	}
#endif

/*	char cmd[256] = {0};
	snprintf(cmd,sizeof(cmd)-1,YDHG_OUT_ROOT_PATH"/bin/ydhg/system_event "E_DNS_REPLY" %s=%d=%s &",mac,ttl,s2c(ipnet));
	system(cmd);
	__debug_native(cmd);
	__debug_native("replyip:mac=%s, lookup=%s, ip=%s, ipnet=%s, ttl=%d",mac,domain,domain_ip, s2c(ipnet), ttl);
*/	
	if(g_portal){
		char type[64] = {0};
		__debug_native("lkup %s",mac);
		if(usrdb_lkup(mac,type,63)){
			__debug_native("result=%s",type);
			if(strlen(type)){
				boolean default_policy = g_portal->getRegFlag(type);
				if(default_policy){
					__debug_native("replyip:mac=%s, ip=%s, ipnet=%s, ttl=%d",mac,domain_ip, s2c(ipnet), ttl);
					wal_clear_expires(type,ipnet,ttl);
				}
			}
		}
	}
	
}


}

