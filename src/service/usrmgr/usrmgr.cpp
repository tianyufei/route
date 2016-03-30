#include <stdio.h>
#include <stdlib.h>
#include <stack>
using namespace std;

#include "config.h"
#include "common.h"
#include "tools.h"
#include "icontext.h"
#include "srv_evt_id.h"
#include "user_db.h"
#include "ap_db.h"

/*
*统一管理用户状态和信息
*/

/*
用户管理要考虑的因数:
1.  有的cls用户可以上网, 有的不能上网
2.  能上网的用户,要判断是否超时
3.  超时的上网用户,要断开链接,并从用户表中踢除
4.  对于bridge模式的ap,同一个帐号,可以在不同设备类型上登录, 但不能在同类型的设备上同时登录
5.  对于router模式的ap, 所有终端可以同时登录. 但必须要一个终端打开app,打开通路.
6.  对于非法ap, 禁止mac地址.
*/



/**
* 用户状态的变化, 不只主设备关心, 有可能从设备也关心.
* 所有的模块启动时,要先从配置文件中获取配置,然后从共享内存中获取状态. 进程之间的通知只当作triger使用.
*/


/*
	/etc/config/usrmgr
	config usrcls visitor
		option expire_check '0'		 #是否进行超时检查
		option expire_cls	'visitor' #超时以后切换成什么cls
		option max_entry	'1'		 #最多允许多少个实例存在
		option repeat_entry	'1'		 #每个类型的终端,最多允许多少个实例

	config usrcls member
		option expire_check '0' 	 #是否进行超时检查
		option expire_cls	'visitor' #超时以后切换成什么cls
		option max_entry	'1' 	 #最多允许多少个实例存在
		option repeat_entry '1' 	 #每个类型的终端,最多允许多少个实例

	config usrcls vip
		option expire_check '1' 	 #是否进行超时检查
		option expire_cls	'member' #超时以后切换成什么cls
		option max_entry	'1' 	 #最多允许多少个实例存在
		option repeat_entry '1' 	 #每个类型的终端,最多允许多少个实例

*/

#define STR_CFG_NAME	"usermgr"
#define STR_VISITOR		"visitor"
#define STR_USRCLS		"usrcls"
#define STR_EXPIRE_CHECK	"expire_check"
#define STR_EXPIRE_CLS		"expire_cls"

/**
*该类描述每个终端(手机,PC,PAD等)的信息.
*每个终端上同时只能有一个帐号登录, 而一个帐号可能同时在不同的终端上等登录.
*
*判断一个帐号是否多实例, 以及是否能在同一类型设备上多实例,是由后台来判断, 这是因为同一帐号可能在不同网关下登录.
*甚至是通过3g/4g登录.
*后台判断后,决定保留哪些实例. 而对于网关来说,则不关心帐号. 只关心帐号与后台认证后的用户级别.
*/
class TerminalNode {
	public:
		string mMac;
		string mCls;
		string mTmpCls; //临时权限
		long mExpire;
		long mDhcpExpire;
		string mIpAddress;
		long mTmpExpire;
		TerminalNode(string mac){
			mMac = mac;
			mExpire = 0;
		}
		~TerminalNode(){}

};


extern void read_db_proc(char* mac, char* cls, long exire);

class UserManager : public IEventObserver, public ITimer {
	private:
		IContext *mContext;
		IEvent* mEvent;
		int mProcTmrFd;
		string mSelfIp;
		map<string,UciWrapper*> mUsrClsCfgList;
		map<string,TerminalNode*> mTerminalList;

		/**
		*检查终端是否经过了胖AP中转. 我们不允许用户在自己的房间里插入非法胖AP.
		*/
		boolean checkLegalAp(string ap_or_term_mac, string report_gw){
			if(mSelfIp != report_gw){
				//终端报告的网关地址和我们本身不一致,说明ap_or_term_mac是胖AP.
				if(!apdb_check(s2c(ap_or_term_mac))){
					//该ap不在我们的ap表中,说明是非法ap.
					mEvent->notify(E_SRV_URM_ILLEGAL_AP,ap_or_term_mac);
					return false;
				}
			}
			return true;
		}
		
		
		string getOption(string section, string optioname){
			map<string,UciWrapper*>::iterator it = mUsrClsCfgList.find(section);
			UciWrapper* cfg = NULL;
			if(it == mUsrClsCfgList.end()){
				cfg = new UciWrapper(STR_CFG_NAME);
				if(cfg){
					cfg->loadSection(STR_USRCLS,section);
					mUsrClsCfgList[section] = cfg;
				}
			}else{
				map<string,UciWrapper*>::value_type pair = *it;
				cfg = pair.second;
			}
			if(cfg){
				return cfg->getOption(optioname);
			}
			return "";
		}

		void getList(string section, string listname, vector<string>& rst){
			map<string,UciWrapper*>::iterator it = mUsrClsCfgList.find(section);
			UciWrapper* cfg = NULL;
			if(it == mUsrClsCfgList.end()){
				cfg = new UciWrapper(STR_CFG_NAME);
				if(cfg){
					cfg->loadSection(STR_USRCLS,section);
					mUsrClsCfgList[section] = cfg;
				}
			}else{
				map<string,UciWrapper*>::value_type pair = *it;
				cfg = pair.second;
			}
			if(cfg){
				cfg->getList(listname,rst);
			}
		}

		void getMacIpList(map<string,string>& mac_ip_list){
/* cat /proc/net/arp
IP address       HW type     Flags       HW address            Mask     Device
192.168.93.1     0x1         0x2         00:50:56:c0:00:08     *        eth0
192.168.93.2     0x1         0x2         00:50:56:e5:f7:5f     *        eth0
*/
			FILE* fp = mContext->popenCall("cat /proc/net/arp");
			if(fp){
				char line[256] = {0};
				if(fgets(line,sizeof(line)-1,fp)){
					while(fgets(line,sizeof(line)-1,fp)){
						char ip[32] = {0};
						char hw[16] = {0};
						char flag[16] = {0};
						char mac[18] = {0};
						sscanf(line,"%s %s %s %s",ip,hw,flag,mac);
						if(strcmp(flag,"0x2")){
							mac_ip_list[mac] = ip;
						}
					}
				}
				pclose(fp);
			}
		}

		void expireProc(){
			//周期性时钟,处理超时的情况.
			//step 1. 处理cls超时
			long now_tmr = getSysTime();
			long now_ticks = getSysTicks();
			map<string,TerminalNode*>::iterator it;
			for(it = mTerminalList.begin(); it != mTerminalList.end(); it++){
				map<string,TerminalNode*>::value_type pair = *it;
				TerminalNode* node = pair.second;
				if(node){
					reportExpireCls(node,now_ticks,now_tmr);
				}
			}
			//step 2. 处理dhcp超时.
			map<string,string> mac_ip_pair;
			vector<string> mac_ip_expire_list;
			getMacIpList(mac_ip_pair);
			for(it = mTerminalList.begin(); it != mTerminalList.end(); it++){
				map<string,TerminalNode*>::value_type pair = *it;
				TerminalNode* node = pair.second;
				if(node){
					if(!node->mIpAddress.empty()) {
						if(now_ticks > node->mDhcpExpire){
							mac_ip_expire_list.push_back(node->mMac);
							if(mEvent){
								mEvent->notify(E_SRV_URM_CLS_REMOVE,node->mMac,node->mCls);
							}
						}else {
							map<string,string>::iterator ita = mac_ip_pair.find(node->mMac);
							if(ita == mac_ip_pair.end()){
								//没找到.ip超时
								mac_ip_expire_list.push_back(node->mMac);
								if(mEvent){
									mEvent->notify(E_SRV_URM_CLS_REMOVE,node->mMac,node->mCls);
								}
							}
						}
					}
				}
			}
			for(unsigned int i = 0; i < mac_ip_expire_list.size(); i++){
				string mac = mac_ip_expire_list[i];
				TerminalNode* node = mTerminalList[mac];
				if(node)
					delete node;
				mTerminalList.erase(mac);
				usrdb_remove(s2c(mac));
			}
			
		}
		
		void clearClsCfg(){
			map<string,UciWrapper*>::iterator it;
			for(it = mUsrClsCfgList.begin(); it != mUsrClsCfgList.end(); it++){
				map<string,UciWrapper*>::value_type pair = *it;
				UciWrapper* node = pair.second;
				if(node)
					delete node;
			}
			mUsrClsCfgList.clear();
		}

		void clearTermList(){
			map<string,TerminalNode*>::iterator it;
			for(it = mTerminalList.begin(); it != mTerminalList.end(); it++){
				map<string,TerminalNode*>::value_type pair = *it;
				TerminalNode* node = pair.second;
				if(node)
					delete node;
			}
			mTerminalList.clear();
		}

		string  getRealCls(TerminalNode* node){
			if(!node->mTmpCls.empty())
				return node->mTmpCls;
			else
				return node->mCls;
		}

		void reportExpireCls(TerminalNode* node,long now_ticks,long now_tmr){
			string old_real_cls = getRealCls(node);
			if(node->mTmpExpire < now_ticks){
				node->mTmpCls.erase();
				node->mTmpExpire = 0;
			}
			string expire_check = getOption(node->mCls,STR_EXPIRE_CHECK);
			if(atoi(s2c(expire_check))){
				if(now_tmr > node->mExpire){
					node->mCls = getOption(node->mCls,STR_EXPIRE_CLS);
				}
			}
			string new_real_cls = getRealCls(node);
			if(old_real_cls != new_real_cls){
				mEvent->notify(E_SRV_URM_CLS_CHANGE,node->mMac,new_real_cls,old_real_cls);
				usrdb_insert(s2c(node->mMac),s2c(new_real_cls),node->mExpire);
			}

		}

		//内部调用,用于从usr_db恢复user_list.
		TerminalNode* userCreate(string mac){
			TerminalNode* node = NULL;
			map<string,TerminalNode*>::iterator it = mTerminalList.find(mac);
			if(it != mTerminalList.end()){
				map<string,TerminalNode*>::value_type pair = *it;
				node = pair.second;
			}else{
				node = new TerminalNode(mac);
				if(node){
					mTerminalList[mac] = node;
				}
			}
			return node;
		}

			
	public:
		UserManager(IContext* ctx):IEventObserver(){
			mContext = ctx;
			mEvent = NULL;
			if(ctx){
				mEvent = ctx->eventFactory();
				if(mEvent){
					mEvent->registObserver(E_SRV_ALL_START,this);
					mEvent->registObserver(E_SRV_ALL_STOP,this);
					mEvent->registObserver(E_YOS_CFG_CHANGED,this);
					mEvent->registObserver(E_SRV_TERM_INFO,this);
					mEvent->registObserver(E_SRV_TERM_TEMPINFO,this);
					mEvent->registObserver(E_SRV_TERM_IPALLOC,this);
				}
				mProcTmrFd = ctx->startTimer(60,0,true,this);
				apdb_init();
				usrdb_init();
				usrdb_readAll(read_db_proc);
			}else{
				EOUT("mContext is NULL, mEvent is NULL");
			}
			IntfInfoStru lan_info;
			memset(&lan_info,0,sizeof(lan_info));
			getIntfInfo("br-lan",&lan_info);
			mSelfIp = lan_info.ip;
		}
		~UserManager(){
			clearClsCfg();
			clearTermList();
		}



		//从user_db恢复.
		TerminalNode* userListAdd(string mac, string cls, long expire){
			TerminalNode* node = userCreate(mac);
			if(node){
				string old_cls =getRealCls(node);
				node->mCls = cls;
				node->mExpire = expire;
				string new_cls = getRealCls(node);
				if(old_cls != new_cls){
					mEvent->notify(E_SRV_URM_CLS_CHANGE,mac,new_cls,old_cls);
				}
			}
			return node;
		}		
	
		//向user_db和user_list中同时填写内容.
		TerminalNode* userAdd(string mac, string cls, long expire){
			if(cls.empty())
				cls = STR_VISITOR;
			TerminalNode* node = userCreate(mac);
			if(node){
				string old_cls =getRealCls(node);
				node->mCls = cls;
				string new_cls = getRealCls(node);
				if((node->mExpire != expire) || (old_cls != new_cls)){
					node->mExpire = expire;
					usrdb_insert(s2c(mac),s2c(new_cls),expire);
				}
				if(old_cls != new_cls){
					mEvent->notify(E_SRV_URM_CLS_CHANGE,mac,new_cls,old_cls);
				}
			}
			return node;
		}

		
		TerminalNode* userTmpAdd(string mac, string cls){
			if(cls.empty())
				cls = STR_VISITOR;
			TerminalNode* node = userCreate(mac);
			if(node){
				string old_cls =getRealCls(node);
				node->mTmpCls = cls;
				node->mTmpExpire = 300 + getSysTicks();
				string new_cls = getRealCls(node);
				if(old_cls != new_cls){
					usrdb_insert(s2c(mac),s2c(new_cls),node->mExpire);
					mEvent->notify(E_SRV_URM_CLS_CHANGE,mac,new_cls,old_cls);
				}
			}
			return node;
		}

		/**
		*如果mac对应的ip地址被老化掉了,则认为终端已经下线
		*/
		TerminalNode* userDhcpAlloc(string mac, string ip, long expire){
			map<string,TerminalNode*>::iterator it = mTerminalList.find(mac);
			TerminalNode* node = NULL;
			
			if(it == mTerminalList.end()){
				node = new TerminalNode(mac);
				if(node){
					node->mCls = STR_VISITOR;
					mTerminalList[mac] = node;
					node->mIpAddress = ip;
					node->mDhcpExpire = expire + getSysTicks();
					if(mEvent){
						mEvent->notify(E_SRV_URM_CLS_CHANGE,mac,STR_VISITOR);
					}
				}
			}else{
				map<string,TerminalNode*>::value_type pair = *it;
				node = pair.second;
				if(node){
					node->mIpAddress = ip;
					node->mDhcpExpire = expire + getSysTicks();
				}
			}
			return node;				
		}

		status update(string event_owner, string event, string data1, string data2, string data3, string data4){
			DOUT("event=%s, data1=%s, data2=%s, data3=%s, data4=%s",
				s2c(event),s2c(data1),s2c(data2),s2c(data3),s2c(data4));
			if(event == E_SRV_ALL_START){
				DOUT("ALL SERVICE START!");
			}
			if(event == E_SRV_ALL_STOP){
				DOUT("ALL SERVICE STOP!");
			}
			if(event == E_YOS_CFG_CHANGED){
				if((data1 == STR_CFG_NAME) && (data2 == STR_USRCLS)){
					DOUT("CONFIG FILE CHANGED!");
					clearClsCfg();
				}
			}
			if(event == E_SRV_TERM_INFO){
				long expire = atol(s2c(data3));
				DOUT("E_SRV_TERM_INFO! MAC=%s, CLS=%s, EXPIRE=%u, CHECKGW=%s",s2c(data1),s2c(data2)
					,expire,s2c(data4));
				if(checkLegalAp(data1,data4)){
					userAdd(data1,data2,expire);
				}
			}
			if(event == E_SRV_TERM_TEMPINFO){
				long expire = atol(s2c(data3));
				DOUT("E_SRV_TERM_INFO! MAC=%s, CLS=%s, EXPIRE=%u, CHECKGW=%s",s2c(data1),s2c(data2)
					,expire,s2c(data4));
				if(checkLegalAp(data1,data4)){
					userTmpAdd(data1,data2);
				}
			}			
			if(event == E_SRV_TERM_IPALLOC){
				long expire = atol(s2c(data3));
				DOUT("E_SRV_TERM_INFO! MAC=%s, IP=%s, EXPIRE=%u",s2c(data1),s2c(data2),expire);
				userDhcpAlloc(data1,data2,expire);
			}
			return s_ok;
		}

		void timeArrive(int fd){
			if(mProcTmrFd == fd){
				expireProc();	
			}
		}
};

static UserManager* mUserMgr = NULL;

int main(int argc, char* argv[]) {
	IContext *context = getContext();
	if(context){
		context->init(argc,argv);
		context->debugSwitch(true,"/tmp/usrmgr_unitest.log");

		ILoop *loop = context->loopFactory();

		mUserMgr = new UserManager(context);

		if(mUserMgr && loop && context){
			loop->run();
		}

		if(mUserMgr){
			delete mUserMgr;
			mUserMgr = NULL;
		}
		
		releaseContext(context);
	}
	return 0;
}

void read_db_proc(char* mac, char* cls, long exire){
	if(mUserMgr){
		mUserMgr->userListAdd(mac,cls,exire);
	}
}


