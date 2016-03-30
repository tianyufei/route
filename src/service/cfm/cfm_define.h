#ifndef _CFM_DEFINE_H
#define _CFM_DEFINE_H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>
#include <vector>
using namespace std;
#include "config.h"
#include "tools.h"

#define DR_VERLIST_REQ_FILE 		"/tmp/cfm_dr_verlist_req.txt"
#define RD_VERLIST_RSP_FILE 		"/tmp/cfm_dr_verlist_rsp.txt"
#define DR_SYNC_CON_FILE			"/tmp/cfm_dr_sync_con.txt"
#define CFM_LOCAL_CFG_VERSION_FILE 	"/etc/config/cfm_version_list.txt"
#define CFM_LOCAL_CFG_CHANGED_FILE 	"/etc/config/cfm_local_changed_version.txt"
#define CFM_CFG_FILE	  			"cfm"

#define STR_SN		"sn"
#define STR_CFM 	"cfm"
#define STR_USR		"usr"
#define STR_PWD		"pwd"
#define STR_FREQ	"freq"
#define STR_SVR		"server"
#define STR_CFGVER	"cfgver"
#define STR_MD5		"md5"
#define STR_FILE	"file"
#define STR_VER		"ver"
#define STR_LIST	"list"
#define STR_ADDRESS "address"
#define STR_REQ		"req"
#define STR_URL		"url"
#define STR_MSG		"msg"
#define STR_RST		"rst"

/**************************************************************************
配置文件格式. 
/etc/config/cfm
config cfm
	option server 	'server_url'
	option usr		'username'
	option pwd		'password'
*/


/**************************************************************************
	版本列表文件(CFM_LOCAL_CFG_VERSION_FILE)中的格式
	{
		"cfgver":1,
		"address":"04434",
		"sn":"adgww",
		"list":[
			{ "file":"usrmgr","md5":"1111"},
			{ "file":"advert","md5":"2222"},
		]
	}
*/
class VersionListFile {
	public:
		int mCfgVer;
		string mSn;
		string mAddress;
		map<string,string> mFileMD5List;
		VersionListFile(){}
		~VersionListFile(){}
		void parser(string filename);
		void writeFile(string filename);
		string getMD5(string filename);
};

/**************************************************************************
RD_VERLIST_REQ
{
	"sn":"xxxx",			#该配置是针对哪个设备的。
	"address":"01087600",	#地址码
	"req":1,
	"msg":"RD_VERLIST_REQ"
	
*/
class contents_DR_VERLIST_REQ{
	public:
		string mSn;
		string mAddress;
		unsigned int mReq;
		contents_DR_VERLIST_REQ(){}
		~contents_DR_VERLIST_REQ(){}
		void writeFile(string filename,unsigned int req);
};


/**************************************************************************
RD_VERLIST_RSP
{
	"sn":"xxxx",			#该配置是针对哪个设备的。
	"address":"01087600",	#地址码
	"cfgver":1, 			#配置版本号，如果有配置更新，会增加
	"url":"http://xxx.com", #下载地址
	"user":"xxx",
	"pwd":"xxx",
	"msg":"RD_VERLIST_RSP",
	"list":[
		{ "file":"usrmgr","md5":"xxxx"},
		{ "file":"advert","md5":"xxxx"},
	]
}
*/
class contents_RD_VERLIST_RSP{
	public:
		string mSn;
		string mAddress;
		int mCfgVer;
		string mUrl;
		string mUser;
		string mPwd;
		string mMsg;
		map<string,string> mFileMD5List;
		contents_RD_VERLIST_RSP(){}
		~contents_RD_VERLIST_RSP(){}
		boolean parser(string filename);
};

/***************************************************************************
	DR_UPLOAD_RSP
	{
		"sn":"xxxx",			#该配置是针对哪个设备的。
		"address":"01087600",	#地址码
		"msg":"DR_UPLOAD_RSP",  #消息类型
		"file":"advert", 		#传输的文件名
		"rst":"ok|failed"		#传输结果
	}
*/
class contents_DR_UPLOAD_RSP{
	public:
		string mSn;
		string mAddress;
		string mMsg;
		string mFile;
		string mResult;
		contents_DR_UPLOAD_RSP(){}
		~contents_DR_UPLOAD_RSP(){}
		boolean parser(string filename);
};


/***************************************************************************
	DR_SYNC_CON
	{
		"sn":"xxxx",			#该配置是针对哪个设备的。
		"address":"01087600",	#地址码
		"cfgver":2, 			#配置版本号，如果有配置更新，会增加
		"msg":"DR_SYNC_CON"
	}
*/
class contents_DR_SYNC_CON{
	public:
		string mSn;
		string mAddress;
		string mCfgVer;
		contents_DR_SYNC_CON(){}
		~contents_DR_SYNC_CON(){}
		void writeFile(string filename, int cfger);
};




extern string getFileMd5(string file);
extern int lockCfm();
extern void unlockCfm(int);


#endif

