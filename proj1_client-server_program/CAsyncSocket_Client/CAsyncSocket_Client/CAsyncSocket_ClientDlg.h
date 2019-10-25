
// CAsyncSocket_ClientDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxsock.h"
#include "Client_Socket.h"

// CCAsyncSocket_ClientDlg 对话框
class CCAsyncSocket_ClientDlg : public CDialogEx
{
// 构造
public:
	CCAsyncSocket_ClientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CASYNCSOCKET_CLIENT_DIALOG };
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
	afx_msg void OnBnClickedOk();
	CComboBox IP_ctrl;
	CString IP_m;
	CString command_m;
	CComboBox port_ctrl;
	CComboBox command_ctrl;
	CString response_m;
	// 客户端socket
	Client_Socket Client;   
	CString m_client_port;
	CString m_client_address;
	CString port_m;
	CComboBox m_client_port_ctrl;
	CListBox command_log_ctrl;
	CString m_host_name;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	// 记录重发次数
	int resend_count;
	// 记录重发次数上限
	int resend_limit;
	// 发送报文函数
	LRESULT my_send(WPARAM wParam = 0, LPARAM lParam = 0);
	//获取时间函数
	CString getDateTime();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
