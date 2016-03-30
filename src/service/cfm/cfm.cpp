#include <stdio.h>
#include <stdlib.h>
#include <stack>
#include <json.h>
using namespace std;

#include "config.h"
#include "common.h"
#include "tools.h"
#include "icontext.h"
#include "srv_evt_id.h"
#include "srv_env_id.h"
#include "cfm_define.h"
#include "sysenv.h"



class CfgMgr: public IEventObserver {
	private:
		unsigned int mReq;
		IContext* mContext;
		IEvent* mEvent;
		boolean mInUpdateStatus;
		status procUpdateProcess(int server_ver){
			/*
				1 �������޸ģ� ��̨���޸ġ� 
				2 �������޸ģ� ��̨���޸�
				3 �������޸ģ� ��̨���޸�
				4 �������޸ģ� ��̨���޸�
				5 ͬ�������У���̨����
				6 ͬ�������У��豸����
			*/
			//����Ƿ��б��ظ���
			VersionListFile version_list_file;
			VersionListFile local_changed_file;
			int fd = lockCfm();
			version_list_file.parser(CFM_LOCAL_CFG_VERSION_FILE);
			local_changed_file.parser(CFM_LOCAL_CFG_CHANGED_FILE);
			unlockCfm(fd);
			if(!mInUpdateStatus ){
				if((version_list_file.mCfgVer < server_ver) || local_changed_file.mFileMD5List.size()){
					mContext->systemCall("cfmtool -s %d &",++mReq);
				}else{
					DOUT("not find config file changed!");
				}
			}
			return s_ok;
		}

		void locationCheck(){
			/*���Ƚ����ƶ��������,Ҳ���ǵ�ǰ��address��Ӳ���ϱ�����Ƿ�һ��.
			* �����һ������������.
			*/
			VersionListFile local_change_list;
			VersionListFile local_version_list;
			int fd = lockCfm();
			local_version_list.parser(CFM_LOCAL_CFG_VERSION_FILE);
			local_change_list.parser(CFM_LOCAL_CFG_CHANGED_FILE);
			char addrbuf[256] = {0};
			get_sys_env((char*)Y_SYS_ADDRESS_CODE,addrbuf,sizeof(addrbuf)-1);
			string address = addrbuf;
			if(mContext && ((address != local_change_list.mAddress) ||
				(address != local_version_list.mAddress))){
				//��ַ�벻һ��, ���ܷ������ƻ�.
				mContext->systemCall("rm "CFM_LOCAL_CFG_CHANGED_FILE);
				mContext->systemCall("rm "CFM_LOCAL_CFG_VERSION_FILE);
				mContext->systemCall("sync");
			}
			unlockCfm(fd);
		}
		
	public:
		CfgMgr(IContext* ctx){
			mReq = 0;	
			mContext = ctx;
			mInUpdateStatus = false;
			if(ctx)
				mEvent = ctx->eventFactory();
			if(mEvent){
				mEvent->registObserver(E_SRV_HB_CFG_INFO,this);
				mEvent->registObserver(E_SRV_CFM_UPDATE_START,this);
				mEvent->registObserver(E_SRV_CFM_UPDATE_STOP,this);
			}
			locationCheck();
		}
		~CfgMgr(){
		}


 		status update(string event_owner, string event, string data1, string data2, string data3, string data4){
			DOUT("update, event=%s, data1=%s, data1=%s",s2c(event),s2c(data1),s2c(data2));
			if(event == E_SRV_CFM_UPDATE_START){
				mInUpdateStatus = true;
			}if(event == E_SRV_CFM_UPDATE_STOP){
				mInUpdateStatus = false;
			}if(event == E_SRV_HB_CFG_INFO){
				//data1�Ǻ�̨�İ汾�ź�״̬
				procUpdateProcess(atoi(s2c(data1)));
			}
			return s_ok;
		}

};

static CfgMgr* mCfmMgr = NULL;

int main(int argc, char* argv[]) {
	IContext *context = getContext();
	if(context){
		context->init(argc,argv);
		context->debugSwitch(true,"/tmp/cfm_unitest.log");

		ILoop *loop = context->loopFactory();

		mCfmMgr = new CfgMgr(context);

		if(mCfmMgr && loop && context){
			loop->run();
		}

		if(mCfmMgr){
			delete mCfmMgr;
			mCfmMgr = NULL;
		}
		
		releaseContext(context);
	}
	return 0;
}




