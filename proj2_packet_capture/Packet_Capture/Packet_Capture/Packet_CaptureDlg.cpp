
// Packet_CaptureDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Packet_Capture.h"
#include "Packet_CaptureDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//忽略localtime函数的版本报错
#pragma warning (disable: 4996)

//自定义一个消息
#define WM_PACKET WM_USER+1

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


// CPacket_CaptureDlg 对话框



CPacket_CaptureDlg::CPacket_CaptureDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PACKET_CAPTURE_DIALOG, pParent)
	, alldevs(NULL)
	, m_capture(NULL)
	, stop_thread(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPacket_CaptureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, EtherNet_interface_ctrl);
	DDX_Control(pDX, IDC_LIST2, interface_detail_ctrl);
	DDX_Control(pDX, IDC_EDIT1, condition_ctrl);
	DDX_Control(pDX, IDC_LIST3, packet_list_ctrl);
}

BEGIN_MESSAGE_MAP(CPacket_CaptureDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_LBN_DBLCLK(IDC_LIST1, &CPacket_CaptureDlg::OnLbnDblclkList1)
	ON_BN_CLICKED(IDCANCEL, &CPacket_CaptureDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CPacket_CaptureDlg::OnBnClickedButton1)
	ON_MESSAGE(WM_PACKET, &CPacket_CaptureDlg::OnPacket)  
	ON_BN_CLICKED(IDC_BUTTON2, &CPacket_CaptureDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CPacket_CaptureDlg 消息处理程序

BOOL CPacket_CaptureDlg::OnInitDialog()
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

	/*
	* 获取当前主机的设备列表，基于pcap_findalldevs_ex()函数  
	* 将返回的设备列表指针赋给类类中定义的alldevs变量 
	*/
	int flag = pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf);		//获取设备列表
	if (flag == -1)						//错误处理
	{
		MessageBox(CString(errbuf), L"ERROR", MB_OKCANCEL | MB_ICONERROR);
	}
	else
	{
		//显示设备接口列表
		for (d = alldevs; d != NULL; d = d->next)
		{
			EtherNet_interface_ctrl.InsertString(0, (CString)d->name);
		}
	}


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPacket_CaptureDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPacket_CaptureDlg::OnPaint()
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
HCURSOR CPacket_CaptureDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//双击本机设备列表项触发本函数，在接口详细信息中显示相应设备接口的详细信息
void CPacket_CaptureDlg::OnLbnDblclkList1()			//显示设备接口详细信息
{
	// TODO: 在此添加控件通知处理程序代码
	//清空之前详细信息栏中的信息
	while (interface_detail_ctrl.GetCount())
		interface_detail_ctrl.DeleteString(0);
	//获取选中的设备接口名称
	int sno = EtherNet_interface_ctrl.GetCurSel();
	CString sitem;
	EtherNet_interface_ctrl.GetText(sno,sitem);
	//将设备接口名写入详细信息栏
	interface_detail_ctrl.InsertString(0, sitem);
	//显示设备接口详细信息
	for (d = alldevs; d != NULL; d = d->next)
		if ((CString)(d->name) == sitem)
		{
			interface_detail_ctrl.InsertString(interface_detail_ctrl.GetCount(), (CString)d->description);
			break;
		}
}

//点解返回按钮触发本函数，释放设备列表后退出程序
void CPacket_CaptureDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码

	//释放设备列表
	pcap_freealldevs(alldevs);
	//如果线程未关闭则关闭线程
	stop_thread = 1;				//重置参数为1
	CDialogEx::OnCancel();
}

//线程执行函数，进行数据包的捕获工作
UINT Capturer(PVOID hwnd) {
	pcap_pkthdr* pkt_header = NULL;		//记录捕获的数据包头
	const u_char* pkt_data = NULL;			//记录捕获的数据包
	CPacket_CaptureDlg* Dlg = (CPacket_CaptureDlg*)AfxGetApp()->m_pMainWnd;  //获取主窗口句柄
	while (!Dlg->stop_thread)			//循环捕获网络数据包，通过参数stop_thread 控制停止捕获
	{
		//捕获数据包的主体代码，利用pcap_next_ex函数执行
		int flag = 0;
		TRY{
			flag = pcap_next_ex(Dlg->opened_pcap,	//pcap_t指针，说明捕获那个网卡上的数据包
				&pkt_header,						//pcap_pkthdr 指针，记录数据包相关信息
				&pkt_data							//uchar*指针，保存数据包数据
				);
		}CATCH(CException, e)
		{
			TCHAR error[1024](L"\0");
			e->GetErrorMessage(error, 1024);
			Dlg->packet_list_ctrl.InsertString(Dlg->packet_list_ctrl.GetCount(), L"$$$$$捕获异常"+(CString)error);
			memset(error, 0, 1024);
			continue;
		}END_CATCH;
		//成功捕获数据,并对捕获的函数进行分类
		if (flag == 1)		
		{
			Dlg->pkthdr_list.AddTail(pkt_header);
			Dlg->pktdata_list.AddTail(pkt_data);
			Dlg->PostMessageW(WM_PACKET, 0, 0);
		}
		else if (flag == 0)
		{
			//Dlg->MessageBox(L"未在规定时间内捕获数据包");
		}
		else if (flag == -1)
		{
			Dlg->MessageBox(L"数据报捕获函数执行错误！");
		}
	}
	Dlg->stop_thread = 0;				//重置参数为0
	Dlg->packet_list_ctrl.InsertString(Dlg->packet_list_ctrl.GetCount(), L"捕获数据包线程停止！");
	Dlg->packet_list_ctrl.InsertString(Dlg->packet_list_ctrl.GetCount(), L"_______________________________________________________________________________________________________________________");;		//提示停止捕获数据包
	return 1;
}

//点击捕获数据包按钮触发本函数
//从设备详细信息窗口获取名字信息然后打开相应网络设备接口
//创建数据包捕获线程
void CPacket_CaptureDlg::OnBnClickedButton1()			
{
	// TODO: 在此添加控件通知处理程序代码
	if (interface_detail_ctrl.GetCount() == 0)			//未选择设备接口，退出函数，进行错误提示
	{
		MessageBox(L"未选择设备接口，请双击设备接口列表选项进行选择！",L"WARNING", MB_OKCANCEL | MB_ICONWARNING);
		return;
	}
	//清空之前数据包信息栏中的信息
	while (packet_list_ctrl.GetCount())
		packet_list_ctrl.DeleteString(0);
	//打开设备接口
	opened_pcap = pcap_open(d->name,			//设备接口名，直接从类成员d指针中获取，一旦选择之后，d指针指向选择的设备接口
						4096,				//获取数据包的最大长度
						PCAP_OPENFLAG_PROMISCUOUS,	//打开设备接口获取网络数据包方式，改参数为混杂模式，获取所有流经该网络接口的数据包
						1000,				//等待一个数据包的最大时间
						NULL,				//远程设备捕获网络数据包使用，本实验只需设置为NULL
						errbuf);			//错误信息缓冲区
	if (opened_pcap == NULL)		//错误处理
		MessageBox(CString(errbuf),L"ERROR", MB_OKCANCEL | MB_ICONERROR);
	else {
		//设备打开成功，创建线程进行数据包捕获,返回一个线程指针
		m_capture = AfxBeginThread(Capturer,	//工作者线程的控制函数
									NULL,		//传给控制函数的参数，一般为某数据结构的指针，这里为空
								THREAD_PRIORITY_NORMAL);	//线程优先级，默认为正常的优先级
		//成功创建线程之后将按钮设置为不可点击状态
		GetDlgItem(IDC_BUTTON1)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON2)->EnableWindow(true);
		//输出日志信息
		packet_list_ctrl.InsertString(packet_list_ctrl.GetCount(), L"监听  " + (CString)d->description);
		packet_list_ctrl.InsertString(packet_list_ctrl.GetCount(), L"_______________________________________________________________________________________________________________________");
		//MessageBox(CString(d->name));
	}
}


// 处理捕获数据包的函数,对数据包进行解析
LRESULT CPacket_CaptureDlg::OnPacket(WPARAM wParam, LPARAM lParam)
{
	TRY{
	//从缓冲队列中提取数据报文
	pcap_pkthdr* pkt_header = pkthdr_list.GetHead();		//记录捕获的数据包头
	const u_char* pkt_data = pktdata_list.GetHead();			//记录捕获的数据包
	//获取帧头部保存的时间戳数据和类型数据
	CString   header_s,data_s;
	char timebuf[128];
	time_t t(pkt_header->ts.tv_sec);
	strftime(timebuf, 64, "%Y-%m-%d %H:%M:%S", localtime(&t));
	//数据格式化
	header_s.Format(L"%s.%d,len:%d",CString(timebuf),pkt_header->ts.tv_usec,pkt_header->caplen);
	FrameHeader_t* FHeader = (FrameHeader_t*)pkt_data;
	//过滤协议报文
	if(ntohs(FHeader->FrameType) == WORD(0x0800))			//ntohs(FHeader->FrameType) == WORD(0x0800)
	{
		Data_t* Data = (Data_t*)pkt_data;
		//显示时间数据报头部信息
		packet_list_ctrl.InsertString(packet_list_ctrl.GetCount(), header_s);
		//数据格式化
		data_s.Format(L"标识:0x%04x           头部校验和:0x%04x           计算出的头部校验和:0x%04x", ntohs(Data->IPHeader.ID), ntohs(Data->IPHeader.Checksum), IPHeader_ckeck((WORD*)(&Data->IPHeader)));
		//数据阵头部校验信息等
		packet_list_ctrl.InsertString(packet_list_ctrl.GetCount(), data_s);
		packet_list_ctrl.InsertString(packet_list_ctrl.GetCount(), L"_______________________________________________________________________________________________________________________");
		/*if (ntohs(Data->IPHeader.Checksum) != IPHeader_ckeck((WORD*)(&Data->IPHeader)))
		{
			packet_list_ctrl.InsertString(packet_list_ctrl.GetCount(), L"##################数据报错误!########################");
		}*/
	}

	pkthdr_list.RemoveHead();							//移除头部数据包
	pktdata_list.RemoveHead();

	}CATCH(CException, e)
	{
		TCHAR error[1024](L"\0");
		e->GetErrorMessage(error,1024);
		packet_list_ctrl.InsertString(packet_list_ctrl.GetCount(), L"$$$$$处理异常"+(CString)error);
		memset(error, 0, 1024);
	}END_CATCH;
	//MessageBox(L"这里是分析函数！");
	return LRESULT();
}

//停止捕获数据包，终止捕获数据包的线程
//重置相关的变量，将捕获数据包的按钮恢复
void CPacket_CaptureDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	stop_thread = 1;
	//恢复捕获数据包按钮的正常状态
	GetDlgItem(IDC_BUTTON1)->EnableWindow(true);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(false);
}


// IP头部检验算法
WORD CPacket_CaptureDlg::IPHeader_ckeck(WORD* IPHeader)
{
	WORD ans = 0;
	CString str;
	for (int i = 0; i < 10; i++)
	{
		if (i == 5) continue;			//跳过检验和字
		if ((ans + IPHeader[i])%0x10000 < ans)
			ans = (ans + IPHeader[i]) % 0x10000 + 1;
		else
			ans += IPHeader[i];
		//str.Format(L"%08x,%08x,%08x,%d", ans, IPHeader[i], ans + IPHeader[i], ans + IPHeader[i]<ans);
		//packet_list_ctrl.InsertString(packet_list_ctrl.GetCount(),str);
	}
	return WORD(ntohs(~ans));
}
