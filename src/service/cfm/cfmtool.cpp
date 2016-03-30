/******************************************************************************
*���ļ���shell��uci.commit����, ����ͬ���޸���Ϣ.
*******************************************************************************/
#include "cfm_define.h"
#include "srv_evt_id.h"

void help(){
	 printf("\n cfmtool -s                          sync file between cfm server and local device. ");
	 printf("\n cfmtool -l <local_cfg_file>			regist config file to changed list");
	 printf("\n");
}


/**
*@brief	����DR_VERLIST_REQ��DR_VERLIST_RSP��Ϣ��
*/
void runVersionListMsg(int req,UciWrapper& wrapper){
	//����DR_VERLIST_REQ��Ϣ������
	contents_DR_VERLIST_REQ dr_version_req;
	dr_version_req.writeFile(DR_VERLIST_REQ_FILE,req);
	//����DR_VERLIST_REQ��Ϣ��������RSP��Ϣ
	char msg[256] = {0};
	string srv = wrapper.getOption(STR_SVR);
	string usr = wrapper.getOption(STR_USR);
	string pwd = wrapper.getOption(STR_PWD);

	if(termString(usr).empty()){
		snprintf(msg,sizeof(msg) - 1,
			"curl -o %s --data @%s %s",
			s2c(RD_VERLIST_RSP_FILE),
			s2c(DR_VERLIST_REQ_FILE),
			s2c(srv));
	}else{
		snprintf(msg,sizeof(msg) - 1,
			"curl -u %s:%s -o %s --data @%s %s",
			s2c(usr),
			s2c(pwd),
			s2c(RD_VERLIST_RSP_FILE),
			s2c(DR_VERLIST_REQ_FILE),
			s2c(srv));
	}
	system(msg);
}


/**
*@brief �Ƚϱ����޸ĵ������ļ�,����δ�޸ĵ������ļ��ͷ������������ļ��Ĳ���.
*
*@param [in] local_changed_list  ����ͨ��UI�޸ĵ������ļ��б�.
*@param [in] local_version_list  �����޸�ǰ�������ļ��б�.
*@param [in] server_version_list �������ϱ���������ļ��б�.
*@param [out] server_update_list ��Ҫ�ɷ�����ͬ�������ص��ļ��б�
*@param [out] local_update_list  ��Ҫ�ɱ���ͬ�������������ļ��б�.
*/
void getChangedFileList(
	map<string,string>& local_changed_list,  
	map<string,string>& local_version_list,
	map<string,string>& server_version_list,
	vector<string>& server_update_list,
	vector<string>& local_update_list){

		map<string,string>::iterator it1;
		map<string,string>::iterator it2;
		map<string,string>::iterator it3;
		for(it1 = server_version_list.begin(); 
			it1 != server_version_list.end(); it1++){
			map<string,string>::value_type pair = *it1;
			string f = pair.first;
			string m1 = pair.second;
			it2 = local_version_list.find(f);
			if(it2 == local_version_list.end()){
				server_update_list.push_back(f);
			}else{
				map<string,string>::value_type pair2 = *it2;
				string m2 = pair2.second;
				if(m1 > m2)
					server_update_list.push_back(f);
			}
		}
		for(it3 = local_changed_list.begin(); 
			it3 != local_changed_list.end(); it3++){
			map<string,string>::value_type pair = *it3;
			string f = pair.first;
			unsigned int i = 0;
			for(i = 0 ; i < server_update_list.size(); i++){
				//���غͷ�����ͬʱ�޸ĵ��ļ�, ���ط���
				if(f == server_update_list[i]){
					break;
				}
			}
			if(i == server_update_list.size()){
				//�����޸���,��������û���޸�, ��Ҫ�ϴ�.
				local_update_list.push_back(f);
			}
		}	
}

/**
*@brief �����������޸ĺ�������ļ�ͬ��������,�������޸ĺ�������ļ�ͬ����������.
*
*@param [in] rd_version_rsp  RD_VERLIST_RSP ��Ϣ�еĲ���.
*@param [in] local_changed_list �����޸ĵ��ļ�.
*@param [in] down_list �ӷ�����ͬ�������ص��ļ�.
*@param [in] up_list �ӱ���ͬ�������������ļ�.
*/
status syncFile(contents_RD_VERLIST_RSP& rd_version_rsp,
	VersionListFile& local_changed_list,
	vector<string>& down_list,vector<string>& up_list){
	char usr_buf[256] = {0};
	if(!rd_version_rsp.mUser.empty()){
		snprintf(usr_buf,sizeof(usr_buf),"-u %s:%s",
					s2c(rd_version_rsp.mUser),
					s2c(rd_version_rsp.mPwd));
	}
	//���ط������޸ĵ�����
	for(unsigned int i = 0; i < down_list.size(); i++){
			char file_path_name[256] = {0};
			char cmd[512] = {0};
			snprintf(file_path_name,sizeof(file_path_name)-1," /tmp/%s.bak",
				s2c(down_list[i]));
			snprintf(cmd,sizeof(cmd)-1,"curl %s -o %s %s/%s",
				usr_buf,
				file_path_name,
				s2c(rd_version_rsp.mUrl),
				s2c(down_list[i]));
			system(cmd);
			string md5 = getFileMd5(file_path_name);
			if(md5 != rd_version_rsp.mFileMD5List[down_list[i]]){
				printf("transfer file(%s) failed!\n",s2c(down_list[i]));
				return s_err_base;
			}
	}
	//�ϴ������޸ĵ�����
	for(unsigned int i = 0; i < up_list.size(); i++){
			char cmd[512] = {0};
			snprintf(cmd,sizeof(cmd)-1,"curl %s -o /tmp/cfm_upload_rsp.txt \
				-F upload=@/etc/config/%s -F address=%s -F filename=%s -F md5=%s %s",
				usr_buf,
				s2c(up_list[i]),
				s2c(rd_version_rsp.mAddress),
				s2c(up_list[i]),
				s2c(local_changed_list.mFileMD5List[up_list[i]]),
				s2c(rd_version_rsp.mUrl));
			system(cmd);
			contents_DR_UPLOAD_RSP dr_upload_rsp;
			if(!dr_upload_rsp.parser("/tmp/cfm_upload_rsp.txt")){
				printf("upload file(%s) failed!\n",s2c(up_list[i]));
				return s_err_base;
			}
			if(dr_upload_rsp.mFile != up_list[i]){
				printf("upload file(%s), but rsp is[%s] failed!\n",s2c(up_list[i]),s2c(dr_upload_rsp.mFile));
				return s_err_base;
			}
			if(dr_upload_rsp.mResult.find("ok") == string::npos){
				printf("upload file(%s), transfer failed, err=%s!\n",s2c(up_list[i]),s2c(dr_upload_rsp.mResult));
				return s_err_base;
			}
			
	}
	//�����ļ��� ����ʱĿ¼������Ŀ¼��
	for(unsigned int i = 0; i < down_list.size(); i++){
			UciWrapper::copyFile(
				string("/tmp/") + down_list[i] + string(".bak"),
				string("/etc/config/") +  down_list[i]);
	}			
	return s_ok;
}

/*
	����DR_SYNC_CON��Ϣ��
*/
void runSyncConMsg(int req, UciWrapper& wrapper){
	//����DR_VERLIST_REQ��Ϣ������
	contents_DR_VERLIST_REQ dr_version_req;
	dr_version_req.writeFile(DR_VERLIST_REQ_FILE,req);
	//����DR_VERLIST_REQ��Ϣ��������RSP��Ϣ
	string srv = wrapper.getOption(STR_SVR);
	string usr = wrapper.getOption(STR_USR);
	string pwd = wrapper.getOption(STR_PWD);
	char cmd[512] = {0};
	if(!termString(usr).empty()){
		snprintf(cmd,sizeof(cmd)-1,"curl -u %s:%s -o /dev/null --data @%s %s",
			s2c(usr),
			s2c(pwd),
			DR_SYNC_CON_FILE,
			s2c(srv));
		system(cmd);
	}else{
		snprintf(cmd,sizeof(cmd)-1,"curl -o /dev/null --data @%s %s",
			DR_SYNC_CON_FILE,
			s2c(srv));
		system(cmd);
	}
}

void intentBegin(){
	char cmd[256] = {0};
	snprintf(cmd,sizeof(cmd)-1,"intent %s",E_SRV_CFM_UPDATE_START);
	system(cmd);
}

void intentFinish(){
	char cmd[256] = {0};
	snprintf(cmd,sizeof(cmd)-1,"intent %s",E_SRV_CFM_UPDATE_STOP);
	system(cmd);
}

int main(int argc, char* argv[]){

	int opt = 0;
	string file;
	int req = 0;
	while( (opt = getopt(argc, argv,"l:s:")) > 0){
		 switch(opt){
			 case 's':
				opt = 's';
				if(NULL == (optarg)){
					help();
					return 0;
				}
				req = atoi((char*)optarg);				
				break;
			 case 'i':
			 	opt = 'l';
				if(NULL == (optarg)){
					help();
					return 0;
				}
				file = (char*)optarg;
				break;
			 default:
				 help();
			 	return 0;
		 }
	}

	if(opt == 'l'){
		string filename = argv[1];
		VersionListFile changed_list_file;
		VersionListFile local_version_list;
		int fd = lockCfm();
		local_version_list.parser(CFM_LOCAL_CFG_VERSION_FILE);
		changed_list_file.parser(CFM_LOCAL_CFG_CHANGED_FILE);
		changed_list_file.mFileMD5List[filename] = getFileMd5(string("/etc/config/") + filename);
		changed_list_file.mCfgVer = local_version_list.mCfgVer + 1;
		changed_list_file.writeFile(CFM_LOCAL_CFG_CHANGED_FILE);
		unlockCfm(fd);
	}else if(opt == 's'){
		intentBegin();
		VersionListFile version_list_file;
		VersionListFile local_changed_file;
		contents_RD_VERLIST_RSP rd_version_rsp;
		UciWrapper wrapper(CFM_CFG_FILE);
		int fd = lockCfm();
		wrapper.loadSection(STR_CFM);
		version_list_file.parser(CFM_LOCAL_CFG_VERSION_FILE);
		local_changed_file.parser(CFM_LOCAL_CFG_CHANGED_FILE);
		//����VersionListReq�ͽ���VersionListRsp��Ϣ
		runVersionListMsg(req,wrapper);
		if(!rd_version_rsp.parser(RD_VERLIST_RSP_FILE)){
			printf("parser contents of RD_VERLIST_RSP failed!\n");
			unlockCfm(fd);
			intentFinish();
			return 0;
		}
		vector<string> down_list;
		vector<string> up_list;
		//�ȽϷ������ͱ��صĲ���
		getChangedFileList(local_changed_file.mFileMD5List,
			version_list_file.mFileMD5List,
			rd_version_rsp.mFileMD5List,
			down_list,up_list);
		//��ʼͬ���ļ���
		if(s_ok != syncFile(rd_version_rsp,local_changed_file,down_list,up_list)){
			printf("sync file failed!\n");
			unlockCfm(fd);
			intentFinish();
			return 0;
		}
		//����汾��Ϣ
		for(unsigned int i = 0; i < down_list.size(); i++){
			version_list_file.mFileMD5List[down_list[i]] = rd_version_rsp.mFileMD5List[down_list[i]];
		}
		for(unsigned int i = 0; i < up_list.size(); i++){
			version_list_file.mFileMD5List[up_list[i]] = local_changed_file.mFileMD5List[up_list[i]];
		}
		if(down_list.size())
			version_list_file.mCfgVer = rd_version_rsp.mCfgVer;
		else if(up_list.size())
			version_list_file.mCfgVer = local_changed_file.mCfgVer;
		version_list_file.writeFile(CFM_LOCAL_CFG_VERSION_FILE);
		//ͬ����Ϣ
		contents_DR_SYNC_CON dr_sync_con;
		dr_sync_con.writeFile(DR_SYNC_CON_FILE,version_list_file.mCfgVer);
		runSyncConMsg(req,wrapper);
		//ɾ�������޸ļ�¼��
		system("rm "CFM_LOCAL_CFG_CHANGED_FILE);
		unlockCfm(fd);
		intentFinish();
		return 0;
	}
	return 0;
}


