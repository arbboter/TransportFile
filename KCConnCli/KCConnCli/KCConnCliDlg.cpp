
// KCConnCliDlg.cpp : ʵ���ļ�
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
CCMSKCBPCli g_oCli;
bool g_bRunningTask = false;

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CKCConnCliDlg �Ի���




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


// CKCConnCliDlg ��Ϣ�������

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
    // ��Ĭ��ֵ
    vecCmd = vecTask;

    // ������������
    map<string, string> mapPara;
    string strPara = m_oDbTask.m_vecPara[nCmd];
    CMS::GetKV(strPara, ",", ":", mapPara);
    if (mapPara["type"] != "secu_auto_ord_cfg")
    {
        return TRUE;
    }

    AddOut("���������Զ�ί������...");
    // ֧�ּ��ϵ��б�trd_id��inst_code��quote_type
    int nTrdId = -1;
    int nCodeId = -1;
    int nQuoteId = -1;
    int nPriceId = -1;
    int nSucc = 0;
    vector<string>& vecHeader = m_oDbTask.m_vecHeader[nCmd];
    for (size_t i = 0; i < vecHeader.size(); i++)
    {
        // �ɹ�������Ԥ��
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
            // ����Ԥ��
            nSucc -= 1;
        }
    }
    if (nSucc != 4)
    {
        LOGE("�Զ��µ�������������ֶβ�ƥ��");
        return FALSE;
    }

    // ��������ָ��
    // �������������ֹ�����ͬһ������
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
        // �������
        strBuf = i->at(nTrdId);
        vecTrdId.clear();
        CMS::Split(strBuf, ",", vecTrdId, true);

        // ��Ʒ����
        strBuf = i->at(nCodeId);
        vecInstCode.clear();
        CMS::Split(strBuf, ",", vecInstCode, true);

        // ��������
        strBuf = i->at(nQuoteId);
        vecQuoteType.clear();
        CMS::Split(strBuf, ",", vecQuoteType, true);

        // �������
        if (vecTrdId.empty() || vecInstCode.empty() || vecQuoteType.empty())
        {
            AddOut("�����¼�������󣬱������ѱ�����");
            continue;
        }

        // ��ѯ������Ϣ����ȡ�ǵ�ͣ�۸����Ϣ
        mapHqInfo.clear();
        for (size_t j = 0; j < vecInstCode.size(); j++)
        {
            map<string, string> mapCurHq;
            // ��������ü۸�ʹ�������ü۸�
            vecItem = *i;
            if (atof(vecItem[nPriceId].c_str()) > 0)
            {
                mapCurHq["BUY_PRICE"] = vecItem[nPriceId];
                mapCurHq["SALE_PRICE"] = vecItem[nPriceId];
                mapHqInfo[vecInstCode[j]] = mapCurHq;
                continue;
            }

            // ��ȡ֤ȯ�۸���Ϣ
            if (!GetInstCodeInfo(vecInstCode[j], mapCurHq))
            {
                // �����ȡʧ�ܣ�֤ȯ�����ÿգ�������
                LOGW("֤ȯ����[%s]�ѱ��Ƴ�����Ϣ��ȡʧ��", vecInstCode[j].c_str());
                vecInstCode[j] = "";
                continue;
            }

            // ���ݲ��Ի�ȡ��ͬ�۸���Ϊί�м۸�
            string strUpperPrice = CMS::Upper(mapPara["ord_price"]);
            if (mapCurHq.find(strUpperPrice) != mapCurHq.end())
            {
                mapCurHq["BUY_PRICE"] = mapCurHq[strUpperPrice];
                mapCurHq["SALE_PRICE"] = mapCurHq[strUpperPrice];
            }
            // ��ȡ�ɽ�
            else if (strUpperPrice == "WANT_MATCH")
            {
                mapCurHq["BUY_PRICE"] = mapCurHq["PRICE_CEILING"];
                mapCurHq["SALE_PRICE"] = mapCurHq["PRICE_FLOOR"];
            }
            // Ĭ������۸�
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

        // �ֲ𵥸�����ָ��
        for (size_t j = 0; j < vecInstCode.size(); j++)
        {
            // �յ�֤ȯ��������
            if (vecInstCode[j].empty())
            {
                continue;
            }

            // �������
            map<string, string>& mapCurHq = mapHqInfo[vecInstCode[j]];
            for (auto trd_id : vecTrdId)
            {
                // ��������
                for (auto quote_type : vecQuoteType)
                {
                    // ����ԭ���񣬶Լ����ֶ��滻
                    vecItem = *i;
                    vecItem[nTrdId] = trd_id;
                    vecItem[nQuoteId] = quote_type;
                    vecItem[nCodeId] = vecInstCode[j];
                    vecItem[nPriceId] = (trd_id == "20B") ? mapCurHq["BUY_PRICE"] : mapCurHq["SALE_PRICE"];
                    // �۸�����
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

    // ����������
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
        AddOut("���ݿ�����ʧ��");
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
            AddOut("��ѯSQLʧ��:" + strSql);
            continue;
        }

        // ������Զ��µ������һ�����⴦����ѯ����ȷ���ǵ�ͣ�۸����ɵ�������ָ��
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
            AddOut("�������:" + *(vecData.end() - 1));
        }
        // ����һ��ָ����˳�
        break;
    }
    AddOut(CMS::Formate("����������ɣ�������:%d ���д���:%d", (int)vecData.size(), oTask.m_nRunTimes));
    return true;
}

BOOL CKCConnCliDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

    // ���ط�������������
    LoadServerConfig();
    OnBnClickedBtnLoadCmd();

    // ��������ı������������ַ���С
    m_vOutput.SetLimitText(-1);

    // ��ʼ�������
    srand((unsigned int)time(NULL));

    // ��ʼ����������ʱ��
    string strBuf;
    TCHAR szBuf[256] = { 0 };
    DWORD nCurTime = atoi(CMS::GetDate("%H%M%S").c_str());
    strBuf = CMS::GetDate("%Y%m%d");
    CString strBegTime((strBuf + "000001").c_str());
    CString strEndTime((strBuf + "235959").c_str());
    
    m_vBegTime.SetWindowText(strBegTime);
    m_vEndTime.SetWindowText(strEndTime);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CKCConnCliDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CKCConnCliDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CKCConnCliDlg::LoadServerConfig()
{
    BOOL bRet = FALSE;

    // ���������ļ�
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

    // �û���
    strKeyName = _T("UserName");
    GetPrivateProfileString(strSecName, strKeyName, _T("KCXP00"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vUid.SetWindowText(szValue);

    // �û�����
    strKeyName = _T("Password");
    GetPrivateProfileString(strSecName, strKeyName, _T("888888"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vPwd.SetWindowText(szValue);

    // ������
    strKeyName = _T("ServerName");
    GetPrivateProfileString(strSecName, strKeyName, _T("KCXP1"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vServerName.SetWindowText(szValue);

    // ������ַ
    strKeyName = _T("Address");
    GetPrivateProfileString(strSecName, strKeyName, _T("10.254.253.46"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vHost.SetWindowText(szValue);

    // �����˿�
    strKeyName = _T("Port");
    GetPrivateProfileString(strSecName, strKeyName, _T("21000"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vPort.SetWindowText(szValue);

    // ���Ͷ���
    strKeyName = _T("SendQName");
    GetPrivateProfileString(strSecName, strKeyName, _T("req_otc_zb"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vReqQName.SetWindowText(szValue);

    // ���ն���
    strKeyName = _T("ReceiveQName");
    GetPrivateProfileString(strSecName, strKeyName, _T("ans_otc_zb"), szValue, sizeof(szValue)-1, strConfigFile);
    m_vAnsQName.SetWindowText(szValue);

    // ��ȡ��������
    string strPubName = "PUBLIC_PARA";
    string strPath = CMS::CSW2Cpps(strConfigFile);

    CMS::ReadIniSectionInfo(strPubName, strPath, m_mapKcbpPublicPara);

    // ��ȡ���ݿ�����
    map<string, string> mapTask;
    CMS::ReadIniSectionInfo("DB_TASK", strPath, mapTask);
    if (mapTask.size() > 3)
    {
        string k, v;
        STDbRunTaskInfo& oTask = m_oDbTask;
        oTask.m_strDbUid = mapTask["UserName"];
        oTask.m_strDbPwd = mapTask["Password"];
        oTask.m_strDbConn = mapTask["ConnInfo"];

        // ���д���
        oTask.m_nRunTimes = 1;
        k = "RunTimes";
        if (CMS::Get(mapTask, k, v))
        {
            oTask.m_nRunTimes = atoi(v.c_str());
        }
        
        // �������в���
        string strPara;
        k = "TaskRunPara";
        CMS::Get(mapTask, k, strPara);
        map<string, string> mapCommPara;
        CMS::GetKV(strPara, ",", ":", mapCommPara);

        for (int i = 1; i < 256; i++)
        {
            // sql���
            k = CMS::Formate("Task%dSql", i);
            if (!CMS::Get(mapTask, k, v))
            {
                break;
            }
            // ������select�ֶ���
            vector<string> vecHeader;
            if (!Sqlapi::GetSqlFields(v, vecHeader))
            {
                LOGE("����[%s]��ѯ�ֶηǷ�", v.c_str());
                break;
            }
            oTask.m_vecSql.push_back(v);
            oTask.m_vecHeader.push_back(vecHeader);

            // ����
            k = CMS::Formate("Task%dName", i);
            v = "";
            CMS::Get(mapTask, k, v);
            if (CMS::Trim(v).empty())
            {
                v = CMS::Formate("��������%d", i);
            }
            oTask.m_vecName.push_back(v);
            m_vTask.AddString(CString(CMS::Formate("%02d-%s", i, v.c_str()).c_str()));

            // ���в���
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
    // ��ȡ���õ�ʱ��
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

    // ����KCXP
    KCXP_Connx((KCXP_CHAR *)strHost.c_str(), 
                atoi(strPort.c_str()), 
                (KCXP_CHAR*)strUser.c_str(), 
                (KCXP_CHAR*)(strPass.c_str()), 
                &hKcxpConn, &lCode, &lReason);
    if (lCode != KCXP_CC_OK)
    {
        LOGE("����KCXPʧ��,Code[%ld] Reason[%ld]. Para[%s,%s,%s,%s]", lCode, lReason,
            strHost.c_str(), strPort.c_str(), strUser.c_str(), strPass.c_str());
        return false;
    }
    LOGD("����KCXP�ɹ�. Para[%s,%s,%s,%s]", strHost.c_str(), strPort.c_str(), strUser.c_str(), strPass.c_str());

    // �򿪶���
    KCXP_HOBJ hHOBJ = NULL;
    KCXP_OD   oDesc = { KCXP_DEFAULT_OD };
    KCXP_LONG lOptions = 0;

    oDesc.iObjectType = KCXP_OT_Q;
    strncpy_s(oDesc.strObjectName, strQReq.c_str(), sizeof(oDesc.strObjectName) - 1);
    lOptions = KCXP_OO_AS_Q_DEF | KCXP_OO_OUTPUT | KCXP_OO_INQUIRE;
    KCXP_Open(hKcxpConn, &oDesc, lOptions, &hHOBJ, &lCode, &lReason);
    if (lCode != KCXP_CC_OK)
    {
        LOGE("��KCXP����ʧ��,Code[%ld] Reason[%ld]. Para[%s]", lCode, lReason, strQReq.c_str());
        KCXP_Disconn(&hKcxpConn, 0L, &lCode, &lReason);
        return false;
    }
    LOGD("��KCXP���гɹ�, Para[%s]", strQReq.c_str());

    return bRet;
}

BOOL CKCConnCliDlg::GetInstCodeInfo(const string& strInstCode, map<string, string>& mapInfo)
{
    // ��������
    map<string, string> mapInput = m_mapKcbpPublicPara;
    mapInput["F_OP_SITE"] = CMS::GetLocalHost();
    mapInput["F_RUNTIME"] = CMS::GetCurDate();

    // ֤ȯ��Ϣ
    mapInput["F_SUBSYS"] = "13";
    mapInput["INST_CODE"] = strInstCode;
    vector<map<string, string>> vecSecuAns;
    g_oCli.CallLbm("L2622102", mapInput, vecSecuAns);
    if (vecSecuAns.empty())
    {
        LOGE("��ȡ֤ȯ[%s]��Ϣʧ��", strInstCode.c_str());
        return FALSE;
    }
    map<string, string> mapSecuInfo = vecSecuAns[0];

    // ������Ϣ
    map<string, string> mapHqInfo;
    vector<map<string, string>> vecHqAns;
    mapInput["MKT_CODE"] = "1";
    g_oCli.CallLbm("L2622103", mapInput, vecHqAns);
    if (vecHqAns.empty())
    {
        LOGE("��ȡ֤ȯ[%s]������Ϣʧ��", strInstCode.c_str());
    }
    else
    {
        mapHqInfo = vecHqAns[0];
    }

    // �����Ϣ
    mapInfo["INST_CODE"] = strInstCode;
    // ����ɽ���
    mapInfo["CURRENT_PRICE"] = mapHqInfo.empty() ? mapSecuInfo["PRICE_CEILING"] : mapHqInfo["CURRENT_PRICE"];
    // ��߳ɽ���
    mapInfo["MAX_PRICE"] = mapHqInfo.empty() ? mapSecuInfo["PRICE_CEILING"] : mapHqInfo["MAX_PRICE"];
    // ��ͳɽ���
    mapInfo["MIN_PRICE"] = mapHqInfo.empty() ? mapSecuInfo["PRICE_FLOOR"] : mapHqInfo["MIN_PRICE"];
    // ��ͣ�۸�
    mapInfo["PRICE_CEILING"] = mapSecuInfo["PRICE_CEILING"];
    // ��ͣ�۸�
    mapInfo["PRICE_FLOOR"] = mapSecuInfo["PRICE_FLOOR"];

    return TRUE;
}

BOOL CKCConnCliDlg::GetInstCodeInfo(map<string, string>& mapReq, map<string, string>& mapInstInfo, map<string, string>& mapHqInfo)
{
    // ֤ȯ��Ϣ
    const string strInstCode = mapReq["INST_CODE"];
    vector<map<string, string>> vecSecuAns;
    g_oCli.CallLbm("L2622102", mapReq, vecSecuAns);
    if (vecSecuAns.empty())
    {
        LOGE("��ȡ֤ȯ[%s]��Ϣʧ��", strInstCode.c_str());
        return FALSE;
    }
    mapInstInfo = vecSecuAns[0];

    // ������Ϣ
    vector<map<string, string>> vecHqAns;
    g_oCli.CallLbm("L2622103", mapReq, vecHqAns);
    if (vecHqAns.empty())
    {
        LOGE("��ȡ֤ȯ[%s]������Ϣʧ��", strInstCode.c_str());
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
        LOGE("LBM������Ϊ��");
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

    // ���������ֶ�
    vector<string> vecPara;
    CMS::Split(strPara, ",", vecPara, true);
    
    // һ������Ĭ��Ϊ����
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
            AddOut(_T("�ֶ���Ϣ[") + CString(vecPara[i].c_str()) + _T("]��ʽ����������"));
            return bRet;
        }
        mapKV[vecPair[0]] = vecPair[1];
    }

    return bRet;
}


BOOL CKCConnCliDlg::CallKxcpLbm(const string& strLbmNo, const map<string, string>& mapReq, vector<map<string, string>>& vecAns, bool bRst/* = true*/)
{
    BOOL bRet = TRUE;

    // ��������
    CString strCallInfo;
    string strErrMsg;

    DWORD dwBeg = GetTickCount();
    int nRet = g_oCli.CallLbm(strLbmNo, mapReq, vecAns);
    DWORD dwEnd = GetTickCount();
    LOGI("Ӧ��:");
    strCallInfo.Format(_T("�ӿں�ʱ:%lums,���ؽ��:\r\n"), dwEnd - dwBeg);
    if (nRet != 0)
    {
        g_oCli.GetLastError(nRet, strErrMsg);
        strCallInfo += _T("�ӿڵ���ʧ��,");
        strCallInfo += CString(("���ش�����:" + CMS::Itoa(nRet)).c_str());
        strCallInfo += CString((", ���ش�����Ϣ:" + strErrMsg).c_str());
        bRet = FALSE;
    }
    else if (vecAns.size() <= 0)
    {
        strCallInfo += _T("���ؽ��Ϊ��");
    }
    else if (bRst)
    {
        // ��ȡ����
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

        // ���������ؽ��
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
                    LOGW("δ����һ���ؽ�������ҵ��������ؽ���ֶΣ����ؽ������ʽ���������");
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

        // ����������ʽ������ֱ�����
        if (vecTitle.size() <= 0)
        {
            // ��ʾ����ֵ
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
        strCallInfo += ("���óɹ������ؼ�¼��:" + CMS::CastFrom<size_t, string>(vecAns.size())).c_str();
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

    // ����ģʽ
    if (!ParseCmd(strCmd, strName, strLBM, mapReq))
    {
        return FALSE;
    }

    // ��ȡ������Ʋ���
    map<string, string> mapCmdPara;
    GetCmdPara(strName, mapCmdPara);

    // �������
    map<string, string> mapInput;
    map<string, string>::iterator iteMapStr;

    mapInput = m_mapKcbpPublicPara;
    mapInput["F_OP_SITE"] = CMS::GetLocalHost();
    mapInput["F_RUNTIME"] = CMS::GetCurDate();
    for (iteMapStr = mapReq.begin(); iteMapStr != mapReq.end(); iteMapStr++)
    {
        mapInput[iteMapStr->first] = iteMapStr->second;
    }
    
    // ������Ʋ�������
    CString strCallName;
    CString strMsg;
    string strKey = "name";
    string strBuf;
    vector<string> vecData;
    if (CMS::Get(mapCmdPara, strKey, strName))
    {
        strCallName = ("����:" + strName + ",").c_str();
    }

    // ����������Ƶ�κ�Ƶ��
    // ����@���(ms)
    int nCallNums = 1;
    int nCallInter = 500; // Ĭ��500ms
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

    // ���ʱ����
    int nRandomTime = 0; // Ĭ��0ms
    strKey = "rand_stay";
    strBuf = "";
    CMS::Get(mapCmdPara, strKey, strBuf);
    if (strBuf.size() > 0)
    {
        nRandomTime = CMS::CastFrom<string, int>(strBuf);
    }

    // �����Ƿ�ֱ�ӷ���
    int nFailRet = 1; // Ĭ����
    strKey = "fail_ret";
    strBuf = "";
    CMS::Get(mapCmdPara, strKey, strBuf);
    if (strBuf.size() > 0)
    {
        nFailRet = CMS::CastFrom<string, int>(strBuf);
    }
    // ��ӡ������Ϣ
    strMsg.Format(_T("�����ܴ���:%d, ʱ����:%d+rand(%d) ms"), nCallNums, nCallInter, nRandomTime);
    AddOut(strMsg);

    // ʱ�����
    string strCurTime = CMS::GetDate("%H%M%S");
    string strBegTime = "000000";
    string strEndTime = "999999";
    strKey = "beg_time";
    CMS::Get(mapCmdPara, strKey, strBegTime);
    strKey = "end_time";
    CMS::Get(mapCmdPara, strKey, strEndTime);
    if ((strCurTime < strBegTime) || (strCurTime > strEndTime))
    {
        strBuf = CMS::Formate("����ʱ��δ��:%s-%s", strBegTime.c_str(), strEndTime.c_str());
        AddOut(strBuf);
        Sleep(nRandomTime);
        return TRUE;
    }

    // �Ƿ��Զ�����
    int nCancelLevel = -1;
    strKey = "ord_cancel_level";
    strBuf = "";
    if (CMS::Get(mapCmdPara, strKey, strBuf))
    {
        nCancelLevel = atoi(strBuf.c_str());
    }

    // ����������ַ���,���Դ�ӡ
    CString strCallInfo = strCallName + ("������Ϣ:" + strLBM + "!").c_str();
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

    // �鿴�Ƿ�Ϊ�Զ��庯��
    if (strLBM.substr(0, 5) == "5405_")
    {
        return CallMyFunc(strLBM, mapInput);
    }

    // ���ú���
    int nAgreeId = atoi(CMS::GetDate("%M%S").c_str())*100;
    int nHaveCalls = 0;
    DWORD nDelay = 0;
    vector<map<string, string>> vecAns;
    for (; nCallNums > 0; --nCallNums)
    {
        // �����Э��ת�ã������Э���Ų���
        if (IsNQAgreeOrder(strLBM))
        {
            mapInput["AGREEMENT_ID"] = CMS::Formate("%d", ++nAgreeId);
        }

        // ǰ��10��������ʾ������飬�м��ֻ��ʾ���ý��
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
        AddOut(CMS::Formate("��ʱ�趨:%.2f", nDelay/1000.0f));
        Sleep(nDelay);
        nHaveCalls++;

        // ֧���Զ�����
        if (nCancelLevel>0 && (rand()%10 < nCancelLevel) && !vecAns.empty())
        {
            mapInput["ORI_APP_SNO"] = vecAns[0]["APP_SNO"];
            mapInput["ORI_APP_DATE"] = vecAns[0]["APP_DATE"];
            vecAns.clear();
            if (CallKxcpLbm("L2622203", mapInput, vecAns))
            {
                AddOut("�����ɹ�");
            }
            else
            {
                AddOut("����ʧ��");
            }
        }

        // �����Э��ת�ã��Զ����ɶ��ַ�����
        else if (!vecAns.empty() && IsNQAgreeOrder(strLBM))
        {
            swap(mapInput["TARGET_SECU_ACC"], mapInput["SECU_ACC"]);
            swap(mapInput["TARGET_CUST_CODE"], mapInput["CUST_CODE"]);
            swap(mapInput["TARGET_SEAT"], mapInput["TRD_SEAT"]);
            mapInput["TRD_ID"] = (mapInput["TRD_ID"] == "20B" ? "20S" : "20B");
            vecAns.clear();
            CallKxcpLbm(strLBM, mapInput, vecAns);
        }

        // ����ʧ��������ֱ���˳�
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

    // ��������
    // (��ע|)LBM��!PN1:VA1,PN2:VA2,...

    // ������Ϣ�������
    vector<string> vecLBM;
    CMS::Split(strCmd, "!", vecLBM, true);
    if (vecLBM.size() != 2)
    {
        return bRet;
    }

    // ����ͷ-��ȡ��LBM�ź�����
    vector<string> vecHead;;
    CMS::Split("|" + vecLBM[0], "|", vecHead, true);
    if (vecLBM.size() < 2)
    {
        AddOut(_T("��ǰ����������ʽͷ���������ԣ�") + CString(strCmd.c_str()));
        return bRet;
    }
    size_t nIdx = vecHead.size() - 2;
    strName = vecHead[nIdx];
    strLBM = vecHead[nIdx + 1];

    // ���������ֶ�
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
    // ���������
    if (strCmd[0] != '_')
    {
        return FALSE;
    }

    string strName, strVal;
    CMS::GetKV(strCmd, strName, strVal);

    // ���д���
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

    if (strText == _T("�Ͽ�������"))
    {
        if (g_oCli.Uninit())
        {
            AddOut(_T("�Ͽ�KCBP�������ɹ�"));
            m_vConnectServerBtn.SetWindowText(_T("���ӷ�����"));
        }
        else
        {
            AddOut(_T("�Ͽ�KCBP������ʧ��"));
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
        AddOut(_T("KCBP���ӳ�ʼ��ʧ��"));
        return;
    }
    AddOut(_T("KCBP���ӳ�ʼ���ɹ�"));
    m_vConnectServerBtn.SetWindowText(_T("�Ͽ�������"));
    return;

    map<string, string> mapData;
    vector< map<string, string> > vecAns;
    g_oCli.SetKCBPPublicInput(mapData);

    mapData["CUST_CODE"] = "88000055";
    mapData["PAGE_RECCNT"] = "100";
    if (g_oCli.CallLbm("L2622126", mapData, vecAns))
    {
        AddOut(_T("KCBP�ӿڵ���ʧ��"));
    }
    else
    {
        // ��ʾ����ֵ
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
        AddOut(_T("�������ļ�cmd.txtʧ��"));
        return;
    }
    vector<string> vecCmds;

    // ��ȡ����
    char szLineMax[512] = { 0 };
    while (!feof(fp))
    {
        memset(szLineMax, 0, sizeof(szLineMax));
        fgets(szLineMax, sizeof(szLineMax)-1, fp);
        vecCmds.push_back(CMS::Trim(szLineMax));
    }
    fclose(fp);

    m_vCmdList.ResetContent();
    // ���ص��б��
    for (size_t i = 0; i < vecCmds.size(); i++)
    {
        m_vCmdList.AddString(CString(vecCmds[i].c_str()));
    }
    AddOut(_T("���������ļ����"));
}

BOOL CKCConnCliDlg::CallWithNoneCmd(const string& strCall)
{
    BOOL bRet = TRUE;

    // �Ƿ�Ϊ�ļ�
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

    // ѭ����������
    g_bRunningTask = true;
    pThis->m_vSwitchBtn.SetWindowText(_T("ֹͣ����"));
    vector<string>::iterator iteTask;
    int nStat = 0;
    while (g_bRunningTask && !vecTask.empty())
    {
        // �鿴�Ƿ���������
        nStat = pThis->TimeRunStat();
        if (nStat < 0)
        {
            pThis->AddOut(_T("δ��ϵͳ�趨����ʱ�䣬������ȴ�"));
            Sleep(10000);
            continue;
        }
        else if (nStat > 0)
        {
            pThis->AddOut(_T("ϵͳ�趨����ʱ���ѹ��������˳�"));
            break;
        }

        // ��������
        if (p->nRunTimes-- == 0)
        {
            break;
        }

        // ���б�������
        int nSucc = 0;
        int nFail = 0;
        int nCurSeq = 0;
        int nTotalNums = vecTask.size();
        for (iteTask = vecTask.begin(); iteTask != vecTask.end(); )
        {
            // ��ʾ�������
            pThis->AddOut(CMS::Formate("���ڴ�������%d/%d", ++nCurSeq, nTotalNums));

            // ���ʧ�ܣ���������б���ɾ��������
            if (!pThis->CallKxcpByCmd(*iteTask))
            {
                iteTask = vecTask.erase(iteTask);
                pThis->AddOut("���ó���ɾ��������");
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

        // ��ʾִ��ͳ�����
        pThis->AddOut(CMS::Formate("��������ִ�н���,�ɹ�:%d, ʧ��:%d", nSucc, nFail));
    }
    g_bRunningTask = false;
    pThis->m_vSwitchBtn.SetWindowText(_T("��������"));
    pThis->AddOut("�߳�����ִ�����");
    // �ڴ�����
    delete p;
    p = NULL;
    return 0;
}

BOOL CKCConnCliDlg::CallByFile(const string& strFile)
{
    BOOL bRet = FALSE;

    ifstream fLog;
    // ���ļ�
    fLog.open(strFile);
    if (!fLog.is_open())
    {
        return bRet;
    }

    // ��Ԥ��ȡ�ļ��������ļ�����ʱ��
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
        
        // ���Ʋ�������
        if (GetCmdPara(strTemp, pPara))
        {
            continue;
        }

        // �ӿ�����
        pPara->vecTasks.push_back(strTemp);
    }
    pPara->pThis = this;

    // �����������߳�
    HANDLE bThr = (HANDLE)_beginthreadex(NULL, 0, RunBatchTaskProc, pPara, 0, NULL);
    return bRet;
}

BOOL CKCConnCliDlg::CallMyFunc(const string& strLbmNo, const map<string, string>& mapReq)
{
    string strCallInfo = "";
    string strFuncName = strLbmNo.substr(5);
    if (strFuncName == "֤ȯ����")
    {
        // ���루��������׼�۸�Ϊ��ʱ��ʾ�����������������룩�걨�۸�
        // �޼�ʱ��ʾ�����������������룩�걨�۸�ģ�Ϊ��ʱ��ʾ��������루����������۸�
        // ˫�������걨�ģ�Ϊ����ɽ��ۣ�
        // �����޳ɽ��ģ�Ϊǰ���̼ۡ�
  
        // ���������ڼ䣬�޼��걨���걨��Ч�۸�ΧӦ�����ǵ����������ڣ�
        // �������걨�۸񲻸��������׼�۸��105%�򲻸��������׼�۸�ʮ����С�۸�䶯��λ�������Ϊ׼����
        // �����걨�۸񲻵���������׼�۸��95%�򲻵���������׼�۸�ʮ����С�۸�䶯��λ�������Ϊ׼����

        // ��ȡ������Ϣ
        map<string, string> mapInput = mapReq;
        map<string, string> mapInstInfo;
        map<string, string> mapHqInfo;
        if (!GetInstCodeInfo(mapInput, mapInstInfo, mapHqInfo))
        {
            return FALSE;
        }

        // �����޼ۼ۸�
        // ����
        double fMinSalePrice = atof(mapHqInfo["SELL_FIVE_PRICE"].c_str());
        // ��һ
        double fMaxBuyPrice = atof(mapHqInfo["BUY_ONE_PRICE"].c_str());
        double fCurrentMatchedPrice = atof(mapHqInfo["CURRENT_PRICE"].c_str());
        double fLastDayFinishedPrice = atof(mapHqInfo["CLOSING_PRICE"].c_str());
        double fMinMatchedPrice = atof(mapHqInfo["MIN_PRICE"].c_str());
        double fMaxMatchedPrice = atof(mapHqInfo["MAX_PRICE"].c_str());
        
        // �������
        // �޼������׼�۸���������۸���������۸������ɽ��ۻ�ǰ���̼�
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

        // ��������
        // �޼�������׼�۸��������۸����������۸������ɽ��ۻ�ǰ���̼�
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

        // �ǵ�ͣ
        double fMinPrice = atof(mapInstInfo["PRICE_FLOOR"].c_str());
        double fMaxPrice = atof(mapInstInfo["PRICE_CEILING"].c_str());
        double fTrdPriceUnit = atof(mapInstInfo["SPREAD_CODE"].c_str());

        // ��۸�����
        // �������걨�۸񲻸��������׼�۸��105%�򲻸��������׼�۸�ʮ����С�۸�䶯��λ�������Ϊ׼����
        double fBuyAMinPrice = fMinPrice;
        double fBuyAMaxPrice = fMaxPrice;
        double fExtMaxPrice = max(fBuyABasePrice + fTrdPriceUnit * 10, fBuyABasePrice*1.05);
        if (fMaxPrice > fExtMaxPrice)
        {
            fBuyAMaxPrice = fExtMaxPrice;
        }

        // ���۸�����
        // �����걨�۸񲻵���������׼�۸��95%�򲻵���������׼�۸�ʮ����С�۸�䶯��λ�������Ϊ׼����
        double fSaleAMinPrice = fMinPrice;
        double fSaleAMaxPrice = fMaxPrice;
        double fExtMinPrice = min(fSaleABasePrice - fTrdPriceUnit * 10, fSaleABasePrice*0.95);
        if (fMinPrice < fExtMinPrice)
        {
            fSaleAMinPrice = fExtMinPrice;
        }

        // ���ڽ��׼۸�
        // �ɽ��۸񲻸���ǰ���̼۵�130%������߳ɽ����еĽϸ��ߣ�
        // �Ҳ�����ǰ���̼۵�70%������ͳɽ����еĽϵ���
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

        // ������Ϣ
        mapHqInfo["01-�������"] = CMS::Formate("%.3lf", fBuyABasePrice);
        mapHqInfo["02-��������"] = CMS::Formate("%.3lf", fSaleABasePrice);
        mapHqInfo["03-�������"] = CMS::Formate("(%.3lf-%.3lf)", fBuyAMinPrice, fBuyAMaxPrice);
        mapHqInfo["04-��������"] = CMS::Formate("(%.3lf-%.3lf)", fSaleAMinPrice, fSaleAMaxPrice);
        mapHqInfo["05-���ڼ���"] = CMS::Formate("(%.3lf-%.3lf)", fTrdGMinPrice, fTrdGMaxPrice);

        // ��װ������Ϣ
        strCallInfo = strFuncName + "\r\n" + CMS::PrettyMap(mapHqInfo);
    }
    // ��ʾ�����Ϣ
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
        pThis->AddOut(("����ֹͣ���������߳�"));
        while (true)
        {
            if (CMS::GetWndText(pThis->m_vSwitchBtn) == "��������")
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
        // �ر���������
        _beginthreadex(NULL, 0, StopTaskProc, this, 0, NULL);
        return;
    }
    // ����ֹͣ
    if (CMS::GetWndText(m_vSwitchBtn) == "ֹͣ����")
    {
        AddOut(_T("����ֹͣ���������ظ�����"));
        return;
    }

    // ���ݿ�����
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
        // ��ȡ����
        string strCMDText = CMS::GetWndText(m_vCmdList);

        // ��������ģʽִ�У�ִ�гɹ��򷵻�
        if (CallWithNoneCmd(strCMDText))
        {
            return;
        }
   
        // ����ģʽ
        pPara = new STRunBatchTaskProcPara();
        pPara->pThis = this;
        pPara->vecTasks.push_back(strCMDText);
        pPara->nRunTimes = 1;
        pPara->nMode = MODE_FILE_CMD;
    }

    // �����̴߳�����
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
        // �����ڴ�С
        GetClientRect(&rtMain);

        m_vCmdList.GetClientRect(&rtCtrl);
        ScreenToClient(&rtCtrl);

        DWORD dwEdgeSize = 20;

        // ���������
        m_vCmdList.GetWindowRect(&rtCtrl);
        ScreenToClient(&rtCtrl);
        rtCtrl.right = rtMain.right - dwEdgeSize;
        m_vCmdList.MoveWindow(rtCtrl);

        // �����
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
    
    CString strMsg = _T("�Ƿ񱣴浱ǰ����\r\n[") + CString(strCmd.c_str()) + _T("]\r\n�����������ļ��У�");
    if (IDYES == MessageBox(strMsg, _T("��ʾ"), MB_ICONQUESTION | MB_YESNO))
    {
        FILE* fp = fopen("cmd.txt", "a+");
        if (fp != NULL)
        {
            strCmd = "\r\n" + strCmd + "\r\n";
            fwrite(strCmd.c_str(), strCmd.length(), 1, fp);
            fclose(fp);
            AddOut(_T("��������ļ��ɹ�"));
        }
        else
        {
            AddOut(_T("�������ļ�ʧ�ܣ�������"));
        }
    }

}


void CKCConnCliDlg::OnBnClickedBtnEditCmd()
{
    // ��ȡ����
    string strCMD = CMS::Trim(CMS::GetWndText(m_vCmdList));
    if (strCMD.length() <= 0)
    {
        return;
    }
    map<string, string> mapData;
    string strName;
    string strLBM;

    // ��������
    if (!ParseCmd(strCMD, strName, strLBM, mapData))
    {
        return;
    }

    CEditList oDlg;
    oDlg.m_strTitle = _T("����") + CString(strName.c_str()) + _T("-") + CString(strLBM.c_str()) + _T("����ҳ");
    oDlg.SetData(mapData);
    int nRet = oDlg.DoModal();
    if (IDOK == nRet)
    {
        mapData.clear();
        mapData = oDlg.m_mapData;

        strCMD = "";
        if (!MakeCmd(strName, strLBM, mapData, strCMD))
        {
            AddOut(_T("������������,������"));
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
    // ��ȡ����
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

    // �������в���
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
    oDlg.m_strTitle = _T("���в������ñ༭����");
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
