#pragma once


// LinkWiFiDlg 对话框

class LinkWiFiDlg : public CDialogEx
{
	DECLARE_DYNAMIC(LinkWiFiDlg)

public:
	LinkWiFiDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~LinkWiFiDlg();

	CString m_password;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonLinkCancel();
	afx_msg void OnBnClickedButtonLinkOk();
};
