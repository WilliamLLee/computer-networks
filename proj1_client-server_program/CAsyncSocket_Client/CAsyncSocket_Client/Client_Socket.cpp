#include "stdafx.h"
#include "Client_Socket.h"
#include "CAsyncSocket_ClientDlg.h"

Client_Socket::Client_Socket()
{
}


Client_Socket::~Client_Socket()
{
}


void Client_Socket::OnReceive(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	CCAsyncSocket_ClientDlg* pDlg = (CCAsyncSocket_ClientDlg*)(AfxGetApp()->GetMainWnd());
	TCHAR  lBuffer[4096](L"");
	CString Server_IP;
	UINT Server_Port;
	int m_length = sizeof(lBuffer);
	m_length = ReceiveFrom(lBuffer, m_length, Server_IP, Server_Port, 0);
	/*CString str;
	str.Format(L"%s,%s,%d", lBuffer, Server_IP, Server_Port);
	pDlg->MessageBox(str);*/
	if (m_length != -1)
	{
		pDlg->response_m = lBuffer;
		pDlg->UpdateData(false);
		pDlg->KillTimer(1);		//成功接收消息，关闭计时器
		pDlg->resend_count = 0;	//重置重发计数器为0
		CString str;
		str.Format(L":接收 IP:%s Port: %d 响应【%s】" , Server_IP  , Server_Port , lBuffer );
		pDlg->command_log_ctrl.InsertString(0, pDlg->getDateTime() + str);
	}
	else
	{
		pDlg->command_log_ctrl.InsertString(0, pDlg->getDateTime() + L"ERROR:未接收到回传报文，请重发或检查相关设置！");
	}
	CAsyncSocket::OnReceive(nErrorCode);
}

