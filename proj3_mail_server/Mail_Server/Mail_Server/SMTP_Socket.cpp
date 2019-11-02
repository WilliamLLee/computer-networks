#include "stdafx.h"
#include "SMTP_Socket.h"
#include "Mail_ServerDlg.h"
#include "base64.h"

SMTP_Socket::SMTP_Socket()
{
	begin_data_recv = false;
}


SMTP_Socket::~SMTP_Socket()
{
}


void SMTP_Socket::OnAccept(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	//获取主窗口句柄
	CMail_ServerDlg* Dlg = (CMail_ServerDlg*)AfxGetApp()->m_pMainWnd;
	//记录日志
	Dlg->log_list_ctrl.InsertString(Dlg->log_list_ctrl.GetCount(), L"***收到连接请求");
	//创建负责连接的套接字
	SMTP_Socket * pSocket = new SMTP_Socket();
	if (Accept(*pSocket))
	{
		char* t = "220 Simple Mail Server Ready for Mail\r\n";
		pSocket->Send(t, strlen(t));
		pSocket->AsyncSelect(FD_READ);			//调用接收函数接收回应
		Dlg->log_list_ctrl.InsertString(Dlg->log_list_ctrl.GetCount(), L"***建立连接");
		//记录日志
		Dlg->log_list_ctrl.InsertString(
			Dlg->log_list_ctrl.GetCount(),
			L"S:220 Simple Mail Server Ready for Mail");
	}
	else
	{
		Dlg->log_list_ctrl.InsertString(Dlg->log_list_ctrl.GetCount(), L"***连接错误");
	}
	CAsyncSocket::OnAccept(nErrorCode);
}



void SMTP_Socket::OnReceive(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	//获取主界面句柄
	CMail_ServerDlg* Dlg = (CMail_ServerDlg*)AfxGetApp()->m_pMainWnd;
	memset(lbuf, 0, bufferlen);
	//接收报文
	int len = Receive(lbuf, strlen(lbuf),0);
	if (len == -1)		//错误处理
	{
		Dlg->log_list_ctrl.InsertString(Dlg->log_list_ctrl.GetCount(), L"***接收消息错误");
		return;
	}
	//转换格式
	CString recv_info;
	recv_info += lbuf;
	//如果没有开始接收数据，输出接收到的命令并进行响应
	if (!begin_data_recv)
	{
		Dlg->log_list_ctrl.InsertString(Dlg->log_list_ctrl.GetCount(), L"C:" + recv_info.Left(len));
		if (recv_info.Left(4) == "EHLO")
		{
			//发送回应
			char *ret= "250 192.168.56.1\r\n";
			Send(ret, strlen(ret));
			//触发接收报文响应函数
			AsyncSelect(FD_READ);
			//记录日志
			Dlg->log_list_ctrl.InsertString(
				Dlg->log_list_ctrl.GetCount(),
				L"S:" + (CString)ret);
			return;
		}
		else if (recv_info.Left(4)=="NOOP")
		{
			char *ret = "250 OK No Operation\r\n";
			Send(ret, strlen(ret));
			//触发接收报文响应函数
			AsyncSelect(FD_READ);
			//记录日志
			Dlg->log_list_ctrl.InsertString(
				Dlg->log_list_ctrl.GetCount(),
				L"S:" + (CString)ret);
			return;
		}
		else if (recv_info.Left(10) == "MAIL FROM:")
		{
			char *ret = "250 Sender OK\r\n";
			Send(ret, strlen(ret));
			//触发接收报文响应函数
			AsyncSelect(FD_READ);
			//记录日志
			Dlg->log_list_ctrl.InsertString(
				Dlg->log_list_ctrl.GetCount(),
				L"S:" + (CString)ret);
			return;
		}
		else if ( recv_info.Left(8) == "RCPT TO:")
		{
			char *ret = "250 Receiver OK\r\n";
			Send(ret, strlen(ret));
			//触发接收报文响应函数
			AsyncSelect(FD_READ);
			//记录日志
			Dlg->log_list_ctrl.InsertString(
				Dlg->log_list_ctrl.GetCount(),
				L"S:" + (CString)ret);
			return;
		}
		else if (recv_info.Left(4) == "QUIT"| recv_info.Left(4)=="RSET")
		{

			char *ret = "221 Quit, Goodbye\r\n";
			Send(ret, strlen(ret));
			//记录日志
			Dlg->log_list_ctrl.InsertString(
				Dlg->log_list_ctrl.GetCount(),
				L"S:" + (CString)ret);
			//关闭连接
			Close();
			return;
		}
		else  if (recv_info.Left(4) == "DATA")
		{
			if (!begin_data_recv)
			{
				//开始接收数据，设置标志位
				begin_data_recv = true;		
			   //清空当前文本区域内的文本空间
				Dlg->mail_text_ctrl.SetWindowTextW(L"");
			}
			//发送响应
			char *ret = "354 Go ahead.End with <CRLF>.<CRLF>\r\n";
			Send(ret, strlen(ret));
			//触发接收报文响应函数
			AsyncSelect(FD_READ);
			//记录日志
			Dlg->log_list_ctrl.InsertString(
				Dlg->log_list_ctrl.GetCount(),
				L"S:" + (CString)ret);
			return;
		}
		else
		{
			char *ret = "500 order is wrong\r\n";
			Send(ret, strlen(ret));
			//触发接收报文响应函数
			AsyncSelect(FD_READ);
			//记录日志
			Dlg->log_list_ctrl.InsertString(
				Dlg->log_list_ctrl.GetCount(),
				L"S:" + (CString)ret);
			//关闭连接
			Close();
			return;
		}
	}else    //接收数据报文并显示在邮件显示区
	{
		//显示收到的邮件报文数据
		CString mail_text;
		Dlg->mail_text_ctrl.GetWindowTextW(mail_text);
		mail_text += recv_info.Left(len);
		Dlg->mail_text_ctrl.SetWindowTextW(mail_text);

		if (recv_info.Find(_T("\r\n.\r\n")) != -1)
		{
			begin_data_recv = false;
			char *ret = "250 Message accepted for delivery\r\n";
			Send(ret, strlen(ret));
			//记录日志
			Dlg->log_list_ctrl.InsertString(
				Dlg->log_list_ctrl.GetCount(),
				L"S:" + (CString)ret);
			////将已经接收到的邮件数据逐行存入队列，实现记录多封邮件的功能
			//int linecount = Dlg->mail_text_ctrl.GetLineCount();
			//mail_t* temp = new mail_t;
			//for (int i = 0; i < linecount; i++)
			//{
			//	TCHAR line[bufferlen];
			//	Dlg->mail_text_ctrl.GetLine(i, line);
			//	temp->mail_text_line.AddTail(line);
			//}
			//Dlg->mail_list.AddTail(temp);
		}
		//触发接收报文响应函数
		AsyncSelect(FD_READ);
	}
	CAsyncSocket::OnReceive(nErrorCode);
}
