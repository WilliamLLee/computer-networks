
// Packet_CaptureDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
//引入pcap类
#include "pcap.h"  

// CPacket_CaptureDlg 对话框
class CPacket_CaptureDlg : public CDialogEx
{
// 构造
public:
	CPacket_CaptureDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PACKET_CAPTURE_DIALOG };
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
	//以太网设备列表显示控件
	CListBox EtherNet_interface_ctrl;
	// 捕获接口详细信息listbox控件
	CListBox interface_detail_ctrl;
	// 过滤条件
	CEdit condition_ctrl;
	// 捕获信息显示窗口控件
	CListBox packet_list_ctrl;
	// 所有的网络设备列表，指针指向第一个设备列表的地址
	pcap_if_t* alldevs;
	//错误信息缓冲区
	char errbuf[PCAP_ERRBUF_SIZE];		
	//记录选中的接口信息
	pcap_if_t *d;
	//指向接口地址列表
	pcap_addr_t a;
	//响应操作请求的消息处理函数
	afx_msg void OnLbnDblclkList1();		//选中相应的接口响应函数
	afx_msg void OnBnClickedCancel();		//退出
	afx_msg void OnBnClickedButton1();		//捕获
	afx_msg void OnBnClickedButton2();		//停止捕获
	//记录打开的pcap接口
	pcap_t* opened_pcap;
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
	// IP头部检验算法
	WORD IPHeader_ckeck(WORD*  IPHeader);
};


//帧数据，IP数据包数据结构定义
#pragma pack(1)
typedef struct FrameHeader_t{		//帧首部
	BYTE DesMAC[6];					//目的地址
	BYTE SrcMAC[6];					//原地址
	WORD FrameType;					//帧类型
}FrameHeader_t;

typedef struct IPHeader_t {
	BYTE Ver_HLen;
	BYTE TOS;
	WORD TotalLen;
	WORD ID;
	WORD Flag_Segment;
	BYTE TLL;
	BYTE Protocol;
	WORD Checksum;
	ULONG SrcIP;
	ULONG DstIP;
}IPHeader_t;

typedef struct Data_t {
	FrameHeader_t FrameHeader;
	IPHeader_t  IPHeader;
}Data_t;

#pragma pack()