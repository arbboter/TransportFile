
// KCConnCliDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "KCConnCli.h"
#include "KCConnCliDlg.h"
#include "afxdialogex.h"
#include "kcxpapi.h"
#include "KCBPCli.h"
#include "Slog.h"
#include "Util.h"
#include "CMSKCBPCli.h"
#include "EditList.h"
#include "CSqlapi.h"
#include <process.h>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib,"kcxpapi.lib")
#pragma comment(lib,"KCBPCli.lib")

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
CCMSKCBPCli g_oCli;
bool g_bRunningTask = false;

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CKCConnCliDlg 对话框




CKCConnCliDlg::CKCConnCliDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CKCConnCliDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CKCConnCliDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_SERVER_NAME, m_vServerName);
    DDX_Control(pDX, IDC_EDIT_UID, m_vUid);
    DDX_Control(pDX, IDC_EDIT_PWD, m_vPwd);
    DDX_Control(pDX, IDC_EDIT_HOST, m_vHost);
    DDX_Control(pDX, IDC_EDIT_PORT, m_vPort);
    DDX_Control(pDX, IDC_EDIT_REQ_QUE, m_vReqQName);
    DDX_Control(pDX, IDC_EDIT_ANS_QUE, m_vAnsQName);
    DDX_Control(pDX, IDC_COMBO_CMDS, m_vCmdList);
    DDX_Control(pDX, IDC_EDIT_OUTPUT, m_vOutput);
    DDX_Control(pDX, IDC_BTN_SERVER, m_vConnectServerBtn);
    DDX_Control(pDX, IDC_EDIT_UID2, m_vBegTime);
    DDX_Control(pDX, IDC_EDIT_UID3, m_vEndTime);
    DDX_Control(pDX, IDC_BTN_SEND_CMD, m_vSwitchBtn);
    DDX_Control(pDX, IDC_COMBO2, m_vTask);
}

BEGIN_MESSAGE_MAP(CKCConnCliDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, &CKCConnCliDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CKCConnCliDlg::OnBnClickedCancel)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BTN_SERVER, &CKCConnCliDlg::OnBnClickedBtnServer)
    ON_BN_CLICKED(IDC_BTN_LOAD_CMD, &CKCConnCliDlg::OnBnClickedBtnLoadCmd)
    ON_BN_CLICKED(IDC_BTN_SEND_CMD, &CKCConnCliDlg::OnBnClickedBtnSendCmd)
    ON_BN_CLICKED(IDC_BTN_LOAD_CLEAR, &CKCConnCliDlg::OnBnClickedBtnLoadClear)
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_BTN_SAVE_CMD, &CKCConnCliDlg::OnBnClickedBtnSaveCmd)
    ON_BN_CLICKED(IDC_BTN_EDIT_CMD, &CKCConnCliDlg::OnBnClickedBtnEditCmd)
    ON_BN_CLICKED(IDC_BTN_EDIT_CMD2, &CKCConnCliDlg::OnClickedBtnEditRunPara)
END_MESSAGE_MAP()


// CKCConnCliDlg 消息处理程序

void CKCConnCliDlg::GetDefaultRunPara(map<string, string>& mapRunPara)
{
    mapRunPara["call_num"] = "";
    mapRunPara["rand_stay"] = "";
    mapRunPara["fail_ret"] = "";
    mapRunPara["beg_time"] = "";
    mapRunPara["end_time"] = "";
}


bool CKCConnCliDlg::IsNQAgreeOrder(const string& strLBM)
{
    return (strLBM == "L2622202");
}

BOOL CKCConnCliDlg::MakeCmd(const int nCmd, const vector<vector<string>>& vecTask, vector<vector<string>>& vecCmd)
{
    // 赋默认值
    vecCmd = vecTask;

    // 过滤任务类型
    map<string, string> mapPara;
    string strPara = m_oDbTask.m_vecPara[nCmd];
    CMS::GetKV(strPara, ",", ":", mapPara);
    if (mapPara["type"] != "secu_auto_ord_cfg")
    {
        return TRUE;
    }

    AddOut("正在生成自动委托数据...");
    // 支持集合的列表：trd_id，inst_code，quote_type
    int nTrdId = -1;
    int nCodeId = -1;
    int nQuoteId = -1;
    int nPriceId = -1;
    int nSucc = 0;
    vector<string>& vecHeader = m_oDbTask.m_vecHeader[nCmd];
    for (size_t i = 0; i < vecHeader.size(); i++)
    {
        // 成功计数，预置
        nSucc += 1;
        if (CMS::Lowwer(vecHeader[i]) == "trd_id")
        {
            nTrdId = i;
        }
        else if (CMS::Lowwer(vecHeader[i]) == "inst_code")
        {
            nCodeId = i;
        }
        else if (CMS::Lowwer(vecHeader[i]) == "quote_type")
        {
            nQuoteId = i;
        }
        else if (CMS::Lowwer(vecHeader[i]) == "ord_price")
        {
            nPriceId = i;
        }
        else
        {
            // 回退预置
            nSucc -= 1;
        }
    }
    if (nSucc != 4)
    {
        LOGE("自动下单结果参数错误，字段不匹配");
        return FALSE;
    }

    // 处理任务指令
    // 新申请变量，防止入参是同一个变量
    vector<vector<string>> vecSubTask;
    vector<string> vecItem;
    vector<string> vecTrdId;
    vector<string> vecInstCode;
    vector<string> vecQuoteType;
    map<string, map<string, string>> mapHqInfo;
    string strBuf;
    m_oDbTask.m_vecPara[nCmd];
    for (auto i = vecTask.begin(); i!=vecTask.end(); ++i)
    {
        // 交易类别
        strBuf = i->at(nTrdId);
        vecTrdId.clear();
        CMS::Split(strBuf, ",", vecTrdId, true);

        // 产品代码
        strBuf = i->at(nCodeId);
        vecInstCode.clear();
        CMS::Split(strBuf, ",", vecInstCode, true);

        // 报价类型
        strBuf = i->at(nQuoteId);
        vecQuoteType.clear();
        CMS::Split(strBuf, ",", vecQuoteType, true);

        // 参数检查
        if (vecTrdId.empty() || vecInstCode.empty() || vecQuoteType.empty())
        {
            AddOut("任务记录参数有误，本任务已被忽略");
            continue;
        }

        // 查询行情信息，获取涨跌停价格等信息
        mapHqInfo.clear();
        for (size_t j = 0; j < vecInstCode.size(); j++)
        {
            map<string, string> mapCurHq;
            // 如果有设置价格，使用已设置价格
            vecItem = *i;
            if (atof(vecItem[nPriceId].c_str()) > 0)
            {
                mapCurHq["BUY_PRICE"] = vecItem[nPriceId];
                mapCurHq["SALE_PRICE"] = vecItem[nPriceId];
                mapHqInfo[vecInstCode[j]] = mapCurHq;
                continue;
            }

            // 获取证券价格信息
            if (!GetInstCodeInfo(vecInstCode[j], mapCurHq))
            {
                // 行情获取失败，证券代码置空，不处理
                LOGW("证券代码[%s]已被移除，信息获取失败", vecInstCode[j].c_str());
                vecInstCode[j] = "";
                continue;
            }

            // 根据策略获取不同价格作为委托价格
            string strUpperPrice = CMS::Upper(mapPara["ord_price"]);
            if (mapCurHq.find(strUpperPrice) != mapCurHq.end())
            {
                mapCurHq["BUY_PRICE"] = mapCurHq[strUpperPrice];
                mapCurHq["SALE_PRICE"] = mapCurHq[strUpperPrice];
            }
            // 争取成交
            else if (strUpperPrice == "WANT_MATCH")
            {
                mapCurHq["BUY_PRICE"] = mapCurHq["PRICE_CEILING"];
                mapCurHq["SALE_PRICE"] = mapCurHq["PRICE_FLOOR"];
            }
            // 默认随机价格
            else
            {
                double fMin = atof(mapCurHq["PRICE_FLOOR"].c_str());
                double fMax = atof(mapCurHq["PRICE_CEILING"].c_str());
                double nInterval = (fMax - fMin) > 0 ? (fMax - fMin) : 10;
                double fCur = fMin + (rand()%10 + 1)*0.1*nInterval;
                mapCurHq["BUY_PRICE"] = CMS::Formate("%.f", fCur);
                mapCurHq["SALE_PRICE"] = mapCurHq["BUY_PRICE"];
            }
            mapHqInfo[vecInstCode[j]] = mapCurHq;
        }

        // 分拆单个任务指令
        for (size_t j = 0; j < vecInstCode.size(); j++)
        {
            // 空的证券代码跳过
            if (vecInstCode[j].empty())
            {
                continue;
            }

            // 交易类别
            map<string, string>& mapCurHq = mapHqInfo[vecInstCode[j]];
            for (auto trd_id : vecTrdId)
            {
                // 报价类型
                for (auto quote_type : vecQuoteType)
                {
                    // 拷贝原任务，对集合字段替换
                    vecItem = *i;
                    vecItem[nTrdId] = trd_id;
                    vecItem[nQuoteId] = quote_type;
                    vecItem[nCodeId] = vecInstCode[j];
                    vecItem[nPriceId] = (trd_id == "20B") ? mapCurHq["BUY_PRICE"] : mapCurHq["SALE_PRICE"];
                    // 价格修正
                    double fPrice = atof(vecItem[nPriceId].c_str());
                    if (fPrice <= 0 || fPrice >= 100000.0f)
                    {
                        vecItem[nPriceId] = CMS::Formate("10");
                    }
                    vecSubTask.push_back(vecItem);
                }
            }
        }
    }
    vecCmd = vecSubTask;

    // 如果乱序输出
    if (mapPara["ord_rand"] != "false")
    {
        random_shuffle(vecCmd.begin(), vecCmd.end());
    }

    return TRUE;
}

bool CKCConnCliDlg::GetDatabaseTaskData(vector<string>& vecData)
{
    STDbRunTaskInfo& oTask = m_oDbTask;
    CSqlapi oSql;
    if (!oSql.Connect(oTask.m_strDbConn, oTask.m_strDbUid, oTask.m_strDbPwd, SA_Oracle_Client))
    {
        AddOut("数据库连接失败");
        return false;
    }
    int nCur = m_vTask.GetCurSel();
    if (nCur < 0)
    {
        return false;
    }

    vector<vector<string>> vecTab;
    string strData;
    for (size_t i = nCur; i < oTask.m_vecSql.size(); i++)
    {
        vecTab.clear();
        string& strSql = oTask.m_vecSql[i];
        if (!oSql.Query(strSql, vecTab))
        {
            AddOut("查询SQL失败:" + strSql);
            continue;
        }

        // 如果是自动下单，需进一步特殊处理：查询行情确定涨跌停价格，生成单个调用指令
        MakeCmd(i, vecTab, vecTab);

        const string& strRunPara = oTask.m_vecPara[i];
        const vector<string>& vecHeader = oTask.m_vecHeader[i];
        for (size_t r = 0; r < vecTab.size(); r++)
        {
            strData = "";
            for (size_t c = 0; c<vecTab[r].size(); c++)
            {
                if (vecHeader[c] == "LBM")
                {
                    strData = vecTab[r][c] + "!" + strData;
                }
                else
                {
                    strData += vecHeader[c] + ":" + vecTab[r][c] + ",";
                }
            }
            vecData.push_back(strRunPara + "|" + strData);
            AddOut("添加命令:" + *(vecData.end() - 1));
        }
        // 运行一条指令后退出
        break;
    }
    AddOut(CMS::Formate("加载任务完成，队列数:%d 运行次数:%d", (int)vecData.size(), oTask.m_nRunTimes));
    return true;
}

BOOL CKCConnCliDlg::OnInitDialog()
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

    // 加载服务器连接配置
    LoadServerConfig();
    OnBnClickedBtnLoadCmd();

    // 设置输出文本框内容容纳字符大小
    m_vOutput.SetLimitText(-1);

    // 初始化随机数
    srand((unsigned int)time(NULL));

    // 初始化运行允许时间
    string strBuf;
    TCHAR szBuf[256] = { 0 };
    DWORD nCurTime = atoi(CMS::GetDate("%H%M%S").c_str());
    strBuf = CMS::GetDate("%Y%m%d");
    CString strBegTime((strBuf + "000001").c_str());
    CString strEndTime((strBuf + "235959").c_str());
    
    m_vBegTime.SetWindowText(strBegTime);
    m_vEndTime.SetWindowText(strEndTime);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CKCConnCliDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CKCConnCliDlg::OnPaint()
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
HCURSOR CKCConnCliDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CKCConnCliDlg::LoadServerConfig()
{
    BOOL bRet = FALSE;

    // 加载配置文件
    CString strConfigFile = _T("./config.ini");
    /*
    [KCBP]
    UserName     =    KCXP00
    Password     =    888888
    ServerName   =    KCXP1
    Address      =    10.254.253.46
    SendQName    =    req_otc_ww
    ReceiveQName =    ans_otc_ww
    Port         =    21000*/

    CString strSecName = _T("KCBP");
    CString strKeyName = _T("UserName");
    TCHAR   szValue[256] = { 0 };

    // 用户名
    strKeyName = _T("UserName");
    GetPrivateProfileString(strSecName, strKeyName, _T("KCXP00"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vUid.SetWindowText(szValue);

    // 用户密码
    strKeyName = _T("Password");
    GetPrivateProfileString(strSecName, strKeyName, _T("888888"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vPwd.SetWindowText(szValue);

    // 服务名
    strKeyName = _T("ServerName");
    GetPrivateProfileString(strSecName, strKeyName, _T("KCXP1"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vServerName.SetWindowText(szValue);

    // 主机地址
    strKeyName = _T("Address");
    GetPrivateProfileString(strSecName, strKeyName, _T("10.254.253.46"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vHost.SetWindowText(szValue);

    // 主机端口
    strKeyName = _T("Port");
    GetPrivateProfileString(strSecName, strKeyName, _T("21000"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vPort.SetWindowText(szValue);

    // 发送队列
    strKeyName = _T("SendQName");
    GetPrivateProfileString(strSecName, strKeyName, _T("req_otc_zb"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vReqQName.SetWindowText(szValue);

    // 接收队列
    strKeyName = _T("ReceiveQName");
    GetPrivateProfileString(strSecName, strKeyName, _T("ans_otc_zb"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vAnsQName.SetWindowText(szValue);

    // 读取公共参数
    string strPubName = "PUBLIC_PARA";
    string strPath = CMS::CSW2Cpps(strConfigFile);

    CMS::ReadIniSectionInfo(strPubName, strPath, m_mapKcbpPublicPara);

    // 读取数据库任务
    map<string, string> mapTask;
    CMS::ReadIniSectionInfo("DB_TASK", strPath, mapTask);
    if (mapTask.size() > 3)
    {
        string k, v;
        STDbRunTaskInfo& oTask = m_oDbTask;
        oTask.m_strDbUid = mapTask["UserName"];
        oTask.m_strDbPwd = mapTask["Password"];
        oTask.m_strDbConn = mapTask["ConnInfo"];

        // 运行次数
        oTask.m_nRunTimes = 1;
        k = "RunTimes";
        if (CMS::Get(mapTask, k, v))
        {
            oTask.m_nRunTimes = atoi(v.c_str());
        }
        
        // 公共运行参数
        string strPara;
        k = "TaskRunPara";
        CMS::Get(mapTask, k, strPara);
        map<string, string> mapCommPara;
        CMS::GetKV(strPara, ",", ":", mapCommPara);

        for (int i = 1; i < 256; i++)
        {
            // sql语句
            k = CMS::Formate("Task%dSql", i);
            if (!CMS::Get(mapTask, k, v))
            {
                break;
            }
            // 分析出select字段名
            vector<string> vecHeader;
            if (!Sqlapi::GetSqlFields(v, vecHeader))
            {
                LOGE("配置[%s]查询字段非法", v.c_str());
                break;
            }
            oTask.m_vecSql.push_back(v);
            oTask.m_vecHeader.push_back(vecHeader);

            // 名字
            k = CMS::Formate("Task%dName", i);
            v = "";
            CMS::Get(mapTask, k, v);
            if (CMS::Trim(v).empty())
            {
                v = CMS::Formate("数据任务%d", i);
            }
            oTask.m_vecName.push_back(v);
            m_vTask.AddString(CString(CMS::Formate("%02d-%s", i, v.c_str()).c_str()));

            // 运行参数
            map<string, string> mapPara = mapCommPara;
            k = CMS::Formate("Task%dRunPara", i);
            v = "";
            mapPara["name"] = oTask.m_vecName[i-1];
            if(CMS::Get(mapTask, k, v))
            {
                CMS::GetKV(v, ",", ":", mapPara);
            }
            
            v.clear();
            for (auto ite = mapPara.begin(); ite != mapPara.end(); ++ite)
            {
                if (!v.empty())
                {
                    v += ",";
                }
                v += ite->first + ":" + ite->second;
            }
            oTask.m_vecPara.push_back(v);
        }
    }
    return TRUE;
}

void CKCConnCliDlg::AddOut(const string& strText, BOOL bNewLine/* = TRUE*/)
{
    AddOut(CString(strText.c_str()), bNewLine);
}
void CKCConnCliDlg::AddOut(const CString& strText, BOOL bNewLine/* = TRUE*/)
{
    static int nLine = 0;

    CString strAddText((CMS::Formate("%04d",++nLine) + " [" + CMS::GetCurTime() + "] ").c_str());
    strAddText += strText;
    if (bNewLine)
    {
        strAddText += _T("\r\n");
    }
    int nCurIdx = m_vOutput.GetWindowTextLength();
    m_vOutput.SetSel(nCurIdx, nCurIdx);
    m_vOutput.ReplaceSel(strAddText);
}




int CKCConnCliDlg::TimeRunStat()
{
    // 获取配置的时间
    string strBegTime = CMS::GetWndText(m_vBegTime);
    string strEndTime = CMS::GetWndText(m_vEndTime);
    string strCurTime = CMS::GetDate("%Y%m%d%H%M%S");

    if (!strBegTime.empty())
    {
        if (strCurTime < strBegTime)
        {
            return -1;
        }
    }
    if (!strEndTime.empty())
    {
        if (strCurTime > strEndTime)
        {
            return 1;
        }
    }
    return 0;

}

void CKCConnCliDlg::OnBnClickedOk()
{
}

void CKCConnCliDlg::OnBnClickedCancel()
{
}

BOOL CKCConnCliDlg::ConnectKcxp()
{
    BOOL bRet = FALSE;

    string strHost = "10.254.253.46";
    string strPort = "21000";
    string strUser = "KCXP00";
    string strPass = "888888";
    string strQReq = "req_otc_zb";
    string strQAns = "req_otc_zb";

    KCXP_HCONN hKcxpConn = NULL;
    KCXP_LONG  lCode = 0;
    KCXP_LONG  lReason = 0;

    // 连接KCXP
    KCXP_Connx((KCXP_CHAR *)strHost.c_str(), 
                atoi(strPort.c_str()), 
                (KCXP_CHAR*)strUser.c_str(), 
                (KCXP_CHAR*)(strPass.c_str()), 
                &hKcxpConn, &lCode, &lReason);
    if (lCode != KCXP_CC_OK)
    {
        LOGE("连接KCXP失败,Code[%ld] Reason[%ld]. Para[%s,%s,%s,%s]", lCode, lReason,
            strHost.c_str(), strPort.c_str(), strUser.c_str(), strPass.c_str());
        return false;
    }
    LOGD("连接KCXP成功. Para[%s,%s,%s,%s]", strHost.c_str(), strPort.c_str(), strUser.c_str(), strPass.c_str());

    // 打开队列
    KCXP_HOBJ hHOBJ = NULL;
    KCXP_OD   oDesc = { KCXP_DEFAULT_OD };
    KCXP_LONG lOptions = 0;

    oDesc.iObjectType = KCXP_OT_Q;
    strncpy_s(oDesc.strObjectName, strQReq.c_str(), sizeof(oDesc.strObjectName) - 1);
    lOptions = KCXP_OO_AS_Q_DEF | KCXP_OO_OUTPUT | KCXP_OO_INQUIRE;
    KCXP_Open(hKcxpConn, &oDesc, lOptions, &hHOBJ, &lCode, &lReason);
    if (lCode != KCXP_CC_OK)
    {
        LOGE("打开KCXP队列失败,Code[%ld] Reason[%ld]. Para[%s]", lCode, lReason, strQReq.c_str());
        KCXP_Disconn(&hKcxpConn, 0L, &lCode, &lReason);
        return false;
    }
    LOGD("打开KCXP队列成功, Para[%s]", strQReq.c_str());

    return bRet;
}

BOOL CKCConnCliDlg::GetInstCodeInfo(const string& strInstCode, map<string, string>& mapInfo)
{
    // 公共参数
    map<string, string> mapInput = m_mapKcbpPublicPara;
    mapInput["F_OP_SITE"] = CMS::GetLocalHost();
    mapInput["F_RUNTIME"] = CMS::GetCurDate();

    // 证券信息
    mapInput["F_SUBSYS"] = "13";
    mapInput["INST_CODE"] = strInstCode;
    vector<map<string, string>> vecSecuAns;
    g_oCli.CallLbm("L2622102", mapInput, vecSecuAns);
    if (vecSecuAns.empty())
    {
        LOGE("获取证券[%s]信息失败", strInstCode.c_str());
        return FALSE;
    }
    map<string, string> mapSecuInfo = vecSecuAns[0];

    // 行情信息
    map<string, string> mapHqInfo;
    vector<map<string, string>> vecHqAns;
    mapInput["MKT_CODE"] = "1";
    g_oCli.CallLbm("L2622103", mapInput, vecHqAns);
    if (vecHqAns.empty())
    {
        LOGE("获取证券[%s]行情信息失败", strInstCode.c_str());
    }
    else
    {
        mapHqInfo = vecHqAns[0];
    }

    // 组合信息
    mapInfo["INST_CODE"] = strInstCode;
    // 最近成交价
    mapInfo["CURRENT_PRICE"] = mapHqInfo.empty() ? mapSecuInfo["PRICE_CEILING"] : mapHqInfo["CURRENT_PRICE"];
    // 最高成交价
    mapInfo["MAX_PRICE"] = mapHqInfo.empty() ? mapSecuInfo["PRICE_CEILING"] : mapHqInfo["MAX_PRICE"];
    // 最低成交价
    mapInfo["MIN_PRICE"] = mapHqInfo.empty() ? mapSecuInfo["PRICE_FLOOR"] : mapHqInfo["MIN_PRICE"];
    // 涨停价格
    mapInfo["PRICE_CEILING"] = mapSecuInfo["PRICE_CEILING"];
    // 跌停价格
    mapInfo["PRICE_FLOOR"] = mapSecuInfo["PRICE_FLOOR"];

    return TRUE;
}

BOOL CKCConnCliDlg::GetInstCodeInfo(map<string, string>& mapReq, map<string, string>& mapInstInfo, map<string, string>& mapHqInfo)
{
    // 证券信息
    const string strInstCode = mapReq["INST_CODE"];
    vector<map<string, string>> vecSecuAns;
    g_oCli.CallLbm("L2622102", mapReq, vecSecuAns);
    if (vecSecuAns.empty())
    {
        LOGE("获取证券[%s]信息失败", strInstCode.c_str());
        return FALSE;
    }
    mapInstInfo = vecSecuAns[0];

    // 行情信息
    vector<map<string, string>> vecHqAns;
    g_oCli.CallLbm("L2622103", mapReq, vecHqAns);
    if (vecHqAns.empty())
    {
        LOGE("获取证券[%s]行情信息失败", strInstCode.c_str());
    }
    else
    {
        mapHqInfo = vecHqAns[0];
    }

    return TRUE;
}

BOOL CKCConnCliDlg::MakeCmd(const string& strName, const string& strLBM, const map<string, string>& mapReq, string& strCmd)
{
    BOOL bRet = FALSE;

    if (CMS::Trim(strLBM).length() <= 0)
    {
        LOGE("LBM不允许为空");
        return bRet;
    }

    string strFix = "!";
    strCmd = CMS::Trim(strName) + "|" + CMS::Trim(strLBM);
    map<string, string>::const_iterator iteMap = mapReq.begin();
    while (iteMap != mapReq.end())
    {
        strCmd += strFix + CMS::Trim(iteMap->first) + ":" + CMS::Trim(iteMap->second);
        strFix = ",";
        iteMap++;
    }

    return TRUE;
}
bool CKCConnCliDlg::GetCmdPara(const string& strPara, map<string, string>& mapKV)
{
    bool bRet = true;

    // 解析参数字段
    vector<string> vecPara;
    CMS::Split(strPara, ",", vecPara, true);
    
    // 一个参数默认为名字
    if (vecPara.size() == 1)
    {
        mapKV["name"] = vecPara[0];
        return bRet;
    }

    for (size_t i = 0; i < vecPara.size(); i++)
    {
        vector<string> vecPair;
        CMS::Split(vecPara[i], ":", vecPair, true);
        if (vecPair.size() != 2)
        {
            AddOut(_T("字段信息[") + CString(vecPara[i].c_str()) + _T("]格式有误，请重试"));
            return bRet;
        }
        mapKV[vecPair[0]] = vecPair[1];
    }

    return bRet;
}


BOOL CKCConnCliDlg::CallKxcpLbm(const string& strLbmNo, const map<string, string>& mapReq, vector<map<string, string>>& vecAns, bool bRst/* = true*/)
{
    BOOL bRet = TRUE;

    // 调用请求
    CString strCallInfo;
    string strErrMsg;

    DWORD dwBeg = GetTickCount();
    int nRet = g_oCli.CallLbm(strLbmNo, mapReq, vecAns);
    DWORD dwEnd = GetTickCount();
    LOGI("应答:");
    strCallInfo.Format(_T("接口耗时:%lums,返回结果:\r\n"), dwEnd - dwBeg);
    if (nRet != 0)
    {
        g_oCli.GetLastError(nRet, strErrMsg);
        strCallInfo += _T("接口调用失败,");
        strCallInfo += CString(("返回错误码:" + CMS::Itoa(nRet)).c_str());
        strCallInfo += CString((", 返回错误信息:" + strErrMsg).c_str());
        bRet = FALSE;
    }
    else if (vecAns.size() <= 0)
    {
        strCallInfo += _T("返回结果为空");
    }
    else if (bRst)
    {
        // 获取标题
        string         strCurText;
        vector<string> vecTitle;
        vector<size_t> vecSize;
        int            nIdx = 0;
        map<string, string>::iterator iteAns = vecAns[0].begin();
        while (iteAns != vecAns[0].end())
        {
            if (CMS::Upper(iteAns->first) == "REC_NUM")
            {
                vecTitle.insert(vecTitle.begin(), iteAns->first);
                vecSize.insert(vecSize.begin(), iteAns->first.length());
            }
            else
            {
                vecTitle.push_back(iteAns->first);
                vecSize.push_back(iteAns->first.length());
            }
            iteAns++;
        }

        // 解析各返回结果
        vector<vector<string>> vecAnsRow;
        vecAnsRow.push_back(vecTitle);
        for (size_t i = 0; i < vecAns.size(); i++)
        {
            vector<string> vecCurRow;
            for (size_t j = 0; j < vecTitle.size(); j++)
            {
                iteAns = vecAns[i].find(vecTitle[j]);
                if (iteAns != vecAns[i].end())
                {
                    vecSize[j] = max(vecSize[j], iteAns->second.length());
                    vecCurRow.push_back(iteAns->second);
                }
                else
                {
                    LOGW("未按第一返回结果标题找到后续返回结果字段，返回结果不格式化处理输出");
                    vecSize.clear();
                    vecTitle.clear();
                    j = vecTitle.size();
                    i = vecAns.size();
                }
            }
            if (vecCurRow.size() == vecTitle.size())
            {
                vecAnsRow.push_back(vecCurRow);
            }
        }

        // 如果结果不格式化处理，直接输出
        if (vecTitle.size() <= 0)
        {
            // 显示返回值
            for (size_t i = 0; i < vecAns.size(); i++)
            {
                map<string, string>::iterator iteAns = vecAns[i].begin();
                CString strMsg;
                CString strtemp;
                while (iteAns != vecAns[i].end())
                {
                    strMsg += (iteAns->first + ":" + iteAns->second + "\t").c_str();
                    iteAns++;
                }
                strCallInfo += strMsg + _T("\r\n");
            }
        }
        else
        {
            string strRstLine;
            for (size_t i = 0; i < vecAnsRow.size(); i++)
            {
                strRstLine = "";
                for (size_t j = 0; j < vecAnsRow[i].size(); j++)
                {
                    strRstLine += CMS::TextFill(vecAnsRow[i][j], vecSize[j]) + " | ";
                }
                strCallInfo += CString((strRstLine + "\r\n").c_str());
            }
        }
    }
    else
    {
        strCallInfo += ("调用成功，返回记录数:" + CMS::CastFrom<size_t, string>(vecAns.size())).c_str();
    }
    AddOut(strCallInfo);

    return bRet;
}


BOOL CKCConnCliDlg::CallKxcpByCmd(const string& strCmd)
{
    BOOL bRet = TRUE;
    string strName;
    string strLBM;
    map<string, string> mapReq;

    // 命令模式
    if (!ParseCmd(strCmd, strName, strLBM, mapReq))
    {
        return FALSE;
    }

    // 获取命令控制参数
    map<string, string> mapCmdPara;
    GetCmdPara(strName, mapCmdPara);

    // 组包请求
    map<string, string> mapInput;
    map<string, string>::iterator iteMapStr;

    mapInput = m_mapKcbpPublicPara;
    mapInput["F_OP_SITE"] = CMS::GetLocalHost();
    mapInput["F_RUNTIME"] = CMS::GetCurDate();
    for (iteMapStr = mapReq.begin(); iteMapStr != mapReq.end(); iteMapStr++)
    {
        mapInput[iteMapStr->first] = iteMapStr->second;
    }
    
    // 命令控制参数处理
    CString strCallName;
    CString strMsg;
    string strKey = "name";
    string strBuf;
    vector<string> vecData;
    if (CMS::Get(mapCmdPara, strKey, strName))
    {
        strCallName = ("名称:" + strName + ",").c_str();
    }

    // 解析出调用频次和频率
    // 次数@间隔(ms)
    int nCallNums = 1;
    int nCallInter = 500; // 默认500ms
    strKey = "call_num";
    strBuf = "";
    CMS::Get(mapCmdPara, strKey, strBuf);
    if (strBuf.size() > 0)
    {
        vecData.clear();
        CMS::Split(strBuf, "@", vecData, true);
        if (vecData.size() > 0)
        {
            nCallNums = CMS::CastFrom<string, int>(vecData[0]);
        }
        if (vecData.size() > 1)
        {
            nCallInter = CMS::CastFrom<string, int>(vecData[1]);
        }
    }

    // 随机时间间隔
    int nRandomTime = 0; // 默认0ms
    strKey = "rand_stay";
    strBuf = "";
    CMS::Get(mapCmdPara, strKey, strBuf);
    if (strBuf.size() > 0)
    {
        nRandomTime = CMS::CastFrom<string, int>(strBuf);
    }

    // 错误是否直接返回
    int nFailRet = 1; // 默认是
    strKey = "fail_ret";
    strBuf = "";
    CMS::Get(mapCmdPara, strKey, strBuf);
    if (strBuf.size() > 0)
    {
        nFailRet = CMS::CastFrom<string, int>(strBuf);
    }
    // 打印调用信息
    strMsg.Format(_T("调用总次数:%d, 时间间隔:%d+rand(%d) ms"), nCallNums, nCallInter, nRandomTime);
    AddOut(strMsg);

    // 时间控制
    string strCurTime = CMS::GetDate("%H%M%S");
    string strBegTime = "000000";
    string strEndTime = "999999";
    strKey = "beg_time";
    CMS::Get(mapCmdPara, strKey, strBegTime);
    strKey = "end_time";
    CMS::Get(mapCmdPara, strKey, strEndTime);
    if ((strCurTime < strBegTime) || (strCurTime > strEndTime))
    {
        strBuf = CMS::Formate("调用时间未到:%s-%s", strBegTime.c_str(), strEndTime.c_str());
        AddOut(strBuf);
        Sleep(nRandomTime);
        return TRUE;
    }

    // 是否自动撤单
    int nCancelLevel = -1;
    strKey = "ord_cancel_level";
    strBuf = "";
    if (CMS::Get(mapCmdPara, strKey, strBuf))
    {
        nCancelLevel = atoi(strBuf.c_str());
    }

    // 组调用请求字符串,调试打印
    CString strCallInfo = strCallName + ("请求信息:" + strLBM + "!").c_str();
    for (iteMapStr = mapInput.begin(); iteMapStr != mapInput.end(); iteMapStr++)
    {
        if (iteMapStr != mapInput.begin())
        {
            strCallInfo += _T(",");
        }
        strCallInfo += CString((iteMapStr->first + ":" + iteMapStr->second).c_str());
    }
    LOGI("%s", CMS::CSW2Cpps(strCallInfo).c_str());
    AddOut(strCallInfo);

    // 查看是否为自定义函数
    if (strLBM.substr(0, 5) == "5405_")
    {
        return CallMyFunc(strLBM, mapInput);
    }

    // 调用函数
    int nAgreeId = atoi(CMS::GetDate("%M%S").c_str())*100;
    int nHaveCalls = 0;
    DWORD nDelay = 0;
    vector<map<string, string>> vecAns;
    for (; nCallNums > 0; --nCallNums)
    {
        // 如果是协议转让，则添加协议编号参数
        if (IsNQAgreeOrder(strLBM))
        {
            mapInput["AGREEMENT_ID"] = CMS::Formate("%d", ++nAgreeId);
        }

        // 前后10个调用显示结果详情，中间的只显示调用结果
        vecAns.clear();
        if ((nCallNums>10) && (nHaveCalls>10))
        {
            bRet = CallKxcpLbm(strLBM, mapInput, vecAns, false);
        }
        else
        {
            bRet = CallKxcpLbm(strLBM, mapInput, vecAns, true);
        }
        nDelay = nCallInter + (rand() % (nRandomTime + 1));
        AddOut(CMS::Formate("延时设定:%.2f", nDelay/1000.0f));
        Sleep(nDelay);
        nHaveCalls++;

        // 支持自动撤单
        if (nCancelLevel>0 && (rand()%10 < nCancelLevel) && !vecAns.empty())
        {
            mapInput["ORI_APP_SNO"] = vecAns[0]["APP_SNO"];
            mapInput["ORI_APP_DATE"] = vecAns[0]["APP_DATE"];
            vecAns.clear();
            if (CallKxcpLbm("L2622203", mapInput, vecAns))
            {
                AddOut("撤单成功");
            }
            else
            {
                AddOut("撤单失败");
            }
        }

        // 如果是协议转让，自动生成对手方订单
        else if (!vecAns.empty() && IsNQAgreeOrder(strLBM))
        {
            swap(mapInput["TARGET_SECU_ACC"], mapInput["SECU_ACC"]);
            swap(mapInput["TARGET_CUST_CODE"], mapInput["CUST_CODE"]);
            swap(mapInput["TARGET_SEAT"], mapInput["TRD_SEAT"]);
            mapInput["TRD_ID"] = (mapInput["TRD_ID"] == "20B" ? "20S" : "20B");
            vecAns.clear();
            CallKxcpLbm(strLBM, mapInput, vecAns);
        }

        // 调用失败且设置直接退出
        if (!g_bRunningTask || ((!bRet) && (nFailRet!=0)))
        {
            break;
        }
    }

    return bRet;
}

BOOL CKCConnCliDlg::ParseCmd(const string& strCmd, string& strName, string& strLBM, map<string, string>& mapReq)
{
    BOOL bRet = FALSE;

    // 解析命令
    // (备注|)LBM号!PN1:VA1,PN2:VA2,...

    // 命令消息体体分离
    vector<string> vecLBM;
    CMS::Split(strCmd, "!", vecLBM, true);
    if (vecLBM.size() != 2)
    {
        return bRet;
    }

    // 解析头-截取出LBM号和名字
    vector<string> vecHead;;
    CMS::Split("|" + vecLBM[0], "|", vecHead, true);
    if (vecLBM.size() < 2)
    {
        AddOut(_T("当前输入的命令格式头有误，请重试：") + CString(strCmd.c_str()));
        return bRet;
    }
    size_t nIdx = vecHead.size() - 2;
    strName = vecHead[nIdx];
    strLBM = vecHead[nIdx + 1];

    // 解析参数字段
    vector<string> vecPara;
    CMS::Split(vecLBM[1], ",", vecPara);
    for (size_t i = 0; i < vecPara.size(); i++)
    {
        vector<string> vecPair;
        CMS::Split(vecPara[i], ":", vecPair, true);
        if (vecPair.size() != 2)
        {
            continue;
        }
        mapReq[vecPair[0]] = vecPair[1];
    }

    return TRUE;
}


BOOL CKCConnCliDlg::GetCmdPara(const string& strCmd, STRunBatchTaskProcPara* pPara)
{
    // 非命令参数
    if (strCmd[0] != '_')
    {
        return FALSE;
    }

    string strName, strVal;
    CMS::GetKV(strCmd, strName, strVal);

    // 运行次数
    if (strName == "_run_times_")
    {
        pPara->nRunTimes = atoi(strVal.c_str());
    }

    return TRUE;
}

BOOL CKCConnCliDlg::SetKCBPPublicInput(map<string, string>& mapKV)
{
    BOOL bRet = FALSE;

    mapKV["F_FUNCTION"]         = "";
    mapKV["F_SUBSYS"]           = "13";
    mapKV["F_OP_USER"]          = "8888";
    mapKV["F_OP_ROLE"]          = "2";
    mapKV["F_OP_SITE"]          = "127.0.0.1";
    mapKV["F_CHANNEL"]          = "0";
    mapKV["F_SESSION"]          = "ZSOTCXTYZMU";
    mapKV["F_RUNTIME"]          = "2018-01-08";
    return TRUE;
}



void CKCConnCliDlg::OnClose()
{
    EndDialog(IDCLOSE);
}


void CKCConnCliDlg::OnBnClickedBtnServer()
{
    CString strText;
    m_vConnectServerBtn.GetWindowText(strText);

    if (strText == _T("断开服务器"))
    {
        if (g_oCli.Uninit())
        {
            AddOut(_T("断开KCBP服务器成功"));
            m_vConnectServerBtn.SetWindowText(_T("连接服务器"));
        }
        else
        {
            AddOut(_T("断开KCBP服务器失败"));
        }
        return;
    }
    

    g_oCli.m_strUserName = CMS::GetWndText(m_vUid);
    g_oCli.m_strPassword = CMS::GetWndText(m_vPwd);
    g_oCli.m_strServerName = CMS::GetWndText(m_vServerName);
    g_oCli.m_strAddress = CMS::GetWndText(m_vHost);
    g_oCli.m_strSendQName = CMS::GetWndText(m_vReqQName);
    g_oCli.m_strReceiveQName = CMS::GetWndText(m_vAnsQName);
    g_oCli.m_strPort = CMS::GetWndText(m_vPort);;

    if (!g_oCli.Init())
    {
        AddOut(_T("KCBP连接初始化失败"));
        return;
    }
    AddOut(_T("KCBP连接初始化成功"));
    m_vConnectServerBtn.SetWindowText(_T("断开服务器"));
    return;

    map<string, string> mapData;
    vector< map<string, string> > vecAns;
    g_oCli.SetKCBPPublicInput(mapData);

    mapData["CUST_CODE"] = "88000055";
    mapData["PAGE_RECCNT"] = "100";
    if (g_oCli.CallLbm("L2622126", mapData, vecAns))
    {
        AddOut(_T("KCBP接口调用失败"));
    }
    else
    {
        // 显示返回值
        for (size_t i = 0; i < vecAns.size(); i++)
        {
            map<string, string>::iterator iteAns = vecAns[i].begin();
            CString strMsg;
            CString strtemp;
            while (iteAns != vecAns[i].end())
            {
                strMsg += (iteAns->first + ":" + iteAns->second + " ").c_str();
                iteAns++;
            }
            AddOut(strMsg);
        }
    }
    g_oCli.Uninit();
}


void CKCConnCliDlg::OnBnClickedBtnLoadCmd()
{
    FILE* fp = fopen("cmd.txt", "rb");
    if (fp == NULL)
    {
        AddOut(_T("打开命令文件cmd.txt失败"));
        return;
    }
    vector<string> vecCmds;

    // 读取命令
    char szLineMax[512] = { 0 };
    while (!feof(fp))
    {
        memset(szLineMax, 0, sizeof(szLineMax));
        fgets(szLineMax, sizeof(szLineMax)-1, fp);
        vecCmds.push_back(CMS::Trim(szLineMax));
    }
    fclose(fp);

    m_vCmdList.ResetContent();
    // 加载到列表框
    for (size_t i = 0; i < vecCmds.size(); i++)
    {
        m_vCmdList.AddString(CString(vecCmds[i].c_str()));
    }
    AddOut(_T("加载命令文件完成"));
}

BOOL CKCConnCliDlg::CallWithNoneCmd(const string& strCall)
{
    BOOL bRet = TRUE;

    // 是否为文件
    DWORD dwAttrib = GetFileAttributesA(strCall.c_str());
    if (INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        CallByFile(strCall);
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}


unsigned int __stdcall RunBatchTaskProc(void* pPara)
{
    STRunBatchTaskProcPara* p = (STRunBatchTaskProcPara *)pPara;
    CKCConnCliDlg* pThis = p->pThis;
    vector<string>& vecTask = p->vecTasks;

    switch (p->nMode)
    {
    case MODE_ORA_TASK:
        pThis->GetDatabaseTaskData(vecTask);
        break;
    default:
        break;
    }

    // 循环运行任务
    g_bRunningTask = true;
    pThis->m_vSwitchBtn.SetWindowText(_T("停止请求"));
    vector<string>::iterator iteTask;
    int nStat = 0;
    while (g_bRunningTask && !vecTask.empty())
    {
        // 查看是否允许允许
        nStat = pThis->TimeRunStat();
        if (nStat < 0)
        {
            pThis->AddOut(_T("未到系统设定运行时间，请继续等待"));
            Sleep(10000);
            continue;
        }
        else if (nStat > 0)
        {
            pThis->AddOut(_T("系统设定运行时间已过，任务退出"));
            break;
        }

        // 次数限制
        if (p->nRunTimes-- == 0)
        {
            break;
        }

        // 运行本次任务
        int nSucc = 0;
        int nFail = 0;
        int nCurSeq = 0;
        int nTotalNums = vecTask.size();
        for (iteTask = vecTask.begin(); iteTask != vecTask.end(); )
        {
            // 显示处理进度
            pThis->AddOut(CMS::Formate("正在处理请求%d/%d", ++nCurSeq, nTotalNums));

            // 如果失败，则从任务列表中删除该任务
            if (!pThis->CallKxcpByCmd(*iteTask))
            {
                iteTask = vecTask.erase(iteTask);
                pThis->AddOut("调用出错，删除该任务");
                nFail++;
            }
            else
            {
                iteTask++;
                nSucc++;
            }

            if (!g_bRunningTask)
            {
                break;
            }
        }

        // 显示执行统计情况
        pThis->AddOut(CMS::Formate("本批任务执行结束,成功:%d, 失败:%d", nSucc, nFail));
    }
    g_bRunningTask = false;
    pThis->m_vSwitchBtn.SetWindowText(_T("发送请求"));
    pThis->AddOut("线程任务执行完毕");
    // 内存清理
    delete p;
    p = NULL;
    return 0;
}

BOOL CKCConnCliDlg::CallByFile(const string& strFile)
{
    BOOL bRet = FALSE;

    ifstream fLog;
    // 打开文件
    fLog.open(strFile);
    if (!fLog.is_open())
    {
        return bRet;
    }

    // 先预读取文件，减少文件访问时间
    string strLine;
    string strTemp;
    stringstream ssBuf;
    ssBuf << fLog.rdbuf();
    fLog.close();

    STRunBatchTaskProcPara* pPara = new STRunBatchTaskProcPara();
    pPara->nRunTimes = -1;
    for (size_t i = 0; getline(ssBuf, strLine); i++)
    {
        strTemp = CMS::Trim(strLine);
        if (strTemp.empty())
        {
            continue;
        }
        
        // 控制参数处理
        if (GetCmdPara(strTemp, pPara))
        {
            continue;
        }

        // 接口配置
        pPara->vecTasks.push_back(strTemp);
    }
    pPara->pThis = this;

    // 启动任务处理线程
    HANDLE bThr = (HANDLE)_beginthreadex(NULL, 0, RunBatchTaskProc, pPara, 0, NULL);
    return bRet;
}

BOOL CKCConnCliDlg::CallMyFunc(const string& strLbmNo, const map<string, string>& mapReq)
{
    string strCallInfo = "";
    string strFuncName = strLbmNo.substr(5);
    if (strFuncName == "证券行情")
    {
        // 买入（卖出）基准价格，为即时揭示的最低卖出（最高买入）申报价格；
        // 无即时揭示的最低卖出（最高买入）申报价格的，为即时揭示的最高买入（最低卖出）价格；
        // 双方均无申报的，为最近成交价；
        // 当日无成交的，为前收盘价。
  
        // 连续竞价期间，限价申报的申报有效价格范围应当在涨跌幅限制以内，
        // 且买入申报价格不高于买入基准价格的105%或不高于买入基准价格十个最小价格变动单位（以孰高为准），
        // 卖出申报价格不低于卖出基准价格的95%或不低于卖出基准价格十个最小价格变动单位（以孰低为准）。

        // 获取行情信息
        map<string, string> mapInput = mapReq;
        map<string, string> mapInstInfo;
        map<string, string> mapHqInfo;
        if (!GetInstCodeInfo(mapInput, mapInstInfo, mapHqInfo))
        {
            return FALSE;
        }

        // 计算限价价格
        // 卖五
        double fMinSalePrice = atof(mapHqInfo["SELL_FIVE_PRICE"].c_str());
        // 买一
        double fMaxBuyPrice = atof(mapHqInfo["BUY_ONE_PRICE"].c_str());
        double fCurrentMatchedPrice = atof(mapHqInfo["CURRENT_PRICE"].c_str());
        double fLastDayFinishedPrice = atof(mapHqInfo["CLOSING_PRICE"].c_str());
        double fMinMatchedPrice = atof(mapHqInfo["MIN_PRICE"].c_str());
        double fMaxMatchedPrice = atof(mapHqInfo["MAX_PRICE"].c_str());
        
        // 限买基价
        // 限价买入基准价格：最低卖出价格或最高买入价格或最近成交价或前收盘价
        double fBuyABasePrice = fLastDayFinishedPrice;
        if (fMinSalePrice > 0)
        {
            fBuyABasePrice = fMinSalePrice;
        }
        else if (fMaxBuyPrice > 0)
        {
            fBuyABasePrice = fMaxBuyPrice;
        }
        else if (fCurrentMatchedPrice > 0)
        {
            fBuyABasePrice = fCurrentMatchedPrice;
        }

        // 限卖基价
        // 限价卖出基准价格：最高买入价格或最低卖出价格或最近成交价或前收盘价
        double fSaleABasePrice = fLastDayFinishedPrice;
        if (fMaxBuyPrice > 0)
        {
            fSaleABasePrice = fMaxBuyPrice;
        }
        else if (fMinSalePrice > 0)
        {
            fSaleABasePrice = fMinSalePrice;
        }
        else if (fCurrentMatchedPrice > 0)
        {
            fSaleABasePrice = fCurrentMatchedPrice;
        }

        // 涨跌停
        double fMinPrice = atof(mapInstInfo["PRICE_FLOOR"].c_str());
        double fMaxPrice = atof(mapInstInfo["PRICE_CEILING"].c_str());
        double fTrdPriceUnit = atof(mapInstInfo["SPREAD_CODE"].c_str());

        // 买价格区间
        // 且买入申报价格不高于买入基准价格的105%或不高于买入基准价格十个最小价格变动单位（以孰高为准），
        double fBuyAMinPrice = fMinPrice;
        double fBuyAMaxPrice = fMaxPrice;
        double fExtMaxPrice = max(fBuyABasePrice + fTrdPriceUnit * 10, fBuyABasePrice*1.05);
        if (fMaxPrice > fExtMaxPrice)
        {
            fBuyAMaxPrice = fExtMaxPrice;
        }

        // 卖价格区间
        // 卖出申报价格不低于卖出基准价格的95%或不低于卖出基准价格十个最小价格变动单位（以孰低为准）。
        double fSaleAMinPrice = fMinPrice;
        double fSaleAMaxPrice = fMaxPrice;
        double fExtMinPrice = min(fSaleABasePrice - fTrdPriceUnit * 10, fSaleABasePrice*0.95);
        if (fMinPrice < fExtMinPrice)
        {
            fSaleAMinPrice = fExtMinPrice;
        }

        // 大宗交易价格
        // 成交价格不高于前收盘价的130%或当日最高成交价中的较高者，
        // 且不低于前收盘价的70%或当日最低成交价中的较低者
        double fTrdGMinPrice = fMinMatchedPrice;
        double fTrdGMaxPrice = fMaxMatchedPrice;
        double fCurMinPrice = fLastDayFinishedPrice * 0.7;
        double fCurMaxPrice = fLastDayFinishedPrice * 1.3;
        if (fCurMinPrice > 0 && fCurMinPrice < fTrdGMinPrice)
        {
            fTrdGMinPrice = fCurMinPrice;
        }
        if (fCurMaxPrice > fTrdGMaxPrice)
        {
            fTrdGMaxPrice = fCurMaxPrice;
        }

        // 额外信息
        mapHqInfo["01-限买基价"] = CMS::Formate("%.3lf", fBuyABasePrice);
        mapHqInfo["02-限卖基价"] = CMS::Formate("%.3lf", fSaleABasePrice);
        mapHqInfo["03-限买价区"] = CMS::Formate("(%.3lf-%.3lf)", fBuyAMinPrice, fBuyAMaxPrice);
        mapHqInfo["04-限卖价区"] = CMS::Formate("(%.3lf-%.3lf)", fSaleAMinPrice, fSaleAMaxPrice);
        mapHqInfo["05-大宗价区"] = CMS::Formate("(%.3lf-%.3lf)", fTrdGMinPrice, fTrdGMaxPrice);

        // 组装行情信息
        strCallInfo = strFuncName + "\r\n" + CMS::PrettyMap(mapHqInfo);
    }
    // 显示输出信息
    if (!strCallInfo.empty())
    {
        AddOut(strCallInfo);
    }
    return TRUE;
}

unsigned int __stdcall StopTaskProc(void* pPara)
{
    static int nRun = 0;
    if (nRun <= 0)
    {
        nRun++;
        CKCConnCliDlg* pThis = (CKCConnCliDlg*)pPara;
        g_bRunningTask = false;
        pThis->AddOut(("正在停止请求任务线程"));
        while (true)
        {
            if (CMS::GetWndText(pThis->m_vSwitchBtn) == "发送请求")
            {
                break;
            }
            Sleep(500);
        }
        nRun = 0;
    }
    return 0;
}


void CKCConnCliDlg::OnBnClickedBtnSendCmd()
{
    if (g_bRunningTask)
    {
        // 关闭运行任务
        _beginthreadex(NULL, 0, StopTaskProc, this, 0, NULL);
        return;
    }
    // 正在停止
    if (CMS::GetWndText(m_vSwitchBtn) == "停止请求")
    {
        AddOut(_T("正在停止任务，请勿重复操作"));
        return;
    }

    // 数据库任务
    STRunBatchTaskProcPara* pPara;
    int nCur = m_vTask.GetCurSel();
    if (nCur >= 0)
    {
        STDbRunTaskInfo& oTask = m_oDbTask;
        pPara = new STRunBatchTaskProcPara();
        pPara->pThis = this;
        pPara->nRunTimes = oTask.m_nRunTimes;
        pPara->nMode = MODE_ORA_TASK;
    }
    else
    {
        // 获取命令
        string strCMDText = CMS::GetWndText(m_vCmdList);

        // 尝试命令模式执行，执行成功则返回
        if (CallWithNoneCmd(strCMDText))
        {
            return;
        }
   
        // 命令模式
        pPara = new STRunBatchTaskProcPara();
        pPara->pThis = this;
        pPara->vecTasks.push_back(strCMDText);
        pPara->nRunTimes = 1;
        pPara->nMode = MODE_FILE_CMD;
    }

    // 启动线程处理函数
    HANDLE bThr = (HANDLE)_beginthreadex(NULL, 0, RunBatchTaskProc, pPara, 0, NULL);
}

void CKCConnCliDlg::OnBnClickedBtnLoadClear()
{
    //m_vCmdList.ResetContent();
    m_vOutput.SetWindowText(_T(""));
}


void CKCConnCliDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    CRect rtCtrl;
    CRect rtMain;

    if (this->IsWindowVisible())
    {
        // 主窗口大小
        GetClientRect(&rtMain);

        m_vCmdList.GetClientRect(&rtCtrl);
        ScreenToClient(&rtCtrl);

        DWORD dwEdgeSize = 20;

        // 命令输入框
        m_vCmdList.GetWindowRect(&rtCtrl);
        ScreenToClient(&rtCtrl);
        rtCtrl.right = rtMain.right - dwEdgeSize;
        m_vCmdList.MoveWindow(rtCtrl);

        // 输出框
        m_vOutput.GetWindowRect(&rtCtrl);
        ScreenToClient(&rtCtrl);
        rtCtrl.right = rtMain.right - dwEdgeSize;
        rtCtrl.bottom = rtMain.bottom - dwEdgeSize;
        m_vOutput.MoveWindow(rtCtrl);
    }
}


void CKCConnCliDlg::OnBnClickedBtnSaveCmd()
{
    string strCmd = CMS::Trim(CMS::GetWndText(m_vCmdList));
    if (strCmd.length() <= 0)
    {
        return;
    }
    
    CString strMsg = _T("是否保存当前命令\r\n[") + CString(strCmd.c_str()) + _T("]\r\n到本地命令文件中？");
    if (IDYES == MessageBox(strMsg, _T("提示"), MB_ICONQUESTION | MB_YESNO))
    {
        FILE* fp = fopen("cmd.txt", "a+");
        if (fp != NULL)
        {
            strCmd = "\r\n" + strCmd + "\r\n";
            fwrite(strCmd.c_str(), strCmd.length(), 1, fp);
            fclose(fp);
            AddOut(_T("保存命令到文件成功"));
        }
        else
        {
            AddOut(_T("打开命令文件失败，请重试"));
        }
    }

}


void CKCConnCliDlg::OnBnClickedBtnEditCmd()
{
    // 获取命令
    string strCMD = CMS::Trim(CMS::GetWndText(m_vCmdList));
    if (strCMD.length() <= 0)
    {
        return;
    }
    map<string, string> mapData;
    string strName;
    string strLBM;

    // 解析命令
    if (!ParseCmd(strCMD, strName, strLBM, mapData))
    {
        return;
    }

    CEditList oDlg;
    oDlg.m_strTitle = _T("属性") + CString(strName.c_str()) + _T("-") + CString(strLBM.c_str()) + _T("详情页");
    oDlg.SetData(mapData);
    int nRet = oDlg.DoModal();
    if (IDOK == nRet)
    {
        mapData.clear();
        mapData = oDlg.m_mapData;

        strCMD = "";
        if (!MakeCmd(strName, strLBM, mapData, strCMD))
        {
            AddOut(_T("返回命令有误,请重试"));
            return;
        }
        else
        {
            m_vCmdList.SetWindowText(CString(strCMD.c_str()));
        }
    }
}


void CKCConnCliDlg::OnClickedBtnEditRunPara()
{
    // 获取命令
    string strCMD = CMS::Trim(CMS::GetWndText(m_vCmdList));
    if (strCMD.length() <= 0)
    {
        return;
    }
    map<string, string> mapData;
    string strName;
    string strKey; 
    string strVal;
    string strPara;

    // 解析运行参数
    if (!CMS::GetKV(strCMD, strName, strPara, "|"))
    {
        return;
    }
    vector<string> vecItem;
    CMS::Split(strName, ",", vecItem, true);
    mapData.clear();
    GetDefaultRunPara(mapData);
    
    for (size_t i = 0; i < vecItem.size(); i++)
    {
        if (!CMS::GetKV(vecItem[i], strKey, strVal, ":"))
        {
            strKey = "name";
            strVal = vecItem[i];
        }
        mapData[strKey] = strVal;
    }

    CEditList oDlg;
    oDlg.m_strTitle = _T("运行参数配置编辑配置");
    oDlg.SetData(mapData);
    int nRet = oDlg.DoModal();
    if (IDOK == nRet)
    {
        mapData.clear();
        mapData = oDlg.m_mapData;

        strName = "";
        strKey = "";
        for (map<string, string>::iterator iteMap = mapData.begin(); iteMap != mapData.end(); ++iteMap)
        {
            if (iteMap->second.empty())
            {
                continue;
            }
            strName += strKey + iteMap->first + ":" + iteMap->second;
            strKey = ",";
        }
        strCMD = strName + "|" + strPara;
        m_vCmdList.SetWindowText(CString(strCMD.c_str()));
    }
}
