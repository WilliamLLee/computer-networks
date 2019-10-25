
// CAsyncSocket_ServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CAsyncSocket_Server.h"
#include "CAsyncSocket_ServerDlg.h"
#include "afxdialogex.h"

//忽略 inet_ntoa 函数调用报错
#pragma warning(disable : 4996)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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


// CCAsyncSocket_ServerDlg 对话框



CCAsyncSocket_ServerDlg::CCAsyncSocket_ServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CASYNCSOCKET_SERVER_DIALOG, pParent)
	, server_port(_T("200"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	char temp[256];
	gethostname(temp, 256);      //获取客户主机名
	hostent* host = gethostbyname(temp); //客户主机IP
	server_hostname = temp;
	server_address = inet_ntoa(*(struct in_addr *)host->h_addr_list[0]);   //客户端IP地址转为字符串类型
}

void CCAsyncSocket_ServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, list_ctrl);
	DDX_Control(pDX, IDC_COMBO1, server_port_ctrl);
	DDX_CBString(pDX, IDC_COMBO1, server_port);
	DDX_Text(pDX, IDC_EDIT1, server_hostname);
	DDX_Text(pDX, IDC_EDIT2, server_address);
}

BEGIN_MESSAGE_MAP(CCAsyncSocket_ServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CCAsyncSocket_ServerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CCAsyncSocket_ServerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CCAsyncSocket_ServerDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CCAsyncSocket_ServerDlg 消息处理程序

BOOL CCAsyncSocket_ServerDlg::OnInitDialog()
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

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCAsyncSocket_ServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCAsyncSocket_ServerDlg::OnPaint()
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
//显示。s
HCURSOR CCAsyncSocket_ServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//获取当前的日期时间，以字符串形式返回
CString getDateTime()
{
	CTime tm; tm = CTime::GetCurrentTime();
	CString date, time;
	date.Format(L"%d/%d/%d", tm.GetYear(), tm.GetMonth(), tm.GetDay());
	time.Format(L"%d:%d:%d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());
	return date + L" " + time;
}


void CCAsyncSocket_ServerDlg::OnBnClickedOk()   // 启动服务器
{
	// TODO: 在此添加控件通知处理程序代码
	if (Server != INVALID_SOCKET)
	{
		list_ctrl.InsertString(0, getDateTime() + L":服务器socket已经存在 port:" + server_port);
	}
	else {
		int flag = Server.Create(_ttoi(server_port), SOCK_DGRAM, FD_WRITE | FD_READ);
		//Server.Bind(m_port);
		if (!flag) {
			MessageBox(L"服务器启动失败，请检查socket设置！");
			list_ctrl.InsertString(0, getDateTime() + L":服务器socket创建失败 port:" + server_port);
		}
		else {
			list_ctrl.InsertString(0, getDateTime() + L":服务器socket创建成功 port:" + server_port);
		}
	}
}


void CCAsyncSocket_ServerDlg::OnBnClickedButton1()		//重置服务器
{
	// TODO: 在此添加控件通知处理程序代码
	list_ctrl.InsertString(0, getDateTime() + L":尝试关闭服务器当前端口的socket！");  //添加日志
	OnBnClickedButton2();									//尝试先关闭服务器
	UpdateData(true);											//获取数据
	if (server_port_ctrl.FindString(0,server_port) == CB_ERR)   //如果设置了新的端口号，记入进combox中去
	{
		server_port_ctrl.AddString(server_port);
	}
	list_ctrl.InsertString(0, getDateTime() + L":尝试重新创建服务器socket！");
	int flag = Server.Create(_ttoi(server_port), SOCK_DGRAM, FD_WRITE | FD_READ);
	if (!flag) {											//错误处理
		MessageBox(L"服务器侦听端口失败，请检查socket设置！");
		list_ctrl.InsertString(0, getDateTime() + L":服务器侦听端口重置失败 port:" + server_port);
	}
	else {
		list_ctrl.InsertString(0, getDateTime() + L":服务器侦听端口重置成功 port:" + server_port);
	}
}


void CCAsyncSocket_ServerDlg::OnBnClickedButton2()    //关闭服务器
{
	// TODO: 在此添加控件通知处理程序代码
	if (Server != INVALID_SOCKET)			//判断当前的套接字是否已经被关闭
	{
		Server.Close();
		list_ctrl.InsertString(0, getDateTime() + L":服务器socket关闭成功 port:" + server_port);
	}
	else
	{
		list_ctrl.InsertString(0, getDateTime() + L":服务器socket已经关闭");
	}
}
