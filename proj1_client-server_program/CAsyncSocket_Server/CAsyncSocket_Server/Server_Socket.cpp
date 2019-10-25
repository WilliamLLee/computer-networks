#include "stdafx.h"
#include "Server_Socket.h"
#include "CAsyncSocket_ServerDlg.h"

Server_Socket::Server_Socket()
{
}


Server_Socket::~Server_Socket()
{
}

void Server_Socket::OnReceive(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	TCHAR lBuffer[4096](L"");   //这里需要进行初始化，否则会报
	int m_length = sizeof(lBuffer);        // 缓冲区的长度 
	CString Client_IP ;						//IP地址
	UINT Client_port;						// 端口号
	// 获取对话框句柄
	CCAsyncSocket_ServerDlg* pDlg  = (CCAsyncSocket_ServerDlg*)(AfxGetApp()->GetMainWnd());
	// 获取报文,返回值为接收报文的长度
	m_length = ReceiveFrom(lBuffer , 8192, Client_IP, Client_port, 0);
	//获取日期时间并转为字符串形式
	CString log;
	CTime tm; tm = CTime::GetCurrentTime();
	CString date, time;
	date.Format(L"%d/%d/%d", tm.GetYear(), tm.GetMonth(), tm.GetDay());
	time.Format(L"%d:%d:%d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());
	
	if (m_length!=-1)		// 错误处理
	{
		CString command = CString(lBuffer).MakeLower();
		CString response; 
		if (command == L"date")
		{
			response=date;
		}
		else if (command == L"time")
		{
			response = time;
		}
		else
		{
			response.Format(L"错误请求");
		}
		// 以下代码回传响应结果，并且判断回传是否成功，如果不成功，解记录错误日志
		if (SendTo(response.GetBuffer(),
			response.GetLength()*sizeof(TCHAR),
			Client_port,
			Client_IP))
		{
			log.Format(L"%s %s:收到IP=%s Port=%d 请求【%s】,响应【%s】", date, time, Client_IP, Client_port, lBuffer, response);
		}
		else
			log.Format(L"ERROR：【消息回传错误】%s %s:收到IP=%s Port=%d 请求【%s】,响应【%s】", date, time, Client_IP, Client_port, lBuffer, response);
		pDlg->list_ctrl.InsertString(0, log);
	}
	else
	{
		pDlg->MessageBox(L"接收报文出现错误！");
		log.Format(L"ERROR：【接收报文错误】%s %s:收到IP=  Port=   请求【】,响应【】", date, time);
		pDlg->list_ctrl.InsertString(0, log);
	}
	CAsyncSocket::OnReceive(nErrorCode);
}
