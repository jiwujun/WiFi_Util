
// MFC_WIFIDlg.h : header file
//

#pragma once

#include "MyWifi.h"

// CMFCWIFIDlg dialog
class CMFCWIFIDlg : public CDialogEx
{
// Construction
public:
	CMFCWIFIDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFC_WIFI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void Link();
	BOOL CopyStringToClipboard(const CString text);

protected:
	int m_index;
	BOOL m_wifi_state;
	MyWifi m_wlan;
	//MyWifi m_mywifi;

public:
	// wifi信息列表
	CListCtrl m_wifi_info;
	CImageList m_ImageList;
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnBnClickedButtonLink();
	afx_msg void OnBnClickedButtonAutoLink();
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void CopyPassword();
	afx_msg void SetWiFiInfo();
	afx_msg void SetNetListInfo();
	afx_msg void RefreshWifiInfo();
	CString UTF8ToCString(std::string utf8str);

	afx_msg void MenuLink();
	afx_msg void MenuRefresh();
};
