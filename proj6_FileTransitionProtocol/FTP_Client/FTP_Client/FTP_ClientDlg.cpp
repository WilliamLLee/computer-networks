
// FTP_ClientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "FTP_Client.h"
#include "FTP_ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
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


// CFTP_ClientDlg 对话框



CFTP_ClientDlg::CFTP_ClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FTP_CLIENT_DIALOG, pParent)
	, hostPort(_T("20"))//客户端绑定端口号默认为20
	, serverPort(_T("200"))//服务端绑定端口号默认为200
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	logined = false;
}

void CFTP_ClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, hostIPCtrl);
	DDX_Control(pDX, IDC_EDIT8, logCtrl);
	DDX_Control(pDX, IDC_EDIT2, hostPortCtrl);
	DDX_Control(pDX, IDC_EDIT3, ServerPortCtrl);
	DDX_Control(pDX, IDC_IPADDRESS1, ServerIPCtrl);
	DDX_Control(pDX, IDC_EDIT5, userNameCtrl);
	DDX_Control(pDX, IDC_EDIT6, passwordCtrl);
	DDX_Control(pDX, IDC_LIST1, FileTreeDisplayCtrl);
	DDX_Control(pDX, IDC_EDIT10, currentPathCtrl);
	DDX_Control(pDX, IDC_EDIT11, commandCtrl);
	DDX_Control(pDX, IDC_EDIT12, argvCtrl);
	DDX_Text(pDX, IDC_EDIT2, hostPort);
	DDX_Text(pDX, IDC_EDIT3, serverPort);
}

BEGIN_MESSAGE_MAP(CFTP_ClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CFTP_ClientDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_StartClient, &CFTP_ClientDlg::OnBnClickedStartclient)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_ConnectServer, &CFTP_ClientDlg::OnBnClickedConnectserver)
	ON_BN_CLICKED(IDC_login, &CFTP_ClientDlg::OnBnClickedlogin)
	ON_BN_CLICKED(IDC_BUTTON4, &CFTP_ClientDlg::OnBnClickedButton4)
	ON_LBN_DBLCLK(IDC_LIST1, &CFTP_ClientDlg::OnLbnDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON5, &CFTP_ClientDlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// CFTP_ClientDlg 消息处理程序

BOOL CFTP_ClientDlg::OnInitDialog()
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

	//初始化本机IP
	hostIPCtrl.SetWindowTextA(getHostIPStr());

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CFTP_ClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CFTP_ClientDlg::OnPaint()
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
HCURSOR CFTP_ClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//获取本机文件路径按钮响应函数
void CFTP_ClientDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	//打开资源文件管理器，获取本机的资源文件所在位置
	CString strPath = ("");
	CFileDialog Open(TRUE, ("*.bmp"), NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		("Files (*.txt)|*.txt|JPEG Files (*.jpg)|*.jpg|All Files (*.*)|*.*||"),
		NULL); //在_T()中，可自行修改，来自定义需要打开的文件类型格式  
	if (Open.DoModal() == IDOK)
	{
		strPath = Open.GetPathName();//获得文件的全路径  
	}
	//MessageBox(strPath);
}

//启动客户端，初始化客户端环境响应函数
void CFTP_ClientDlg::OnBnClickedStartclient()
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

	//创建本机的客户端连接控制套接字
	CNSocket = new ConnectSocket;
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
	((CButton*)GetDlgItem(IDC_StartClient))->EnableWindow(false);
}
//显示文件目录树
void CFTP_ClientDlg::displayFileTree(CString files)
{
	//清除相关设置
	while (FileTreeDisplayCtrl.GetCount())
	{
		FileTreeDisplayCtrl.DeleteString(0);
	}
	currentPathCtrl.SetWindowTextA("");
	commandCtrl.SetWindowTextA("");
	argvCtrl.SetWindowTextA("");
	CNSocket->files_list.RemoveAll();

	int pos = 0;
	while (pos < files.GetLength())
	{
		int nextpos = files.Find("\r\n", pos);
		CString file;
		file = files.Mid(pos, nextpos - pos);
		pos = nextpos + 2;
		FileTreeDisplayCtrl.InsertString(FileTreeDisplayCtrl.GetCount(), file);
	}
}

//日志显示函数
void CFTP_ClientDlg::log(CString logInfo)
{
	CString alreadyLog, date, time;
	logCtrl.GetWindowTextA(alreadyLog);
	getDateTimeStr(date, time);
	logCtrl.SetWindowTextA(date + "-" + time + " : " + logInfo + "\r\n" + alreadyLog);
}

//计时器响应函数，发送缓冲区数据超时未获答复，重传报文
void CFTP_ClientDlg::OnTimer(UINT_PTR nIDEvent)
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
			CNSocket->sendPKT_list.GetAt(pos)->ResendTime++;		//计数器增加1
			CString logText;
			logText.Format("超时重传：SEQ:%d", ((FTPHeader_t*)(CNSocket->sendPKT_list.GetAt(pos)->PktData))->SEQNO);
			log(logText);
			break;
		}
		CNSocket->sendPKT_list.GetNext(pos);
	}
	CDialogEx::OnTimer(nIDEvent);
}

//连接服务器，获取服务器IP，和端口，向其发起连接请求
void CFTP_ClientDlg::OnBnClickedConnectserver()
{
	// TODO: 在此添加控件通知处理程序代码
	ServerIPCtrl.GetWindowTextA(CNSocket->Server_IP);
	//MessageBox(CNSocket->Server_IP);
	CString Sport;
	ServerPortCtrl.GetWindowTextA(Sport);
	CNSocket->Server_Port = (UINT)(atoi(Sport));
	UpdateData(true);
	//创建一个发送数据包结构体
	u_char res[256] = "HELO this is LI WEI!";
	int res_len = strlen((char*)(res));
	//更新发送序列号
	CNSocket->Seq += res_len;
	//发送请求
	sendFTPPacket(CNSocket, CNSocket->Server_IP, CNSocket->Server_Port, res, res_len, CNSocket->userID, 0, CNSocket->Seq, CNSocket->Ack, SYN);
	CNSocket->status = isConnecting;
	//日志记录 
	CString logText;
	logText.Format("Send '%s' to IP:%s Port:%d", res, CNSocket->Server_IP, CNSocket->Server_Port);
	log(logText);
	//连接服务器成功，将启动客户端按钮设置为不可用状态
	//((CButton*)GetDlgItem(IDC_StartClient))->EnableWindow(false);
}


//登录注册
void CFTP_ClientDlg::OnBnClickedlogin()
{
	// TODO: 在此添加控件通知处理程序代码
	if (CNSocket->status != isCommunicating)
	{
		MessageBox("请先连接服务器！");
		return;
	}
	CString username;
	userNameCtrl.GetWindowTextA(username);
	
	CString pass;
	passwordCtrl.GetWindowTextA(pass);

	//创建一个发送数据包结构体
	u_char res[128] = "LOGI ";
	strcat((char*)res, username+" "+pass+"\r\n");
	int res_len = strlen((const char*)res);
	//更新发送序列号
	CNSocket->Seq += res_len;
	//发送请求
	sendFTPPacket(CNSocket, CNSocket->Server_IP, CNSocket->Server_Port,(u_char*)(&res), res_len, CNSocket->userID, 0, CNSocket->Seq, CNSocket->Ack, ACK);
}

//发送命令函数
void CFTP_ClientDlg::OnBnClickedButton4()
{
	if (logined == false)
	{
		MessageBox("请先登录服务器！");
		return;
	}
	// TODO: 在此添加控件通知处理程序代码
	CString command;
	commandCtrl.GetWindowTextA(command);
	if (command == "")
	{
		MessageBox("命令输入不能为空！");
		return;
	}
	else if (command == "RETR")
	{
		CNSocket->status = isTransfer;
	}
	CString argvs;
	argvCtrl.GetWindowTextA(argvs);

	//创建一个发送数据包结构体
	u_char res[128] = "";
	strcat((char*)res,command+" "+argvs + "\r\n");
	int res_len = strlen((const char*)res);

	//更新发送序列号
	CNSocket->Seq += res_len;
	//发送请求
	sendFTPPacket(CNSocket, CNSocket->Server_IP, CNSocket->Server_Port, (u_char*)(&res), res_len, CNSocket->userID, 0, CNSocket->Seq, CNSocket->Ack, ACK);
}

//选中下载文件，将准备下载的文件加入参数列表，同时做好去重工作
void CFTP_ClientDlg::OnLbnDblclkList1()
{
	if (logined == false)
	{
		MessageBox("请先登录服务器！");
		return;
	}

	CString command;
	commandCtrl.GetWindowTextA(command);
	CString allowType;
	if (command == "LIST"||command == "MDIR")
	{
		allowType = "D";
	}
	else if (command == "RETR"||command=="STOR")
	{
		allowType = "F";
	}
	else if (command == "DELE")
	{
		allowType = "DF";
	}
	else {
		MessageBox("请先填写正确命令！");
		return;
	}

	int cursel = FileTreeDisplayCtrl.GetCurSel();
	CString fileinfo;
	FileTreeDisplayCtrl.GetText(cursel, fileinfo);
	//MessageBox(fileinfo);

	CString Type = fileinfo.Left(1);
	if (allowType != "DF")
	{
		if (Type != allowType)
		{
			MessageBox("所填写命令不支持选中文件类型！");
			return;
		}
	}

	
	int p1 = fileinfo.Find(" ", 0);
	int p2 = fileinfo.Find(" ", p1 + 1);
	CString filename = fileinfo.Mid(p1+1,p2-p1-1);
	CString length = fileinfo.Mid(p2 + 1, fileinfo.GetLength() - p2 - 1);
	//MessageBox(filename+","+length);

	POSITION pos = CNSocket->files_list.GetHeadPosition();
	while (pos != NULL)
	{
		if (CNSocket->files_list.GetAt(pos)->filename == filename)
		{
			MessageBox("文件已在参数队列中，请勿重复添加！");
			return;
		}
		CNSocket->files_list.GetNext(pos);
	}

	DownloadFile_t * d = new DownloadFile_t;
	d->DataID = CNSocket->files_list.GetCount() + 1;
	d->filename = filename;
	//d->len = atoll(length);
	d->len = 0;
	CNSocket->files_list.AddTail(d);
	CString argvs;
	argvCtrl.GetWindowTextA(argvs);
	if(argvs=="")
		argvCtrl.SetWindowTextA(filename);
	else
		argvCtrl.SetWindowTextA(argvs + "\r\n" + filename);
	// TODO: 在此添加控件通知处理程序代码
}

//重置命令
void CFTP_ClientDlg::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	commandCtrl.SetWindowTextA("");
	argvCtrl.SetWindowTextA("");
	CNSocket->files_list.RemoveAll();
}
