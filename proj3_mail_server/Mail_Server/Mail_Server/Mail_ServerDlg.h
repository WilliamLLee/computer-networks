
// Mail_ServerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CMail_ServerDlg 对话框
class CMail_ServerDlg : public CDialogEx
{
// 构造
public:
	CMail_ServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIL_SERVER_DIALOG };
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
	// 显示邮件信息
	CEdit mail_text_ctrl;
	// 日志显示
	CEdit log_list_ctrl;
};
