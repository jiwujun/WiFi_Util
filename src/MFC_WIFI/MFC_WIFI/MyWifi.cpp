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
打开 WLAN 服务 （为后续所有操作的基础）
返回值：
	是否打开成功
*/
bool MyWifi::OpenWLANService()
{
	DWORD dwResult = 0;
	dwResult = WlanOpenHandle(WLAN_API_VERSION    //客户端支持的WLAN API的最高版本
		, NULL, &m_cur_version    //指定这次会话中将会使用的版本
		, &m_wlan_client    //指定客户端在这次会话中使用的句柄，这个句柄会贯穿整个会话被其他函数使用 。
	);
	if (ERROR_SUCCESS != dwResult)
	{
		switch (dwResult)
		{
		case ERROR_INVALID_PARAMETER:    //参数一、四为空或参数二不为空
			break;
		case ERROR_NOT_ENOUGH_MEMORY:    //没有足够的内存空间
			break;
		case ERROR_REMOTE_SESSION_LIMIT_EXCEEDED:    //超过服务器限定的句柄数量
			break;
		default:
			dwResult = GetLastError();
			//strbuf.Format("WlanOpenHandle Fail：%wd\n",dwResult);
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
获取第一块无线网卡的信息 （需要先打开 WLAN Service）（默认用户只有一块无线网卡，如果有多块无线网卡可修改此方法）（如果 WIFI 被禁用了，则无法搜索到）

返回值：
	是否获取成功
	获取网卡信息失败！\n请确认您的电脑是否包含无线网卡功能或者网卡驱动是否安装正常
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
获取网卡状态 （需要先获取网卡信息）

返回值：
	WLAN_STATE_INFO_ERROR		获取错误，获取网卡信息
	WLAN_STATE_OPEN_ERROR		执行获取函数错误

	WLAN_STATE_NOT_READY		未准备就绪

	WLAN_STATE_CONNECTED		已连接
	WLAN_STATE_NOT_CONNECTED	未连接

	WLAN_STATE_OTHER			其它状态

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
检查 WIFI功能 是否打开 （需要先获取网卡信息）

返回值：
	打开 返回 true
	没打开 返回 false
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
设置 WIFI_状态

参数：
	WIFI_SWITCH_CLOSE	关闭状态
	WIFI_SWITCH_OPEN	打开状态
*/
void MyWifi::SetWlanSwitch(int state) {
	WLAN_PHY_RADIO_STATE wifi_state; // wifi开关状态
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

			// 如果可以连接
			if (pBssEntry->bNetworkConnectable) {
				//CString ssid2 = UTF8ToCString((char*)pBssEntry->dot11Ssid.ucSSID);
				//std::wcout << ssid2 << std::endl;

				WifiInfo wifinfo = { 0 };
				wifinfo.level = pBssEntry->wlanSignalQuality;
				memcpy_s(wifinfo.ssid, sizeof(wifinfo.ssid), (char*)pBssEntry->dot11Ssid.ucSSID, pBssEntry->dot11Ssid.uSSIDLength);
				if (pBssEntry->bSecurityEnabled) {
					//没有保存过密码
					if (pBssEntry->strProfileName == NULL || wcslen(pBssEntry->strProfileName) == 0) {
						wifinfo.type = WIFI_TYPE_PRIVATE;
						m_private_wifi_info.push_back(wifinfo);
					}
					//保存过密码
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

	CString szProfileXML("");  //Profile XML流
	LPWSTR wscProfileXML = NULL;
	/*组合参数XML码流*/
	CString szTemp("");
	// char p[1024];
		/*头*/
	szProfileXML += CString("<?xml version=\"1.0\"?><WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\"><name>");
	/*name，一般与SSID相同*/
	szTemp = CString((char*)(net->dot11Ssid.ucSSID));    //配置文件名
	/*strbuf.Format("profile文件名为：%s\n",szTemp);
	MessageBox(strbuf);*/
	szProfileXML += szTemp;
	/*SSIDConfig*/
	szProfileXML += CString("</name><SSIDConfig><SSID><name>");
	szProfileXML += szTemp;    //已搜索到的网络对应的SSID
	szProfileXML += CString("</name></SSID></SSIDConfig>");
	/*connectionType*/
	szProfileXML += CString("<connectionType>");
	switch (net->dot11BssType)    //网络类型
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
		//printMess("未知的网络类型：Unknown BSS type\n");
		return false;
	}
	/*MSM*/
	szProfileXML += CString("</connectionType><MSM><security><authEncryption><authentication>");
	switch (net->dot11DefaultAuthAlgorithm)    //网络加密方式
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
		//如果加密方式为WEP，keyType必须改为networkKey
	szProfileXML += CString("</encryption></authEncryption><sharedKey><keyType>passPhrase</keyType><protected>false</protected><keyMaterial>");
	if (net->dot11DefaultAuthAlgorithm == DOT11_AUTH_ALGO_80211_OPEN)
	{
		szProfileXML += "";
	}
	else
	{
		szProfileXML += password;   //需要连接的wifi的密码,由于protect为false所以这里可以为明文输入，如果为ture则需要编码
	}
	/*尾*/
	szProfileXML += CString("</keyMaterial></sharedKey></security></MSM></WLANProfile>");
	/*XML码流转换成双字节*/
	USES_CONVERSION;
	wscProfileXML = A2W(szProfileXML);

	if (NULL == wscProfileXML)
	{
		//printMess("Change wscProfileXML fail\n");
		return false;
	}
	/*if (pNet->strProfileName)
	{
		strbuf.Format("已找到profile Name : %ws\n",pNet->strProfileName);
		printMess(strbuf);
		WlanDeleteProfile(hClient, &pInterface->InterfaceGuid,pNet->strProfileName,NULL);
	}*/
	/*设置网络参数*/
	dwResult = WlanSetProfile(m_wlan_client, &m_wlan_info->InterfaceGuid,
		0x00    //设置用户文件类型
		, wscProfileXML    //用户文件内容
		, NULL    //在XP SP1和SP2下必须为NULL
		, TRUE    //是否覆盖已存在的用户文件
		, NULL    //在XP SP1和SP2下必须为NULL
		, &dwReasonCode);
	if (ERROR_SUCCESS != dwResult)
	{
		switch (dwResult)
		{
		case ERROR_INVALID_PARAMETER:    //参数一、二、四、八为空或在XP SP1和SP2下参数三不为0
		   // printMess("参数一、二、四、八为空或在XP SP1和SP2下参数三不为0    Para is NULL\n");
			break;
		case ERROR_NO_MATCH:    //网络接口不支持的加密类型
		   // printMess("网络接口不支持的加密类型   NIC NOT SUPPORT\n");
			break;
		case ERROR_NOT_ENOUGH_MEMORY:    //没有足够的内存空间
		   // printMess("没有足够的内存空间   Failed to allocate memory \n");
			break;
		case ERROR_BAD_PROFILE:    //用户文件格式错误
		   // printMess("用户文件格式错误   The profile specified by strProfileXml is not valid \n");
			break;
		case ERROR_ALREADY_EXISTS:    //设置的用户文件已存在
			//strbuf.Format("设置的用户文件已存在  strProfileXml specifies a network that already exists : %wd \n",dwResult);
			//printMess(strbuf); 
			break;
		case ERROR_ACCESS_DENIED:    //用户没有权限设置用户文件
		   // printMess("用户没有权限设置用户文件  The caller does not set the profile. \n");
			break;
		default:
			dwResult = GetLastError();
			//strbuf.Format("WlanSetProfile Fail： %wd\n", dwResult);
			//printMess(strbuf); 
			break;
		}
		if (dwResult != 183)   //错误代码183，即设置的用户名已存在
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
	/*设置网络连接参数*/
	if (NULL == pConnPara)
	{
		//printMess("pConnPara calloc fail\n");
		return false;
	}
	pConnPara->wlanConnectionMode = (WLAN_CONNECTION_MODE)0; //XP SP2,SP3 must be 0
	pConnPara->strProfile = net->strProfileName;    //指定的用户文件
	pConnPara->pDot11Ssid = &net->dot11Ssid;    //指定的SSID
	pConnPara->pDesiredBssidList = NULL; //XP SP2,SP3 must be NULL
	pConnPara->dot11BssType = net->dot11BssType;    //网络类型
	pConnPara->dwFlags = 0x00000000; //XP SP2,SP3 must be 0

	dwResult = WlanConnect(m_wlan_client    //客户端句柄
		, &m_wlan_info->InterfaceGuid    //连接使用的接口的GUID
		, pConnPara    //指向结构体 WLAN_CONNECTION_PARAMETERS ，其中指定了连接类型，模式，网络概况，SSID 等其他参数。
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
			//strbuf.Format("WlanConnect Fail： %wd\n", dwResult);
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
		case ERROR_INVALID_PARAMETER:    //参数一、二为空
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
			//strbuf.Format("WlanConnect Fail： %wd\n", dwResult); 
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
		dwResult = WlanCloseHandle(m_wlan_client    //要关闭的连接的客户端句柄
			, NULL);
		if (ERROR_SUCCESS != dwResult)
		{
			switch (dwResult)
			{
			case ERROR_INVALID_PARAMETER:    //参数一为空或参数二不为空
				//printMess("Para is NULL\n");
				break;
			case ERROR_INVALID_HANDLE:
				//printMess("Failed to INVALID HANDLE \n");
				break;
			default:
				dwResult = GetLastError();
				//strbuf.Format("WlanCloseHandle Fail： %wd\n", dwResult);
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
	/* 预转换，得到所需空间的大小 */
	int nLen = ::MultiByteToWideChar(CP_UTF8, NULL,
		utf8str.data(), utf8str.size(), NULL, 0);
	/* 转换为Unicode */
	std::wstring wbuffer;
	wbuffer.resize(nLen);
	::MultiByteToWideChar(CP_UTF8, NULL, utf8str.data(), utf8str.size(),
		(LPWSTR)(wbuffer.data()), wbuffer.length());

	return(CString(wbuffer.data(), wbuffer.length()));
}
