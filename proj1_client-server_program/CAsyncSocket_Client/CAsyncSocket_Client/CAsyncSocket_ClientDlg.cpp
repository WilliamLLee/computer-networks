
// CAsyncSocket_ClientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CAsyncSocket_Client.h"
#include "CAsyncSocket_ClientDlg.h"
#include "afxdialogex.h"
 
//忽略 inet_ntoa 函数调用报错
#pragma warning(disable : 4996)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_MYMESSAGE  WM_USER+1

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


// CCAsyncSocket_ClientDlg 对话框



CCAsyncSocket_ClientDlg::CCAsyncSocket_ClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CASYNCSOCKET_CLIENT_DIALOG, pParent)
	, IP_m(_T("192.168.56.1"))
	, port_m(_T("200"))
	, command_m(_T("Date"))
	, response_m(_T(""))
	, resend_count(0)
	, resend_limit(5)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_client_port = L"2000";					 //客户端端口号
	char temp[256];
	gethostname(temp,256);      //获取客户主机名
	hostent* host = gethostbyname(temp); //客户主机IP
	m_host_name = temp;
	m_client_address = inet_ntoa(*(struct in_addr *)host->h_addr_list[0]);   //客户端IP地址转为字符串类型
}

void CCAsyncSocket_ClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, IP_ctrl);
	DDX_CBString(pDX, IDC_COMBO1, IP_m);
	DDX_CBString(pDX, IDC_COMBO3, port_m);
	DDX_CBString(pDX, IDC_COMBO2, command_m);
	DDX_Control(pDX, IDC_COMBO3, port_ctrl);
	DDX_Control(pDX, IDC_COMBO2, command_ctrl);
	DDX_Text(pDX, IDC_EDIT1, response_m);
	DDX_Text(pDX, IDC_EDIT3, m_client_address);
	DDX_Text(pDX, IDC_COMBO4, m_client_port);
	DDX_Control(pDX, IDC_COMBO4, m_client_port_ctrl);
	DDX_Control(pDX, IDC_LIST1, command_log_ctrl);
	DDX_Text(pDX, IDC_EDIT4, m_host_name);
}

BEGIN_MESSAGE_MAP(CCAsyncSocket_ClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CCAsyncSocket_ClientDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CCAsyncSocket_ClientDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CCAsyncSocket_ClientDlg::OnBnClickedButton2)
	ON_MESSAGE(WM_MYMESSAGE, &CCAsyncSocket_ClientDlg::my_send)
	ON_WM_TIMER()
END_MESSAGE_MAP()

//获取当前的日期时间，以字符串形式返回
CString CCAsyncSocket_ClientDlg::getDateTime()
{
	CTime tm; tm = CTime::GetCurrentTime();
	CString date, time;
	date.Format(L"%d/%d/%d", tm.GetYear(), tm.GetMonth(), tm.GetDay());
	time.Format(L"%d:%d:%d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());
	return date + L" " + time;
}

// CCAsyncSocket_ClientDlg 消息处理程序

BOOL CCAsyncSocket_ClientDlg::OnInitDialog()
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
	BOOL bFlag = Client.Create(_ttoi(m_client_port), SOCK_DGRAM, FD_READ|FD_WRITE);   //创建客户端套接字
	if (!bFlag)								 //错误处理
	{
		MessageBox(L"客户端Socket创建错误，请检查端口使用情况！");
		command_log_ctrl.InsertString(0, getDateTime() + L":客户端socket创建失败");
	}
	else
	{
		command_log_ctrl.InsertString(0, getDateTime() + L":客户端socket创建成功 port:" + m_client_port);
	}
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCAsyncSocket_ClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCAsyncSocket_ClientDlg::OnPaint()
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
HCURSOR CCAsyncSocket_ClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CCAsyncSocket_ClientDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (Client == INVALID_SOCKET)		//套接字处于关闭状态不能发送报文，提供报错
	{
		command_log_ctrl.InsertString(0, getDateTime() + L":客户端已经关闭，请启动客户端后重新发送");
		return;
	}

	UpdateData(true);
	//设置时钟进行计时 ，200ms重发一次，可以修改这一参数，调节超时的时间限制
	SetTimer(1, 200, NULL);         
	my_send();                      //发送请求报文

	//以下将新添加的命令或参数记录进comb box 中
	if (IP_ctrl.FindString(0, IP_m) == CB_ERR)
	{
		IP_ctrl.AddString(IP_m);
	}
	if (command_ctrl.FindString(0, command_m)== CB_ERR)
	{
		command_ctrl.AddString(command_m);
	}
	if (port_ctrl.FindString(0, port_m) == CB_ERR)
	{
		port_ctrl.AddString(port_m);
	}
	UpdateData(false);
}


void CCAsyncSocket_ClientDlg::OnBnClickedButton1()			//重置当前的socket设置
{
	// TODO: 在此添加控件通知处理程序代码
	command_log_ctrl.InsertString(0, getDateTime() + L":尝试关闭当前端口的客户端socket");
	OnBnClickedButton2();	         //先调用关闭socket函数
	UpdateData(true);
	if (m_client_port_ctrl.FindString(0, m_client_port) == CB_ERR)   //如果设置了新的端口号，记入进combox中去
	{
		m_client_port_ctrl.AddString(m_client_port);
	}
	command_log_ctrl.InsertString(0, getDateTime() + L":尝试重启客户端socket");
	BOOL bFlag = Client.Create(_ttoi(m_client_port), SOCK_DGRAM, FD_READ | FD_WRITE);   //创建客户端套接字
	if (!bFlag)   //错误处理
	{
		MessageBox(L"客户端Socket创建错误，请检查端口使用情况！");
		command_log_ctrl.InsertString(0, getDateTime() + L":客户端socket创建失败");
	}
	else
	{
		command_log_ctrl.InsertString(0, getDateTime() + L":客户端socket创建成功 port:" + m_client_port);
	}
}


void CCAsyncSocket_ClientDlg::OnBnClickedButton2()			// 关闭单当前的socket
{
	// TODO: 在此添加控件通知处理程序代码
	if (Client != INVALID_SOCKET)        
	{
		Client.Close();
		command_log_ctrl.InsertString(0, getDateTime() + L":客户端socket关闭 port:" + m_client_port);
	}
	else
	{
		command_log_ctrl.InsertString(0, getDateTime() + L":客户端socket已经关闭" );
	}
}


// 发送报文函数
LRESULT  CCAsyncSocket_ClientDlg::my_send(WPARAM wParam, LPARAM lParam)     //封装了发送报文的操作
{
	int flag = Client.SendTo(command_m.GetBuffer(), command_m.GetLength()*sizeof(TCHAR), _ttoi(port_m), IP_m);   //返回值为发送的字符串字节长度
	if (flag == -1) {
		//MessageBox(L"ERROR:发送报文出错，请检查套接字设置！");
		command_log_ctrl.InsertString(0, getDateTime() + L"ERROR:发送报文出错，请检查套接字设置！");
	}
	else {
		command_log_ctrl.InsertString(0, getDateTime() + L"向 IP:" + IP_m + L" Port:" + port_m + L"发送请求【" + command_m + L"】");
	}
	return false;
}


void CCAsyncSocket_ClientDlg::OnTimer(UINT_PTR nIDEvent)   // 超时重发的主要操作实现函数，基于计时器实现
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (resend_count >= resend_limit)		//超过一定重发次数，停止重发，关闭计时器
	{
		KillTimer(1);
		resend_count = 0;
		command_log_ctrl.InsertString(0, getDateTime() + L"ERROR:重发次数超限，发送失败，请检查相关设置！");
	}
	else {
		my_send();    //重发消息
		resend_count++;   //重发次数增加
		CString str; str.Format(L"第 %d 次重发 port:",resend_count);
		command_log_ctrl.InsertString(0, getDateTime() + str + port_m);
	}
	CDialogEx::OnTimer(nIDEvent);
}
