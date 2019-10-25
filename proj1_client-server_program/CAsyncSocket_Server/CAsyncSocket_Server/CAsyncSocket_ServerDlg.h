
// CAsyncSocket_ServerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "Server_Socket.h"

// CCAsyncSocket_ServerDlg 对话框
class CCAsyncSocket_ServerDlg : public CDialogEx
{
// 构造
public:
	CCAsyncSocket_ServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CASYNCSOCKET_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListBox list_ctrl;
	Server_Socket  Server;
	// 服务器套接字端口号
	CComboBox server_port_ctrl;
	CString server_address;
	CString server_hostname;
	CString server_port;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
