#include "stdafx.h"
#include "ConnectSocket.h"
#include	"FTP_ClientDlg.h"

//忽略 inet_ntoa 函数调用报错
#pragma warning(disable : 4996)

ConnectSocket::ConnectSocket()
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
	status = 0;
	userID = 0;
}


ConnectSocket::~ConnectSocket()
{
}

//定义连接请求响应的命令
/*
命令：
LOGI 登录/注册
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
336 服务器准备数据传输
*/

//连接套接字数据接收反馈函数
void ConnectSocket::OnReceive(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (status == finished)													//断开连接后不再接收响应
		return;	
	if (files_list.GetCount() == 0&&status==isTransfer)						//数据接收完毕，接收新的数据，应该回到命令交互模式
	{
		status = isCommunicating;
	}
	CFTP_ClientDlg*  Dlg = (CFTP_ClientDlg*)(AfxGetApp()->GetMainWnd());
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
	logText.Format("recv ACK %d SEQ %d", FTPH->ACKNO, FTPH->SEQNO);
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
				if (header->SEQNO >= recvACK || header->SEQNO<= Seq)
				{
					this->SendTo(item->PktData,item->len, item->TargetPort, item->TargetIP,0);
					CString logText;
					logText.Format("接收冗余三次ACK，重传报文: SEQ = %d", header->SEQNO);
					Dlg->log(logText);
					item->ResendTime++;			//重传次数记录自增1
				}
				sendPKT_list.GetNext(pos);
			}
		}
	}
	else  if(((FTPHeader_t*)(sendPKT_list.GetHead()->PktData))->SEQNO == FTPH->ACKNO -1)	//按序接收ACK,由于缓冲区中的SEQ按序排列，因此只需要比对缓冲区中第一个即可
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
	case SYN | ACK:
	{
		//接受到连接请求响应
		if (status == isConnecting)			//接收到服务器链接请求响应
		{
			//保存返回的用户ID
			this->userID = FTPH->UserID;
			//修改连接状态为正在通信状态
			this->status = isCommunicating;
			//创建一个发送数据包结构体
			u_char res[256] = "OK Client is ready!";
			int res_len = strlen((char*)(res));
			//更新发送序列号
			Seq += res_len;
			//回送一个ACK报文，确认已收到连接返回响应，确认建立连接
			sendFTPPacket(this, serverIP, serverPort, res, res_len, userID, 0, Seq, Ack, ACK);
			//日志记录 
			Dlg->log(((CString)Data).Left(byteLen - sizeof(FTPHeader_t)));
		}
		break;
	}
	case FIN | ACK:{
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
			sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK|FIN);//回送FIN|ACK报文
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
		else if (this->status == isCommunicating)
		{
			if (byteLen == sizeof(FTPHeader_t))	//不带指令信息，为ACK报文，不需要做响应
			{
				break;
			}
			CString command = CString(Data);
			if (command.Left(3) == "332")		//验证通过，登录成功
			{
				Dlg->log("成功登陆服务器");
				Dlg->logined = true;
				sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK);//回送ACK报文
				break;
			}
			if (command.Left(3) == "333")		//注册成功生成目录并返回
			{
				Dlg->log("不存在该用户，已自动注册成功！");
				Dlg->logined = true;
				sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK);//回送ACK报文
				break;
			}
			if (command.Left(3) == "334")		//密码错误
			{
				Dlg->log("用户名与密码不匹配，请重新输入用户名及密码！");
				sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK);//回送ACK报文
				break;
			}
			if (command.Left(3) == "200")		//操作成功
			{
				Dlg->log("操作成功！");
				sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK);//回送ACK报文
				break;
			}

			if (command.Left(3) == "500")		//操作失败
			{
				Dlg->log("操作失败");
				sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK);//回送ACK报文
				break;
			}

			if (command.Left(3) == "335")		//获取目录成功
			{
				Dlg->log("远程目录获取成功！");
				//Dlg->MessageBox(command.Left(byteLen-sizeof(FTPHeader_t)));
				Dlg->displayFileTree(command.Mid(4,byteLen - sizeof(FTPHeader_t)-6));
				sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK);//回送ACK报文
				break;
			}

			if (command.Left(3) == "501")		//退出
			{
				Dlg->log("退出成功！");
				status = start;
				sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK);//回送ACK报文
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
				status = 0;
				userID = 0;
				break;
			}

		}
		else if (status == isTransfer)			//数据传输报文
		{
			if (byteLen == sizeof(FTPHeader_t))	//不带指令数据段信息，为ACK报文，不需要做响应
			{
				break;
			}
			//根据不同的DataID,将数据存入不同的filecontent中
			POSITION pos = files_list.GetHeadPosition();
			while (pos != NULL)
			{
				if (files_list.GetAt(pos)->DataID == FTPH->DataID)			//数据标签匹配
				{
					//数据结尾为\r\n\r\n
					if (CString(Data).Left(byteLen - sizeof(FTPHeader_t)) == "\r\n\r\n\r\n\r\n")
					{
						if(writeFile("./test/"+files_list.GetAt(pos)->filename,
							files_list.GetAt(pos)->filecontent,
							files_list.GetAt(pos)->len))
						{
							//将文件删除
							Dlg->log(files_list.GetAt(pos)->filename + "文件下载成功！");
							files_list.RemoveAt(pos);
							break;
						}
						else {
							//写文件错误，也删除下载的文件
							Dlg->log(files_list.GetAt(pos)->filename + "写文件出错！");
							files_list.RemoveAt(pos);
							break;
						}
					}
					//将数据按顺序存入，由于发送端是按序发送，接收端也是按序接收，因此只需要直接合并即可
					copyData(Data, files_list.GetAt(pos)->filecontent + files_list.GetAt(pos)->len, byteLen - sizeof(FTPHeader_t));
					files_list.GetAt(pos)->len += byteLen - sizeof(FTPHeader_t);
					break;
				}
				files_list.GetNext(pos);
			}
			sendFTPPacket(this, serverIP, serverPort, new u_char, 0, userID, 0, Seq, Ack, ACK);//回送ACK报文
		}


		break;
	}
	default: {

		Dlg->log("无法解析报文！");
	}
	}
	CAsyncSocket::OnReceive(nErrorCode);
}

//获取定时器的ID
UINT_PTR ConnectSocket::getTimerID() {
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
	assert(ID >= 1||ID==0);
	assert(ID < TIMERLIMITNUM);
	return ID;
}

//回收定时器ID
void ConnectSocket::backTimerID(UINT_PTR ID) {
	CFTP_ClientDlg*  Dlg = (CFTP_ClientDlg*)(AfxGetApp()->GetMainWnd());
	if (ID >= TIMERLIMITNUM) return;
	assert(ID >= 1);
	assert(ID < TIMERLIMITNUM);
	Dlg->KillTimer(ID);
	sendTimerLimit[ID] = ID;
}

//创建计时器
bool newTimer(UINT_PTR ID) {
	CFTP_ClientDlg*  Dlg = (CFTP_ClientDlg*)(AfxGetApp()->GetMainWnd());
	Dlg->SetTimer(ID, 5000, NULL);				//设置计时器，时间为5s
	return true;
}

//获取本机的日期时间信息，字符串格式化
void getDateTimeStr(CString&date, CString&time) {
	//获取日期时间并转为字符串形式
	CTime tm; tm = CTime::GetCurrentTime();
	date.Format("%d/%d/%d", tm.GetYear(), tm.GetMonth(), tm.GetDay());
	time.Format("%d:%d:%d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());
}

//获取本机的HostIP地址
CString getHostIPStr() {
	char temp[256];
	gethostname(temp, 256);											//获取客户主机名
	hostent* host = gethostbyname(temp);							//客户主机IP
	return inet_ntoa(*(struct in_addr *)host->h_addr_list[0]);		//客户端IP地址转为字符串类型返回
}

//将IP地址转为字符串形式
CString getIPStr(DWORD IP) {
	CString IPStr;
	u_char* ip_int = (u_char*)&IP;
	IPStr.Format("%d:%d:%d:%d", ip_int[0], ip_int[1], ip_int[2], ip_int[3]);
	return IPStr;
}

//按字节复制数据
void copyData(u_char* srcData, u_char*destData, int len) {
	assert(srcData != NULL&&destData != NULL);
	for (int i = 0; i < len; i++)
	{
		destData[i] = srcData[i];
	}
}

//发送数据包函数
bool sendFTPPacket(CAsyncSocket* socket,			//发送套接字
	CString toIP,									//发送至IP
	UINT toPort,									//发送至端口
	u_char* sendData,								//发送数据首地址
	int dataLen,									//发送数据长度
	WORD userID ,									//用户ID标识
	WORD dataID ,									//数据字段标识
	DWORD seq ,										//发送序列号
	DWORD ack ,										//确认序列号
	WORD flags)										//标识位
{
	int totalLen = dataLen + sizeof(FTPHeader_t);
	assert(totalLen <= 1472);							//保证传输的其他数据长度小于剩余的传输空间1472 - 16 = 1456字节
	u_char* buffer = new u_char[totalLen];					//考虑到MTU的大小，将UDP发送数据的最大大小限制在1472字节以内
	FTPHeader_t* FTPH = (FTPHeader_t*)(buffer);
	FTPH->UserID = userID;
	FTPH->DataID = dataID;
	FTPH->Flags = flags;
	FTPH->SEQNO = seq;
	FTPH->ACKNO = ack;
	copyData(sendData, (buffer + sizeof(FTPHeader_t)), dataLen);	//复制数据到发送缓冲中
	FTPH->CheckSum = 0;												//先将校验和置位0
	FTPH->CheckSum = ChecksumCompute((unsigned short*)buffer, totalLen);

	int flg = socket->SendTo(buffer, totalLen, toPort, toIP, 0);	//发送数据报
	if (flg < 0)
	{
		return false;
	}
	//如果数据包只有包头，没有数据，不用添加到发送缓冲区
	if (totalLen - sizeof(FTPHeader_t) == 0)
	{
		return true;
	}
	//每发送一个数据包，需要把数据包存入数据缓冲区，并添加计时器
	SendPacket_t* packet = new SendPacket_t;
	copyData(buffer,packet->PktData, totalLen);
	packet->len = totalLen;
	packet->TargetIP = toIP;
	packet->TargetPort = toPort;
	packet->Timer = ((ConnectSocket*)(socket))->getTimerID();
	packet->ResendTime = 0;												//重发次数初始化为0
	((ConnectSocket*)(socket))->sendPKT_list.AddTail(packet);			//加入缓冲区	
	assert(newTimer(packet->Timer));									//添加计时器
	return true;
}


// 计算校验和
unsigned short ChecksumCompute(unsigned short * buffer, int size)
{
	// 32位，延迟进位
	unsigned long cksum = 0;
	while (size > 1)
	{
		cksum += *buffer++;
		// 16位相加
		size -= sizeof(unsigned short);
	}
	if (size)
	{
		// 最后可能有单独8位
		cksum += *(unsigned char *)buffer;
	}
	// 将高16位进位加至低16位
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	// 取反
	return (unsigned short)(~cksum);
}


//读文件内容
bool readFile(CString filePath, u_char* &data, long long int&d_len)
{
	CFile file;
	file.Open(filePath, CFile::modeRead, NULL);
	long long int len = file.GetLength();
	u_char* Buf = new u_char[len];
	file.Read(Buf, len);
	data = new u_char[len];
	copyData(Buf, data, len);
	d_len = len;
	return  true;
}
//写文件内容
bool writeFile(CString filePath, u_char* content, long long int len)
{
	//AfxMessageBox(CString(content));
	assert(content != NULL);
	CFile file;

	file.Open(filePath, CFile::modeCreate | CFile::modeWrite, NULL);

	file.Write(content, len);

	file.Close();
	return true;
}