
// Mail_ServerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "SMTP_Socket.h"
#include "afxtempl.h"

//自定义数据结构记录邮件信息
typedef struct mail_info {
	CList<CString>  mail_text_line;	//记录每一行中的文本
	CString get_mail_text() {
		CString text;
		POSITION pos = mail_text_line.GetHeadPosition();
		for (int i = 0;POSITION( pos + i) != mail_text_line.GetTailPosition(); i++)
		{
			text += mail_text_line.GetAt(pos + i);
		}
		return text;
	}
}mail_t;

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
	//创建SMTP服务器Socket
	SMTP_Socket SMTP_Server;
	// 显示日志信息
	CListBox log_list_ctrl;
	// 记录接收到的邮件队列,mail_t为自定义的结构类型，记录收件人和发件人邮箱等信息
	CList<mail_t*> mail_list;
	afx_msg void OnDestroy();
	// 转发邮件至目标SMTP服务器的线程
	CWinThread* m_mail_sender;
};


