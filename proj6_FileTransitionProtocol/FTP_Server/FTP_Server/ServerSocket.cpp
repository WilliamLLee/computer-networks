#include "stdafx.h"
#include "ServerSocket.h"
#include  "FTP_ServerDlg.h"

#pragma warning(disable : 4996)

typedef struct DownloadFile_t {
	WORD DataID;			//文件ID
	CString filename;		//文件名
	ServerSocket* server;	//套接字，获取SEQ和ACK数据
	CString toIP;
	UINT port;
} DownloadFile_t;

ServerSocket::ServerSocket()
{
	//初始化定时器ID池
	for (int i = 0; i < TIMERLIMITNUM; i++)
	{
		sendTimerLimit[i] = i;
	}
	//初始化确认序列号和发送序列号
	Ack = 0;
	Seq = 0;
	sameTimes = 0;
	recvACK = 0;
	recvSEQ = 0;
	status = start;
	currentDir = ".";
	//初始化一个test用户
	UserInfo_t * test = new UserInfo_t;
	test->userName = "test";
	test->password = "test";
	UserInfo_list.AddTail(test);
}


ServerSocket::~ServerSocket()
{
}
//定义连接请求响应的命令
/*
命令：
USER 发送用户名
PASS 发送用户密码
AUTH 注册
LIST 获取远程服务器用户目录下所有文件列表，后接参数即为打开特定文件夹
STOR 上传本机文件到远程目录的即时目录下
RETR 获取远程目录下的文件，支持多文件同时下载
MDIR 在远程用户目录下创建文件夹
DELE 在远程用户目录下删除文件夹或文件

响应：
100 获取连接请求成功
200 操作成功响应
404 目标对象不存在
500 操作失败响应
332 验证通过
333 注册成功，创建用户目录
334 密码错误
335 获取目录成功
*/

void ServerSocket::OnReceive(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	// TODO: 在此添加专用代码和/或调用基类
	if (status == finished)													//断开连接后不再接收响应
		return;
	CFTP_ServerDlg*  Dlg = (CFTP_ServerDlg*)(AfxGetApp()->GetMainWnd());
	//设置接收缓冲区
	const int BUFFERLEN = 4096;
	u_char buffer[BUFFERLEN];
	CString serverIP;		//IP 
	UINT serverPort;		//Port
							//接收数据
	int byteLen = ReceiveFrom(buffer, BUFFERLEN, serverIP, serverPort, 0);
	if (byteLen < 0)
	{
		Dlg->log("连接请求数据接收出错！");
		return;
	}

	if (ChecksumCompute((unsigned short *)buffer, byteLen) != 0)			//差错检验
	{
		Dlg->log("数据包校验和计算错误！");
		return;
	}
	//数据接收正常，进行命令分析
	FTPHeader_t* FTPH = (FTPHeader_t*)(buffer);					//FTP首部

																//输出ACK SEQ数据日志
	CString logText;
	logText.Format("recv ACK %d SEQ %d from IP:%s Port:%d", FTPH->ACKNO, FTPH->SEQNO, serverIP, serverPort);
	Dlg->log(logText);

	//对数据报头部进行判断
	if (FTPH->ACKNO == recvACK)									//接收重复ACK，计数
	{
		sameTimes++;
		if (sameTimes == 4)										//接收到三次重复的冗余ACK,需要重传之后的所有数据包
		{
			//重传recvACK-Seq之间的所有数据报文,由于缓冲区中从头到尾的发送序列号递增且均小于SEQ，所以全部需要重传
			POSITION pos = sendPKT_list.GetHeadPosition();
			while (pos != NULL)
			{
				SendPacket_t* item = sendPKT_list.GetAt(pos);
				FTPHeader_t* header = ((FTPHeader_t*)(&item->PktData));
				if (header->SEQNO >= recvACK || header->SEQNO <= Seq)
				{
					this->SendTo(item->PktData, item->len, item->TargetPort, item->TargetIP, 0);
					CString logText;
					logText.Format("接收冗余三次ACK，重传报文: SEQ = %d", header->SEQNO);
					Dlg->log(logText);
					item->ResendTime++;			//重传次数记录自增1
				}
				sendPKT_list.GetNext(pos);
			}
		}
	}
	else  if (((FTPHeader_t*)(sendPKT_list.GetHead()->PktData))->SEQNO == FTPH->ACKNO - 1)	//按序接收ACK,由于缓冲区中的SEQ按序排列，因此只需要比对缓冲区中第一个即可
	{
		CString logText;
		logText.Format("从缓冲区移除报文: SEQ = %d", ((FTPHeader_t*)(sendPKT_list.GetHead()->PktData))->SEQNO);
		Dlg->log(logText);
		//移除报文
		backTimerID(sendPKT_list.GetHead()->Timer);
		sendPKT_list.RemoveHead();
		
	}
	else if (((FTPHeader_t*)(sendPKT_list.GetHead()->PktData))->SEQNO < Ack)			//重复报文
	{
		sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK );//回送FIN|ACK报文
		return;
	}
	recvACK = FTPH->ACKNO;		//更新接收的新的ACK编号，重新计算冗余ACK
	sameTimes = 1;
	Ack = FTPH->SEQNO + 1;										//ACK更新

	u_char* Data = (u_char*)(buffer + sizeof(FTPHeader_t));		//FTP数据部分
	switch (FTPH->Flags)
	{
	case SYN:						//接收请求进行响应
	{
		if (status == start)
		{//创建一个发送数据包结构体
			u_char res[256] = "OK Server is ready!";
			int res_len = strlen((char*)(res));
			//更新发送序列号
			Seq += res_len;
			//回送一个SYN|ACK报文，确认已收到连接请求，同意建立连接
			sendFTPPacket(this, serverIP, serverPort, res, res_len, userID, 0, Seq, Ack, SYN | ACK);
			//日志记录 
			CString logText;
			logText.Format("recv '%s' from IP:%s Port:%d",
				((CString)Data).Left(byteLen - sizeof(FTPHeader_t)),
				serverIP, serverPort);
			Dlg->log(logText);
			status = isConnecting;
		}
		break;
	}
	case FIN | ACK: {
		//接受到拆除连接响应
		if (this->status == sendedFIN)			//发送过连接断开请求，主动断开连接
		{
			sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK);
			Dlg->log("接收到连接断开响应，关闭连接！");
			this->status = finished;
		}
		if (this->status == isTransfer || this->status == isCommunicating)           //被动断开连接
		{
			sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK);		//回送ACK
			Sleep(200);
			sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK | FIN);//回送FIN|ACK报文
			this->status = recvedFIN;
			Dlg->log("接收到连接断开请求,准备断开连接！");
		}
		break;
	}
	case RST | ACK:
	{
		//接收到强制拆除连接报文，立即清空所有的相关内容关闭套接字
		Dlg->log("接收到重置连接请求，直接关闭连接！");
		this->status = finished;			//将连接状态直接设置为断开状态
		break;
	}
	case  ACK:
	{
		//接收到正常响应报文，根据相关状态进行相应的处理
		if (this->status == sendedFIN)					//发出FIN请求，只响应FIN报文
		{
			Dlg->log("接收到FIN报文的ACK响应");
			return;
		}
		else if (this->status == recvedFIN)				//已经接收到FIN报文，获得ACK响应，关闭连接
		{
			this->status = finished;
			Dlg->log("接收到FIN报文的ACK响应,关闭连接！");
			return;
		}
		//正常报文响应的处理
		if (status == isConnecting)
		{
			//回送一个ACK报文，确认已收到连接请求，同意建立连接
			sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK);//回送ACK报文
			//日志记录 
			Dlg->log(((CString)Data).Left(byteLen - sizeof(FTPHeader_t)));
			status = isCommunicating;
			Dlg->log("与客户端建立连接,等待输入验证信息！");
		}
		if (status == isCommunicating)
		{
			if (byteLen == sizeof(FTPHeader_t))	//不带指令信息，为ACK报文，不需要做响应
			{
				break;   
			}
			CString command = CString(Data);

			if (command.Left(4) == "LOGI")		//授权信息
			{
				int usernamebegin = command.Find(" ",0);
				int usernameend = command.Find(" ", usernamebegin + 1);
				CString username = command.Mid(usernamebegin + 1, usernameend-usernamebegin-1);
				int passend = command.Find("\r\n", usernameend);
				CString password = command.Mid(usernameend + 1, passend-usernameend-1);
				Dlg->log("获取用户名和登录密码：userName:" + username + " password:" + password);
				//验证密码和注册处理
				if (UserInfo_list.GetCount() == 0) //没有用户数据
				{
					//直接写入，认为注册成功
					u_char res[256] = "333 register successful";
					int res_len = strlen((char*)(res));
					//更新发送序列号
					Seq += res_len;
					//回送一个SYN|ACK报文，确认已收到连接请求，同意建立连接
					sendFTPPacket(this, serverIP, serverPort, res, res_len, userID, 0, Seq, Ack, ACK);
					currentDir = currentDir + "/" + username;
					makeDir(currentDir);
					return;
				}
				else
				{
					//不为空，循环遍历，看是否存在匹配
					POSITION pos = UserInfo_list.GetHeadPosition();
					while (pos != NULL) {
						if (UserInfo_list.GetAt(pos)->userName == username)
						{
							if (UserInfo_list.GetAt(pos)->password == password)
							{
							//匹配成功
								u_char res[256] = "332 login successful";
								int res_len = strlen((char*)(res));
								//更新发送序列号
								Seq += res_len;
								//回送一个SYN|ACK报文，确认已收到连接请求，同意建立连接
								sendFTPPacket(this, serverIP, serverPort, res, res_len, userID, 0, Seq, Ack, ACK);
								currentDir = currentDir + "/" + username;
								//makeDir(currentDir);
								return;
							}
							else
							{
								//匹配失败
								u_char res[256] = "334 login failed";
								int res_len = strlen((char*)(res));
								//更新发送序列号
								Seq += res_len;
								//回送一个SYN|ACK报文，确认已收到连接请求，同意建立连接
								sendFTPPacket(this, serverIP, serverPort, res, res_len, userID, 0, Seq, Ack, ACK);
								return;
							}
						}
						UserInfo_list.GetNext(pos);
					}
					//不存在对应的用户名
					//添加新的用户名和密码
					UserInfo_t * test = new UserInfo_t;
					test->userName = username;
					test->password = password;
					UserInfo_list.AddTail(test);
					//直接写入，认为注册成功
					u_char res[256] = "333 register successful";
					int res_len = strlen((char*)(res));
					//更新发送序列号
					Seq += res_len;
					//回送一个SYN|ACK报文，确认已收到连接请求，同意建立连接
					sendFTPPacket(this, serverIP, serverPort, res, res_len, userID, 0, Seq, Ack, ACK);
					currentDir = currentDir + "/" + username;
					makeDir(currentDir);
					return;
				}
				break;
			}

			if (command.Left(4) == "LIST")			//打开文件夹
			{
				int dirbegin = command.Find(" ", 0);
				int dirend = command.Find("\r\n", dirbegin + 1);
				CString dir = command.Mid(dirbegin + 1, dirend-dirbegin-1);
				if (dir == ".")
					currentDir = currentDir;
				else if (dir == "..")
				{
					;
				}
				else 
					currentDir = currentDir + "/" + dir;
				CString dirInfo = getDirInfo(currentDir);
				//响应数据
				u_char res[1472] = "335 ";
				strcat((char*)res, dirInfo+"\r\n");
				int res_len = strlen((char*)(res));
				//更新发送序列号
				Seq += res_len;
				//回送一个SYN|ACK报文，确认已收到连接请求，同意建立连接
				sendFTPPacket(this, serverIP, serverPort, res, res_len, userID, 0, Seq, Ack, ACK);
				break;
			}

			if (command.Left(4) == "RETR")
			{
				//Dlg->MessageBox(command.Left(byteLen-sizeof(FTPHeader_t)));
				//回送一个ACK报文，确认已收到连接请求，同意建立连接
				sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK);//回送ACK报文
				int count = 1;
				int pos = command.Find(" ", 0)+1;
				while (pos < (byteLen - sizeof(FTPHeader_t)))
				{
					int nextpos = command.Find("\r\n", pos);
					CString file;
					file = command.Mid(pos, nextpos - pos);
					pos = nextpos + 2;
					//Dlg->MessageBox(file);
					DownloadFile_t *d = new DownloadFile_t;
					d->DataID = count++;
					d->filename = currentDir+"/"+file; //文件所在目录
					d->server = this;
					d->toIP = serverIP;
					d->port = serverPort;
					CWinThread *pThread = AfxBeginThread(			//启动发送文件线程
						dataThread,
						d,
						THREAD_PRIORITY_NORMAL,
						0,
						CREATE_SUSPENDED
						);
					pThread->ResumeThread();
				}
				
			}


			if (command.Left(4) == "DELE")			//删除文件夹或文件夹
			{
				int dirbegin = command.Find(" ", 0);
				int dirend = command.Find("\r\n", dirbegin + 1);
				CString dir = command.Mid(dirbegin + 1, dirend - dirbegin - 1);
			
				bool flag  = RecycleFileOrFolder(currentDir + "/" + dir);
				if (flag) {
					//响应数据
					u_char res[1472] = "200 dir delete successful";
					int res_len = strlen((char*)(res));
					//更新发送序列号
					Seq += res_len;
					//回送一个SYN|ACK报文，确认已收到连接请求，同意建立连接
					sendFTPPacket(this, serverIP, serverPort, res, res_len, userID, 0, Seq, Ack, ACK);
					break;
				}
				else
				{
					//响应数据
					u_char res[1472] = "500 dir delete failed";
					int res_len = strlen((char*)(res));
					//更新发送序列号
					Seq += res_len;
					//回送一个SYN|ACK报文，确认已收到连接请求，同意建立连接
					sendFTPPacket(this, serverIP, serverPort, res, res_len, userID, 0, Seq, Ack, ACK);
					break;
				}
			}

			if (command.Left(4) == "MDIR")			//创建文件夹
			{
				int dirbegin = command.Find(" ", 0);
				int dirend = command.Find("\r\n", dirbegin + 1);
				CString dir = command.Mid(dirbegin + 1, dirend - dirbegin - 1);

				bool flag = makeDir(currentDir + "/" + dir);
				if (flag) {
					//响应数据
					u_char res[1472] = "200 dir make successful";
					int res_len = strlen((char*)(res));
					//更新发送序列号
					Seq += res_len;
					//回送一个SYN|ACK报文，确认已收到连接请求，同意建立连接
					sendFTPPacket(this, serverIP, serverPort, res, res_len, userID, 0, Seq, Ack, ACK);
					break;
				}
				else
				{
					//响应数据
					u_char res[1472] = "500 dir make failed";
					int res_len = strlen((char*)(res));
					//更新发送序列号
					Seq += res_len;
					//回送一个SYN|ACK报文，确认已收到连接请求，同意建立连接
					sendFTPPacket(this, serverIP, serverPort, res, res_len, userID, 0, Seq, Ack, ACK);
					break;
				}
			}

			if (command.Left(4) == "EXIT")			//创建文件夹
			{
			//响应数据
				u_char res[1472] = "501 out successful";
				int res_len = strlen((char*)(res));
				//更新发送序列号
					Seq += res_len;
					//回送一个SYN|ACK报文，确认已收到连接请求，同意建立连接
					sendFTPPacket(this, serverIP, serverPort, res, res_len, userID, 0, Seq, Ack,ACK);
					//初始化定时器ID池
					for (int i = 0; i < TIMERLIMITNUM; i++)
					{
						sendTimerLimit[i] = i;
					}
					//初始化确认序列号和发送序列号
					Ack = 0;
					Seq = 0;
					sameTimes = 0;
					recvACK = 0;
					recvSEQ = 0;
					status = start;
					currentDir = ".";
					//初始化一个test用户
					UserInfo_t * test = new UserInfo_t;
					test->userName = "test";
					test->password = "test";
					UserInfo_list.AddTail(test);
					Dlg->log("用户退出成功！");
					break;
			}
		}
		break;
	}
	default: {

		Dlg->log("无法解析报文！");
	}
	}
	CAsyncSocket::OnReceive(nErrorCode);
}

UINT_PTR ServerSocket::getTimerID()
{
	UINT_PTR ID = 0;
	for (int i = 1; i < TIMERLIMITNUM; i++)
	{
		if (sendTimerLimit[i] != 0)
		{
			ID = sendTimerLimit[i];
			sendTimerLimit[i] = 0;
			break;
		}
	}
	assert(ID >= 1 || ID == 0);
	assert(ID < TIMERLIMITNUM);
	return ID;
	return UINT_PTR();
}

void ServerSocket::backTimerID(UINT_PTR ID)
{
	CFTP_ServerDlg*  Dlg = (CFTP_ServerDlg*)(AfxGetApp()->GetMainWnd());
	if (ID >= TIMERLIMITNUM) return;
	assert(ID >= 1);
	assert(ID < TIMERLIMITNUM);
	Dlg->KillTimer(ID);
	sendTimerLimit[ID] = ID;
}
