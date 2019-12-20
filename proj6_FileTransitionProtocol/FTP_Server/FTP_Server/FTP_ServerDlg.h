
// FTP_ServerDlg.h : 头文件

#pragma once
#include "afxwin.h"
#include "ServerSocket.h"

// CFTP_ServerDlg 对话框
class CFTP_ServerDlg : public CDialogEx
{
// 构造
public:
	CFTP_ServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FTP_SERVER_DIALOG };
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
	//本机连接管理套接字
	ServerSocket*  CNSocket;

	// 本机IP地址显示
	CEdit hostIPCtrl;
	// 本机端口获取
	CEdit hostPortCtrl;
	// 显示日志
	CEdit logCtrl;
	//输出日志信息，记录之前先输出记录的时间
	void log(CString logInfo);

	//启动服务器控制线程
	afx_msg void OnBnClickedStartserver();
	// 本机端口设置
	CString hostPort;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};



