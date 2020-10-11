
// MFC_WIFIDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MFC_WIFI.h"
#include "MFC_WIFIDlg.h"
#include "afxdialogex.h"
#include "LinkWiFiDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCWIFIDlg dialog



CMFCWIFIDlg::CMFCWIFIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFC_WIFI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_LOGO);
}

void CMFCWIFIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_wifi_info);
}

BEGIN_MESSAGE_MAP(CMFCWIFIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CMFCWIFIDlg::OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_LINK, &CMFCWIFIDlg::OnBnClickedButtonLink)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_LINK, &CMFCWIFIDlg::OnBnClickedButtonAutoLink)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CMFCWIFIDlg::OnNMDblclkList1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CMFCWIFIDlg::OnNMRClickList1)
	ON_COMMAND(ID_32771, &CMFCWIFIDlg::CopyPassword)
	ON_COMMAND(ID_32772, &CMFCWIFIDlg::MenuLink)
	ON_COMMAND(ID_32773, &CMFCWIFIDlg::MenuRefresh)
END_MESSAGE_MAP()


// CMFCWIFIDlg message handlers

BOOL CMFCWIFIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	// 初始化 MyWiFI

	bool ret = true;

	ret = m_wlan.OpenWLANService();
	if (!ret) {
		MessageBox("初始失败！");
		exit(0);
	}

	ret = m_wlan.GetFirstWLANInfo();
	if (!ret) {
		MessageBox("初始失败！");
		exit(0);
	}

	m_ImageList.Create(16, 16, ILC_COLOR32, 0, 4);
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON_WIFI1));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON_WIFI2));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON_WIFI3));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON_WIFI4));

	m_wifi_info.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_wifi_info.SetImageList(&m_ImageList, LVSIL_SMALL);

	m_wifi_info.InsertColumn(0, "WIFI 名称", LVCFMT_IMAGE | LVCFMT_LEFT, 150);
	m_wifi_info.InsertColumn(1, "信号强度", LVCFMT_LEFT, 65);
	m_wifi_info.InsertColumn(2, "WIFI 网络类型 ", LVCFMT_LEFT, 200);


	if (!m_wlan.CheckWLANOpenState()) {
		if (IDYES != AfxMessageBox("您的 WiFi 功能未开启，是否要打开WiFi？", MB_ICONQUESTION | MB_YESNO)) {
			exit(0);
		}
		else
		{
			m_wlan.SetWlanSwitch(WIFI_SWITCH_OPEN);
		}
	}

	SetNetListInfo();
	SetWiFiInfo();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCWIFIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCWIFIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCWIFIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCWIFIDlg::Link()
{
	int i = m_index;
	CString in_ssid = m_wifi_info.GetItemText(i, 0);
	CString password = m_wifi_info.GetItemText(i, 2);
	bool ret;

	for (int i = 0; i < m_wlan.m_wifi_info.size(); i++)
	{
		CString ssid = UTF8ToCString(m_wlan.m_wifi_info[i].ssid);
		if (ssid.Compare(in_ssid) == 0) {
			if (m_wlan.m_wifi_info[i].type == WIFI_TYPE_PUBLIC) {
				// 公开 WiFi
				ret = m_wlan.ConnectWLANPublic(ssid);
				if (ret) {
					MessageBox("连接成功！");
					RefreshWifiInfo();
				}
				else {
					MessageBox("连接失败，请重试！");
				}
			}
			else if (m_wlan.m_wifi_info[i].type == WIFI_TYPE_PASSWORD)
			{
				// 私密_已记录密码
				ret = m_wlan.ConnectWLANPassWord(ssid);
				if (ret) {
					MessageBox("连接成功！");
					RefreshWifiInfo();
				}
				else {
					MessageBox("连接失败，请重试！");
				}
			}
			else
			{
				// 私密
				LinkWiFiDlg link_dlg;
				ret = link_dlg.DoModal();
				if (ret) {
					ret = m_wlan.ConnectWLANPrivate(ssid, link_dlg.m_password);
					if (ret) {
						MessageBox("连接成功！");
						RefreshWifiInfo();
					}
					else {
						MessageBox("密码错误！");
					}
				}
			}
			return;
		}
	}

}


// 刷新列表
void CMFCWIFIDlg::OnBnClickedButtonRefresh()
{
	RefreshWifiInfo();
}


// 连接 wifi
void CMFCWIFIDlg::OnBnClickedButtonLink()
{
	m_index = m_wifi_info.GetSelectionMark();
	Link();
}

#include <algorithm>

bool sortFun(const  WifiInfo&p1, const WifiInfo&p2)
{
	return p1.level > p2.level;//降序排列  
}


//自动连接 （先连接记住过密码的 私有 wifi，然后再连接记住过密码的 共有 wifi）
void CMFCWIFIDlg::OnBnClickedButtonAutoLink()
{
	bool ret;
	std::sort(m_wlan.m_password_wifi_info.begin(), m_wlan.m_password_wifi_info.end(), sortFun);
	for (int i = 0; i < m_wlan.m_password_wifi_info.size(); i++)
	{
		CString ssid = UTF8ToCString(m_wlan.m_password_wifi_info[i].ssid);
		ret = m_wlan.ConnectWLANPassWord(ssid);
		if (ret) {
			// 等待 WiFi 连接上
			Sleep(1000);
			RefreshWifiInfo();
			return;
		}
	}

	std::sort(m_wlan.m_public_wifi_info.begin(), m_wlan.m_public_wifi_info.end(), sortFun);
	for (int i = 0; i < m_wlan.m_public_wifi_info.size(); i++)
	{
		CString ssid = UTF8ToCString(m_wlan.m_public_wifi_info[i].ssid);
		ret = m_wlan.ConnectWLANPublic(ssid);
		if (ret) {
			// 等待 WiFi 连接上
			Sleep(1000);
			RefreshWifiInfo();
			return;
		}
	}

	MessageBox("自动连接失败！\n请手动进行连接！");

}


// 双击也为连接 wifi
void CMFCWIFIDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	m_index = pNMItemActivate->iItem;
	Link();
	*pResult = 0;
}


void CMFCWIFIDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1)
	{
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		CMenu menu;
		//添加线程操作
		m_index = m_wifi_info.GetSelectionMark();
		VERIFY(menu.LoadMenu(IDR_MENU1));			//这里是我们在1中定义的MENU的文件名称
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT(popup != NULL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

	}


}


// 复制 密码
void CMFCWIFIDlg::CopyPassword()
{
	CString password = m_wifi_info.GetItemText(m_index, 2);
	CopyStringToClipboard(password);
}


// 右键 连接
void CMFCWIFIDlg::MenuLink()
{
	Link();
}


// 右键 刷新列表
void CMFCWIFIDlg::MenuRefresh()
{
	RefreshWifiInfo();
}

// 复制内容到剪切板
BOOL CMFCWIFIDlg::CopyStringToClipboard(const CString text)
{
	if (!::OpenClipboard(NULL))
	{
		return FALSE;
	}

	CString src = text;
	::EmptyClipboard();
	int len = src.GetLength();
	int size = (len + 1) * 2;
	HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE, size);
	if (!clipbuffer)
	{
		::CloseClipboard();
		return FALSE;
	}
	char *buffer = (char*)::GlobalLock(clipbuffer);
	memcpy(buffer, src.GetBuffer(), size);
	src.ReleaseBuffer();
	::GlobalUnlock(clipbuffer);
	::SetClipboardData(CF_UNICODETEXT, clipbuffer);
	::CloseClipboard();
	return TRUE;
}


CString CMFCWIFIDlg::UTF8ToCString(std::string utf8str)
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


void CMFCWIFIDlg::SetWiFiInfo() {
	// 当前连接的WiFi：test\t\t信号：强
	WifiInfo wifi_info = m_wlan.GetCurrentWifiInfo();
	CString str_info;
	if (wifi_info.level == 0) {
		str_info = "当前没有连接 WiFi";
	}
	else
	{
		CString str_level;
		str_level.Format("%d", wifi_info.level);
		str_info = "当前连接的WiFi：" + UTF8ToCString(wifi_info.ssid) + "\t信号：" + str_level;
	}

	SetDlgItemText(IDC_STATIC_INFO, str_info);
}


void CMFCWIFIDlg::SetNetListInfo() {
	m_wlan.GetNetListInfo();
	for (int i = 0; i < m_wlan.m_wifi_info.size(); i++)
	{
		CString ssid = UTF8ToCString(m_wlan.m_wifi_info[i].ssid);
		CString level;
		level.Format("%d", m_wlan.m_wifi_info[i].level);
		CString type;

		if (m_wlan.m_wifi_info[i].type == WIFI_TYPE_PUBLIC) {
			type = "公开";
		}
		else if (m_wlan.m_wifi_info[i].type == WIFI_TYPE_PASSWORD)
		{
			type = "私密_已记录密码";
		}
		else
		{
			type = "私密";
		}

		// 不同强度的 wifi 显示不同的图标
		int temp = 0;

		if (m_wlan.m_wifi_info[i].level >  80)
		{
			temp = 3;
		}
		else if (m_wlan.m_wifi_info[i].level > 60)
		{
			temp = 2;
		}
		else if (m_wlan.m_wifi_info[i].level > 40)
		{
			temp = 1;
		}

		m_wifi_info.InsertItem(i, ssid, temp);
		m_wifi_info.SetItemText(i, 1, level);
		m_wifi_info.SetItemText(i, 2, type);
	}
}

void CMFCWIFIDlg::RefreshWifiInfo() {
	if (!m_wlan.CheckWLANOpenState()) {
		if (IDYES != AfxMessageBox("您的 WiFi 功能未开启，是否要打开WiFi？", MB_ICONQUESTION | MB_YESNO)) {
			exit(0);
		}
		else
		{
			m_wlan.SetWlanSwitch(WIFI_SWITCH_OPEN);
		}
	}

	int ret = m_wlan.GetFirstWLANInfo();
	if (!ret) {
		MessageBox("程序运行异常！");
		exit(0);
	}

	m_wifi_info.DeleteAllItems();

	SetNetListInfo();
	SetWiFiInfo();
}