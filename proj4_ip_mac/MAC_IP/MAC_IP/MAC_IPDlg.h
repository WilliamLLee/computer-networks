
// MAC_IPDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

//引入pcap类
#include "pcap.h"  

#pragma pack(1)
typedef struct FrameHeader_t {  //帧首部
	BYTE DesMAC[6];
	BYTE SrcMAC[6];
	WORD FrameType;
}FrameHeader_t;

typedef struct ARPFrame_t {   //ARP帧
	FrameHeader_t FrameHeader;
	WORD		  HardwareType;
	WORD		  ProcotolType;
	BYTE		  HLen;
	BYTE		  PLen;
	WORD		  Openration;
	BYTE		  SendHa[6];
	DWORD		  SendIP;
	BYTE		  RecvHa[6];
	DWORD		  RecvIP;
}ARPFrame_t;

#pragma pack()



// CMAC_IPDlg 对话框
class CMAC_IPDlg : public CDialogEx
{
// 构造
public:
	CMAC_IPDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAC_IP_DIALOG };
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
	//错误信息缓存区
	char errbuf[PCAP_ERRBUF_SIZE];
	// 设备列表显示窗口控件
	CListBox EtherNet_interface_ctrl;
	// 接口详情显示窗口控件
	CEdit interface_detail_ctrl;
	//响应获取IPMAC对应关系
	afx_msg void OnBnClickedGetipMac();
	// IP MAC 对应关系显示窗口
	CEdit IP_MAC_ctrl;
	//响应打开设备接口
	afx_msg void OnLbnDblclkList1();
	// 所有的网络设备列表，指针指向第一个设备列表的地址
	pcap_if_t* alldevs;
	// 记录选中的接口信息
	pcap_if_t *d;
	// 指向接口地址列表
	pcap_addr_t* a;
	// 将地址转换为字符串返回	
	CString convert_addr_to_str(sockaddr* addr);
	//记录捕获的数据包头
	CList<pcap_pkthdr*> pkthdr_list;
	//记录捕获的数据包
	CList<const u_char*> pktdata_list;
	// 当前创建的捕获数据包线程指针
	CWinThread* m_capture;
	// 处理捕获数据包的函数
	LRESULT OnPacket(WPARAM wParam, LPARAM lParam);
	// 终止进程的标志
	int stop_thread;
	// 记录打开的pcap接口
	pcap_t* opened_pcap;
	afx_msg void OnBnClickedCancel();
	// ARP帧数据包
	ARPFrame_t *ARP_Frame;
	// 传入参数，制作ARP数据包,其中帧头部的目的MAC地址默认为广播地址
	ARPFrame_t* mk_ARPFrame(BYTE* SendHa_t, DWORD SendIP_t, BYTE* RecvHa_t, DWORD RecvIP_t, BYTE* SrcMAC_t, BYTE* DesMAC_t = NULL);
	// 将14字节地址协议地址转为4字节IP地址类型
	DWORD GetAddr_IP(sockaddr* addr);
	// 记录本机的MAC地址
	BYTE* host_MAC;
	// 等待获得本机MAC地址的循环次数，防止陷入死循环
	int waiter;
	// 记录本机选取的接口设备信息
	CString interface_info;
	// 处理接收到IPMAC地址映射之后的显示函数
	LRESULT OnDisplayInfo(WPARAM wParam, LPARAM lParam);
	// 输入IP地址
	CIPAddressCtrl IPAddr_ctrl;
};

//报文捕获线程控制函数声明
UINT Capturer(PVOID hwnd);

