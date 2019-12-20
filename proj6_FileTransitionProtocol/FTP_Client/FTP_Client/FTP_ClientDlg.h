
// FTP_ClientDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "ConnectSocket.h"

// CFTP_ClientDlg 对话框
class CFTP_ClientDlg : public CDialogEx
{
// 构造
public:
	CFTP_ClientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FTP_CLIENT_DIALOG };
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
	//判断是否已经登陆
	bool logined;
	//本机的连接套接字
	ConnectSocket* CNSocket;
	// 显示本机IP地址
	CEdit hostIPCtrl;
	// 日志显示
	CEdit logCtrl;
	//输出日志信息，记录之前先输出记录的时间
	void log(CString logInfo);
	afx_msg void OnBnClickedButton1();
	// 本机绑定端口号
	CEdit hostPortCtrl;
	// 服务器连接端口
	CEdit ServerPortCtrl;
	// 服务器IP地址获取控件
	CIPAddressCtrl ServerIPCtrl;
	// 用户名获取控件
	CEdit userNameCtrl;
	// 密码获取控件
	CEdit passwordCtrl;
	// 文件树显示列表控件
	CListBox FileTreeDisplayCtrl;
	// 当前所在路径显示控件
	CEdit currentPathCtrl;
	// 命令获取控件
	CEdit commandCtrl;
	// 命令参数或选项控件
	CEdit argvCtrl;
	afx_msg void OnBnClickedStartclient();
	//显示文件目录
	void displayFileTree(CString files);
	// 客户段套接字绑定端口号
	CString hostPort;
	// 服务器连接端口号
	CString serverPort;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedConnectserver();
	afx_msg void OnBnClickedlogin();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnLbnDblclkList1();
	afx_msg void OnBnClickedButton5();
};

