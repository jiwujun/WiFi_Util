#include "pch.h"
#include "MyWifi.h"
#include <windows.h>
#include <wlanapi.h>

#pragma  comment(lib,"wlanapi.lib")
#pragma  comment(lib,"ole32.lib")


// IDI_LOGO
MyWifi::MyWifi()
{

	m_wlan_info = NULL;

	m_cur_version = 0;
	m_wlan_client = NULL;
	m_net_list = NULL;

	m_wifi_info = std::vector<WifiInfo>();

}

MyWifi::~MyWifi()
{
}

/*
�� WLAN ���� ��Ϊ�������в����Ļ�����
����ֵ��
	�Ƿ�򿪳ɹ�
*/
bool MyWifi::OpenWLANService()
{
	DWORD dwResult = 0;
	dwResult = WlanOpenHandle(WLAN_API_VERSION    //�ͻ���֧�ֵ�WLAN API����߰汾
		, NULL, &m_cur_version    //ָ����λỰ�н���ʹ�õİ汾
		, &m_wlan_client    //ָ���ͻ�������λỰ��ʹ�õľ������������ᴩ�����Ự����������ʹ�� ��
	);
	if (ERROR_SUCCESS != dwResult)
	{
		switch (dwResult)
		{
		case ERROR_INVALID_PARAMETER:    //����һ����Ϊ�ջ��������Ϊ��
			break;
		case ERROR_NOT_ENOUGH_MEMORY:    //û���㹻���ڴ�ռ�
			break;
		case ERROR_REMOTE_SESSION_LIMIT_EXCEEDED:    //�����������޶��ľ������
			break;
		default:
			dwResult = GetLastError();
			//strbuf.Format("WlanOpenHandle Fail��%wd\n",dwResult);
			//printMess(strbuf);
			break;
		}
		return false;
	}
	//strbuf.Format("\n\nWlanOpenHandle success \n");
	//printMess(strbuf);
	return true;
}

/*
��ȡ��һ��������������Ϣ ����Ҫ�ȴ� WLAN Service����Ĭ���û�ֻ��һ����������������ж�������������޸Ĵ˷���������� WIFI �������ˣ����޷���������

����ֵ��
	�Ƿ��ȡ�ɹ�
	��ȡ������Ϣʧ�ܣ�\n��ȷ�����ĵ����Ƿ���������������ܻ������������Ƿ�װ����
*/
bool MyWifi::GetFirstWLANInfo() {
	DWORD dwResult = 0;
	PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
	PWLAN_INTERFACE_INFO pIfInfo = NULL;

	dwResult = WlanEnumInterfaces(m_wlan_client, NULL, &pIfList);
	if (dwResult != ERROR_SUCCESS) {
		wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
		return false;
		// You can use FormatMessage here to find out why the function failed
	}

	if ((pIfList->dwNumberOfItems) < 1) {
		return false;
	}

	m_wlan_info = (WLAN_INTERFACE_INFO *)& pIfList->InterfaceInfo[0];
	return true;

	WlanFreeMemory(pIfList);

}

/*
��ȡ����״̬ ����Ҫ�Ȼ�ȡ������Ϣ��

����ֵ��
	WLAN_STATE_INFO_ERROR		��ȡ���󣬻�ȡ������Ϣ
	WLAN_STATE_OPEN_ERROR		ִ�л�ȡ��������

	WLAN_STATE_NOT_READY		δ׼������

	WLAN_STATE_CONNECTED		������
	WLAN_STATE_NOT_CONNECTED	δ����

	WLAN_STATE_OTHER			����״̬

*/
int MyWifi::GetWLANState() {
	switch (m_wlan_info->isState) {
	case wlan_interface_state_not_ready:
		return WLAN_STATE_NOT_READY;
		break;
	case wlan_interface_state_connected:
		return WLAN_STATE_CONNECTED;
		break;
	case wlan_interface_state_ad_hoc_network_formed:
		return WLAN_STATE_OTHER;
		break;
	case wlan_interface_state_disconnecting:
		return WLAN_STATE_NOT_CONNECTED;
		break;
	case wlan_interface_state_disconnected:
		return WLAN_STATE_NOT_CONNECTED;
		break;
	case wlan_interface_state_associating:
		return WLAN_STATE_NOT_CONNECTED;
		break;
	case wlan_interface_state_discovering:
		return WLAN_STATE_NOT_CONNECTED;
		break;
	case wlan_interface_state_authenticating:
		return WLAN_STATE_NOT_CONNECTED;
		break;
	default:
		return WLAN_STATE_OTHER;
		break;
	}
	return WLAN_STATE_OTHER;
}


/*
��� WIFI���� �Ƿ�� ����Ҫ�Ȼ�ȡ������Ϣ��

����ֵ��
	�� ���� true
	û�� ���� false
*/
bool MyWifi::CheckWLANOpenState() {
	DWORD dwResult = 0;
	PWLAN_RADIO_STATE pradioStateInfo = NULL;
	DWORD radioStateInfoSize = sizeof(WLAN_RADIO_STATE);
	WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_invalid;

	dwResult = WlanQueryInterface(m_wlan_client,
		&m_wlan_info->InterfaceGuid,
		wlan_intf_opcode_radio_state,
		NULL,
		&radioStateInfoSize,
		(PVOID *)& pradioStateInfo, &opCode);

	if (dwResult != ERROR_SUCCESS) {
		//wprintf(L"WlanQueryInterface failed with error: %u\n", dwResult);
		// You can use FormatMessage to find out why the function failed
		return false;
	}
	else {

		switch (pradioStateInfo->PhyRadioState[0].dot11SoftwareRadioState) {
		case dot11_radio_state_unknown:
			//wprintf(L"Unknown\n");
			return false;
			break;
		case dot11_radio_state_on:
			//wprintf(L"On\n");
			return true;
			break;
		case dot11_radio_state_off:
			return false;
			break;
		default:
			//wprintf(L"Other Unknown state %ld\n", pradioStateInfo->PhyRadioState[0].dot11SoftwareRadioState);
			return false;
			break;
		}

	}
}

/*
���� WIFI_״̬

������
	WIFI_SWITCH_CLOSE	�ر�״̬
	WIFI_SWITCH_OPEN	��״̬
*/
void MyWifi::SetWlanSwitch(int state) {
	WLAN_PHY_RADIO_STATE wifi_state; // wifi����״̬
	wifi_state.dwPhyIndex = 0;
	if (state == WIFI_SWITCH_OPEN)
	{
		wifi_state.dot11SoftwareRadioState = dot11_radio_state_on;
	}
	else
	{
		wifi_state.dot11SoftwareRadioState = dot11_radio_state_off;
	}
	PVOID pData = &wifi_state;

	WlanSetInterface(m_wlan_client, &m_wlan_info->InterfaceGuid, wlan_intf_opcode_radio_state, sizeof(WLAN_PHY_RADIO_STATE), pData, NULL);
}


bool MyWifi::GetNetListInfo() {
	m_wifi_info.clear();
	DWORD dwResult = 0;
	PWLAN_AVAILABLE_NETWORK pBssEntry = NULL;
	int iRSSI = 0;

	dwResult = WlanScan(m_wlan_client, &m_wlan_info->InterfaceGuid, NULL, NULL, NULL);
	if (dwResult != ERROR_SUCCESS)
	{
		return false;
	}

	dwResult = WlanGetAvailableNetworkList(m_wlan_client,
		&m_wlan_info->InterfaceGuid,
		0,
		NULL,
		&m_net_list);

	if (dwResult != ERROR_SUCCESS) {
		//wprintf(L"WlanGetAvailableNetworkList failed with error: %u\n", dwResult);
		// You can use FormatMessage to find out why the function failed
		return false;
	}
	else {
		//wprintf(L"WLAN_AVAILABLE_NETWORK_LIST for this interface\n");
		//wprintf(L"  Num Entries: %lu\n\n", m_net_list->dwNumberOfItems);

		for (int j = 0; j < m_net_list->dwNumberOfItems; j++) {
			pBssEntry =
				(WLAN_AVAILABLE_NETWORK *)& m_net_list->Network[j];

			// �����������
			if (pBssEntry->bNetworkConnectable) {
				//CString ssid2 = UTF8ToCString((char*)pBssEntry->dot11Ssid.ucSSID);
				//std::wcout << ssid2 << std::endl;

				WifiInfo wifinfo = { 0 };
				wifinfo.level = pBssEntry->wlanSignalQuality;
				memcpy_s(wifinfo.ssid, sizeof(wifinfo.ssid), (char*)pBssEntry->dot11Ssid.ucSSID, pBssEntry->dot11Ssid.uSSIDLength);
				if (pBssEntry->bSecurityEnabled) {
					//û�б��������
					if (pBssEntry->strProfileName == NULL || wcslen(pBssEntry->strProfileName) == 0) {
						wifinfo.type = WIFI_TYPE_PRIVATE;
						m_private_wifi_info.push_back(wifinfo);
					}
					//���������
					else {
						wifinfo.type = WIFI_TYPE_PASSWORD;
						m_password_wifi_info.push_back(wifinfo);
					}
				}
				else {
					wifinfo.type = WIFI_TYPE_PUBLIC;
					m_public_wifi_info.push_back(wifinfo);
				}
				
				m_wifi_info.push_back(wifinfo);

			}
		}
	}

	return false;
}


bool MyWifi::SetProfile(const PWLAN_AVAILABLE_NETWORK& net, CString password)
{
	DWORD dwResult = 0;
	DWORD dwReasonCode = 0;

	CString szProfileXML("");  //Profile XML��
	LPWSTR wscProfileXML = NULL;
	/*��ϲ���XML����*/
	CString szTemp("");
	// char p[1024];
		/*ͷ*/
	szProfileXML += CString("<?xml version=\"1.0\"?><WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\"><name>");
	/*name��һ����SSID��ͬ*/
	szTemp = CString((char*)(net->dot11Ssid.ucSSID));    //�����ļ���
	/*strbuf.Format("profile�ļ���Ϊ��%s\n",szTemp);
	MessageBox(strbuf);*/
	szProfileXML += szTemp;
	/*SSIDConfig*/
	szProfileXML += CString("</name><SSIDConfig><SSID><name>");
	szProfileXML += szTemp;    //���������������Ӧ��SSID
	szProfileXML += CString("</name></SSID></SSIDConfig>");
	/*connectionType*/
	szProfileXML += CString("<connectionType>");
	switch (net->dot11BssType)    //��������
	{
	case dot11_BSS_type_infrastructure:
		szProfileXML += "ESS";
		break;
	case dot11_BSS_type_independent:
		szProfileXML += "IBSS";
		break;
	case dot11_BSS_type_any:
		szProfileXML += "Any";
		break;
	default:
		//printMess("δ֪���������ͣ�Unknown BSS type\n");
		return false;
	}
	/*MSM*/
	szProfileXML += CString("</connectionType><MSM><security><authEncryption><authentication>");
	switch (net->dot11DefaultAuthAlgorithm)    //������ܷ�ʽ
	{
	case DOT11_AUTH_ALGO_80211_OPEN:
		szProfileXML += "open";
		//printMess("Open 802.11 authentication\n");
		break;
	case DOT11_AUTH_ALGO_80211_SHARED_KEY:
		szProfileXML += "shared";
		//printMess("Shared 802.11 authentication\n");
		break;
	case DOT11_AUTH_ALGO_WPA:
		szProfileXML += "WPA";
		//printMess("WPA-Enterprise 802.11 authentication\n");
		break;
	case DOT11_AUTH_ALGO_WPA_PSK:
		szProfileXML += "WPAPSK";
		//printMess("WPA-Personal 802.11 authentication\n");
		break;
	case DOT11_AUTH_ALGO_WPA_NONE:
		szProfileXML += "none";
		//printMess("WPA-NONE,not exist in MSDN\n");
		break;
	case DOT11_AUTH_ALGO_RSNA:
		szProfileXML += "WPA2";
		//printMess("WPA2-Enterprise 802.11 authentication\n");
		break;
	case DOT11_AUTH_ALGO_RSNA_PSK:
		szProfileXML += "WPA2PSK";
		//printMess("WPA2-Personal 802.11 authentication\n");
		break;
	default:
		//printMess("Unknown authentication\n");
		return false;
	}
	szProfileXML += CString("</authentication><encryption>");
	/*sprintf(p, "%d", pNet->dot11DefaultCipherAlgorithm);
	szProfileXML += string(p);*/
	switch (net->dot11DefaultCipherAlgorithm)
	{
	case DOT11_CIPHER_ALGO_NONE:
		szProfileXML += "none";
		break;
	case DOT11_CIPHER_ALGO_WEP40:
		szProfileXML += "WEP";
		break;
	case DOT11_CIPHER_ALGO_TKIP:
		szProfileXML += "TKIP";
		break;
	case DOT11_CIPHER_ALGO_CCMP:
		szProfileXML += "AES";
		break;
	case DOT11_CIPHER_ALGO_WEP104:
		szProfileXML += "WEP";
		break;
	case DOT11_CIPHER_ALGO_WEP:
		szProfileXML += "WEP";
		break;
	case DOT11_CIPHER_ALGO_WPA_USE_GROUP:
		//printMess("USE-GROUP not exist in MSDN\n");
	default:
		//printMess("Unknown encryption\n");
		return false;
	}
	//szProfileXML += string	("</encryption><useOneX>false</useOneX></authEncryption></security></MSM>");
		//������ܷ�ʽΪWEP��keyType�����ΪnetworkKey
	szProfileXML += CString("</encryption></authEncryption><sharedKey><keyType>passPhrase</keyType><protected>false</protected><keyMaterial>");
	if (net->dot11DefaultAuthAlgorithm == DOT11_AUTH_ALGO_80211_OPEN)
	{
		szProfileXML += "";
	}
	else
	{
		szProfileXML += password;   //��Ҫ���ӵ�wifi������,����protectΪfalse�����������Ϊ�������룬���Ϊture����Ҫ����
	}
	/*β*/
	szProfileXML += CString("</keyMaterial></sharedKey></security></MSM></WLANProfile>");
	/*XML����ת����˫�ֽ�*/
	USES_CONVERSION;
	wscProfileXML = A2W(szProfileXML);

	if (NULL == wscProfileXML)
	{
		//printMess("Change wscProfileXML fail\n");
		return false;
	}
	/*if (pNet->strProfileName)
	{
		strbuf.Format("���ҵ�profile Name : %ws\n",pNet->strProfileName);
		printMess(strbuf);
		WlanDeleteProfile(hClient, &pInterface->InterfaceGuid,pNet->strProfileName,NULL);
	}*/
	/*�����������*/
	dwResult = WlanSetProfile(m_wlan_client, &m_wlan_info->InterfaceGuid,
		0x00    //�����û��ļ�����
		, wscProfileXML    //�û��ļ�����
		, NULL    //��XP SP1��SP2�±���ΪNULL
		, TRUE    //�Ƿ񸲸��Ѵ��ڵ��û��ļ�
		, NULL    //��XP SP1��SP2�±���ΪNULL
		, &dwReasonCode);
	if (ERROR_SUCCESS != dwResult)
	{
		switch (dwResult)
		{
		case ERROR_INVALID_PARAMETER:    //����һ�������ġ���Ϊ�ջ���XP SP1��SP2�²�������Ϊ0
		   // printMess("����һ�������ġ���Ϊ�ջ���XP SP1��SP2�²�������Ϊ0    Para is NULL\n");
			break;
		case ERROR_NO_MATCH:    //����ӿڲ�֧�ֵļ�������
		   // printMess("����ӿڲ�֧�ֵļ�������   NIC NOT SUPPORT\n");
			break;
		case ERROR_NOT_ENOUGH_MEMORY:    //û���㹻���ڴ�ռ�
		   // printMess("û���㹻���ڴ�ռ�   Failed to allocate memory \n");
			break;
		case ERROR_BAD_PROFILE:    //�û��ļ���ʽ����
		   // printMess("�û��ļ���ʽ����   The profile specified by strProfileXml is not valid \n");
			break;
		case ERROR_ALREADY_EXISTS:    //���õ��û��ļ��Ѵ���
			//strbuf.Format("���õ��û��ļ��Ѵ���  strProfileXml specifies a network that already exists : %wd \n",dwResult);
			//printMess(strbuf); 
			break;
		case ERROR_ACCESS_DENIED:    //�û�û��Ȩ�������û��ļ�
		   // printMess("�û�û��Ȩ�������û��ļ�  The caller does not set the profile. \n");
			break;
		default:
			dwResult = GetLastError();
			//strbuf.Format("WlanSetProfile Fail�� %wd\n", dwResult);
			//printMess(strbuf); 
			break;
		}
		if (dwResult != 183)   //�������183�������õ��û����Ѵ���
		{
			return false;
		}
	}
	//printMess("set profile success\n"); 
	return true;
}


bool MyWifi::ConnectWLAN(const PWLAN_AVAILABLE_NETWORK& net)
{
	DWORD dwResult = 0;
	PWLAN_CONNECTION_PARAMETERS pConnPara = (PWLAN_CONNECTION_PARAMETERS)calloc(1, sizeof(WLAN_CONNECTION_PARAMETERS));
	/*�����������Ӳ���*/
	if (NULL == pConnPara)
	{
		//printMess("pConnPara calloc fail\n");
		return false;
	}
	pConnPara->wlanConnectionMode = (WLAN_CONNECTION_MODE)0; //XP SP2,SP3 must be 0
	pConnPara->strProfile = net->strProfileName;    //ָ�����û��ļ�
	pConnPara->pDot11Ssid = &net->dot11Ssid;    //ָ����SSID
	pConnPara->pDesiredBssidList = NULL; //XP SP2,SP3 must be NULL
	pConnPara->dot11BssType = net->dot11BssType;    //��������
	pConnPara->dwFlags = 0x00000000; //XP SP2,SP3 must be 0

	dwResult = WlanConnect(m_wlan_client    //�ͻ��˾��
		, &m_wlan_info->InterfaceGuid    //����ʹ�õĽӿڵ�GUID
		, pConnPara    //ָ��ṹ�� WLAN_CONNECTION_PARAMETERS ������ָ�����������ͣ�ģʽ������ſ���SSID ������������
		, NULL);
	if (ERROR_SUCCESS != dwResult)
	{
		switch (dwResult)
		{
		case ERROR_INVALID_PARAMETER:
			//printMess("Para is NULL\n");
			break;
		case ERROR_INVALID_HANDLE:
			//printMess("Failed to INVALID HANDLE \n");
			break;
		case ERROR_ACCESS_DENIED:
			//printMess("The caller does not have sufficient permissions. \n");
			break;
		default:
			dwResult = GetLastError();
			//strbuf.Format("WlanConnect Fail�� %wd\n", dwResult);
			//printMess(strbuf);
			break;
		}
		free(pConnPara);
		return false;
	}
	else
	{
		free(pConnPara);
		return true;
	}
}


bool MyWifi::ConnectWLANPublic(CString in_ssid) {

	PWLAN_AVAILABLE_NETWORK pBssEntry = NULL;
	for (int j = 0; j < m_net_list->dwNumberOfItems; j++) {
		pBssEntry =
			(WLAN_AVAILABLE_NETWORK *)& m_net_list->Network[j];
		CString ssid = UTF8ToCString((char*)pBssEntry->dot11Ssid.ucSSID);
		if (ssid.Compare(in_ssid) == 0) {
			CString password = "";
			SetProfile(pBssEntry, password);
			return ConnectWLAN(pBssEntry);
		}
	}
	return true;
}


bool MyWifi::ConnectWLANPrivate(CString in_ssid, CString password) {

	PWLAN_AVAILABLE_NETWORK pBssEntry = NULL;
	for (int j = 0; j < m_net_list->dwNumberOfItems; j++) {
		pBssEntry =
			(WLAN_AVAILABLE_NETWORK *)& m_net_list->Network[j];
		CString ssid = UTF8ToCString((char*)pBssEntry->dot11Ssid.ucSSID);
		if (ssid.Compare(in_ssid) == 0) {
			SetProfile(pBssEntry, password);
			return ConnectWLAN(pBssEntry);
		}
	}
	return true;
}


bool MyWifi::ConnectWLANPassWord(CString in_ssid) {

	PWLAN_AVAILABLE_NETWORK pBssEntry = NULL;
	for (int j = 0; j < m_net_list->dwNumberOfItems; j++) {
		pBssEntry =
			(WLAN_AVAILABLE_NETWORK *)& m_net_list->Network[j];
		CString ssid = UTF8ToCString((char*)pBssEntry->dot11Ssid.ucSSID);
		if (ssid.Compare(in_ssid) == 0) {
			return ConnectWLAN(pBssEntry);
		}
	}
	return true;
}


bool MyWifi::WLANDisconnect()
{
	DWORD dwResult = 0;
	dwResult = WlanDisconnect(m_wlan_client, &m_wlan_info->InterfaceGuid, NULL);
	if (ERROR_SUCCESS != dwResult)
	{
		switch (dwResult)
		{
		case ERROR_INVALID_PARAMETER:    //����һ����Ϊ��
			//printMess("Para is NULL\n");
			break;
		case ERROR_INVALID_HANDLE:
			//printMess("Failed to INVALID HANDLE \n");
			break;
		case ERROR_NOT_ENOUGH_MEMORY:
			//printMess("Failed to allocate memory \n");
			break;
		case ERROR_ACCESS_DENIED:
			//printMess("The caller does not have sufficient permissions. \n");
			break;
		default:
			dwResult = GetLastError();
			//strbuf.Format("WlanConnect Fail�� %wd\n", dwResult); 
			//printMess(strbuf);
			break;
		}
	}
	return true;
}



bool MyWifi::CloseWLANService()
{
	DWORD dwResult = 0;

	if (NULL != m_wlan_client)
	{
		dwResult = WlanCloseHandle(m_wlan_client    //Ҫ�رյ����ӵĿͻ��˾��
			, NULL);
		if (ERROR_SUCCESS != dwResult)
		{
			switch (dwResult)
			{
			case ERROR_INVALID_PARAMETER:    //����һΪ�ջ��������Ϊ��
				//printMess("Para is NULL\n");
				break;
			case ERROR_INVALID_HANDLE:
				//printMess("Failed to INVALID HANDLE \n");
				break;
			default:
				dwResult = GetLastError();
				//strbuf.Format("WlanCloseHandle Fail�� %wd\n", dwResult);
				//printMess(strbuf);
				break;
			}
			return false;
		}
		m_wlan_client = NULL;
	}

	if (NULL != m_wlan_info)
	{
		WlanFreeMemory(m_wlan_info);
		m_wlan_info = NULL;
	}

	if (NULL != m_net_list)
	{
		WlanFreeMemory(m_net_list);
		m_net_list = NULL;
	}

	if (NULL != m_net)
	{
		WlanFreeMemory(m_net);
		m_net = NULL;
	}
	return true;
}


WifiInfo MyWifi::GetCurrentWifiInfo() {

	if (GetWLANState() != WLAN_STATE_CONNECTED) {
		return {0};
	}

	DWORD dwResult = 0;
	DWORD connectInfoSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
	PWLAN_CONNECTION_ATTRIBUTES pConnectInfo = NULL;
	WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_invalid;

	dwResult = WlanQueryInterface(m_wlan_client,
		&m_wlan_info->InterfaceGuid,
		wlan_intf_opcode_current_connection,
		NULL,
		&connectInfoSize,
		(PVOID *)&pConnectInfo,
		&opCode);

	if (dwResult != ERROR_SUCCESS) {
		//wprintf(L"WlanQueryInterface failed with error: %u\n", dwResult);
		// You can use FormatMessage to find out why the function failed
		return {0};
	}
	else {

		WifiInfo wifinfo = { 0 };
		memcpy_s(wifinfo.ssid, sizeof(wifinfo.ssid), (char*)pConnectInfo->wlanAssociationAttributes.dot11Ssid.ucSSID, pConnectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength);
		wifinfo.level = pConnectInfo->wlanAssociationAttributes.wlanSignalQuality;

		return wifinfo;

	}

}


CString MyWifi::UTF8ToCString(std::string utf8str)
{
	/* Ԥת�����õ�����ռ�Ĵ�С */
	int nLen = ::MultiByteToWideChar(CP_UTF8, NULL,
		utf8str.data(), utf8str.size(), NULL, 0);
	/* ת��ΪUnicode */
	std::wstring wbuffer;
	wbuffer.resize(nLen);
	::MultiByteToWideChar(CP_UTF8, NULL, utf8str.data(), utf8str.size(),
		(LPWSTR)(wbuffer.data()), wbuffer.length());

	return(CString(wbuffer.data(), wbuffer.length()));
}
