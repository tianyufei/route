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

#define STR_PORTAL_FILE "portal"
#define STR_GLOBAL		"global"
#define STR_SQUID		"squid"
#define STR_MASK		"mask"

/*相关配置内容
/etc/config/portal

config xxx
	list 	white			'.'									#白名单放行
	list 	black			'gateway|self .qq.' 				#黑名单定向
	list 	accept			'2.2.2.2'
	list 	deny			'3.3.3.3'
	option order 			'black,white' 						#哪个名单优先，如果是白名单优先，最后要drop不在名单的报文。
	option default			'deny|accept'
	option mask				'1' 								#在iptabes中的标签
	option exfile			'/sbin/yyy.sh'						#扩展规则，default之前执行，可以在其中添加高级限制
	option squid			'1'

config global
	option squid '1'
	
*/

class PortalMgr : public IEventObserver {
	private:
		IContext* mContext;
		IEvent* mEvent;
		map<string,UciWrapper*> mUserClsCfg;

		void portalStart(string section){
			if(mContext && (section != STR_GLOBAL)){
				mContext->systemCall("/sbin/portal_start.sh %s",s2c(section));
			}
			if(mContext && (section == STR_GLOBAL)){
				mContext->systemCall("/sbin/portal_global.sh start",s2c(section));
			}
		}

		void portalStop(string section){
			if(mContext && (section != STR_GLOBAL)){
				mContext->systemCall("/sbin/portal_stop.sh %s",s2c(section));
			}
			if(mContext && (section == STR_GLOBAL)){
				mContext->systemCall("/sbin/portal_global.sh stop",s2c(section));
			}
		}

		string getOption(string section, string optioname){
			map<string,UciWrapper*>::iterator it = mUserClsCfg.find(section);
			UciWrapper* cfg = NULL;
			if(it == mUserClsCfg.end()){
				cfg = new UciWrapper(STR_PORTAL_FILE);
				if(cfg){
					cfg->loadSection(section);
					mUserClsCfg[section] = cfg;
					portalStart(section);
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
			map<string,UciWrapper*>::iterator it = mUserClsCfg.find(section);
			UciWrapper* cfg = NULL;
			if(it == mUserClsCfg.end()){
				cfg = new UciWrapper(STR_PORTAL_FILE);
				if(cfg){
					cfg->loadSection(section);
					mUserClsCfg[section] = cfg;
					portalStart(section);
				}
			}else{
				map<string,UciWrapper*>::value_type pair = *it;
				cfg = pair.second;
			}
			if(cfg){
				cfg->getList(listname,rst);
			}
		}

		void clsCfgClear(){
			map<string,UciWrapper*>::iterator it;
			for(it = mUserClsCfg.begin(); it != mUserClsCfg.end(); it++){
				map<string,UciWrapper*>::value_type pair = *it;
				UciWrapper* node = pair.second;
				if(node){
					delete node;
					portalStop(pair.first);
				}
			}
			mUserClsCfg.clear();
		}

		void usrClsChanged(string mac, string old_cls, string new_cls){
			if(old_cls != new_cls){
				if(!old_cls.empty()){
					int mask = atoi(s2c(getOption(old_cls,STR_MASK)));
					if(mask){
						mContext->systemCall("iptables -t mangle -D PREROUTING -m mac --mac-source %s -j MARK --set-mark %d/%d", 
							s2c(mac), mask,mask);
					}
				}
				if(!new_cls.empty()){
					int mask = atoi(s2c(getOption(new_cls,STR_MASK)));
					if(mask){
						mContext->systemCall("iptables -t mangle -I PREROUTING -m mac --mac-source %s -j MARK --set-mark %d/%d", 
							s2c(mac), mask,mask);
					}
				}
			}
		}


	public:
		PortalMgr(IContext* ctx){
			mContext = ctx;
			mEvent = NULL;
			if(ctx){
				mEvent = ctx->eventFactory();
				if(mEvent){
					mEvent->registObserver(E_SRV_ALL_START,this);
					mEvent->registObserver(E_SRV_ALL_STOP,this);
					mEvent->registObserver(E_YOS_CFG_CHANGED,this);
					mEvent->registObserver(E_SRV_URM_CLS_CHANGE,this);
					mEvent->registObserver(E_SRV_URM_CLS_REMOVE,this);
					mEvent->registObserver(E_SRV_NETWORK_RESTART,this);
				}
			}
			portalStart(STR_GLOBAL);
		}
		~PortalMgr(){
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
				if((data1 == STR_PORTAL_FILE) && (data2 == STR_GLOBAL)){
					DOUT("CONFIG FILE CHANGED!");
					portalStop(STR_GLOBAL);
					clsCfgClear();
					portalStart(STR_GLOBAL);
				}
				if((data1 == STR_PORTAL_FILE) && (data2 != STR_GLOBAL)){
					DOUT("CONFIG FILE CHANGED!");
					clsCfgClear();
				}
			}
			if(event == E_SRV_URM_CLS_CHANGE){
				DOUT("E_SRV_URM_CLS_CHANGE! MAC=%s, NOW_CLS=%s, OLD_CLS=%s",s2c(data1),s2c(data2),s2c(data3));
				usrClsChanged(data1,data2,data3);
			}
			if(event == E_SRV_URM_CLS_REMOVE){
				DOUT("E_SRV_URM_CLS_REMOVE! MAC=%s, NOW_CLS=%s, OLD_CLS=%s",s2c(data1),s2c(data2),s2c(data3));
				usrClsChanged(data1,data2,"");
			}	
			if(event == E_SRV_NETWORK_RESTART){
				//network restart是否对iptables有影响， 需要观察。
			}
			return s_ok;
		}


};


static PortalMgr* mPortalMgr = NULL;

int main(int argc, char* argv[]) {
	IContext *context = getContext();
	if(context){
		context->init(argc,argv);
		context->debugSwitch(true,"/tmp/portal_unitest.log");

		ILoop *loop = context->loopFactory();

		mPortalMgr = new PortalMgr(context);

		if(mPortalMgr && loop && context){
			loop->run();
		}

		if(mPortalMgr){
			delete mPortalMgr;
			mPortalMgr = NULL;
		}
		
		releaseContext(context);
	}
	return 0;
}


