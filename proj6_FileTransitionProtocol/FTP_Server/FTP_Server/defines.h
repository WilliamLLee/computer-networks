#pragma once

//描述连接状态
#define start        0		//启动初始化状态
#define isConnecting 1		//正在进行链接
#define isCommunicating 2	//正在进行命令交互
#define isTransfer	3		//正在进行数据传输
#define sendedFIN 4			//正在进行拆除连接工作,已发送拆除连接命令
#define recvedFIN 5			//接收到服务器发送来的断开连接请求
#define finished 6			//连接已经拆除


//定义标识位宏
#define SYN 0x0001				//标识连接请求位
#define ACK 0x0002				//标识ACK位有效
#define FIN 0x0004				//标识结束连接请求位
#define RST 0x0008				//标识重置连接，立即关闭连接

//计时器限制
#define TIMERLIMITNUM  40        //缓冲区最多存在20个待发送数据包
//重发次数上限
#define RESENDTIMELIMIT 5		//重发次数上限为5次，五次重发没有收到响应，进行报错
//报文数据段最大长度
#define DATAMAXLEN 1456

#pragma pack(1)
//定义FTP协议报文格式
typedef struct FTPHeader_t {	//FTP报文首部
	WORD UserID;				//用户标识，区别不同的用户
	WORD DataID;				//数据包标识，识别同一个包的不同数据报文
	DWORD SEQNO;					//发送序列号
	DWORD ACKNO;					//确认序列号
	WORD CheckSum;				//校验和，由于文件较大，只对头部数据以及IP地址和端口数据做校验
	WORD Flags;					//标志位字段
}FrameHeader_t;

//发送数据包在缓冲区中的格式
typedef struct SendPacket_t {
	int len;				//数据包长度
	BYTE PktData[2000];		//数据包
	CString TargetIP;		//目的IP地址，使用字符串格式
	UINT   TargetPort;		//目的端口号
	UINT_PTR Timer;			//计时器句柄
	int ResendTime;			//记录重传次数
}SendPacket_t;


//用户信息
typedef struct UserInfo_t {
	CString userName;
	CString password;
}UserInfo_t;

#pragma pack()

// 计算校验和
unsigned short ChecksumCompute(unsigned short * buffer, int size);
//获取本机的日期时间信息，字符串格式化
void getDateTimeStr(CString&date, CString&time);
//获取本机的HostIP地址
CString getHostIPStr();
//将IP地址转为字符串形式
CString getIPStr(DWORD IP);
//发送数据包函数
bool sendFTPPacket(CAsyncSocket* socket,			//发送套接字
	CString toIP,									//发送至IP
	UINT toPort,									//发送至端口
	u_char* sendData,								//发送数据首地址
	int dataLen,									//发送数据长度
	WORD userID = 0,									//用户ID标识
	WORD dataID = 0,									//数据字段标识,大文件分包时使用
	DWORD seq = 0,										//发送序列号
	DWORD ack = 0,										//确认序列号
	WORD flags = 0);									//标识位

														//复制数据
void copyData(u_char* srcData, u_char*destData, int len);
//创建计时器
bool newTimer(UINT_PTR ID);
//创建文件夹目录
bool makeDir(CString DirName);
//获取文件目录下所有文件信息,以固定格式字符串返回
CString getDirInfo(CString DirName);
// 删除文件（第二个参数bDelete表示是否删除至回收站,默认删除到回收站）
BOOL RecycleFileOrFolder(CString strPath, BOOL bDelete=FALSE);
//读取文件内容，按字符处理，读取的所有数据按字符串返回
bool readFile(CString filePath,u_char* &data,long long int& len);
//写文件内容，按字符串写入
bool writeFile(CString filePath, u_char* content, long long int len);
//下载文件控制线程，获取文件名参数，向目标用户传输文件
UINT dataThread(LPVOID lpParam);
