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
	WLAN_STATE_INFO_ERROR,		//��ȡ���󣬻�ȡ������Ϣ
	WLAN_STATE_OPEN_ERROR,		//ִ�л�ȡ��������

	WLAN_STATE_NOT_READY,		//δ׼������

	WLAN_STATE_CONNECTED,		//������
	WLAN_STATE_NOT_CONNECTED,	//δ����

	WLAN_STATE_OTHER,			//����״̬
};


enum wifi_type {
	WIFI_TYPE_PUBLIC,	//����
	WIFI_TYPE_PRIVATE,	//˽��
	WIFI_TYPE_PASSWORD	//˽��_�ѱ�������
};


struct WifiInfo
{
	char ssid[DOT11_SSID_MAX_LENGTH];	// wifi����
	int type;	//wifi ����
	int level;	//wifi �ź�
};


class MyWifi
{
public:
	MyWifi();
	~MyWifi();

public:
	bool OpenWLANService();			// ��WLAN����
	bool GetFirstWLANInfo();		// ��ȡ��һ������������Ϣ
	int  GetWLANState();			// ��ȡ��ǰWIFI״̬
	bool CheckWLANOpenState();		// ���WIFI�Ƿ��
	void SetWlanSwitch(int state);	// ����WIFI״̬
	bool GetNetListInfo();			// ���ص�ǰ�����ӵ�����WIFI����Ϣ
	bool SetProfile(const PWLAN_AVAILABLE_NETWORK& net, CString password);	// ���������ļ����൱���������룬����û�������WifiҲҪ���ô˷�����password �������գ�
	bool ConnectWLAN(const PWLAN_AVAILABLE_NETWORK& net);	// ����WiFi���ⲿ������ô˷�����
	bool ConnectWLANPublic(CString in_ssid);
	bool ConnectWLANPrivate(CString in_ssid, CString password);
	bool ConnectWLANPassWord(CString in_ssid);
	bool WLANDisconnect();			// �Ͽ�����
	bool CloseWLANService();		// �رշ����˳�����֮ǰ����ô˷�����
	WifiInfo GetCurrentWifiInfo();	// ��ȡ��ǰ���ӵ�Wifi��Ϣ
	CString UTF8ToCString(std::string utf8str);

	//void printMess(const char* out);

public:
	std::vector<WifiInfo> m_wifi_info;
	std::vector<WifiInfo> m_public_wifi_info;
	std::vector<WifiInfo> m_password_wifi_info;
	std::vector<WifiInfo> m_private_wifi_info;

private:


	HANDLE							m_wlan_client;		//Wlan���������ᴩ�������ӹ���
	DWORD							m_cur_version;		//ָ����λỰ�н���ʹ�õİ汾
	PWLAN_INTERFACE_INFO			m_wlan_info;		//��һ������������Ϣ
	PWLAN_AVAILABLE_NETWORK_LIST	m_net_list;			//ָ�򷵻صĿ�������� WLAN_AVAILABLE_NETWORK_LIST ��ָ��
	PWLAN_AVAILABLE_NETWORK			m_net;				//��ǰ���ӵ�Wifi��Ϣ

};