// LinkWiFiDlg.cpp: 实现文件
//

#include "pch.h"
#include "MFC_WIFI.h"
#include "LinkWiFiDlg.h"
#include "afxdialogex.h"


// LinkWiFiDlg 对话框

IMPLEMENT_DYNAMIC(LinkWiFiDlg, CDialogEx)

LinkWiFiDlg::LinkWiFiDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

LinkWiFiDlg::~LinkWiFiDlg()
{
}

void LinkWiFiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(LinkWiFiDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_LINK_CANCEL, &LinkWiFiDlg::OnBnClickedButtonLinkCancel)
	ON_BN_CLICKED(IDC_BUTTON_LINK_OK, &LinkWiFiDlg::OnBnClickedButtonLinkOk)
END_MESSAGE_MAP()


// LinkWiFiDlg 消息处理程序


void LinkWiFiDlg::OnBnClickedButtonLinkCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	EndDialog(0);
}


void LinkWiFiDlg::OnBnClickedButtonLinkOk()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItemText(IDC_EDIT_PWD, m_password);
	EndDialog(1);
}
