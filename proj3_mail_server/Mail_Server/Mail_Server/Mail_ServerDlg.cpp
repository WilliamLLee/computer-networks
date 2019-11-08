
// Mail_ServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Mail_Server.h"
#include "Mail_ServerDlg.h"
#include "afxdialogex.h"
#include "base64.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMail_ServerDlg 对话框



CMail_ServerDlg::CMail_ServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MAIL_SERVER_DIALOG, pParent)
	, m_mail_sender(NULL)
	, picture_ctrl(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMail_ServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, mail_text_ctrl);
	DDX_Control(pDX, IDC_LIST1, log_list_ctrl);
	DDX_Control(pDX, IDC_EDIT2, mail_display_ctrl);
	DDX_Control(pDX, IDC_EDIT3, txt_ctrl);
	DDX_Control(pDX, IDC_EDIT6, cur_ctrl);
	DDX_Control(pDX, IDC_EDIT5, total_ctrl);
}

BEGIN_MESSAGE_MAP(CMail_ServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_DISPLAYMAIL,OnDisplayMail)
	ON_BN_CLICKED(IDC_BUTTON1, &CMail_ServerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMail_ServerDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CMail_ServerDlg 消息处理程序

BOOL CMail_ServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	cur_ctrl.SetWindowTextA("0");
	total_ctrl.SetWindowTextA("0");
	//创建TCP套接字，绑定25端口
	BOOL bFlag = SMTP_Server.Create(25, SOCK_STREAM, FD_ACCEPT | FD_READ | FD_WRITE );
	if (!bFlag) {
		log_list_ctrl.InsertString(log_list_ctrl.GetCount(),"***SMTP服务器启动失败");
	}
	else
	{
		log_list_ctrl.InsertString(log_list_ctrl.GetCount(), "***SMTP服务器准备好");
	}
	log_list_ctrl.InsertString(log_list_ctrl.GetCount(), "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$");
	if (!SMTP_Server.Listen()) //设为侦听连接请求，最多允许建立连接数量为5
		log_list_ctrl.InsertString(log_list_ctrl.GetCount(), "侦听25端口设置失败,端口可能已被占据");
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMail_ServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMail_ServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMail_ServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//转发邮件线程控制函数
UINT Send_Mails(PVOID hWnd)
{
	return 0;
}

void CMail_ServerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	if(SMTP_Server!= INVALID_SOCKET)
		SMTP_Server.Close();
}

CString CMail_ServerDlg::UTF8toANSI(CString &strUTF8)
{
	//获取转换为多字节后需要的缓冲区大小，创建多字节缓冲区
	UINT nLen = MultiByteToWideChar(CP_UTF8, NULL, strUTF8, -1, NULL, NULL);
	WCHAR *wszBuffer = new WCHAR[nLen+1];
	nLen = MultiByteToWideChar(CP_UTF8, NULL, strUTF8, -1, wszBuffer, nLen);
	wszBuffer[nLen] = 0;

	nLen = WideCharToMultiByte(936, NULL, wszBuffer, -1, NULL, NULL, NULL, NULL);
	CHAR *szBuffer = new CHAR[nLen+1];
	nLen = WideCharToMultiByte(936, NULL, wszBuffer, -1, szBuffer, nLen, NULL, NULL);
	szBuffer[nLen] = 0;
	strUTF8 = szBuffer;
	//清理内存
	delete[]szBuffer;
	delete[]wszBuffer;
	return  strUTF8;
}


// 解析邮件报文的消息处理函数
afx_msg LRESULT CMail_ServerDlg::OnDisplayMail(WPARAM wParam, LPARAM lParam)
{
	if (mail_text_ctrl.GetLineCount() == 0) 
		return afx_msg LRESULT();		//报文显示区域为空，不做解析处理
	//解析结果将新的内容填充之前需要先清空原文件内容
	mail_display_ctrl.SetWindowTextA("");
	txt_ctrl.SetWindowTextA("");
	//设置空间的图片为空
	((CStatic*)GetDlgItem(IDC_picture))->SetBitmap(NULL);
	UpdateData(true);
	//获取报文文本
	CString mail_post;
	mail_text_ctrl.GetWindowTextA(mail_post);
	//获取对应的编码文本
	int start_base64_code = 0;  
	while (1) {
		start_base64_code = mail_post.Find("Content-Transfer-Encoding: base64", start_base64_code);
		if (start_base64_code == -1)
		{
			break;
		}
		int text_type_begin = mail_post.Find("\r\n", start_base64_code);
		int code_begin = mail_post.Find("\r\n\r\n", text_type_begin+2);//找到base64编码开始位置
		int code_end = mail_post.Find("\r\n\r\n", code_begin+4);//找到图片附件编码结束位置
		CString code= mail_post.Mid(code_begin + 4, code_end - code_begin - 4); //截断编码;
		while (1) {
			int pos = code.Find("\r\n");
			if (pos == -1) break;
			CString a = code.Mid(0, pos);
			CString b = code.Mid(pos + 2, code.GetLength()-2);
			code = a + b;
		}
		CString type = mail_post.Mid(text_type_begin + 2, code_begin - text_type_begin - 2);
		if (type.Find("attachment") != -1)		//处理附件
		{
			int p1 = type.Find('"', 0);
			int p2 = type.Find('"', p1 + 1);
			CFile file;
			CString file_name = type.Mid(p1 + 1, p2 - p1 - 1);
			//log_list_ctrl.InsertString(log_list_ctrl.GetCount(),"打开附件："+file_name);
			file.Open(file_name, CFile::modeWrite | CFile::modeCreate| CFile::typeBinary);
			char *output = new char[code.GetLength() * 3 / 4];
			base64_decode(code, output);
			file.Write(output, code.GetLength() * 3 / 4);
			file.SeekToEnd();
			file.Close();
			if (type.Find("txt") != -1)
			{
				txt_ctrl.SetWindowTextA(output);
			}
			else if(type.Find("jpg")!=-1||
				type.Find("bmp") != -1||
				type.Find("PNG") != -1||
				type.Find("png") != -1||
				type.Find("BMP") != -1||
				type.Find("JPG") != -1) {

				CImage image;
				image.Load(file_name);

				//以下两个矩形主要作用是，获取对话框上面的Picture Control的width和height，
				//并设置到图片矩形rectPicture，根据图片矩形rectPicture对图片进行处理，
				//最后绘制图片到对话框上Picture Control上面
				CRect rectControl;                        //控件矩形对象
				CRect rectPicture;                        //图片矩形对象


				int x = image.GetWidth();
				int y = image.GetHeight();
				//Picture Control的ID为IDC_picture
				CWnd  *pWnd = GetDlgItem(IDC_picture);
				pWnd->GetClientRect(rectControl);


				CDC *pDc = GetDlgItem(IDC_picture)->GetDC();
				SetStretchBltMode(pDc->m_hDC, STRETCH_HALFTONE);
				//重新设置图片的大小为空间的规格
				rectPicture = CRect(rectControl.TopLeft(), CSize((int)rectControl.Width(), (int)rectControl.Height()));
				//设置空间的图片为空
				((CStatic*)GetDlgItem(IDC_picture))->SetBitmap(NULL);

				//以下两种方法都可绘制图片，这里是直接在控件的上面贴图，能够实现相同的功能
				image.StretchBlt(pDc->m_hDC, rectPicture, SRCCOPY); //将图片绘制到Picture控件表示的矩形区域
				//image.Draw(pDc->m_hDC, rectPicture);                //将图片绘制到Picture控件表示的矩形区域

				image.Destroy();
				pWnd->ReleaseDC(pDc);
				UpdateData(true);
			}
		}
		else if (type.Find("charset=") != -1) //处理正文文本
		{
			char *output = new char[code.GetLength() * 3 / 4];
			output = base64_decode(code, output);
			mail_display_ctrl.SetWindowTextA(UTF8toANSI(CString(output))); //显示编码
		}
		start_base64_code = code_end;
	}
	return afx_msg LRESULT();
}


void CMail_ServerDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString cur, total;
	cur_ctrl.GetWindowTextA(cur);
	total_ctrl.GetWindowTextA(total);
	int cur_mail = _ttoi(cur),total_mail = _ttoi(total);
	//处理条件检验
	if (total_mail == 0 || cur_mail == 1) return;
	//更新解析的邮件编号标识
	cur.Format("%d", cur_mail -1);
	cur_ctrl.SetWindowTextA(cur);
	//将前一份邮件的报文显示在展示区与
	mail_text_ctrl.SetWindowTextA(mail_list.GetAt(mail_list.FindIndex(cur_mail - 2))->mail_post);
	PostMessage(WM_DISPLAYMAIL, 0, 0);    //发送消息调用解析邮件函数
	if (cur_mail-1 == 0)  //禁用上一封按钮
	{
		CWnd *pwnd = GetDlgItem(IDC_BUTTON1);
		pwnd->EnableWindow(false);
	}
	//激活下一封按钮
	CWnd *pwnd = GetDlgItem(IDC_BUTTON2);
	pwnd->EnableWindow(true);
}


void CMail_ServerDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	CString cur, total;
	cur_ctrl.GetWindowTextA(cur);
	total_ctrl.GetWindowTextA(total);
	int cur_mail = _ttoi(cur), total_mail = _ttoi(total);
	//处理条件检验
	if (total_mail == 0 || cur_mail == total_mail) return;
	//更新解析的邮件编号标识
	cur.Format("%d", cur_mail + 1);
	cur_ctrl.SetWindowTextA(cur);
	//将前一份邮件的报文显示在展示区与
	mail_text_ctrl.SetWindowTextA(mail_list.GetAt(mail_list.FindIndex(cur_mail))->mail_post);
	PostMessage(WM_DISPLAYMAIL, 0, 0);    //发送消息调用解析邮件函数
	if (cur_mail+1 == total_mail)  //禁用下一封按钮
	{
		CWnd *pwnd = GetDlgItem(IDC_BUTTON2);
		pwnd->EnableWindow(false);
	}
	//激活上一封按钮
	CWnd *pwnd = GetDlgItem(IDC_BUTTON1);
	pwnd->EnableWindow(true);
}
