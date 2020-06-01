
// TransFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TransFile.h"
#include "TransFileDlg.h"
#include "afxdialogex.h"
#include "InputDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 自定义消息
#define WM_MSG_LOG      (WM_USER+1)
#define WM_MSG_CHGSOCK  (WM_USER+2)

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTransFileDlg dialog
CTransFileDlg* g_pMainView = NULL;
void GLog(const char* pFormat, ...)
{
    if(g_pMainView == NULL)
    {
        return;
    }
    va_list va;
    string  strRet;

    // 获取日志文本
    va_start(va, pFormat);
    int nLen = _vscprintf(pFormat, va) + 1;
    char* pBuf = NULL;
    try
    {
        pBuf = new char[nLen];
        memset(pBuf, 0, sizeof(char)*nLen);
        vsprintf_s(pBuf, nLen, pFormat, va);
    }
    catch (...)
    {
        // 异常错误
        return;
    }
    va_end(va);

    // 写日志
    PostMessage(g_pMainView->m_hWnd, WM_MSG_LOG, (WPARAM)pBuf, 0);
}
void NotifySocketChange(CNetFile* pNet, int nStatus)
{
    PostMessage(g_pMainView->m_hWnd, WM_MSG_CHGSOCK, (WPARAM)pNet, nStatus);
}


CTransFileDlg::CTransFileDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTransFileDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    g_pMainView = this;
}

void CTransFileDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO1, m_vMode);
    DDX_Control(pDX, IDC_EDIT1, m_vHost);
    DDX_Control(pDX, IDC_EDIT2, m_vPort);
    DDX_Control(pDX, IDC_LIST1, m_vList);
    DDX_Control(pDX, IDC_EDIT3, m_vLog);
    DDX_Control(pDX, IDC_BUTTON1, m_vStart);
}

BEGIN_MESSAGE_MAP(CTransFileDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, &CTransFileDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CTransFileDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BUTTON1, &CTransFileDlg::OnClickedButtonStart)
    ON_WM_CLOSE()
    ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CTransFileDlg::OnRclickListHost)
    ON_COMMAND(ID_32771, &CTransFileDlg::OnSendFile)
    ON_COMMAND(ID_32772, &CTransFileDlg::OnSendMsg)
    ON_COMMAND(ID_32773, &CTransFileDlg::OnBroadcastFile)
    ON_COMMAND(ID_32774, &CTransFileDlg::OnBroadcastMsg)
    ON_MESSAGE(WM_MSG_LOG, &CTransFileDlg::OnMyLog)
    ON_MESSAGE(WM_MSG_CHGSOCK, &CTransFileDlg::OnChgSocket)
    ON_WM_DROPFILES()
    ON_CBN_SELCHANGE(IDC_COMBO1, &CTransFileDlg::OnSelchangeRunMode)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// 获取本机IP地址
std::string GetLocalHost()
{
    char szHostName[MAX_PATH] = { 0 };
    int nRetCode = 0;
    nRetCode = gethostname(szHostName, sizeof(szHostName));
    if (nRetCode != 0)
    {
        return "";
    }
    return HostName2IP(szHostName);
}

std::string HostName2IP(const string& strName)
{
    // 转换为IP地址
    char* szHostIP = NULL;
    PHOSTENT hostinfo;
    hostinfo = gethostbyname(strName.c_str());
    szHostIP = inet_ntoa(*(struct in_addr*)*hostinfo->h_addr_list);
    return szHostIP;
}

// 获取当前时间：%Y-%m-%d %H:%M:%S
std::string GetCurDate(const char *format="%Y-%m-%d %H:%M:%S")
{
    time_t tCur;
    struct tm* tmCur;
    char szBuf[128] = {0};

    time(&tCur);
    tmCur = localtime(&tCur);
    strftime(szBuf, sizeof(szBuf)-1, format, tmCur);
    return szBuf;
}
bool GetFileNameExtByPath(const string& strFilePath, string& strName, string& strExt)
{
    string strFileName = strFilePath;
    int nPos = 0;
    nPos = strFilePath.find_last_of('\\');
    if(nPos == string::npos)
    {
        nPos = strFilePath.find_last_of('/');
    }
    if(nPos >= 0)
    {
        strFileName = strFilePath.substr(nPos+1);
    }

    nPos = strFileName.find_last_of('.');
    if (nPos >= 0)
    {
        strName = strFileName.substr(0, nPos);
        strExt = strFileName.substr(nPos + 1);
    }
    else
    {
        strName = strFileName;
        strExt = "";
    }
    return true;
}
std::string GetFileByFileDlg()
{
    string strPath;
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, NULL, NULL);
    const DWORD MAXFILE = 8192;
    dlg.m_ofn.nMaxFile = MAXFILE;
    TCHAR pc[MAXFILE] = {0};
    dlg.m_ofn.lpstrFile = pc;
    dlg.m_ofn.lpstrFile[0] = NULL;
    int iReturn = dlg.DoModal();
    vector<CString> strNames;
    if(iReturn == IDOK)
    {
        strPath = dlg.GetPathName();
    }
    return strPath;
}
void GetFileByFileDlg(vector<string>& vecFile)
{
    CFileDialog oFileDlg(TRUE,
        NULL,
        NULL,
        OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_HIDEREADONLY,
        NULL);
    const int nMaxFileNum = 32; 
    oFileDlg.m_ofn.lpstrFile = new TCHAR[_MAX_PATH * nMaxFileNum];
    memset(oFileDlg.m_ofn.lpstrFile, 0, _MAX_PATH * nMaxFileNum);
    oFileDlg.m_ofn.nMaxFile = _MAX_PATH * nMaxFileNum;
    string strCurFile;
    if (IDOK == oFileDlg.DoModal())
    {
        POSITION pos = oFileDlg.GetStartPosition();
        while (NULL != pos)
        {
            strCurFile = oFileDlg.GetNextPathName(pos);
            vecFile.push_back(strCurFile);
        }
    }
    delete[] oFileDlg.m_ofn.lpstrFile; 
}

INT64 MGetFileSize(const string& strFile)
{
    FILE *fp = fopen(strFile.c_str(),"r");
    INT64 nSize = MGetFileSize(fp);
    fclose(fp);
    return nSize;
}

INT64 MGetFileSize(FILE *fp)
{
    INT64 nCur = _ftelli64(fp);
    _fseeki64(fp, 0, SEEK_END);
    INT64 nSize = _ftelli64(fp);
    _fseeki64(fp, nCur, SEEK_SET);
    return nSize;
}

// 写日志到日志框
void CTransFileDlg::Log(const string& strLog)
{
    static int nLine = 0;
    int nCurIdx = 0;

    CString strAddText;
    strAddText.Format("[%03d %s]%s\r\n", ++nLine, GetCurDate().c_str(), strLog.c_str());
    nCurIdx = m_vLog.GetWindowTextLength();
    if(nLine > 40960)
    {
        nLine = 0;
        nCurIdx = 0;
        m_vLog.SetWindowText(strAddText);
    }
    else
    {
        m_vLog.SetSel(nCurIdx, nCurIdx);
        m_vLog.ReplaceSel(strAddText);
    }
}

void CTransFileDlg::Log(const char* pFormat, ...)
{
    va_list args;
    string  strRet;

    va_start(args, pFormat);
    Log(pFormat, args);
    va_end(args);
}

void CTransFileDlg::Log(const char* pFormat, va_list va)
{
    Log(Format(pFormat, va));
}

std::string CTransFileDlg::Format(const char * pFormat, va_list va)
{
    int     nLen = 0;
    string  strRet;

    nLen = _vscprintf(pFormat, va) + 1;
    char* pBuf = NULL;
    char* pNewBuf = NULL;
    try
    {
        static char pMiniBuf[8192] = { 0 };

        // 小内存不额外分配内存
        if (nLen < 1024)
        {
            pBuf = pMiniBuf;
        }
        else
        {
            pNewBuf = new char[nLen];
            if (pNewBuf == NULL)
            {
                // 内存分配失败
                throw runtime_error("alloc memory failed.");
            }
            pBuf = pNewBuf;
        }

        memset(pBuf, 0, sizeof(char)*nLen);
        vsprintf_s(pBuf, nLen, pFormat, va);
        strRet = pBuf;
    }
    catch (...)
    {
        // 异常错误
    }
    if (pNewBuf)
    {
        delete[] pBuf;
        pBuf = NULL;
    }
    return strRet;
}

void CTransFileDlg::OnAccept()
{
    if(!m_pSocket)
    {
        return;
    }

    /* CCliAsyncNet* pCli = new CCliAsyncNet(this);
    if(m_pSocket->Accept(*pCli))
    {
    CString strHost;
    UINT nPort;
    pCli->GetSockName(strHost, nPort);
    pCli->m_strHost = strHost;
    pCli->m_nPort = pCli->m_hSocket;
    Log("已建立新的连接[%s:%d]", pCli->m_strHost.c_str(), pCli->m_nPort);
    SetConnectHostStatus(pCli, "连接成功");
    AddClient(pCli);
    }
    else
    {
    Log("处理新的连接请求失败:%d", WSAGetLastError());
    }*/
}

void CTransFileDlg::SetConnectHostStatus(const CNetFile* pNet, const string& strStatus)
{
    // 列表查找是否存在改连接
    CString strSocket;
    strSocket.Format("%d", pNet->Socket());

    CString strBuf;
    int i = 0;
    for (; i<m_vList.GetItemCount(); i++)
    {
        strBuf = m_vList.GetItemText(i, 2);
        if (strBuf == strSocket)
        {
            break;
        }
    }
    // 没找到则新增
    if(strBuf != strSocket)
    {
        strBuf.Format("%d", i+1);
        m_vList.InsertItem(i, strBuf);
        m_vList.SetItemText(i, 1, pNet->GetAddr().c_str());
        m_vList.SetItemText(i, 2, strSocket);
        m_vList.SetItemText(i, 3, GetCurDate().c_str());
        m_vList.SetItemText(i, 4, strStatus.c_str());
    }
    // 更新
    else
    {
        m_vList.SetItemText(i, 3, GetCurDate().c_str());
        m_vList.SetItemText(i, 4, strStatus.c_str());
    }
}

void CTransFileDlg::DelConnectHost(const CNetFile* pNet)
{
    // 列表查找是否存在改连接
    CString strSocket;
    strSocket.Format("%d", pNet->Socket());
    CString strBuf;
    int nIdx = -1;
    for (int i = 0; i<m_vList.GetItemCount(); i++)
    {
        strBuf = m_vList.GetItemText(i, 2);
        if (strBuf == strSocket)
        {
            nIdx = i;
            break;
        }
    }
    if(nIdx < 0)
    {
        return;
    }

    // 客户端
    if(m_nMode == RUN_MODE_CLI)
    {
        OnSwitch(FALSE);
    }
    else
    {
        // 删除连接信息
        m_vList.DeleteItem(nIdx);
    }
}

CNetFileClient* CTransFileDlg::GetClient(int nSocket)
{
    return NULL;
}

CNetFileClient* CTransFileDlg::GetCurSelClient()
{
    CString str; 
    POSITION pos = m_vList.GetFirstSelectedItemPosition();       
    while (pos)            
    {                 
        int nItem = m_vList.GetNextSelectedItem(pos);               
        int nSocket = _ttoi(m_vList.GetItemText(nItem, 2));
        CNetFileServer* pNet = dynamic_cast<CNetFileServer*>(m_pSocket);
        return pNet->GetClient(nSocket);
    }
    return NULL;
}

bool CTransFileDlg::AddClient(CNetFileClient* pCli)
{
    //map<int, CCliAsyncNet*>::iterator e = m_mapCliSocket.find(pCli->m_hSocket);
    //m_mapCliSocket[pCli->m_hSocket] = pCli;
    //return (e == m_mapCliSocket.end());
    return false;
}

bool CTransFileDlg::SendFile(CNetFile* pNet, const string& strFile)
{
    pNet->SendFile(strFile);
    return true;
}

bool CTransFileDlg::SendMsg(CNetFile* pNet, const string& strMsg)
{
    pNet->SendMsg(strMsg);
    return true;
}

// CTransFileDlg message handlers

BOOL CTransFileDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// 设置工作模式
    m_vMode.AddString("客户端");
    m_vMode.AddString("服务器");
    m_vMode.SetCurSel(HasSameProcRun() ? 0:1);

    // 设置地址和端口
    string strHost = GetLocalHost();
    m_vPort.SetWindowText("5405");
    m_vHost.SetWindowText(strHost.c_str());
    m_pSocket = NULL;

    // 设置列表框
    LONG lListStyle;
    lListStyle = GetWindowLong(m_vList.m_hWnd, GWL_STYLE);
    lListStyle &= ~LVS_TYPEMASK;
    lListStyle |= LVS_REPORT;
    SetWindowLong(m_vList.m_hWnd, GWL_STYLE, lListStyle);
    m_vList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);

    //初始化列表的标题栏  
    CRect rt;
    m_vList.GetClientRect(&rt);
    TCHAR pColName[][16] = {
        _T("序号"), _T("主机地址"), _T("句柄"), _T("最后时间"), _T("状态")
    };
    // 默认显示列表框前三列
    int pColWidth[] = {40, 120, 75, 150, 100};
    pColWidth[1] = rt.Width() - pColWidth[0];
    int nLen = sizeof(pColName) / sizeof(pColName[0]);
    for (int i = 0; i < nLen; i++)
    {
        m_vList.InsertColumn(i, pColName[i], 0, pColWidth[i]);
    }

    // 设置焦点
    m_vList.SetFocus();
    m_nMode = 0;
	return TRUE;
}

void CTransFileDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTransFileDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTransFileDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTransFileDlg::OnClickedButtonStart()
{
    // 运行状态
    CString strStatus;
    m_vStart.GetWindowText(strStatus);
    if(strStatus == "停止")
    {
        OnSwitch(FALSE);
        return;
    }

    // 运行模式
    int nCurSel = m_vMode.GetCurSel();
    if(nCurSel < 0)
    {
        MessageBox("请选择程序运行模式", "提示", MB_ICONWARNING | MB_OK);
        return;
    }
    CString strMode;
    m_vMode.GetLBText(nCurSel, strMode);

    CString strHost, strPort;
    m_vHost.GetWindowText(strHost);
    m_vPort.GetWindowText(strPort);
    string host = strHost;
    int nPort = atoi(strPort);
    if(strMode == "客户端")
    {
        CNetFileClient* pNet = new CNetFileClient();
        if(pNet->Connect(host, nPort))
        {
            return;
        }
        // 添加到到已连接列表
        SetConnectHostStatus(pNet, "连接成功");
        m_pSocket = pNet;
        m_nMode = RUN_MODE_CLI;
    }
    else if(strMode == "服务器")
    {
        CNetFileServer* pNet = new CNetFileServer();
        if(pNet->Create(nPort))
        {
            delete pNet;
            return;
        }
        m_pSocket = pNet;
        m_nMode = RUN_MODE_SVR;
    }
    OnSwitch(TRUE);
}

void CTransFileDlg::OnSwitch(BOOL bEnable)
{
    if(!bEnable)
    {
        // 关闭连接
        m_pSocket->Close();

        // 清空连接列表
        m_vList.DeleteAllItems();

        m_nMode = 0;
    }

    // 修改操作界面
    m_vStart.SetWindowText(bEnable ? "停止":"启动");
    m_vMode.EnableWindow(!bEnable);
    m_vHost.EnableWindow(!bEnable);
    m_vPort.EnableWindow(!bEnable);
}


void CTransFileDlg::OnClose()
{
    this->EndDialog(IDCLOSE);
}


void CTransFileDlg::OnRclickListHost(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    *pResult = 0;

    // 非运行状态下不允许右键
    if(m_nMode <= 0) return;

    // 右键菜单
    CMenu menu, *pPopup;
    menu.LoadMenu(IDR_MENU_RCLICK);
    pPopup = menu.GetSubMenu(0);
    CPoint myPoint;
    ClientToScreen(&myPoint);
    GetCursorPos(&myPoint);
    pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);
}

unsigned int __stdcall SendFileProc(void* pPara)
{
    CTransFileDlg* pDlg = (CTransFileDlg*)pPara;
    // 发送文件
    for (size_t i=0; i<pDlg->m_vecCurClient.size(); i++)
    {
        for (size_t j=0; j<pDlg->m_vecCurFile.size(); j++)
        {
            pDlg->SendFile(pDlg->m_vecCurClient[i], pDlg->m_vecCurFile[j]);
        }
    }
    pDlg->m_vecCurClient.clear();
    pDlg->m_vecCurFile.clear();
    return 0;
}

void CTransFileDlg::OnSendFile()
{
    if(m_vList.GetItemCount() <= 0)
    {
        return;
    }
    if(m_oTask.IsRunning())
    {
        Log("任务正在运行中，请等待结束后重试");
        return;
    }
    // 获取客户端
    m_vecCurClient.clear();
    CNetFileClient* pNet = NULL;
    if(m_nMode == RUN_MODE_CLI)
    {
        pNet = dynamic_cast<CNetFileClient*>(m_pSocket);
    }
    else
    {
        pNet = GetCurSelClient();
    }
    if(pNet==NULL)
    {
        return;
    }
    m_vecCurClient.push_back(pNet);

    // 获取发送文件
    m_vecCurFile.clear();
    GetFileByFileDlg(m_vecCurFile);
    if(m_vecCurFile.empty())
    {
        return;
    }
    m_oTask.RunTask(SendFileProc, this, NULL);
}


unsigned int __stdcall SendMsgProc(void* pPara)
{
    CTransFileDlg* pDlg = (CTransFileDlg*)pPara;

    // 获取消息
    string strMsg;
    CInputDlg oInputDlg;
    oInputDlg.m_strTitle = "请输入要发送的消息内容:";
    oInputDlg.DoModal();
    strMsg = oInputDlg.m_strText;
    if(strMsg.empty())
    {
        return 0;
    }
    // 发送消息
    for (size_t i=0; i<pDlg->m_vecCurClient.size(); i++)
    {
        pDlg->SendMsg(pDlg->m_vecCurClient[i], strMsg);
    }
    pDlg->m_vecCurClient.clear();
    return 0;
}

void CTransFileDlg::OnSendMsg()
{
    if(m_vList.GetItemCount() <= 0)
    {
        return;
    }
    if(m_oTask.IsRunning())
    {
        Log("任务正在运行中，请等待结束后重试");
        return;
    }
    // 获取客户端
    m_vecCurClient.clear();
    CNetFileClient* pNet = NULL;
    if(m_nMode == RUN_MODE_CLI)
    {
        pNet = dynamic_cast<CNetFileClient*>(m_pSocket);
    }
    else
    {
        pNet = GetCurSelClient();
    }
    if(pNet==NULL)
    {
        return;
    }
    m_vecCurClient.push_back(pNet);
    m_oTask.RunTask(SendMsgProc, this, NULL);
}


void CTransFileDlg::OnBroadcastFile()
{
    if(m_oTask.IsRunning())
    {
        Log("任务正在运行中，请等待结束后重试");
        return;
    }
    // 获取客户端
    m_vecCurClient.clear();
    m_pSocket->GetClient(m_vecCurClient);
    if(m_vecCurClient.empty())
    {
        return;
    }

    // 获取发送文件
    m_vecCurFile.clear();
    GetFileByFileDlg(m_vecCurFile);
    if(m_vecCurFile.empty())
    {
        return;
    }
    m_oTask.RunTask(SendFileProc, this, NULL);
}


void CTransFileDlg::OnBroadcastMsg()
{
    if(m_oTask.IsRunning())
    {
        Log("任务正在运行中，请等待结束后重试");
        return;
    }
    // 获取客户端
    m_vecCurClient.clear();
    m_pSocket->GetClient(m_vecCurClient);
    if(m_vecCurClient.empty())
    {
        return;
    }
    m_oTask.RunTask(SendMsgProc, this, NULL);
}

LRESULT CTransFileDlg::OnMyLog(WPARAM wParam,LPARAM lParam)
{
    char* pLog = (char*)wParam;
    this->Log(pLog);

    // 释放内存
    delete[] pLog;
    return 0;
}

LRESULT CTransFileDlg::OnChgSocket(WPARAM wParam,LPARAM lParam)
{
    CNetFile* pNet = (CNetFile*)wParam;
    if (pNet == NULL)
    {
        return 0;
    }
    switch(lParam)
    {
    case SOCK_CHG_ACCEPT:
        SetConnectHostStatus(pNet, "连接成功");
        break;
    case SOCK_CHG_SEND:
        SetConnectHostStatus(pNet, "数据发送");
        break;
    case SOCK_CHG_RECV:
        SetConnectHostStatus(pNet, "数据接收");
        break;
    case SOCK_CHG_CLOSE:
        DelConnectHost(pNet);
        break;
    }
    return 0;
}


void CTransFileDlg::OnDropFiles(HDROP hDropInfo)
{
    if(m_oTask.IsRunning())
    {
        Log("任务正在运行中，请等待结束后重试");
        return;
    }
    if(m_pSocket == NULL)
    {
        return;
    }
    //取得被拖动文件的数目
    m_vecCurFile.clear();
    string strInfo = "添加文件";
    TCHAR szName[MAX_PATH] = {0};
    int nNum = DragQueryFile(hDropInfo,-1,NULL,0);
    for(int i=0;i< nNum; i++)
    {
        DragQueryFile(hDropInfo,i, szName, MAX_PATH);
        m_vecCurFile.push_back(szName);
        Log(strInfo + szName);
    } 
    //拖放结束后,释放内存
    DragFinish(hDropInfo);

    // 获取客户端
    m_vecCurClient.clear();
    m_pSocket->GetClient(m_vecCurClient);
    if(m_vecCurClient.empty())
    {
        return;
    }
    m_oTask.RunTask(SendFileProc, this, NULL);

    CDialogEx::OnDropFiles(hDropInfo);
}


void CTransFileDlg::OnSelchangeRunMode()
{
    CString strText;
    m_vMode.GetLBText(m_vMode.GetCurSel(), strText);
    if(strText != "服务器")
    {
        return;
    }
    string strHost = GetLocalHost();
    m_vHost.SetWindowText(strHost.c_str());
}


void CTransFileDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);
    if(!IsWindowVisible()) return;

    CRect rtParent;
    CRect rtMain;

    // 父窗口大小
    GetClientRect(rtParent);
    DWORD dwEdgeSize = 20;

    // 获取父窗口坐标信息
    GetWindowRect(&rtMain); 
    ScreenToClient(&rtMain); 

    CRect rtCtrl;
    // 主机列表
    m_vList.GetWindowRect(&rtCtrl); 
    ScreenToClient(&rtCtrl);
    rtCtrl.bottom = rtMain.bottom - dwEdgeSize;
    m_vList.MoveWindow(rtCtrl);

    //  日志框
    m_vLog.GetWindowRect(&rtCtrl);
    ScreenToClient(&rtCtrl);
    rtCtrl.bottom = rtMain.bottom - dwEdgeSize;
    rtCtrl.right = rtMain.right - dwEdgeSize;
    m_vLog.MoveWindow(rtCtrl);
}
