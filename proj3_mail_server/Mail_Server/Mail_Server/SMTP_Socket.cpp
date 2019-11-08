#include "stdafx.h"
#include "SMTP_Socket.h"
#include "Mail_ServerDlg.h"

SMTP_Socket::SMTP_Socket()
	: recv_post(_T(""))
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
	Dlg->log_list_ctrl.InsertString(Dlg->log_list_ctrl.GetCount(), "***收到连接请求");
	//创建负责连接的套接字
	SMTP_Socket * pSocket = new SMTP_Socket();
	if (Accept(*pSocket))
	{
		char* t = "220 Simple Mail Server Ready for Mail\r\n";
		pSocket->Send(t, strlen(t));
		pSocket->AsyncSelect(FD_READ);			//调用接收函数接收回应
		Dlg->log_list_ctrl.InsertString(Dlg->log_list_ctrl.GetCount(), "***建立连接");
		//记录日志
		Dlg->log_list_ctrl.InsertString(
			Dlg->log_list_ctrl.GetCount(),
			"S:220 Simple Mail Server Ready for Mai");
	}
	else
	{
		Dlg->log_list_ctrl.InsertString(Dlg->log_list_ctrl.GetCount(), "***连接错误");
	}
	CAsyncSocket::OnAccept(nErrorCode);
}




void SMTP_Socket::OnReceive(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	//获取主界面句柄
	CMail_ServerDlg* Dlg = (CMail_ServerDlg*)AfxGetApp()->m_pMainWnd;
	//接收报文
	int len = Receive(lbuf, sizeof(lbuf),0);
	if (len == -1)		//错误处理
	{
		Dlg->log_list_ctrl.InsertString(Dlg->log_list_ctrl.GetCount(), "***接收消息错误");
		return;
	}
	else if (len == 0)
	{
		Dlg->log_list_ctrl.InsertString(Dlg->log_list_ctrl.GetCount(), "***接收报文为空");
		return;
	}
	//转换格式
	CString recv_info =(CString)lbuf;
	//如果没有开始接收数据，输出接收到的命令并进行响应
	if (!begin_data_recv)
	{
		Dlg->log_list_ctrl.InsertString(Dlg->log_list_ctrl.GetCount(), "C:" + recv_info.Left(len));
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
				"S:" + (CString)ret);
			return;
		}
		else if (recv_info.Left(4)== "NOOP")
		{
			char *ret = "250 OK No Operation\r\n";
			Send(ret, strlen(ret));
			//触发接收报文响应函数
			AsyncSelect(FD_READ);
			//记录日志
			Dlg->log_list_ctrl.InsertString(
				Dlg->log_list_ctrl.GetCount(),
				"S:" + (CString)ret);
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
				"S:" + (CString)ret);
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
				"S:" + (CString)ret);
			return;
		}
		else if (recv_info.Left(4) == "QUIT"||recv_info.Left(4)== "RSET")
		{

			char *ret = "221 Quit, Goodbye\r\n";
			Send(ret, strlen(ret));
			//记录日志
			Dlg->log_list_ctrl.InsertString(
				Dlg->log_list_ctrl.GetCount(),
				"S:" + (CString)ret);
			//关闭连接
			this->Close();
			return;
		}
		else  if (recv_info.Left(4) == "DATA")
		{
			if (!begin_data_recv)
			{
				//开始接收数据，设置标志位
				begin_data_recv = true;		
			   //清空当前文本区域内的文本空间
				Dlg->mail_text_ctrl.SetWindowTextA("");
			}
			//发送响应
			char *ret = "354 Go ahead.End with <CRLF>.<CRLF>\r\n";
			Send(ret, strlen(ret));
			//触发接收报文响应函数
			AsyncSelect(FD_READ);
			//记录日志
			Dlg->log_list_ctrl.InsertString(
				Dlg->log_list_ctrl.GetCount(),
				"S:" + (CString)ret);
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
				"S:" + (CString)ret);
			//关闭连接
			this->Close();
			return;
		}
	}else    //接收数据报文并显示在邮件显示区
	{	
		//接收多个连接，将相应的数据先独立存好
		recv_post += recv_info.Left(len);

		if (recv_info.Find("\r\n.\r\n") != -1)
		{
			//结束接收数据
			begin_data_recv = false;
			//将本次连接接收的报文输出到显示报文的文本框中
			Dlg->mail_text_ctrl.SetWindowTextA(recv_post);

			////延迟发送确认收到消息，用于测试程序的并发通信
			//srand((unsigned)time(NULL));
			//if((rand()%15)<3)   //随机延迟
			//	Sleep(10000);

			//发送数据接收结束的响应
			char *ret = "250 Message accepted for delivery\r\n";
			Send(ret, strlen(ret));
			//记录日志
			Dlg->log_list_ctrl.InsertString(
				Dlg->log_list_ctrl.GetCount(),
				"S:" + (CString)ret);
			//将报文传入全局的报文缓存队列
			mail_t* cur_mail = new mail_t;
			Dlg->mail_text_ctrl.GetWindowTextA(cur_mail->mail_post);
			Dlg->mail_list.AddTail(cur_mail);
			//更新接收邮件数目
			CString cur;
			cur.Format("%d", Dlg->mail_list.GetCount());
			Dlg->cur_ctrl.SetWindowTextA(cur);
			Dlg->total_ctrl.SetWindowTextA(cur);
			Dlg->UpdateData(true);
			//发送显示邮件内容消息，让主线程解析邮件，进行内容显示
			Dlg->PostMessage(WM_DISPLAYMAIL,0,0);
		}
		//触发接收报文响应函数
		AsyncSelect(FD_READ);
	}
	//memset(lbuf, 0, sizeof(lbuf) / sizeof(char));
	CAsyncSocket::OnReceive(nErrorCode);
}
