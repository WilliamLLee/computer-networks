#pragma once
#include "afxsock.h"
#include "defines.h"

class ServerSocket :
	public CAsyncSocket
{
public:
	ServerSocket();
	~ServerSocket();
	//接收报文响应函数
	virtual void OnReceive(int nErrorCode);
	//获取定时器的ID
	UINT_PTR getTimerID();
	//回收定时器ID
	void backTimerID(UINT_PTR ID);

	//用户名与密码保存文件
	CList<UserInfo_t*> UserInfo_list;
	//接收到的确认序列号
	DWORD recvACK;
	//接收到相同确认序列号的次数
	int	  sameTimes;
	//接收到的上一个seq编号
	DWORD recvSEQ;
	//当前发送序列号
	DWORD Seq;
	//当前已确认接收报文序列号
	DWORD Ack;
	//标识当前连接的状态
	int status;
	//服务器返回的用户ID
	WORD userID;
	//当前文件目录
	CString currentDir;
	//发送缓冲区
	CList<SendPacket_t*> sendPKT_list;
	//发送缓冲区计时器使用与分配，限制发送缓冲区的大小
	UINT_PTR  sendTimerLimit[TIMERLIMITNUM];
};

