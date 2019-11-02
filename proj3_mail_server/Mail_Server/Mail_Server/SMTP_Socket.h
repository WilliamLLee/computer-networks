#pragma once
#include "afxsock.h"
#define bufferlen 8192

//自定义socket类，负责完成邮件服务器相关数据交互和报文接收工作
class SMTP_Socket :					
	public CAsyncSocket
{
public:
	SMTP_Socket();
	~SMTP_Socket();
	//连接响应函数
	virtual void OnAccept(int nErrorCode);
	//接收缓存
	char lbuf[bufferlen];
	//标记数据是否开始传送
	bool begin_data_recv;
	//接收消息响应函数
	virtual void OnReceive(int nErrorCode);
};

