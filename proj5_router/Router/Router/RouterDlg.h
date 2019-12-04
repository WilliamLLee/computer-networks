
// RouterDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

//引入pcap类
#include "pcap.h"  

#define IPFrameBufLen 16


//帧数据结构定义
#pragma pack(1)
typedef struct FrameHeader_t {		//帧首部
	BYTE DesMAC[6];					//目的地址
	BYTE SrcMAC[6];					//原地址
	WORD FrameType;					//帧类型
}FrameHeader_t;

typedef struct ARPFrame_t {		  // ARP帧
	FrameHeader_t	FrameHeader;  // 帧首部
	WORD			HardwareType; // 硬件类型
	WORD			ProtocolType; // 协议类型
	BYTE			HLen;         // 硬件地址长度
	BYTE			PLen;         // 协议地址长度
	WORD			Operation;    // 操作值
	UCHAR			SendHa[6];    // 源MAC地址
	ULONG			SendIP;       // 源IP地址
	UCHAR			RecvHa[6];    // 目的MAC地址
	ULONG			RecvIP;       // 目的IP地址
} ARPFrame_t;



typedef struct IPHeader_t {		  // IP首部
	BYTE	Ver_HLen;             // 版本+头部长度
	BYTE	TOS;                  // 服务类型
	WORD	TotalLen;             // 总长度
	WORD	ID;                   // 标识
	WORD	Flag_Segment;         // 标志+片偏移
	BYTE	TTL;                  // 生存时间
	BYTE	Protocol;             // 协议
	WORD	Checksum;             // 头部校验和
	ULONG	SrcIP;                // 源IP地址
	ULONG	DstIP;                // 目的IP地址
} IPHeader_t;


typedef struct ICMPHeader_t {     // ICMP首部
	BYTE    Type;                 // 类型
	BYTE    Code;                 // 代码
	WORD    Checksum;             // 校验和
	WORD    Id;                   // 标识
	WORD    Sequence;             // 序列号
} ICMPHeader_t;

typedef struct Data_t {
	FrameHeader_t FrameHeader;
	IPHeader_t  IPHeader;
}Data_t;

//路由表项
typedef struct RouteTable_t {
	DWORD	netMask;			//子网掩码
	DWORD	destNet;			//目的网络
	DWORD	nextHops;			//下一跳步地址
}RouteTable_t;

//IP MAC地址映射表项
typedef struct IP_MAC_t {
	DWORD  IP;
	BYTE   MAC[6];
}IP_MAC_t;

//IP 地址信息存储结构体
typedef struct IP_Info_t {
	DWORD  IP;
	DWORD  netMask;
	BYTE   MAC[6];
}IP_Info_t;


typedef struct SendPacket_t {	  // 发送数据包结构
	int				len;          // 长度
	BYTE			PktData[2000];		// 数据缓存
	ULONG			TargetIP;     // 目的IP地址
	UINT_PTR		n_mTimer;     // 定时器
} SendPacket_t;

#pragma pack()


// CRouterDlg 对话框
class CRouterDlg : public CDialogEx
{
// 构造
public:
	CRouterDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ROUTER_DIALOG };
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
//WinPcap使用相关数据成员定义
	//错误信息缓存区
	char errbuf[PCAP_ERRBUF_SIZE];
	// 所有的网络设备列表，指针指向第一个设备列表的地址
	pcap_if_t* alldevs;
	// 记录选中的接口信息
	pcap_if_t *d;
	// 指向接口地址列表
	pcap_addr_t* a;
	// 记录打开的pcap接口
	pcap_t* opened_pcap;
	// 记录本机的MAC地址
	BYTE*  host_MAC;
	//网卡IP地址,两个
	IP_Info_t IP_addr[2];
	//路由表
	CList<RouteTable_t*> RouteTable;
	//IPMAC映射表
	CList<IP_MAC_t*> IP_MAC;
//线程控制参数
	//停止ARP数据包捕获线程
	volatile bool stop_ARP_Catch_Thread;	
	//停止路由数据包捕获线程
	volatile bool stop_Route_Catch_Thread;
	//ARP报文捕获线程
	CWinThread* m_ARPCaptureThread;
	//路由报文捕获线程
	CWinThread* m_RouteCaptureThread;
//对话框设计及交互控件
	// 设备列表显示窗口控件
	CListBox EtherNet_interface_ctrl;
	// 日志显示窗口
	CEdit log_ctrl;
	// 路由表显示窗口
	CListBox router_table_ctrl;
	// 子网掩码输入窗口
	CIPAddressCtrl netmask_ctrl;
	// 目的网络输入控件
	CIPAddressCtrl des_net_ctrl;
	// 下一跳步输入窗口
	CIPAddressCtrl next_hops_ctrl;
//数据包缓冲区
	//记录捕获的路由数据包包头
	CList<pcap_pkthdr*> pkthdr_list;
	//记录捕获的路由数据包
	CList<const u_char*> pktdata_list;
	//记录捕获的ARP应答报文缓冲区
	CList<const u_char*> ARP_pktdata_list;
	//发送缓冲区
	CList<SendPacket_t*> sendIPFrame_list;
	//发送缓冲区最多保存十五个待发送的数据帧
	UINT_PTR SendTimerLimit[IPFrameBufLen];
//相关函数声明
	//双击目标网卡，检测获得该网卡上的IP地址以及MAC地址，并初始化路由器路由环境
	afx_msg void OnLbnDblclkList2();
	//路由转发函数
	LRESULT OnROUTEPacket(WPARAM wParam, LPARAM lParam);
	//ARP报文处理函数
	LRESULT OnARPPacket(WPARAM wParam, LPARAM lParam);
	//路由环境初始化
	LRESULT OnBeginRoute(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedDel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

// 将14字节地址协议地址转为4字节IP地址类型
DWORD GetAddr_IP(sockaddr* addr);
// 将sockaddr中地址转换为字符串返回
CString convert_sockaddr_to_str(sockaddr* addr);
// 将IPaddr中地址转换为字符串返回
CString convert_IPaddr_to_str(DWORD addr);
// 将MAC addr中地址转换为字符串返回
CString convert_MAC_to_str(BYTE* MAC);
// 传入参数，制作ARP数据包，并将数据包发送,发送成功返回true
bool SendARP(pcap_t* opened_pcap, BYTE* SendHa_t, DWORD SendIP_t, DWORD RecvIP_t);
//线程执行函数，接收ARP数据包
UINT RecvARP(PVOID hwnd);
//线程执行函数，接收需要路由的数据包
UINT RecvRouteP(PVOID hwnd);
//赋值MAC地址
void CopyMAC(BYTE* MAC1, BYTE*MAC2);
//刷新显示的路由表
void updateRouteTable();
// IP头部检验算法
WORD IPHeader_ckeck(WORD* IPHeader);
// ICMP头部检验算法
WORD ICMPHeader_ckeck(WORD* ICMPHeader);
//判断MAC地址是否相等
bool compMAC(BYTE* m1, BYTE* m2);
//判断IP地址是否相等
bool compIP(DWORD ip1, DWORD ip2);
//存储IPMAC映射关系
bool saveIP_MAC(DWORD IP, BYTE* MAC);
//获取IP对应的MAC地址
BYTE* getMACForIP(DWORD IP);
//转发数据包的函数
bool sendIPFrame(pcap_t* opened_pcap, u_char* IPFrame, int Framelen);
//发送ICMP报文
bool sendICMP(pcap_t* opened_pcap, BYTE type,BYTE code, const u_char* pkt_data);		
//获取定时器的ID
UINT_PTR getTimerID();
//回收定时器ID
void backTimerID(UINT_PTR ID );
  


