
// MAC_IPDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MAC_IP.h"
#include "MAC_IPDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//自定义一个消息处理解析报文
#define WM_PACKET WM_USER+1
//定义触发获取本机IPMAC地址并显示接口信息的消息
#define WM_DISPLAY WM_USER+2


//忽略localtime函数的版本报错
#pragma warning (disable: 4996)

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


// CMAC_IPDlg 对话框



CMAC_IPDlg::CMAC_IPDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MAC_IP_DIALOG, pParent)
	, alldevs(NULL)
	, d(NULL)
	, a(NULL)
	, opened_pcap(NULL)
	, m_capture(NULL)
	, stop_thread(0)
	, ARP_Frame(NULL)
	, host_MAC(NULL)
	, waiter(0)
	, interface_info(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMAC_IPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, EtherNet_interface_ctrl);
	DDX_Control(pDX, IDC_EDIT1, interface_detail_ctrl);
	DDX_Control(pDX, IDC_EDIT2, IP_MAC_ctrl);
	DDX_Control(pDX, IDC_IPADDRESS2, IPAddr_ctrl);
}

BEGIN_MESSAGE_MAP(CMAC_IPDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_GETIP_MAC, &CMAC_IPDlg::OnBnClickedGetipMac)
	ON_LBN_DBLCLK(IDC_LIST1, &CMAC_IPDlg::OnLbnDblclkList1)
	ON_MESSAGE(WM_PACKET, &CMAC_IPDlg::OnPacket)
	ON_BN_CLICKED(IDCANCEL, &CMAC_IPDlg::OnBnClickedCancel)
	ON_MESSAGE(WM_DISPLAY, &CMAC_IPDlg::OnDisplayInfo)
END_MESSAGE_MAP()


// CMAC_IPDlg 消息处理程序

BOOL CMAC_IPDlg::OnInitDialog()
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
		MessageBox(CString(errbuf), "ERROR", MB_OKCANCEL | MB_ICONERROR);
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

void CMAC_IPDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMAC_IPDlg::OnPaint()
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
HCURSOR CMAC_IPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//获取IP_MAC 地址映射关系
void CMAC_IPDlg::OnBnClickedGetipMac()
{
	// TODO: 在此添加控件通知处理程序代码
	DWORD IP;
	IPAddr_ctrl.GetAddress(IP);   //获取输入的IP地址
	if (opened_pcap != NULL)
	{
		mk_ARPFrame(host_MAC, GetAddr_IP(a->addr), NULL, ntohl(IP), host_MAC);  //发送获取该IP地址映射关系的ARP报文
	}
	else {
		MessageBox("未选择设备接口！");
	}
}

// 将14字节地址协议地址转为4字节IP地址类型
DWORD CMAC_IPDlg::GetAddr_IP(sockaddr* addr)
{
	if (addr == NULL)
		return 0x0;
	else
	{
		DWORD* address = (DWORD *)((addr->sa_data)+2);
		return *address;
	}
	return 0;
}

LRESULT CMAC_IPDlg::OnDisplayInfo(WPARAM wParam, LPARAM lParam)
{
	if (host_MAC == NULL)
	{
		stop_thread = 1;
		MessageBox("获取本机IP_MAC映射失败！");
		return LRESULT();
	}
	//格式化MAC地址数据
	CString hostmac;
	hostmac.Format("%02x-%02x-%02x-%02x-%02x-%02x\r\n", host_MAC[0], host_MAC[1], host_MAC[2], host_MAC[3], host_MAC[4], host_MAC[5]);
	//MessageBox(hostmac);
	//将MAC地址插入对应的显示位置并显示接口信息
	interface_info.Insert(interface_info.Find("\r\n", interface_info.Find("\r\n") + 2) + 2, hostmac);
	interface_detail_ctrl.SetWindowTextA(interface_info);   
	UpdateData(true);
	return LRESULT();
}

//双击设备列表项，选择特定列表的信息进行显示，
//同时打开选中的设备接口，并在此接口创建接收报文线程
//线程接收该网卡上的所有报文，
//经过筛选之后将ARP报文帧存储报文缓冲区并发送消息调用处理函数OnPacket进行解析
void CMAC_IPDlg::OnLbnDblclkList1()
{
	// TODO: 在此添加控件通知处理程序代码
	//打开新的设备接口，无论是否有线程正在运行，都将stop_thread参数置位为1
	stop_thread = 1;
	//清空设备接口详情显示窗口
	interface_detail_ctrl.SetWindowTextA("");
	//获取选中的设备接口名称
	int sno = EtherNet_interface_ctrl.GetCurSel();
	//创建CString变量记录接口详细信息，以及记录设备接口名
	CString  sitem;
	//获取设备接口名
	EtherNet_interface_ctrl.GetText(sno, sitem);
	//通过选中的接口名获得设备接口的地址，赋值给了类成员d，
	//之后可以通过d获取该接口信息，并将设备接口描述和各类地址显示
	for (d = alldevs; d != NULL; d = d->next)
		if ((CString)(d->name) == sitem)
			break;
	//将信息输出
	sitem += "\r\n";   //换行
	sitem += (CString)d->description;  //设备描述
	//获取该设备的IP地址信息并显示
	for (a = d->addresses; a != NULL; a = a->next)
	{
		if (a->addr->sa_family == AF_INET)     // 判断改地址是否为IP地址
		{
			sitem += "\r\n";   //换行
			sitem += convert_addr_to_str(a->addr);  //获取IP地址
			sitem += "\r\n";   //换行
			sitem += convert_addr_to_str(a->netmask); //获取网络掩码
			sitem += "\r\n";   //换行
			sitem += convert_addr_to_str(a->broadaddr); //获取广播地址
			sitem += "\r\n";   //换行
			sitem += convert_addr_to_str(a->dstaddr); //获取目的地址
			break;
		}
	}
	if (a == NULL) {
		MessageBox("该设备接口没有IP地址，请选择新的设备！");
		return;
	}
	interface_info = sitem;       //将记录的部分信息保存在类变量中
	//打开设备接口
	opened_pcap = pcap_open(d->name,			//设备接口名，直接从类成员d指针中获取，一旦选择之后，d指针指向选择的设备接口
		4096,				//获取数据包的最大长度
		PCAP_OPENFLAG_PROMISCUOUS,	//打开设备接口获取网络数据包方式，改参数为混杂模式，获取所有流经该网络接口的数据包
		1000,				//等待一个数据包的最大时间
		NULL,				//远程设备捕获网络数据包使用，本实验只需设置为NULL
		errbuf);			//错误信息缓冲区
	if (opened_pcap == NULL)		//错误处理
		MessageBox(CString(errbuf), "ERROR", MB_OKCANCEL | MB_ICONERROR);
	else {
		//创建新的线程之前，需要先将原来的线程关闭,并将线程控制参数stop_thread置位为0
		stop_thread =0;
		//设备打开成功，创建线程进行数据包捕获,返回一个线程指针
		m_capture = AfxBeginThread(Capturer,	//工作者线程的控制函数
			NULL,		                //传给控制函数的参数，一般为某数据结构的指针，这里为空
			THREAD_PRIORITY_NORMAL);	//线程优先级，默认为正常的优先级
										//成功创建线程之后将按钮设置为不可点击状态
		//MessageBox("数据包捕获线程创建成功！");
	}
	//通过对本机发送一个ARP请求获取本机的IPMAC映射关系
	BYTE SendHa_t[6]{ 0x66,0x66,0x66,0x66,0x66,0x66 };  //虚假源MAC地址 66-66-66-66-66-66
	DWORD SendIP_t = 0x70707070;				//虚假源IP地址 112.112.112.112
	DWORD RecvIP_t = GetAddr_IP(a->addr);			//本机IP地址作为目的地址
													//CString str;
													//str.Format("0x%08x.0x%08x.0x%02x", SendIP_t, RecvIP_t,SendHa_t[0]);
													//MessageBox(str);
	//CString str;
	//str.Format("%08x,%08x", GetAddr_IP(a->addr), GetAddr_IP(a->addr));
	//MessageBox("接收到ARP数据报文" + str);
	mk_ARPFrame(SendHa_t, SendIP_t, NULL, RecvIP_t, SendHa_t);   //发送一个ARP 报文	
}


// 将地址转换为字符串返回
CString CMAC_IPDlg::convert_addr_to_str(sockaddr* addr)
{
	if (addr == NULL)
		return "NULL";
	else
	{
		CString str;
		unsigned char * temp = (unsigned char *)addr->sa_data;
		str.Format("%d.%d.%d.%d",  temp[2], temp[3], temp[4], temp[5]);
		return str;
	}
	return CString();
}

//线程执行函数，进行数据包的捕获工作
UINT Capturer(PVOID hwnd) {
	pcap_pkthdr* pkt_header = NULL;							//记录捕获的数据包头
	const u_char* pkt_data = NULL;			//记录捕获的数据包
	CMAC_IPDlg* Dlg = (CMAC_IPDlg*)AfxGetApp()->m_pMainWnd;  //获取主窗口句柄
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
			TCHAR error[1024]("\0");
			e->GetErrorMessage(error, 1024);
			Dlg->MessageBox( "$$$$$捕获异常" + (CString)error);
			memset(error, 0, 1024);
			continue;
		}END_CATCH;
		//成功捕获数据,并对捕获的函数进行分类
		if (flag == 1)
		{
			FrameHeader_t* FHeader = (FrameHeader_t*)pkt_data;
			if (ntohs(FHeader->FrameType) == WORD(0x0806))//接收到ARP报文
			{
				ARPFrame_t* ARP = (ARPFrame_t*)pkt_data;
				if (ntohs(ARP->Openration) == 2)     //接收到ARP应答报文
				{
					/*CString str;
					str.Format("%08x,%08x", ARP->SendIP, Dlg->GetAddr_IP(Dlg->a->addr));
					Dlg->MessageBox("接收到ARP数据报文" + str);*/
					if (ARP->SendIP == Dlg->GetAddr_IP(Dlg->a->addr))  //如果获取的ARP响应来自本机，则可通过该ARP响应获取本机的MAC地址
					{
						Dlg->host_MAC = ARP->SendHa;
						//CString str;
						//str.Format("%02x-%02x-%02x-%02x-%02x-%02x", Dlg->host_MAC[0]
						//	, Dlg->host_MAC[1]
						//	, Dlg->host_MAC[2]
						//	, Dlg->host_MAC[3]
						//	, Dlg->host_MAC[4]
						//	, Dlg->host_MAC[5]);
						// Dlg->MessageBox(str);
						Dlg->PostMessageA(WM_DISPLAY, 0, 0);  //发送消息显示本机接口信息包含IPMAC映射关系
						
						//Dlg->pkthdr_list.AddTail(pkt_header);
						//Dlg->pktdata_list.AddTail(pkt_data);     //将报文传入缓冲区
						//Dlg->PostMessageA(WM_PACKET, 0, 0);
					}
					else {
						//Dlg->MessageBox("接收到非本机发送的ARP报文");
						Dlg->pkthdr_list.AddTail(pkt_header);   
						Dlg->pktdata_list.AddTail(pkt_data);     //将报文传入缓冲区
						Dlg->PostMessageA(WM_PACKET, 0, 0);      //发送报文接收成功消息触发报文解析函数  
					}
				}
			}
		}
		else if (flag == 0)
		{
			//Dlg->MessageBox(L"未在规定时间内捕获数据包");
		}
		else if (flag == -1)
		{
			Dlg->MessageBox("数据报捕获函数执行错误！");
		}
	}
	Dlg->stop_thread = 0;				//重置参数为0
	Dlg->MessageBox( "捕获数据包线程停止！");
	return 0;
}

// 处理捕获数据包的函数,对数据包进行解析
LRESULT CMAC_IPDlg::OnPacket(WPARAM wParam, LPARAM lParam)
{
	TRY{
	//从缓冲队列中提取数据报文
	pcap_pkthdr* pkt_header = pkthdr_list.GetHead();		//记录捕获的数据包头
	const u_char* pkt_data = pktdata_list.GetHead();			//记录捕获的数据包
																//获取帧头部保存的时间戳数据和类型数据
	//处理ARP报文，解析IP地址和MAC地址的映射关系并显示在IPMAC显示窗口
	ARPFrame_t*  ARP = (ARPFrame_t*)pkt_data;
	unsigned char* IP = (unsigned char*)(&ARP->SendIP);
	BYTE* MAC = ARP->SendHa;
	CString IP_str,MAC_str;
	//格式化
	IP_str.Format("%d.%d.%d.%d",IP[0],IP[1],IP[2],IP[3]); 
	//MessageBox(IP_str);
	MAC_str.Format(" %02x-%02x-%02x-%02x-%02x-%02x\r\n",MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
	//MessageBox(MAC_str);
	CString IP_MAC;
	IP_MAC_ctrl.GetWindowTextA(IP_MAC);
	IP_MAC_ctrl.SetWindowTextA(IP_MAC + IP_str+"-->"+MAC_str);

	pkthdr_list.RemoveHead();							//移除头部数据包
	pktdata_list.RemoveHead();

	}CATCH(CException, e)
	{
		TCHAR error[1024]("\0");
		e->GetErrorMessage(error, 1024);
		MessageBox("$$$$$处理异常" + (CString)error);
		memset(error, 0, 1024);
	}END_CATCH;
	//MessageBox(L"这里是分析函数！");
	return LRESULT();
}

void CMAC_IPDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	
	//释放设备列表
	pcap_freealldevs(alldevs);
	//如果线程未关闭则关闭线程
	stop_thread = 1;				//重置参数为1

	CDialogEx::OnCancel();
}



// 传入参数，制作ARP数据包，并将数据包发送
ARPFrame_t* CMAC_IPDlg::mk_ARPFrame(BYTE* SendHa_t, DWORD SendIP_t, BYTE* RecvHa_t, DWORD RecvIP_t, BYTE* SrcMAC_t, BYTE* DesMAC_t)
{
	ARP_Frame = new ARPFrame_t;
	for (int i = 0; i < 6; i++) {
		ARP_Frame->FrameHeader.DesMAC[i] = 0xff;       //默认为广播地址，即全1
		ARP_Frame->FrameHeader.SrcMAC[i] = SrcMAC_t[i];  //源MAC地址为本机MAC地址
		ARP_Frame->SendHa[i] = SendHa_t[i];           //发送端MAC地址
		ARP_Frame->RecvHa[i] = 0x0;			 //目的MAC地址，应该设置为0
	}
	ARP_Frame->FrameHeader.FrameType = htons(0x0806); //帧类型为ARP
	ARP_Frame->HardwareType = htons(0x0001);   //硬件类型为以太网
	ARP_Frame->ProcotolType = htons(0x0800); //协议类型为IP
	ARP_Frame->HLen = 6;				//硬件地址长度为6
	ARP_Frame->PLen = 4;				//协议地址长度为4
	ARP_Frame->Openration = htons(0x0001); //操作类型为ARP请求
	//根据参数自定义设置
	ARP_Frame->SendIP = SendIP_t;    //发送端IP地址
	ARP_Frame->RecvIP = RecvIP_t;	//接收端IP地址，即为请求的IP地址

	if (pcap_sendpacket(opened_pcap,   //通过哪块网卡发送报文
		(u_char*)ARP_Frame, //报文数据
		sizeof(ARPFrame_t)) != 0) // 报文长度
	{				//错误处理
		MessageBox("ARP数据包发送失败！");
	}
	else
	{
		//发送成功
		MessageBox("ARP数据包发送成功！");
	}
	return ARP_Frame;
}



