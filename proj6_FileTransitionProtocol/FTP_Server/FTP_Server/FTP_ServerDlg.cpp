
// FTP_ServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "FTP_Server.h"
#include "FTP_ServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//忽略 inet_ntoa 函数调用报错
#pragma warning(disable : 4996)

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
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


// CFTP_ServerDlg 对话框



CFTP_ServerDlg::CFTP_ServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FTP_SERVER_DIALOG, pParent)
	, hostPort(_T("200"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFTP_ServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, hostIPCtrl);
	DDX_Control(pDX, IDC_EDIT1, hostPortCtrl);
	DDX_Control(pDX, IDC_EDIT3, logCtrl);
	DDX_Text(pDX, IDC_EDIT1, hostPort);
}

BEGIN_MESSAGE_MAP(CFTP_ServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_StartServer, &CFTP_ServerDlg::OnBnClickedStartserver)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CFTP_ServerDlg 消息处理程序

BOOL CFTP_ServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//初始化本机的IP地址
	hostIPCtrl.SetWindowTextA(getHostIPStr());
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CFTP_ServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFTP_ServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFTP_ServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//日志显示函数
void CFTP_ServerDlg::log(CString logInfo)
{
	CString alreadyLog,date,time;
	logCtrl.GetWindowTextA(alreadyLog);
	getDateTimeStr(date, time);
	logCtrl.SetWindowTextA(date + "-" + time + " : " + logInfo + "\r\n" + alreadyLog);
}

//启动本机的FTP控制服务进程，接收用户连接请求，进行识别判断授权信息
void CFTP_ServerDlg::OnBnClickedStartserver()
{
	// TODO: 在此添加控件通知处理程序代码
	//获取输入的客户端绑定端口
	hostPortCtrl.GetWindowTextA(hostPort);
	if (hostPort == "")
	{
		MessageBox("端口不能为空！");
		return;
	}
	UpdateData(true);
	UINT port = atoi(hostPort);
	if (port > 65535 || port < 0)
	{
		MessageBox("端口设置错误！");
		return;
	}
	//启动服务器连接管理套接字，开始接收用户端的连接请求,并将启动按钮设置为失效
	CNSocket = new ServerSocket;
	bool flag = CNSocket->Create(port, SOCK_DGRAM, FD_READ);
	if (!flag)
	{
		log("套接字创建失败！");
	}
	else
	{
		CString logText;
		logText.Format("创建UDP套接字，监听端口%d", port);
		log(logText);
	}
	//创建套接字完毕，将启动客户端按钮设置为不可用状态
	((CButton*)GetDlgItem(IDC_StartServer))->EnableWindow(false);
}

//计时器响应
void CFTP_ServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (CNSocket->sendPKT_list.GetCount() == 0)
	{
		MessageBox("计时器错误");
		return;
	}
	POSITION pos = CNSocket->sendPKT_list.GetHeadPosition();
	while (pos != NULL)
	{
		if (CNSocket->sendPKT_list.GetAt(pos)->Timer == nIDEvent)
		{
			if (CNSocket->sendPKT_list.GetAt(pos)->ResendTime >= RESENDTIMELIMIT)		//重传次数超过上限，报错，删除重传报文
			{
				CString logText;
				logText.Format("重传次数超限，删除报文：SEQ:", ((FTPHeader_t*)(CNSocket->sendPKT_list.GetAt(pos)->PktData))->SEQNO);
				log(logText);
				CNSocket->backTimerID(nIDEvent);
				CNSocket->sendPKT_list.RemoveAt(pos);
				break;
			}
			CNSocket->SendTo(CNSocket->sendPKT_list.GetAt(pos)->PktData,
				CNSocket->sendPKT_list.GetAt(pos)->len,
				CNSocket->sendPKT_list.GetAt(pos)->TargetPort,
				CNSocket->sendPKT_list.GetAt(pos)->TargetIP);
			CNSocket->sendPKT_list.GetAt(pos)->ResendTime++;
			CString logText;
			logText.Format("超时重传：SEQ:%d", ((FTPHeader_t*)(CNSocket->sendPKT_list.GetAt(pos)->PktData))->SEQNO);
			log(logText);
			break;
		}
		CNSocket->sendPKT_list.GetNext(pos);
	}
	CDialogEx::OnTimer(nIDEvent);
}
