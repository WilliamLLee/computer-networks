#include "stdafx.h"
#include "defines.h"
#include "FTP_ServerDlg.h"
#include "shlwapi.h"
#pragma comment(lib,"shlwapi.lib")
#include <afx.h>
#include "ServerSocket.h"


//忽略 inet_ntoa 函数调用报错
#pragma warning(disable : 4996)

// 互斥对象
HANDLE hMutex = NULL;
//数据下载单元
typedef struct DownloadFile_t {
	WORD DataID;			//文件ID
	CString filename;		//文件名
	ServerSocket* server;	//套接字，获取SEQ和ACK数据
	CString toIP;
	UINT port;
} DownloadFile_t;


//创建计时器
bool newTimer(UINT_PTR ID) {
	CFTP_ServerDlg*  Dlg = (CFTP_ServerDlg*)(AfxGetApp()->GetMainWnd());
	Dlg->SetTimer(ID, 5000, NULL);				//设置计时器，时间为5s
	return true;
}


// 删除文件（第二个参数bDelete表示是否删除至回收站,默认删除到回收站）
BOOL RecycleFileOrFolder(CString strPath, BOOL bDelete/*=FALSE*/)
{
	strPath += '\0';
	SHFILEOPSTRUCT  shDelFile;
	memset(&shDelFile, 0, sizeof(SHFILEOPSTRUCT));
	shDelFile.fFlags |= FOF_SILENT;				// 不显示进度
	shDelFile.fFlags |= FOF_NOERRORUI;			// 不报告错误信息
	shDelFile.fFlags |= FOF_NOCONFIRMATION;		// 直接删除，不进行确认

												// 设置SHFILEOPSTRUCT的参数为删除做准备
	shDelFile.wFunc = FO_DELETE;		// 执行的操作
	shDelFile.pFrom = strPath;			// 操作的对象，也就是目录（注意：以“\0\0”结尾）
	shDelFile.pTo = NULL;				// 必须设置为NULL
	if (bDelete) //根据传递的bDelete参数确定是否删除到回收站
	{
		shDelFile.fFlags &= ~FOF_ALLOWUNDO;    //直接删除，不进入回收站
	}
	else
	{
		shDelFile.fFlags |= FOF_ALLOWUNDO;    //删除到回收站
	}

	BOOL bres = SHFileOperation(&shDelFile);    //删除
	return !bres;
}
//读文件内容
bool readFile(CString filePath, u_char* &data, long long int&d_len)
{
	CFile file;
	file.Open(filePath, CFile::modeRead, NULL);
	long long int len = file.GetLength();
	u_char* Buf =new u_char[len];
	file.Read(Buf, len);
	data = new u_char[len];
	copyData(Buf,data,len);
	d_len = len;
	return  true;
}
//写文件内容
bool writeFile(CString filePath,u_char* content,long long int len)
{
	assert(content!=NULL);
	CFile file;

	file.Open(filePath, CFile::modeCreate | CFile::modeWrite, NULL);

	file.Write(content, len);

	file.Close();
	return true;
}
//线程控制函数
UINT dataThread(LPVOID lpParam)
{
	DownloadFile_t *pInfo = (DownloadFile_t*)lpParam;     //指向结构体的实例。
	CFTP_ServerDlg*  Dlg = (CFTP_ServerDlg*)(AfxGetApp()->GetMainWnd());
	//记录日志
	CString t;
	t.Format("启动发送文件线程%d,%s", pInfo->DataID, pInfo->filename);
	// 等待互斥对象通知
	WaitForSingleObject(hMutex, INFINITE);
	Dlg->log(t);
	// 释放互斥对象
	ReleaseMutex(hMutex);
	//启动数据读取
	u_char* data;
	long long int data_len;
	readFile(pInfo->filename,data, data_len);
	t.Format("%s 开始传输！文件大小:%d", pInfo->filename, data_len);
	Dlg->log(t);
	//Dlg->MessageBox((CString)data);
	//开始发送
	long long  sendedSeq = 0 ;
	while (sendedSeq < data_len)
	{
		if (data_len - sendedSeq>=DATAMAXLEN)		//数据长度大于报文承载长度
		{
			// 等待互斥对象通知
			WaitForSingleObject(hMutex, INFINITE);
			u_char* tt = new u_char[DATAMAXLEN];
			copyData(data + sendedSeq, tt, DATAMAXLEN);
			pInfo->server->Seq += DATAMAXLEN;
			sendFTPPacket(pInfo->server, pInfo->toIP, pInfo->port, tt, DATAMAXLEN,pInfo->server->userID, pInfo->DataID, pInfo->server->Seq, pInfo->server->Ack, ACK);
			sendedSeq += DATAMAXLEN;
			t.Format("%s 传输进度:%d/%d", pInfo->filename,sendedSeq, data_len);
			Dlg->log(t);
			// 释放互斥对象
			ReleaseMutex(hMutex);
		}
		else {
			// 等待互斥对象通知
			WaitForSingleObject(hMutex, INFINITE);
			u_char* tt = new u_char[data_len - sendedSeq];
			copyData(data + sendedSeq, tt, data_len - sendedSeq);
			pInfo->server->Seq += data_len - sendedSeq;
			sendFTPPacket(pInfo->server, pInfo->toIP, pInfo->port, tt, data_len - sendedSeq, pInfo->server->userID, pInfo->DataID, pInfo->server->Seq, pInfo->server->Ack, ACK);
			sendedSeq += data_len - sendedSeq;
			t.Format("%s 传输进度:%d/%d", pInfo->filename, sendedSeq, data_len);
			Dlg->log(t);
			// 释放互斥对象
			ReleaseMutex(hMutex);
		}
	}
	// 等待互斥对象通知
	WaitForSingleObject(hMutex, INFINITE);
	u_char end[123] = "\r\n\r\n\r\n\r\n";
	int str_len = strlen((char *)end);
	pInfo->server->Seq += str_len;
	sendFTPPacket(pInfo->server, pInfo->toIP, pInfo->port, end,str_len, pInfo->server->userID, pInfo->DataID, pInfo->server->Seq, pInfo->server->Ack, ACK);
	// 释放互斥对象
	ReleaseMutex(hMutex);
	t.Format("%s 文件传输完毕！文件大小:%d", pInfo->filename, data_len);
	// 等待互斥对象通知
	WaitForSingleObject(hMutex, INFINITE);
	Dlg->log(t);
	// 释放互斥对象
	ReleaseMutex(hMutex);
	return 0;
}

//创建文件夹，如果文件夹已经存在返回false，创建成功返回true
bool makeDir(CString DirName)
{
	if (!PathIsDirectory(DirName))
	{
		::CreateDirectory(DirName, 0);
		return true;
	}
	else
	{
		return false;
	}
}
//获取一个文件目录下所有文件信息
CString getDirInfo(CString DirName)
{
	DirName += "\\*";
	HANDLE file;
	WIN32_FIND_DATA fileData;
	file = FindFirstFile(DirName.GetBuffer(), &fileData);
	CString fileString = "";
	if (file != INVALID_HANDLE_VALUE)
	{
		if (fileData.dwFileAttributes& FILE_ATTRIBUTE_DIRECTORY)
		{
			CString fileAttr;
			fileAttr.Format("D %s %d\r\n", fileData.cFileName, fileData.nFileSizeLow | (ULONGLONG)fileData.nFileSizeHigh << 32);
			fileString+= fileAttr;
		}
		else {
			CString fileAttr;
			fileAttr.Format("F %s %d\r\n", fileData.cFileName, fileData.nFileSizeLow | (ULONGLONG)fileData.nFileSizeHigh << 32);
			fileString += fileAttr;
		}
		bool bState = false;
		bState = FindNextFile(file, &fileData);
		while (bState) {
			if (fileData.dwFileAttributes& FILE_ATTRIBUTE_DIRECTORY)
			{
			CString fileAttr;
			fileAttr.Format("D %s %d\r\n", fileData.cFileName, fileData.nFileSizeLow | (ULONGLONG)fileData.nFileSizeHigh << 32);
			fileString += fileAttr;
			}
			else
			{
				CString fileAttr;
				fileAttr.Format("F %s %d\r\n", fileData.cFileName, fileData.nFileSizeLow | (ULONGLONG)fileData.nFileSizeHigh << 32);
				fileString += fileAttr;
			}
			bState = FindNextFile(file, &fileData);
		}
	}
	return fileString;
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
	WORD userID,									//用户ID标识
	WORD dataID,									//数据字段标识
	DWORD seq,										//发送序列号
	DWORD ack,										//确认序列号
	WORD flags)										//标识位
{
	int totalLen = dataLen + sizeof(FTPHeader_t);
	assert(totalLen <= DATAMAXLEN+ sizeof(FTPHeader_t));							//保证传输的其他数据长度小于剩余的传输空间1472 - 16 = 1456字节
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
	copyData(buffer, packet->PktData, totalLen);
	packet->len = totalLen;
	packet->TargetIP = toIP;
	packet->TargetPort = toPort;
	packet->ResendTime = 0;
	packet->Timer = ((ServerSocket*)(socket))->getTimerID();
	((ServerSocket*)(socket))->sendPKT_list.AddTail(packet);			//加入缓冲区	
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


