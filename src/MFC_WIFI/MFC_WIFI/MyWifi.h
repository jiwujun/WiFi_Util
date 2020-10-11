#pragma once

#include <afx.h>
#include <windows.h>
#include <wlanapi.h>
#include <vector>
#include <iostream>
#include <string>

#define WIFI_SWITCH_CLOSE 0
#define WIFI_SWITCH_OPEN  1


enum wlan_state {
	WLAN_STATE_INFO_ERROR,		//获取错误，获取网卡信息
	WLAN_STATE_OPEN_ERROR,		//执行获取函数错误

	WLAN_STATE_NOT_READY,		//未准备就绪

	WLAN_STATE_CONNECTED,		//已连接
	WLAN_STATE_NOT_CONNECTED,	//未连接

	WLAN_STATE_OTHER,			//其它状态
};


enum wifi_type {
	WIFI_TYPE_PUBLIC,	//公开
	WIFI_TYPE_PRIVATE,	//私密
	WIFI_TYPE_PASSWORD	//私密_已保存密码
};


struct WifiInfo
{
	char ssid[DOT11_SSID_MAX_LENGTH];	// wifi名称
	int type;	//wifi 类型
	int level;	//wifi 信号
};


class MyWifi
{
public:
	MyWifi();
	~MyWifi();

public:
	bool OpenWLANService();			// 打开WLAN服务
	bool GetFirstWLANInfo();		// 获取第一块无线网卡信息
	int  GetWLANState();			// 获取当前WIFI状态
	bool CheckWLANOpenState();		// 检查WIFI是否打开
	void SetWlanSwitch(int state);	// 设置WIFI状态
	bool GetNetListInfo();			// 加载当前能连接的所有WIFI的信息
	bool SetProfile(const PWLAN_AVAILABLE_NETWORK& net, CString password);	// 设置配置文件（相当于设置密码，对于没有密码的Wifi也要调用此方法，password 参数传空）
	bool ConnectWLAN(const PWLAN_AVAILABLE_NETWORK& net);	// 连接WiFi（外部无需调用此方法）
	bool ConnectWLANPublic(CString in_ssid);
	bool ConnectWLANPrivate(CString in_ssid, CString password);
	bool ConnectWLANPassWord(CString in_ssid);
	bool WLANDisconnect();			// 断开连接
	bool CloseWLANService();		// 关闭服务（退出程序之前请调用此方法）
	WifiInfo GetCurrentWifiInfo();	// 获取当前连接的Wifi信息
	CString UTF8ToCString(std::string utf8str);

	//void printMess(const char* out);

public:
	std::vector<WifiInfo> m_wifi_info;
	std::vector<WifiInfo> m_public_wifi_info;
	std::vector<WifiInfo> m_password_wifi_info;
	std::vector<WifiInfo> m_private_wifi_info;

private:


	HANDLE							m_wlan_client;		//Wlan服务句柄，贯穿整个连接过程
	DWORD							m_cur_version;		//指定这次会话中将会使用的版本
	PWLAN_INTERFACE_INFO			m_wlan_info;		//第一块无线网卡信息
	PWLAN_AVAILABLE_NETWORK_LIST	m_net_list;			//指向返回的可用网络的 WLAN_AVAILABLE_NETWORK_LIST 的指针
	PWLAN_AVAILABLE_NETWORK			m_net;				//当前连接的Wifi信息

};