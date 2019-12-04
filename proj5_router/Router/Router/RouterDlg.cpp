
// RouterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Router.h"
#include "RouterDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//自定义一个消息处理解析路由报文
#define WM_ROUTEPACKET WM_USER+1
//自定义一个消息处理解析ARP报文
#define WM_ARPPACKET WM_USER+2
//获取本机网卡MAC地址之后开始初始化路由环境
#define WM_BEGINROUTE WM_USER+3

CMutex mMutex(0, 0, 0);          //互斥
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


// CRouterDlg 对话框



CRouterDlg::CRouterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ROUTER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//初始化线程控制参数
	stop_ARP_Catch_Thread = true;
	stop_Route_Catch_Thread = true;
}

void CRouterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, EtherNet_interface_ctrl);
	DDX_Control(pDX, IDC_EDIT1, log_ctrl);
	DDX_Control(pDX, IDC_LIST1, router_table_ctrl);
	DDX_Control(pDX, IDC_IPADDRESS1, netmask_ctrl);
	DDX_Control(pDX, IDC_IPADDRESS2, des_net_ctrl);
	DDX_Control(pDX, IDC_IPADDRESS3, next_hops_ctrl);
}

BEGIN_MESSAGE_MAP(CRouterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_LBN_DBLCLK(IDC_LIST2, &CRouterDlg::OnLbnDblclkList2)
	ON_MESSAGE(WM_ARPPACKET, &CRouterDlg::OnARPPacket)
	ON_MESSAGE(WM_ROUTEPACKET, &CRouterDlg::OnROUTEPacket)
	ON_MESSAGE(WM_BEGINROUTE, &CRouterDlg::OnBeginRoute)
	ON_BN_CLICKED(IDC_ADD, &CRouterDlg::OnBnClickedAdd)
	ON_BN_CLICKED(IDC_DEL, &CRouterDlg::OnBnClickedDel)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CRouterDlg 消息处理程序

BOOL CRouterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	//assert((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	//assert(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		//assert(bNameValid);
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
	if (flag == -1)																	//错误处理
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
	
	//输出用户交互指示信息
	log_ctrl.SetWindowTextA("欢迎启动LW-1711350路由器，设备列表已获取，请双击打开目标网卡开始路由！\r\n\
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");

	//获取设备列表保存完毕，释放设备列表
	//pcap_freealldevs(alldevs);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRouterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CRouterDlg::OnPaint()
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
HCURSOR CRouterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//%%%%%%自定义函数定义部分

// 将14字节地址协议地址转为4字节IP地址类型
DWORD GetAddr_IP(sockaddr* addr)
{
	if (addr == NULL)
		return 0x0;
	else
	{
		DWORD* address = (DWORD *)((addr->sa_data) + 2);
		return *address;
	}
	return 0;
}


// 将sockaddr中地址转换为字符串返回
CString convert_sockaddr_to_str(sockaddr* addr)
{
	if (addr == NULL)
		return "";
	else
	{
		CString str;
		unsigned char * temp = (unsigned char *)addr->sa_data;
		str.Format("%d.%d.%d.%d", temp[2], temp[3], temp[4], temp[5]);
		return str;
	}
	return CString();
}

// 将IPaddr中地址转换为字符串返回
CString convert_IPaddr_to_str(DWORD addr)
{
	CString str;
	unsigned char * temp = (unsigned char *)&addr;
	str.Format("%d.%d.%d.%d", temp[0], temp[1], temp[2], temp[3]);
	return str;
}

// 将MAC addr中地址转换为字符串返回
CString convert_MAC_to_str(BYTE* MAC)
{
	if (MAC == NULL)
		return "";
	else
	{
		//格式化MAC地址数据
		CString hostmac;
		hostmac.Format("%02x:%02x:%02x:%02x:%02x:%02x", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
		return hostmac;
	}
	return CString();
}

//复制MAC地址
void CopyMAC(BYTE* MAC1, BYTE*MAC2) {
	for (int i = 0; i < 6; i++)
	{
		MAC2[i] = MAC1[i];
	}
}

//存储IPMAC映射关系
bool saveIP_MAC(DWORD IP, BYTE* MAC) {
	//获取主窗口句柄，非类成员可通过此句柄获取窗口类成员并修改
	CRouterDlg*  Dlg = (CRouterDlg*)AfxGetApp()->m_pMainWnd;
	POSITION pos = Dlg->IP_MAC.GetHeadPosition();
	while (pos != NULL)
	{
		if (compIP(Dlg->IP_MAC.GetAt(pos)->IP, IP))					//地址映射关系已经存在
		{
			return false;											//退出
		}
		Dlg->IP_MAC.GetNext(pos);
	}
	//添加IP MAC 映射记录
	IP_MAC_t* ipmac = new IP_MAC_t;
	ipmac->IP = IP;
	CopyMAC(MAC, ipmac->MAC);
	Dlg->IP_MAC.AddTail(ipmac);
	return true;
}
//获取IP对应的MAC地址
BYTE* getMACForIP(DWORD IP) {
	//获取主窗口句柄，非类成员可通过此句柄获取窗口类成员并修改
	CRouterDlg*  Dlg = (CRouterDlg*)AfxGetApp()->m_pMainWnd;
	POSITION pos = Dlg->IP_MAC.GetHeadPosition();
	while (pos != NULL)
	{
		if (compIP(Dlg->IP_MAC.GetAt(pos)->IP, IP))					//地址映射关系已经存在
		{
			return (Dlg->IP_MAC.GetAt(pos)->MAC);						//退出,返回MAC地址
		}
		Dlg->IP_MAC.GetNext(pos);
	}
	return NULL;	//不存在映射关系，返回空指针
}

//判断MAC地址是否相等
bool compMAC(BYTE* m1, BYTE* m2)
{
	for (int i = 0; i < 6; i++)
	{
		if (int(m1[i]) != int(m2[i]))
			return false;
	}
	return true;
}
//判断IP地址是否相等
bool compIP(DWORD ip1, DWORD ip2)
{
	return (int(ip1) == int(ip2));
}
// IP头部检验算法
WORD IPHeader_ckeck(WORD* IPHeader)
{
	WORD ans = 0;
	CString str;
	for (int i = 0; i < 10; i++)
	{
		if (i == 5) continue;			//跳过检验和字
		if ((ans + IPHeader[i]) % 0x10000 < ans)
			ans = (ans + IPHeader[i]) % 0x10000 + 1;
		else
			ans += IPHeader[i];
		//str.Format(L"%08x,%08x,%08x,%d", ans, IPHeader[i], ans + IPHeader[i], ans + IPHeader[i]<ans);
		//packet_list_ctrl.InsertString(packet_list_ctrl.GetCount(),str);
	}
	return WORD(ntohs(~ans));
}

// ICMP头部检验算法
WORD ICMPHeader_ckeck(WORD* ICMPHeader)
{
	WORD ans = 0;
	CString str;
	for (int i = 0; i < 4; i++)
	{
		if (i == 1) continue;			//跳过检验和字
		if ((ans + ICMPHeader[i]) % 0x10000 < ans)
			ans = (ans + ICMPHeader[i]) % 0x10000 + 1;
		else
			ans += ICMPHeader[i];
		//str.Format(L"%08x,%08x,%08x,%d", ans, IPHeader[i], ans + IPHeader[i], ans + IPHeader[i]<ans);
		//packet_list_ctrl.InsertString(packet_list_ctrl.GetCount(),str);
	}
	return WORD(ntohs(~ans));
}

// 传入参数，制作ARP数据包，并将数据包发送,发送成功返回true
bool SendARP(pcap_t* opened_pcap ,BYTE* SendHa_t, DWORD SendIP_t, DWORD RecvIP_t)
{
	ARPFrame_t* ARP_Frame = new ARPFrame_t;
	for (int i = 0; i < 6; i++) {
		ARP_Frame->FrameHeader.DesMAC[i] = 0xff;			//默认为广播地址，即全1
		ARP_Frame->FrameHeader.SrcMAC[i] = SendHa_t[i];		//源MAC地址为本机MAC地址
		ARP_Frame->SendHa[i] = SendHa_t[i];					//发送端MAC地址
		ARP_Frame->RecvHa[i] = 0x0;							//目的MAC地址，应该设置为0
	}

	ARP_Frame->FrameHeader.FrameType = htons(0x0806);		//帧类型为ARP
	ARP_Frame->HardwareType = htons(0x0001);				//硬件类型为以太网
	ARP_Frame->ProtocolType = htons(0x0800);				//协议类型为IP
	ARP_Frame->HLen = 6;									//硬件地址长度为6
	ARP_Frame->PLen = 4;									//协议地址长度为4
	ARP_Frame->Operation = htons(0x0001);					//操作类型为ARP请求
															//根据参数自定义设置
	ARP_Frame->SendIP = SendIP_t;							//发送端IP地址
	ARP_Frame->RecvIP = RecvIP_t;							//接收端IP地址，即为请求的IP地址

	if (pcap_sendpacket(opened_pcap,   //通过哪块网卡发送报文
		(u_char*)ARP_Frame, //报文数据
		sizeof(ARPFrame_t)) != 0) // 报文长度
	{				//错误处理
		delete ARP_Frame;
		return false;
	}
	delete ARP_Frame;
	return true;
}

//转发数据包的函数,参数传入的是带帧头部的数据包
bool sendIPFrame(pcap_t* opened_pcap,u_char* IPFrame,int FrameLen) {
	if (pcap_sendpacket(opened_pcap,   //通过哪块网卡发送报文
		(u_char*)IPFrame, //报文数据
		FrameLen) != 0) // 报文长度
	{				//错误处理
		return false;
	}
	return true;
}
// 计算校验和
unsigned short ChecksumCompute(unsigned short * buffer, int size)
{
	// 32位，延迟进位
	unsigned long cksum = 0;
	while (size > 1)
	{
		cksum += *buffer++;
		// 16位相加
		size -= sizeof(unsigned short);
	}
	if (size)
	{
		// 最后可能有单独8位
		cksum += *(unsigned char *)buffer;
	}
	// 将高16位进位加至低16位
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	// 取反
	return (unsigned short)(~cksum);
}

//发送ICMP报文
bool sendICMP(pcap_t* opened_pcap, BYTE type, BYTE code, const u_char* pkt_data) {
	//获取主窗口句柄，非类成员可通过此句柄获取窗口类成员并修改
	CRouterDlg*  Dlg = (CRouterDlg*)AfxGetApp()->m_pMainWnd;

	u_char * ICMPBuf = new u_char[70];

	// 填充帧首部
	memcpy(((FrameHeader_t *)ICMPBuf)->DesMAC, ((FrameHeader_t *)pkt_data)->SrcMAC, 6);
	memcpy(((FrameHeader_t *)ICMPBuf)->SrcMAC, ((FrameHeader_t *)pkt_data)->DesMAC, 6);
	((FrameHeader_t *)ICMPBuf)->FrameType = htons(0x0800);

	// 填充IP首部
	((IPHeader_t *)(ICMPBuf + 14))->Ver_HLen = ((IPHeader_t *)(pkt_data + 14))->Ver_HLen;
	((IPHeader_t *)(ICMPBuf + 14))->TOS = ((IPHeader_t *)(pkt_data + 14))->TOS;
	((IPHeader_t *)(ICMPBuf + 14))->TotalLen = htons(56);
	((IPHeader_t *)(ICMPBuf + 14))->ID = ((IPHeader_t *)(pkt_data + 14))->ID;
	((IPHeader_t *)(ICMPBuf + 14))->Flag_Segment = ((IPHeader_t *)(pkt_data + 14))->Flag_Segment;
	((IPHeader_t *)(ICMPBuf + 14))->TTL = 64;
	((IPHeader_t *)(ICMPBuf + 14))->Protocol = 1;
	((IPHeader_t *)(ICMPBuf + 14))->SrcIP =Dlg->IP_addr[0].IP;//((IPHeader_t *)(pkt_data+14))->DstIP;
	((IPHeader_t *)(ICMPBuf + 14))->DstIP = ((IPHeader_t *)(pkt_data + 14))->SrcIP;
	((IPHeader_t *)(ICMPBuf + 14))->Checksum = 0;
	((IPHeader_t *)(ICMPBuf + 14))->Checksum = ChecksumCompute((unsigned short *)(ICMPBuf + 14), 20);

	// 填充ICMP首部
	((ICMPHeader_t *)(ICMPBuf + 34))->Type = type;
	((ICMPHeader_t *)(ICMPBuf + 34))->Code = code;
	((ICMPHeader_t *)(ICMPBuf + 34))->Id = 0;
	((ICMPHeader_t *)(ICMPBuf + 34))->Sequence = 0;
	((ICMPHeader_t *)(ICMPBuf + 34))->Checksum = 0;
	
	// 填充数据
	memcpy((u_char *)(ICMPBuf + 42), (IPHeader_t *)(pkt_data + 14), 20);
	memcpy((u_char *)(ICMPBuf + 62), (u_char *)(pkt_data + 34), 8);
	((ICMPHeader_t *)(ICMPBuf + 34))->Checksum = ChecksumCompute((unsigned short *)(ICMPBuf + 34), 36);

	// 发送数据包
	pcap_sendpacket(Dlg->opened_pcap, (u_char *)ICMPBuf, 70);

	delete[] ICMPBuf;
	return true;
}

//线程执行函数，接收ARP数据包
UINT RecvARP(PVOID hwnd) {
	//获取主窗口句柄，非类成员可通过此句柄获取窗口类成员并修改
	CRouterDlg*  Dlg = (CRouterDlg*)AfxGetApp()->m_pMainWnd;
	pcap_pkthdr* pkt_header = NULL;		//记录捕获的数据包头
	const u_char* pkt_data = NULL;				//记录捕获的数据包
	while (!Dlg->stop_ARP_Catch_Thread)			//循环捕获网络数据包，通过参数stop_thread 控制停止捕获
	{
		//捕获数据包的主体代码，利用pcap_next_ex函数执行
		int flag = 0;
		TRY{
			flag = pcap_next_ex(
					Dlg->opened_pcap,					//pcap_t指针，说明捕获那个网卡上的数据包
					&pkt_header,						//pcap_pkthdr 指针，记录数据包相关信息
					&pkt_data							//uchar*指针，保存数据包数据
				);
		}CATCH(CException, e)
		{
			TCHAR error[1024]("\0");
			e->GetErrorMessage(error, 1024);
			Dlg->MessageBox("$$$$$捕获异常" + (CString)error);
			memset(error, 0, 1024);
			continue;
		}END_CATCH;
		//成功捕获数据,并对捕获的函数进行分类
		if (flag == 1)
		{
			FrameHeader_t* FHeader = (FrameHeader_t*)pkt_data;
			if (int(ntohs(FHeader->FrameType)) == int(0x0806))				//接收到ARP报文
			{
				ARPFrame_t* ARP = (ARPFrame_t*)pkt_data;
				if (int(ntohs(ARP->ProtocolType)) == int(0x0800))
				{
					if (int(ntohs(ARP->Operation)) == int(0x0002))						//接收到ARP应答报文
					{
						if ((ARP->SendIP == Dlg->IP_addr[0].IP || ARP->SendIP == Dlg->IP_addr[1].IP))		//如果获取的ARP响应来自本机，则可通过该ARP响应获取本机的MAC地址
						{
							if (Dlg->host_MAC != NULL) continue;											//如果已经接收过了，则放弃该报文
							//assert(!Dlg->host_MAC);
							Dlg->host_MAC = new BYTE[6];
							CopyMAC(ARP->SendHa, Dlg->host_MAC);										//获取本机的MAC地址，在日志中进行输出，并启动相应的环境
							CString log_t;
							Dlg->log_ctrl.GetWindowTextA(log_t);
							Dlg->log_ctrl.SetWindowTextA(log_t + ">>网卡MAC地址：" + convert_MAC_to_str(Dlg->host_MAC) + "\r\n");
							Dlg->PostMessageA(WM_BEGINROUTE, 0, 0);
						}
						else {
							//Dlg->MessageBox("接收到非本机发送的ARP报文");
							Dlg->ARP_pktdata_list.AddTail(pkt_data);			//将报文传入缓冲区
							Dlg->PostMessageA(WM_ARPPACKET, 0, 0);				//发送报文接收成功消息触发ARP报文解析函数  
						}
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
	Dlg->stop_ARP_Catch_Thread = 0;								//重置参数为0
	CString log_t;
	Dlg->log_ctrl.GetWindowTextA(log_t);
	Dlg->log_ctrl.SetWindowTextA(log_t + ">>捕获本地ARP数据包线程终止！" + "\r\n");
	return 0;
}

//线程执行函数，接收Route数据包
UINT RecvRouteP(PVOID hwnd) {
	//获取主窗口句柄，非类成员可通过此句柄获取窗口类成员并修改
	CRouterDlg*  Dlg = (CRouterDlg*)AfxGetApp()->m_pMainWnd;
	pcap_pkthdr* pkt_header = NULL;				//记录捕获的数据包头
	const u_char* pkt_data = NULL;				//记录捕获的数据包
	while (!Dlg->stop_Route_Catch_Thread)			//循环捕获网络数据包，通过参数stop_thread 控制停止捕获
	{
		//捕获数据包的主体代码，利用pcap_next_ex函数执行
		int flag = 0;
		TRY{
			flag = pcap_next_ex(
			Dlg->opened_pcap,					//pcap_t指针，说明捕获那个网卡上的数据包
				&pkt_header,						//pcap_pkthdr 指针，记录数据包相关信息
				&pkt_data							//uchar*指针，保存数据包数据
				);
		}CATCH(CException, e)
		{
			TCHAR error[1024]("\0");
			e->GetErrorMessage(error, 1024);
			Dlg->MessageBox("$$$$$捕获异常" + (CString)error);
			memset(error, 0, 1024);
			continue;
		}END_CATCH;
		//成功捕获数据,并对捕获的函数进行分类
		if (flag == 1)
		{
			FrameHeader_t* FHeader = (FrameHeader_t*)pkt_data;
			if (int(ntohs(FHeader->FrameType))== int(0x0800))				//接收到IPv4报文
			{
				//Data_t* IP_Data = (Data_t*)pkt_data;
				//if (!(compMAC(IP_Data->FrameHeader.DesMAC, Dlg->host_MAC)
				//	&& !compIP(IP_Data->IPHeader.DstIP, Dlg->IP_addr[0].IP)
				//	&& !compIP(IP_Data->IPHeader.DstIP, Dlg->IP_addr[1].IP)))	//不是需要路由的报文，直接抛弃
				//	continue;
				Dlg->pkthdr_list.AddTail(pkt_header);
				Dlg->pktdata_list.AddTail(pkt_data);     //将报文传入缓冲区
				Dlg->PostMessageA(WM_ROUTEPACKET, 0, 0);      //发送报文接收成功消息触发报文解析函数  
			}
			if (int(ntohs(FHeader->FrameType)) == int(0x0806))				//接收到ARP报文
			{
				Dlg->ARP_pktdata_list.AddTail(pkt_data);			//将报文传入缓冲区
				Dlg->PostMessageA(WM_ARPPACKET, 0, 0);				//发送报文接收成功消息触发ARP报文解析函数  
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
	Dlg->stop_Route_Catch_Thread = 0;								//重置参数为0
	CString log_t;
	Dlg->log_ctrl.GetWindowTextA(log_t);
	Dlg->log_ctrl.SetWindowTextA(log_t + ">>捕获路由数据包线程终止！" + "\r\n");
	return 0;
}



// 处理捕获路由数据包的函数,对数据包进行转发
LRESULT CRouterDlg::OnROUTEPacket(WPARAM wParam, LPARAM lParam)
{
	TRY{
		//从缓冲队列中提取数据报文
	pcap_pkthdr* pkt_header = pkthdr_list.GetHead();			//记录捕获的数据包头
	const u_char* pkt_data = pktdata_list.GetHead();			//记录捕获的数据包
	//将获得的IPV4报文按照IP数据包格式化
	Data_t* IP_Data = (Data_t*)pkt_data;
	//输出日志
	CString log_t;
	//解析报文，看是否是需要路由的报文,需要路由的报文特点是报文的目的MAC地址是本机，但是目的IP地址不是本机
	//if (!(compMAC(IP_Data->FrameHeader.DesMAC, host_MAC)
	//	&& !compIP(IP_Data->IPHeader.DstIP, IP_addr[0].IP)
	//	&& !compIP(IP_Data->IPHeader.DstIP, IP_addr[1].IP)))	//不是需要路由的报文，直接抛弃
	//{
	//	pkthdr_list.RemoveHead();							//移除解析完的头部数据包
	//	pktdata_list.RemoveHead();
	//	log_ctrl.GetWindowTextA(log_t);
	//	log_ctrl.SetWindowTextA(log_t + "=========================================================\r\n" +
	//		">>接收不可路由报文:" +
	//		convert_IPaddr_to_str(IP_Data->IPHeader.SrcIP) + " -- " +
	//		convert_IPaddr_to_str(IP_Data->IPHeader.DstIP) + "  " +
	//		convert_MAC_to_str(IP_Data->FrameHeader.SrcMAC) + " -- " +
	//		convert_MAC_to_str(IP_Data->FrameHeader.DesMAC) + "\r\n");
	//	return LRESULT();
	//}

	log_ctrl.GetWindowTextA(log_t);
	log_ctrl.SetWindowTextA(log_t + "=========================================================\r\n"+
		">>接收可路由报文:"  +
		convert_IPaddr_to_str(IP_Data->IPHeader.SrcIP) + " -- " +
		convert_IPaddr_to_str(IP_Data->IPHeader.DstIP) + "  " +
		convert_MAC_to_str(IP_Data->FrameHeader.SrcMAC) + " -- " +
		convert_MAC_to_str(IP_Data->FrameHeader.DesMAC) + "\r\n");
	//将接收到的数据包源IP地址和MAC地址映射关系，存入IPMAC映射表
	saveIP_MAC(IP_Data->IPHeader.SrcIP, IP_Data->FrameHeader.SrcMAC);
	//进行路由报文处理
	IP_Data->IPHeader.TTL = IP_Data->IPHeader.TTL - 1;   //TTL 递减
	IP_Data->IPHeader.Checksum = htons(IPHeader_ckeck((WORD*)&IP_Data->IPHeader));//重新计算校验和
	//数据包超时，转入ICMP报文处理流程
	CString t;
	t.Format("%d", IP_Data->IPHeader.TTL);
	log_ctrl.SetWindowTextA(log_t + "--->>TTL:" +
		t+ "\r\n");
	
	if (int(IP_Data->IPHeader.TTL) <= 0)
	{
		//assert(IP_Data->IPHeader.TTL <= 0);
		pkthdr_list.RemoveHead();							//移除解析完的头部数据包
		pktdata_list.RemoveHead();

		//ICMP处理  超时 type = 11, code =0  
		if (sendICMP(opened_pcap, 11, 0, pkt_data))
		{
			//发送ICMP成功
			log_ctrl.GetWindowTextA(log_t);
			log_ctrl.SetWindowTextA(log_t + "--->>成功向源地址发送ICMP超时报文: 源IP地址:"+
				convert_IPaddr_to_str(IP_Data->IPHeader.SrcIP) + "\r\n");
		}
		else
		{
			//发送失败
			log_ctrl.GetWindowTextA(log_t);
			log_ctrl.SetWindowTextA(log_t + "--->>ERROR:向源地址发送ICMP超时报文失败: 源IP地址:" +
				convert_IPaddr_to_str(IP_Data->IPHeader.SrcIP) + "\r\n");
		}
		return LRESULT();
	}

	if (IPHeader_ckeck((WORD*)(&IP_Data->IPHeader)) != ntohs(IP_Data->IPHeader.Checksum))
	{

		//日志信息输出
		CString checksum;
		checksum.Format("报文校验和:0x%04x,计算校验和:0x%04x", IP_Data->IPHeader.Checksum, IPHeader_ckeck((WORD*)(&IP_Data->IPHeader)));		//输出格式化
		log_ctrl.GetWindowTextA(log_t);
		log_ctrl.SetWindowTextA(log_t + "--->>ERROR:校验和错误！\r\n======>" +checksum+ "\r\n");
		//ICMP处理
		pkthdr_list.RemoveHead();							//移除解析完的头部数据包
		pktdata_list.RemoveHead();
		return LRESULT();
	}
	//路由表查询
	DWORD nextHop_IP = 0x0;
	DWORD longestNetMask = 0x0;
	bool find = false;
	//遍历路由表
	POSITION pos = RouteTable.GetHeadPosition();
	while (pos != NULL)
	{
		RouteTable_t* item = RouteTable.GetAt(pos);
		if (compIP(item->netMask&IP_Data->IPHeader.DstIP, item->destNet)
			&&int(item->netMask)>int(longestNetMask))			//匹配到相应的路由并且掩码长度大于之前的掩码长度，默认设置为0
		{
			find = true;
			longestNetMask = item->netMask;
			nextHop_IP = item->nextHops;
		}
		RouteTable.GetNext(pos);
	}
	if (!find)													//未匹配到路由
	{
		pkthdr_list.RemoveHead();							//移除解析完的头部数据包
		pktdata_list.RemoveHead();
		//ICMP处理
		//ICMP处理  网络不可达 type = 3, code =0  
		if (sendICMP(opened_pcap, 3, 0, pkt_data))
		{
			//发送ICMP成功
			log_ctrl.GetWindowTextA(log_t);
			log_ctrl.SetWindowTextA(log_t + "--->>成功向源地址发送ICMP网络不可达报文: 源IP地址:" +
				convert_IPaddr_to_str(IP_Data->IPHeader.SrcIP) + "\r\n");
		}
		else
		{
			//发送失败
			log_ctrl.GetWindowTextA(log_t);
			log_ctrl.SetWindowTextA(log_t + "--->>ERROR:向源地址发送ICMP网络不可达报文失败: 源IP地址:" +
				convert_IPaddr_to_str(IP_Data->IPHeader.SrcIP) + "\r\n");
		}
		return LRESULT();
	}

	//匹配到路由表项，查询IPMAC映射表，获取MAC地址，如果是直接路由
	if (nextHop_IP == 0x0)			//直接路由
	{
		//直接将目的IP地址赋给下一跳步地址进行后续操作
		nextHop_IP = IP_Data->IPHeader.DstIP;

		CString test;
		test.Format("0x%04x-->0x%04x", ntohs(IP_Data->IPHeader.Checksum), IPHeader_ckeck((WORD*)&IP_Data->IPHeader));
		log_ctrl.GetWindowTextA(log_t);
		log_ctrl.SetWindowTextA(log_t + "--->>直接路由，重新计算校验和:" + test + "\r\n");
	}
	else
	{	
		//非直接路由
		CString test;
		test.Format("0x%04x-->0x%04x", ntohs(IP_Data->IPHeader.Checksum),IPHeader_ckeck((WORD*)&IP_Data->IPHeader));
		log_ctrl.GetWindowTextA(log_t);
		log_ctrl.SetWindowTextA(log_t + "--->>非直接路由，重新计算校验和:" + test + "\r\n");
	}
	
	//路由的进一步处理
	//利用总长度字段,以8位字节为单位
	int totalLen = ntohs(IP_Data->IPHeader.TotalLen);						//获取IP报文总长度
	totalLen += sizeof(FrameHeader_t);
	if (getMACForIP(IP_Data->IPHeader.DstIP) == NULL)
	{
		//没有找到对应IP的MAC地址,通过ARP报文获取MAC地址
		//通过对本机发送一个ARP请求获取目的地址的IPMAC映射关系
		DWORD SendIP_t=0x0;													//本机IP地址和MAC地址
		BYTE* SendHa_t = NULL;
		for (int i = 0; i < 2; i++)
		{
			if (compIP(nextHop_IP&IP_addr[i].netMask, IP_addr[i].IP&IP_addr[i].netMask))
			{
				SendIP_t = IP_addr[i].IP;
				SendHa_t = new BYTE[6];
				CopyMAC(IP_addr[i].MAC, SendHa_t);
				break;
			}
		}
		//assert(SendIP_t!=0x0&&SendHa_t!=NULL);
		DWORD RecvIP_t = nextHop_IP;										//目的IP地址作为目的地址
		SendARP(opened_pcap, SendHa_t, SendIP_t, RecvIP_t);
		//记录日志
		log_ctrl.GetWindowTextA(log_t);
		log_ctrl.SetWindowTextA(log_t + "--->>发送ARP请求报文到 IP:"+convert_IPaddr_to_str(RecvIP_t) + "\r\n");	
		//缓存发送报文
		int ID = 0;
		if ((ID = getTimerID()) == 0)
		{
			//缓冲区满了
			//输出日志
			log_ctrl.GetWindowTextA(log_t);
			log_ctrl.SetWindowTextA(log_t + ">>发送缓冲区已满(15/15)，丢弃数据包:" +
				convert_IPaddr_to_str(IP_Data->IPHeader.SrcIP) + " -- " +
				convert_IPaddr_to_str(IP_Data->IPHeader.DstIP) + "  " +
				convert_MAC_to_str(IP_Data->FrameHeader.SrcMAC) + " -- " +
				convert_MAC_to_str(IP_Data->FrameHeader.DesMAC) + "\r\n");
		}
		else
		{
			mMutex.Lock(INFINITE);
			//assert(ID > 0);
			//assert(ID < IPFrameBufLen);
			SendPacket_t* IPFrame = new SendPacket_t;
			IPFrame->len = totalLen;
			for (int i = 0; i < IPFrame->len; i++)
		r	{
				IPFrame->PktData[i] = ((BYTE*)(pkt_data))[i];
			}
			IPFrame->TargetIP = nextHop_IP;
			SetTimer(ID, 10000, 0);		//定时器设为10秒
			sendIPFrame_list.AddTail(IPFrame);
			mMutex.Unlock();
			//输出日志
			log_ctrl.GetWindowTextA(log_t);
			log_ctrl.SetWindowTextA(log_t + ">>缓存报文，等待接收ARP报文:" +
				convert_IPaddr_to_str(IP_Data->IPHeader.SrcIP) + " -- " +
				convert_IPaddr_to_str(IP_Data->IPHeader.DstIP) + "  " +
				convert_MAC_to_str(IP_Data->FrameHeader.SrcMAC) + " -- " +
				convert_MAC_to_str(IP_Data->FrameHeader.DesMAC) + "\r\n");
		}
		//退出转发函数
		pkthdr_list.RemoveHead();							//移除解析完的头部数据包
		pktdata_list.RemoveHead();
		return LRESULT();
	}
	//存在MAC映射，继续发送
	BYTE * nextHop_MAC = new BYTE[6];
	CopyMAC(getMACForIP(IP_Data->IPHeader.DstIP), nextHop_MAC);
	//log_ctrl.GetWindowTextA(log_t);
	//log_ctrl.SetWindowTextA(log_t + "--->>成功获取下一跳步IPMAC映射关系: IP:" + convert_IPaddr_to_str(nextHop_IP)
	//	+" -- MAC:"+convert_MAC_to_str(nextHop_MAC)+ "\r\n");

	
	//修改帧头部信息并取得帧头部长度
	CopyMAC(nextHop_MAC,IP_Data->FrameHeader.DesMAC );
	CopyMAC(host_MAC, IP_Data->FrameHeader.SrcMAC);
	if (sendIPFrame(opened_pcap, (u_char*)IP_Data, totalLen))
	{
		//成功转发
		log_ctrl.GetWindowTextA(log_t);
		log_ctrl.SetWindowTextA(log_t + "--->>转发数据包成功！from:"+
			convert_IPaddr_to_str(IP_Data->IPHeader.SrcIP)+" -- "
			+"to:"+convert_IPaddr_to_str(IP_Data->IPHeader.DstIP)+
			"\r\n next hop IP<->MAC:"+
			convert_IPaddr_to_str( nextHop_IP)+
			" <--> "+convert_MAC_to_str(nextHop_MAC)+"\r\n");
	}
	else
	{
		//未转发成功
		log_ctrl.GetWindowTextA(log_t);
		log_ctrl.SetWindowTextA(log_t + "--->>ERROR:转发失败！from:" +
			convert_IPaddr_to_str(IP_Data->IPHeader.SrcIP) + " -- "
			+ "to:" + convert_IPaddr_to_str(IP_Data->IPHeader.DstIP) +
			"\r\n next hop IP<->MAC:" +
			convert_IPaddr_to_str(nextHop_IP) +
			" <--> " + convert_MAC_to_str(nextHop_MAC) + "\r\n");
	}
	pkthdr_list.RemoveHead();							//移除解析完的头部数据包
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

// 处理捕获ARP数据包的函数,对数据包进行解析
LRESULT CRouterDlg::OnARPPacket(WPARAM wParam, LPARAM lParam)
{
	TRY{
		//从缓冲队列中提取数据报文
	const u_char* pkt_data = ARP_pktdata_list.GetHead();			//记录捕获的数据包
																//获取帧头部保存的时间戳数据和类型数据
																//处理ARP报文，解析IP地址和MAC地址的映射关系并显示在IPMAC显示窗口
	ARPFrame_t*  ARP = (ARPFrame_t*)pkt_data;

	bool getNewMAC =  saveIP_MAC(ARP->SendIP, ARP->SendHa);				//存储IPMAC映射关系
	//输出日志
	CString log_t;
	//POSITION pos = IP_MAC.GetHeadPosition();
	//while (pos != NULL)
	//{
	//	IP_MAC_t*t = IP_MAC.GetAt(pos);
	//	log_ctrl.GetWindowTextA(log_t);
	//	log_ctrl.SetWindowTextA(log_t + ">>IP-MAC:" +
	//		"IP:" + convert_IPaddr_to_str(t->IP) +  " <--> " +
	//		"MAC:" + convert_MAC_to_str(t->MAC) + "\r\n");
	//	IP_MAC.GetNext(pos);
	//}
	log_ctrl.GetWindowTextA(log_t);
	log_ctrl.SetWindowTextA(log_t + ">>捕获ARP报文解析成功:" +
		"IP:"+convert_IPaddr_to_str(ARP->SendIP)+" <--> "+
		"MAC:"+convert_MAC_to_str(ARP->SendHa) +"\r\n");

	ARP_pktdata_list.RemoveHead();							//移除解析成功的头部数据包

	if (getNewMAC)			//获取新的MAC映射关系。检查发送缓冲区中是否有报文能够发送
	{
		mMutex.Lock(INFINITE);
		int time = sendIPFrame_list.GetCount();
		while (time--) {
			POSITION pos = sendIPFrame_list.GetHeadPosition();
			while (pos != NULL)
			{
				SendPacket_t* item = sendIPFrame_list.GetAt(pos);
				if (compIP(item->TargetIP, ARP->SendIP))			//匹配可以发送的报文
				{
					//修改帧头部信息并取得帧头部长度
					CopyMAC(ARP->SendHa, ((Data_t*)(item->PktData))->FrameHeader.DesMAC);
					CopyMAC(host_MAC, ((Data_t*)(item->PktData))->FrameHeader.SrcMAC);
					if (sendIPFrame(opened_pcap, (u_char*)item->PktData, item->len))
					{
						//成功转发
						log_ctrl.GetWindowTextA(log_t);
						log_ctrl.SetWindowTextA(log_t + "--->>转发缓冲区数据包成功！from:" +
							convert_IPaddr_to_str(((Data_t*)(item->PktData))->IPHeader.SrcIP) + " -- "
							+ "to:" + convert_IPaddr_to_str(((Data_t*)(item->PktData))->IPHeader.DstIP) +
							"\r\n next hop IP<->MAC:" +
							convert_IPaddr_to_str(item->TargetIP) +
							" <--> " + convert_MAC_to_str(ARP->SendHa) + "\r\n");
					}
					else
					{
						//未转发成功
						log_ctrl.GetWindowTextA(log_t);
						log_ctrl.SetWindowTextA(log_t + "--->>ERROR:转发缓冲区数据包失败，丢弃数据包！from:" +
							convert_IPaddr_to_str(((Data_t*)(item->PktData))->IPHeader.SrcIP) + " -- "
							+ "to:" + convert_IPaddr_to_str(((Data_t*)(item->PktData))->IPHeader.DstIP) +
							"\r\n next hop IP<->MAC:" +
							convert_IPaddr_to_str(item->TargetIP) +
							" <--> " + convert_MAC_to_str(ARP->SendHa) + "\r\n");
					}
					backTimerID(item->n_mTimer);
					sendIPFrame_list.RemoveAt(pos);
					break;
				}
				sendIPFrame_list.GetNext(pos);
			}
		}
		mMutex.Unlock();
	}
	}CATCH(CException, e)
	{
		TCHAR error[1024]("\0");
		e->GetErrorMessage(error, 1024);
		MessageBox("$$$$$处理异常" + (CString)error);
		memset(error, 0, 1024);
	}END_CATCH;
	return LRESULT();
}


//初始化路由环境
LRESULT CRouterDlg::OnBeginRoute(WPARAM wParam, LPARAM lParam)
{
	CString log_t;
	log_ctrl.GetWindowTextA(log_t);
	log_ctrl.SetWindowTextA(log_t + ">>开始初始化路由表！" + "\r\n");
	//初始化缓冲区
	for (int i = 0; i < IPFrameBufLen; i++)
	{
		SendTimerLimit[i] = i;			//发送缓冲区最多保存十五个待发送的数据帧
	}
	sendIPFrame_list.RemoveAll();
	//删除已有的路由表信息和IPMAC映射表
	int count = router_table_ctrl.GetCount();
	for (int i = 0; i < count; i++)
	{
		router_table_ctrl.DeleteString(0);
	}
	RouteTable.RemoveAll();
	IP_MAC.RemoveAll();
	//关闭获取本机IPMAC映射的数据包捕获线程和数据包捕获线程
	stop_ARP_Catch_Thread = true;
	stop_Route_Catch_Thread = true;
	//数据结构初始化
	for (int i = 0; i < 2; i++)
	{
		//开始设置IPMAC映射表的内容，首先将本机的IPMAC映射存进去
		IP_MAC_t* ipmac = new IP_MAC_t;
		ipmac->IP = IP_addr[i].IP;
		//将本机的IP地址赋值给IP信息存放结构体
		for (int j = 0; j < 6; j++)
		{
			IP_addr[i].MAC[j] = host_MAC[j];
			ipmac->MAC[j] = host_MAC[j];
		}
		IP_MAC.AddTail(ipmac);
		//将本机的默认直接路由存入路由表，并进行显示
		RouteTable_t* routeItem = new RouteTable_t;
		routeItem->netMask = IP_addr[i].netMask;
		routeItem->destNet = IP_addr[i].IP&IP_addr[i].netMask;
		routeItem->nextHops = 0x0;		//直接投递
		RouteTable.AddTail(routeItem);		
		//将默认的直接路由输出
		CString rt;						
		rt.Format("%s -- %s -- %s (直接投递)",
			convert_IPaddr_to_str(routeItem->netMask),
			convert_IPaddr_to_str(routeItem->destNet),
			convert_IPaddr_to_str(routeItem->nextHops));
		router_table_ctrl.InsertString(router_table_ctrl.GetCount(), rt);
	}

	log_ctrl.GetWindowTextA(log_t);
	log_ctrl.SetWindowTextA(log_t + ">>完成路由表初始化！" + "\r\n");

	// 设置过滤规则:仅仅接收arp响应帧和需要路由的帧
	
	// 设置过滤规则:仅仅接收arp响应帧和需要路由的帧
	CString Filter;
	struct bpf_program fcode;
	Filter.Format("(ether dst %s) and ((arp and (ether[21]=0x2))\
 or (not((ip dst host %s) or (ip dst host %s))))", convert_MAC_to_str(host_MAC),
		convert_IPaddr_to_str(IP_addr[0].IP),
		convert_IPaddr_to_str(IP_addr[1].IP));
	
	if ((pcap_compile(opened_pcap, &fcode, Filter, 1, IP_addr[0].netMask) <0) || (pcap_setfilter(opened_pcap, &fcode)<0))
	{
			MessageBox(Filter + L"过滤规则编译不成功，请检查书写的规则语法是否正确！设置过滤器错误！");
			return LRESULT();
	}
	//MessageBox(Filter);
	//开启路由数据包捕获线程
	stop_Route_Catch_Thread = false;						//创建新的线程之前，需要先将原来的线程关闭,并将线程控制参数stop_thread置位为0

	m_RouteCaptureThread = AfxBeginThread(RecvRouteP,	//工作者线程的控制函数
		NULL,										//传给控制函数的参数，一般为某数据结构的指针，这里为空
		THREAD_PRIORITY_NORMAL);					//线程优先级，默认为正常的优先级

	log_ctrl.GetWindowTextA(log_t);
	log_ctrl.SetWindowTextA(log_t + ">>成功创建数据包捕获线程！" + "\r\n"
		+ ">>>>>>>>>>>>>>>>>>>>>>> 开始路由 <<<<<<<<<<<<<<<<<<<<<<<" + "\r\n");

	return LRESULT();
}


//双击目标网卡，检测获得该网卡上的IP地址以及MAC地址，并初始化路由器路由环境
void CRouterDlg::OnLbnDblclkList2()
{
	// TODO: 在此添加控件通知处理程序代码
	//重置环境
	host_MAC = NULL;
	//关闭所以正在运行的数据包捕获线程
	if (stop_ARP_Catch_Thread == false)
	{
		stop_ARP_Catch_Thread = true;
		CString log_t;
		log_ctrl.GetWindowTextA(log_t);
		log_ctrl.SetWindowTextA(log_t + ">>正在关闭ARP报文捕获线程！" + "\r\n");
	}
	if (stop_Route_Catch_Thread == false)
	{
		stop_Route_Catch_Thread = true;
		CString log_t;
		log_ctrl.GetWindowTextA(log_t);
		log_ctrl.SetWindowTextA(log_t + ">>正在关闭路由报文捕获线程！" + "\r\n");
	}
	Sleep(1000);										//暂定200ms，同步线程
	//获取选择网卡设备的信息
	int sno = EtherNet_interface_ctrl.GetCurSel();	//获取选中的设备接口序号
	CString  sitem;									//创建CString变量记录接口详细信息，以及记录设备接口名
	EtherNet_interface_ctrl.GetText(sno, sitem);	//获取设备接口名
	for (d = alldevs; d != NULL; d = d->next)		//通过选中的接口名获得设备接口的地址，赋值给了类成员d，
		if ((CString)(d->name) == sitem)			//之后可以通过d获取该接口信息，并将设备接口描述和各类地址显示
			break;
	
	
	//将信息输出
	sitem += "\r\n>>设备描述：";					//换行
	sitem += (CString)d->description;				//设备描述


	//输出日志信息
	CString log_t;
	log_ctrl.GetWindowTextA(log_t);
	log_ctrl.SetWindowTextA(log_t + ">>选择网卡："+sitem+"\r\n");


	//判断该网卡是否具有两个IP地址并进行显示，否则进行报错
	int IPCount = 0;								//统计IP地址个数
	for (a = d->addresses; a != NULL; a = a->next)
	{
		if (a->addr->sa_family == AF_INET)					// 判断改地址是否为IP地址
		{
			CString  ip_str;
			ip_str.Format(">>IP地址%d：%s  子网掩码%d：%s", IPCount, convert_sockaddr_to_str(a->addr), IPCount, convert_sockaddr_to_str(a->netmask));
			//输出日志信息
			CString log_t;
			log_ctrl.GetWindowTextA(log_t);
			log_ctrl.SetWindowTextA(log_t + ip_str + "\r\n");
			IP_addr[IPCount].IP = GetAddr_IP(a->addr);			//记录网卡的两个IP地址
			IP_addr[IPCount].netMask = GetAddr_IP(a->netmask);	//记录两个网卡的子网掩码	
			IPCount++;											//记录加1
			if (IPCount == 2) break;							//获得两个IP立即退出，当前的a保存网卡的第二个IP地址
		}
	}

	if (IPCount < 2)										//该网卡不存在两个IP地址报告错误退出函数
	{
		//输出日志信息
		CString log_t;
		log_ctrl.GetWindowTextA(log_t);
		log_ctrl.SetWindowTextA(log_t + ">>ERROR:该网卡不具备两个IP地址，无法作为路由设备，请重新选择！" + "\r\n\
====================================================\r\n");
		return;
	}



	//通过获取的IP地址取得相应的mac地址

	//打开选择的设备
	//打开设备接口
	opened_pcap = pcap_open(d->name,			//设备接口名，直接从类成员d指针中获取，一旦选择之后，d指针指向选择的设备接口
		4096,									//获取数据包的最大长度
		PCAP_OPENFLAG_PROMISCUOUS,				//打开设备接口获取网络数据包方式，改参数为混杂模式，获取所有流经该网络接口的数据包
		1000,									//等待一个数据包的最大时间
		NULL,									//远程设备捕获网络数据包使用，本实验只需设置为NULL
		errbuf);								//错误信息缓冲区
	if (opened_pcap == NULL)					//错误处理
		MessageBox(CString(errbuf), "ERROR", MB_OKCANCEL | MB_ICONERROR);
	else
	{
		stop_ARP_Catch_Thread = false;						//创建新的线程之前，需要先将原来的线程关闭,并将线程控制参数stop_thread置位为0

		m_ARPCaptureThread = AfxBeginThread(RecvARP,	//工作者线程的控制函数
			NULL,										//传给控制函数的参数，一般为某数据结构的指针，这里为空
			THREAD_PRIORITY_NORMAL);					//线程优先级，默认为正常的优先级
		CString log_t;
		log_ctrl.GetWindowTextA(log_t);
		log_ctrl.SetWindowTextA(log_t + ">>成功创建ARP报文捕获线程！" + "\r\n");
	}
	//开启ARP数据包捕获线程


	//通过对本机发送一个ARP请求获取本机的IPMAC映射关系
	BYTE SendHa_t[6]{ 0x66,0x66,0x66,0x66,0x66,0x66 };				//虚假源MAC地址 66-66-66-66-66-66
	DWORD SendIP_t = 0x70707070;									//虚假源IP地址 112.112.112.112
	DWORD RecvIP_t = IP_addr[0].IP;									//本机IP地址作为目的地址

	//assert(RecvIP_t != 0);


	if (!SendARP(opened_pcap, SendHa_t, SendIP_t, RecvIP_t))				//发送一个ARP 报文	
	{																						//失败，退出函数
		CString log_t;
		log_ctrl.GetWindowTextA(log_t);
		log_ctrl.SetWindowTextA(log_t + ">>ERROR:发送ARP请求到本机失败！" + "\r\n\
====================================================\r\n");
		return;
	}
	else {
		CString log_t;
		log_ctrl.GetWindowTextA(log_t);
		log_ctrl.SetWindowTextA(log_t + ">>成功发送ARP请求到本机！" + "\r\n");
	}
}
//刷新显示的路由表
void updateRouteTable() {
	//获取主窗口句柄，非类成员可通过此句柄获取窗口类成员并修改
	CRouterDlg*  Dlg = (CRouterDlg*)AfxGetApp()->m_pMainWnd;
	//删除已有的路由表信息，除了前两个默认路由
	int count = Dlg->router_table_ctrl.GetCount();
	for (int i = 0; i < count-2; i++)			
	{
		Dlg->router_table_ctrl.DeleteString(2);
	}
	//逐一重新输出路由信息
	POSITION pos = Dlg->RouteTable.GetHeadPosition();
	while (pos != NULL)
	{
		RouteTable_t* item = Dlg->RouteTable.GetNext(pos);
		//判断修改的路由是否为自动生成的直接路由，跳过输出
		if ((int(item->destNet) == int(Dlg->IP_addr[0].IP&Dlg->IP_addr[0].netMask) && int(item->netMask) == int(Dlg->IP_addr[0].netMask))
			|| (int(item->destNet) == int(Dlg->IP_addr[1].IP&Dlg->IP_addr[1].netMask) && int(item->netMask) == int(Dlg->IP_addr[1].netMask)))
			continue;
		//将路由输出
		CString rt;
		rt.Format("%s -- %s -- %s",
			convert_IPaddr_to_str(item->netMask),
			convert_IPaddr_to_str(item->destNet),
			convert_IPaddr_to_str(item->nextHops));
		Dlg->router_table_ctrl.InsertString(Dlg->router_table_ctrl.GetCount(), rt);
	}
}

//添加路由
void CRouterDlg::OnBnClickedAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	//获取输入的路由信息
	DWORD des_net;
	des_net_ctrl.GetAddress(des_net);
	des_net = ntohl(des_net);
	DWORD netMask;
	netmask_ctrl.GetAddress(netMask);
	netMask = ntohl(netMask);
	DWORD nexthop_ip;
	next_hops_ctrl.GetAddress(nexthop_ip);
	nexthop_ip = ntohl(nexthop_ip);
	//判断添加的路由信息是否能够应用于本接口，即其是否属于本机网卡能够进行路由的
	if (!compIP(des_net&netMask, des_net))
	{
		MessageBox("添加的路由错误！", "ERROR", MB_OKCANCEL | MB_ICONERROR);
		return;
	}
	//通过判断其下一跳步的IP地址是否是本机网卡所在的物理网络
	bool InTheSameNet = false;
	for (int i = 0; i < 2; i++)
	{
		////assert(int(IP_addr[i].netMask&nexthop_ip) - int(IP_addr[i].netMask&IP_addr[i].IP)!=0);
		if (int(IP_addr[i].netMask&nexthop_ip) == int(IP_addr[i].netMask&IP_addr[i].IP))
		{
			InTheSameNet = true;
			break;
		}
	}
	if (!InTheSameNet)
	{
		MessageBox("添加的路由下一跳步无法抵达！", "ERROR", MB_OKCANCEL | MB_ICONERROR);
		return;
	}
	//判断路由信息是否和已有的重复如果重复进行提示，并修改为更新的路由表项
	POSITION pos = RouteTable.GetHeadPosition();
	while (pos != NULL)
	{
		RouteTable_t* item = RouteTable.GetAt(pos);
		//MessageBox(convert_IPaddr_to_str(item->destNet)+" "+convert_IPaddr_to_str(item->nextHops));
		if (int(item->destNet)==int(des_net)&&int(item->netMask)==int(netMask))			//路由关系已经存在
		{		
			//判断修改的路由是否为自动生成的直接路由，不允许修改
			if ((int(item->destNet) == int(IP_addr[0].IP&IP_addr[0].netMask)&&int(item->netMask )== int(IP_addr[0].netMask))
				|| (int(item->destNet) == int(IP_addr[1].IP&IP_addr[1].netMask)&&int(item->netMask) ==int( IP_addr[1].netMask)))
			{
				MessageBox("试图修改直接路由，非法请求未通过！", "ERROR", MB_OKCANCEL | MB_ICONERROR);
				return;
			}
			CString log;
			log.Format("$$添加路由已存在, 将修改为最新提交版本:\r\n\
>>> netmask:%s dest net:%s nexthop: %s(old)-->%s(new)\r\n",
				convert_IPaddr_to_str(netMask),
				convert_IPaddr_to_str(des_net),
				convert_IPaddr_to_str(item->nextHops),
				convert_IPaddr_to_str(nexthop_ip));
			item->nextHops = nexthop_ip;							//修改为最新的路由跳步
			CString log_t;											//记录日志信息
			log_ctrl.GetWindowTextA(log_t);
			log_ctrl.SetWindowTextA(log_t + log);
			MessageBox("添加路由已存在,将修改为最新提交版本！", "提示", MB_OKCANCEL | MB_ICONINFORMATION);
			updateRouteTable();										//刷新路由表内容之后退出函数
			return;
		}
		RouteTable.GetNext(pos);
	}
	//将添加的路由存入路由表，并进行显示
	RouteTable_t* routeItem = new RouteTable_t;
	routeItem->netMask =netMask;
	routeItem->destNet = des_net;
	routeItem->nextHops =nexthop_ip;		//下一跳步
	RouteTable.AddTail(routeItem);			//将路由保存
	CString rt;
	rt.Format("%s -- %s -- %s ",
		convert_IPaddr_to_str(routeItem->netMask),
		convert_IPaddr_to_str(routeItem->destNet),
		convert_IPaddr_to_str(routeItem->nextHops));
	router_table_ctrl.InsertString(router_table_ctrl.GetCount(), rt);
	CString log;
	log.Format("$$添加路由:netmask:%s dest net:%s nexthop: %s\r\n",
		convert_IPaddr_to_str(netMask),
		convert_IPaddr_to_str(des_net),
		convert_IPaddr_to_str(nexthop_ip));
	CString log_t;											//记录日志信息
	log_ctrl.GetWindowTextA(log_t);
	log_ctrl.SetWindowTextA(log_t + log );
	
}

//删除路由
void CRouterDlg::OnBnClickedDel()
{
	// TODO: 在此添加控件通知处理程序代码
	//获取输入的路由信息 ,注意主机序和网络序问题
	DWORD des_net;
	des_net_ctrl.GetAddress(des_net);
	des_net = ntohl(des_net);
	DWORD netMask;
	netmask_ctrl.GetAddress(netMask);
	netMask = ntohl(netMask);
	DWORD nexthop_ip;
	next_hops_ctrl.GetAddress(nexthop_ip);
	nexthop_ip = ntohl(nexthop_ip);
	//判断路由信息是否和已有的相同，相同则删除，否则提示不存在
	POSITION pos = RouteTable.GetHeadPosition();
	while (pos != NULL)
	{
		RouteTable_t* item = RouteTable.GetAt(pos);
		if (int(item->destNet) == int(des_net) && int(item->netMask) == int(netMask))			//路由关系已经存在
		{
			//判断修改的路由是否为自动生成的直接路由，不允许删除
			if ((int(item->destNet) == int(IP_addr[0].IP&IP_addr[0].netMask) && int(item->netMask) == int(IP_addr[0].netMask))
				|| (int(item->destNet) == int(IP_addr[1].IP&IP_addr[1].netMask) && int(item->netMask) == int(IP_addr[1].netMask)))
			{
				MessageBox("试图删除直接路由，非法请求未通过！", "ERROR", MB_OKCANCEL | MB_ICONERROR);
				return;
			}
			RouteTable.RemoveAt(pos);  
			CString log;
			log.Format("$$删除路由成功: netmask:%s dest net:%s nexthop: %s\r\n",
				convert_IPaddr_to_str(netMask),
				convert_IPaddr_to_str(des_net),
				convert_IPaddr_to_str(item->nextHops));
			CString log_t;											//记录日志信息
			log_ctrl.GetWindowTextA(log_t);
			log_ctrl.SetWindowTextA(log_t + log);
			updateRouteTable();										//刷新路由表内容之后退出函数
			return;
		}
		RouteTable.GetNext(pos);
	}
	MessageBox("删除的路由表项不存在！", "ERROR", MB_OKCANCEL | MB_ICONERROR);
}


void CRouterDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//获取主窗口句柄，非类成员可通过此句柄获取窗口类成员并修改
	CRouterDlg*  Dlg = (CRouterDlg*)AfxGetApp()->m_pMainWnd;
	mMutex.Lock(INFINITE);
	POSITION pos = Dlg->sendIPFrame_list.GetHeadPosition();
	while (pos != NULL)
	{
		if (Dlg->sendIPFrame_list.GetAt(pos)->n_mTimer == nIDEvent)
		{
			Dlg->sendIPFrame_list.RemoveAt(pos);				//缓冲区超时
			backTimerID(nIDEvent);								//回收ID
			CDialogEx::OnTimer(nIDEvent);
			return;
		}
		Dlg->sendIPFrame_list.GetNext(pos);
	}
	mMutex.Unlock();
	CDialogEx::OnTimer(nIDEvent);
}
//获取定时器的ID
UINT_PTR getTimerID() {
	//获取主窗口句柄，非类成员可通过此句柄获取窗口类成员并修改
	CRouterDlg*  Dlg = (CRouterDlg*)AfxGetApp()->m_pMainWnd;
	UINT_PTR ID = 0;
	for (int i = 1; i < IPFrameBufLen; i++)
	{
		if (Dlg->SendTimerLimit[i] != 0)
		{
			ID = Dlg->SendTimerLimit[i];
			Dlg->SendTimerLimit[i] = 0;
			break;
		}
	}
	//assert(ID >= 1||ID==0);
	//assert(ID < IPFrameBufLen);
	return ID;
}
//回收定时器ID
void backTimerID(UINT_PTR ID) {
	//获取主窗口句柄，非类成员可通过此句柄获取窗口类成员并修改
	CRouterDlg*  Dlg = (CRouterDlg*)AfxGetApp()->m_pMainWnd;
	if (ID >= IPFrameBufLen) return;
	//assert(ID >= 1);
	//assert(ID < IPFrameBufLen);
	Dlg->KillTimer(ID);
	Dlg->SendTimerLimit[ID] = ID;
}


