/*****************************************************************************
*  Yaya Smart Router System on Openwrt.                                      *
*  Copyright (C) 2016 ZhengDajun  zhengdajun@vip.sina.com.                   *
*                                                                            *
*                                                                            *
*  @file     tools.h                                                         *
*  @brief    该文件封装了一些常用的功能函数.								 *
*  Details.                                                                  *
*            封装一些常用的，但C++和Openwwrt没有的函数						 *
*  @author   ZhengDajun                                                      *
*  @email    zhengdajun@vip.sina.com.                                        *
*  @version  1.0.0.1(版本号)                                                 *
*  @date     zhengdajun@vip.sina.com.                                        *
*  @license  GNU Lesser General Public License (LGPL)                        *
*                                                                            *
*----------------------------------------------------------------------------*
*  Remark         : Description                                              *
*----------------------------------------------------------------------------*
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2016/03/05 | 1.0.0.1   | ZhengDajun     | Create file                     *
*----------------------------------------------------------------------------*
*                                                                            *
*****************************************************************************/

#ifndef _TOOLS_H
#define _TOOLS_H

#include <string>
#include <vector>
#include <map>
using namespace std;
#include "type.h"


/**
*@brief 分割字符串。
*
*将字符串分割，并存储到vector表中。
*@param [in] src 被分割的字符串。
*@param [in] flag 分隔符，该字符串中的任意一个字符都是分隔符。
*@param [out] result 分割后的字符串表。
*/
extern void splitString(string src, string flag, vector<string>& result);

/** 
*@brief 字符串替换。
*
*将一个字符串中的指定内容替换成另一个内容。. 
* @param [in] src 原始字符串
* @param [in] org 要被替换掉的字符串内容。 
* @param [in] dest 替换后的新内容。 
* @retun 完成字符串替换的新字符串。 
*/ 
string replaceInString(string src, string org, string dest);

/**
*@brief 去除字符串两边多余字符。
*
*去除字符串起始位置和结束位置上指定的字符。
*@param [in] src 原始字符串。
*@param [in] arg 原始字符两端要去除的字符。
*@return 处理后的字符串。
*/
string termString(string src, string arg = " ");
	

/** 
* @brief 获取指定接口的MAC地址。
*
*获取指定接口的MAC地址。
*
* @param [in] intf 接口名称，比如eth0, ath0等等。
* @param [out] buf 存放MAC地址的BUF.  MAC以字符数组的方式保存。
* @param [in] size buf的大小，不能小于6。
* @return 0-成功获取;其它值表示失败
* 
*/ 
extern int getIntfMac(char* intf, unsigned char *buf, int size);

/** 
* @brief 获取基准MAC地址。
*
*获取基准MAC地址(一般是eth0的地址)。
*
* @param [out] buf 存放MAC地址的BUF.  MAC以字符数组的方式保存。
* @param [in] size buf的大小，不能小于6。
* @return 0-成功获取;其它值表示失败
*/ 
extern int getBaseIntfMac(unsigned char *buf, int size);

/**
*@brief 字符串转换
*
*将char*字符串转换成string
*@param v 被转换的字符串。 如果v=NULL, 则返回值为"".
*@return 转换后的string.
*/
extern string c2s(char* v);

/**
*@brief 字符串转换
*
*将char*字符串转换成string
*@param v 被转换的字符串。 如果v=NULL, 则返回值为"".
*@return 转换后的string.
*/
extern string c2s(const char* v);

/**
*@brief 字符串转换
*
*将char*字符串转换成string
*@param v 被转换的字符串。 如果v="", 则返回值为"".
*@return 转换后的字符串.
*/
extern char* s2c(string v);

/**
*@brief 十六进制字符串转换成整数。
*
*十六进制字符串转换成整数。
*@param [in] s 被转换的字符串。
*@param [out] u 转换后的整数指针。
*/
extern void s2u(char *s, char *u );


/**
*@brief 检查是否系统有硬盘。
*
*检查是否系统有硬盘。
*@return true-有； false-没有。
*/
extern boolean checkHDD();

/**
*@brief 转换ip和掩码的表达方式。
*
*将字符串的ip和掩码转换成组合表达方式, 比如将"192.168.1.0" "255.255.255.0" 转换成"192.168.1.0/24" 这样的格式
*@param [in] ip 被转换的ip网段.
*@param [in] mask 被转换的mask掩码.
*@param [out] ip_mask 转换后的网段和掩码.
*@return s_ok 转换成功; 其它值表示转换失败.
*/
extern status ipToNet(string ip, string mask, string &ip_mask);

/** 
*@brief 比较表
*
*比较2个表，将减少的和增加的表项找出来
*@param [in] src 表一
*@param [in] dst 表二
*@param [out] same 表一和表二相同的表项
*@param [out] less 表二比表一少的表项.
*@param [out] more 表二比表一多的表项.
*/
extern void compareList(vector<string>& src, vector<string>& dst, vector<string>& same, vector<string>& less, vector<string>& more);

/** 
*@brief 获取当前运行的时间长度.
*
*获取设备启动后,到当前运行了多长时间(秒数) 
*@return 运行时间长度
*/
extern long getSysTicks();
/** 
*@brief 获取当前时间.
*
*获取1970年1月1日到现在的秒数(秒数) 
*@return 秒数
*/
extern long getSysTime();

/** BASE64 编码, dlen 要足够大*/
int encode_base64( unsigned char *dst, int *dlen,unsigned char *src, int slen);

/** BASE64 解码, dlen 要足够大*/
int decode_base64( unsigned char *dst, int *dlen,unsigned char *src, int slen);

extern size_t Base64_Encode_GB2312(char *pDest, const char *pSrc, size_t srclen);

extern int Base64_Decode_GB2312(char *lpString, char *lpSrc, int sLen);

/** 
*@brief 获取设备当前的SN号.
*
*每个设备都有SN(一般情况下是eth0的mac地址)
*@return 设备sn.
*/
extern string getOwnDevSn();


/** 生成session*/
extern string generateSession();

/** 
*@brief 计算md5.
*
*计算一段buffer的md5值.
*@param src 被计算的buffer.
*@param len buffer的长度.
*@return md5字符串.
*/
extern string calMd5(unsigned char *src, unsigned int len);

/** 
*@brief 获取当前默认网关信息.
*
*获取当前默认网关以及对应的物理出口.
*@param [out] intf 默认网关所对应的设备物理出口.
*@param [out] gw 默认网关的ip地址.
*/
extern void getCurrentRouteIntf(string& intf, string& gw);

/**
*@brief 配置文件操作类。
*
*本类封装了对/etc/config下的配置文件的操作，使通过C++读写配置文件变的简单。
*
*/
class UciWrapper {
	private:
		string mSection;
		string mSectionName;
		string mToken;
		string mTokenKey;
		string mFileName;
		boolean mForce;
		boolean mChanged;
		map<string,string> mOptionList;
		class ListContent{
			public:
				vector<string> mList;
				ListContent(){}
				~ListContent(){}
		};
		map <string, ListContent*> mMutiList;
		/*
			清除所有list
		*/
		status clearListAll();
		status loadFromFile();
	public:
		/**
		*@param filename: 要操作的配置文件名称，该文件保存在/etc/config目录下。
		*/
		UciWrapper(string filename);
		~UciWrapper();

		/**
		*@brief 加载或创建section.
		*
		*指定要加载或创建的section, 举例:\n
		*    config wifi-device  'radio0'
		*        option channel '1'
		*        list   black	'00:11:22:33:44:55'
		*        list   black   '22:44:66:88:aa:bb' 
		*@param section_class  section的类型, 比如wifi-device
		*@param section_name   section的名字, 比如radio0, 有的section并没有指定名字,要注意.
		*@return s_ok 成功； 其它值表示失败。
		*/
		status loadSection(string section_class, string section_name="");

		/**
		*@brief 加载或创建section.
		*
		*指定要加载或创建的section, 举例:\n
		*    config wifi-device  'radio0'
		*        option channel '1'
		*        list   black	'00:11:22:33:44:55'
		*        list   black   '22:44:66:88:aa:bb' 
		*@param section_class  section的类型, 比如wifi-device
		*@param token_key 	  section中关键字，比如channel
		*@param token_val 	  section中关键字对应的值，比如1
		*@return s_ok 成功； 其它值表示失败。
		*/
		status loadSection(string section_class, string token_key, string token_val);
	
		/**
		*@brief 添加或修改option
		*
		*创建或修改指定option的值。
		*@param opt option的名称。
		*@param val option的值。
		*@return s_ok 成功； 其它值表示失败。
		*/
		status setOption(string opt, string val);

		/**
		*@brief 获取option的值。
		*
		*获取指定option的值。
		*@param opt option的名称。
		*@return option的值， 如果表项不存在，则返回空串。
		*/
		string getOption(string opt);

		/**
		*@brief 添加或修改option
		*
		*创建或修改指定option的值。
		*@param opt option的名称。
		*@param val option的值。
		*@return s_ok 成功； 其它值表示失败。
		*/
		status setOption(string opt, int val);

		/**
		*@brief 创建添list的表项。
		*
		*创建指定list的表项。
		*@param lst list的名称。
		*@param val list的表项。
		*@return s_ok 成功； 其它值表示失败。
		*/
		status setList(string lst, string val);

		/**
		*@brief 删除list的表项。
		*
		*删除list的指定表项。
		*@param lst list的名称。
		*@param val list中被删除表项。
		*@return s_ok 成功； 其它值表示失败。
		*/
		status rmList(string lst,string val);

		/**
		*@brief 删除list。
		*
		*删除指定的list。
		*@param lst list的名称。
		*@return s_ok 成功； 其它值表示失败。
		*/
		status clearList(string lst);

		/**
		*@brief 获取list的表项。
		*
		*获取list的所有表项。
		*@param [in] lst list的名称。
		*@param [out] result 表项列表。
		*@return s_ok 成功； 其它值表示失败。
		*/
		status getList(string lst, vector<string>& result);
	
		/**
		*@brief 将配置信息保存到文件。
		*
		*将内存中的内容保存到指定的配置文件中
		*@return s_ok 成功； 其它值表示失败。
		*/
		status commit();

		/**
		*@brief 判断配置数据是否有变化。
		*
		*判断配置数据是否有变化
		*@return true-有变化， false-无变化
		*/
		boolean checkChanged();

		/**
		*@brief 比较两个Uci中的配置是否相同。
		*
		*比较2个uci 是否内容相同
		*@param uci 被比较的uci.
		*@return true-相同, false-不相同。
		*/
		boolean same(UciWrapper* uci);

		/**
		*@brief 复制配置文件，并广播E_YOS_CONFIG_CHANGE消息
		*
		*@param [in] src 源文件
		*@param [in] dst 目标文件
		*/
		static void copyFile(string src, string dst);

		
};




/*
*@brief 接口状态。
*
*保存运行时的接口状态数据.
*/
typedef struct IntfInfoStru{
	char name[32]; 
	char ip[32];
	char mask[32];
	char mac[18];
	char rxBytes[32];
	char rxPackages[32];
	char rxErrors[32];
	char txErrors[32];
	char rxDropped[32];
	char txDropped[32];
	char txBytes[32];
	char txPackages[32];
}IntfInfoStru;

/*
*@brief 获取指定接口的参数。
*
*@param [in] devname 接口名称, 比如eth0, br-lan, wlan0等等.
*@param [out] info 返回的参数结构.
*@return s_ok 获取成功.
*/
extern boolean getIntfInfo(char* devname/*eth0,eth0.2,ppp0...*/,IntfInfoStru* info) ;

/*
*@brief 根据分配的IP地址获取对应的mac地址.
*
*@param [in] ip地址
*@return  对应的mac地址
*/
extern string getMacFromIp(string ips);

#endif



