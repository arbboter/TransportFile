#include "stdafx.h"
#include "CMSKCBPCli.h"
#include "KCBPCli.h"
#include "Slog.h"
#include "Util.h"

CCMSKCBPCli::CCMSKCBPCli()
{
    m_hKCBPCli = NULL;
}


CCMSKCBPCli::~CCMSKCBPCli()
{
}

bool CCMSKCBPCli::Init()
{
    bool bRet = false;
    int& nRet = m_nLastError;
    char* szBuf = m_szErrMsg;

    memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
    // 支持重连
    Uninit();

    // 初始化
    nRet = KCBPCLI_Init(&m_hKCBPCli);
    if (nRet != 0)
    {
        KCBPCLI_GetErrorMsg(m_hKCBPCli, szBuf);
        LOGE("KCBPCLI 初始化失败, Ret[%d] ErrMsg[%s]", nRet, szBuf);
        m_hKCBPCli = NULL;
        return bRet;
    }
    LOGD("KCBPCLI 初始化成功");

    // 初始化信息
    tagKCBPConnectOption oKcbpOpt = { 0 };
    memset(&oKcbpOpt, 0, sizeof(oKcbpOpt));
    oKcbpOpt.nProtocal = 0;
    oKcbpOpt.nPort = atoi(m_strPort.c_str());
    strncpy_s(oKcbpOpt.szServerName, m_strServerName.c_str(), sizeof(oKcbpOpt.szServerName) - 1);
    strncpy_s(oKcbpOpt.szAddress, m_strAddress.c_str(), sizeof(oKcbpOpt.szAddress) - 1);
    strncpy_s(oKcbpOpt.szSendQName, m_strSendQName.c_str(), sizeof(oKcbpOpt.szSendQName) - 1);
    strncpy_s(oKcbpOpt.szReceiveQName, m_strReceiveQName.c_str(), sizeof(oKcbpOpt.szReceiveQName) - 1);
    nRet = KCBPCLI_SetConnectOption(m_hKCBPCli, oKcbpOpt);
    if (nRet != 0)
    {
        KCBPCLI_GetErrorMsg(m_hKCBPCli, szBuf);
        LOGE("KCBPCLI 设置Option失败, Ret[%d] ErrMsg[%s]", nRet, szBuf);
        KCBPCLI_Exit(m_hKCBPCli);
        m_hKCBPCli = NULL;
        return bRet;
    }
    LOGD("KCBPCLI 初始化参数成功");

    // 设置连接超时
    nRet = KCBPCLI_SetCliTimeOut(m_hKCBPCli, 10);
    if (nRet != 0)
    {
        KCBPCLI_GetErrorMsg(m_hKCBPCli, szBuf);
        LOGE("KCBPCLI 设置超时失败, Ret[%d] ErrMsg[%s]", nRet, szBuf);
        KCBPCLI_Exit(m_hKCBPCli);
        m_hKCBPCli = NULL;
        return bRet;
    }
    LOGD("KCBPCLI 超时设置成功");

    // 连接KCBP
    nRet = KCBPCLI_SQLConnect(m_hKCBPCli, (char *)m_strServerName.c_str(), (char *)m_strUserName.c_str(), (char *)m_strPassword.c_str());
    if (nRet != 0)
    {
        KCBPCLI_GetErrorMsg(m_hKCBPCli, szBuf);
        LOGE("KCBPCLI 连接失败, Ret[%d] ErrMsg[%s]", nRet, szBuf);
        KCBPCLI_Exit(m_hKCBPCli);
        m_hKCBPCli = NULL;
        return bRet;
    }
    LOGD("KCBPCLI 连接成功");

    return true;
}

bool CCMSKCBPCli::Uninit()
{
    m_nLastError = 0;
    memset(m_szErrMsg, 0, sizeof(m_szErrMsg));

    // 支持重连
    if (m_hKCBPCli != NULL)
    {
        LOGI("断开现有的KCBP连接");
        KCBPCLI_SQLDisconnect(m_hKCBPCli);
        KCBPCLI_Exit(m_hKCBPCli);
        m_hKCBPCli = NULL;
    }
    return true;
}


int CCMSKCBPCli::CallLbm(const string& strLbmNo, const map<string, string>& mapReq, vector<map<string, string>>& vecAns)
{
    int& nRet = m_nLastError;
    char* szBuf = m_szErrMsg;

    LOGDH();
    if (m_hKCBPCli == NULL)
    {
        nRet = -1;
        strcpy_s(szBuf, sizeof(m_szErrMsg)-1, "服务器尚未连接，请先连接调用Init函数连接KCBP成功后再试");
        LOGE("%s", szBuf);
        return -1;
    }

    nRet = KCBPCLI_BeginWrite(m_hKCBPCli);
    if (nRet != 0)
    {
        KCBPCLI_GetErrorMsg(m_hKCBPCli, szBuf);
        LOGE("KCBPCLI_BeginWrite 调用失败. Code[%s] ErrMsg[%s]", nRet, szBuf);
        return nRet;
    }

    // 设置参数
    string strCallInfo;
    strCallInfo = strLbmNo + "!";
    map<string, string>::const_iterator iterVal = mapReq.begin();
    for (; iterVal != mapReq.end(); iterVal++)
    {
        KCBPCLI_SetValue(m_hKCBPCli, (char*)iterVal->first.c_str(), (char*)iterVal->second.c_str());
        
        if (iterVal != mapReq.begin())
        {
            strCallInfo += ", ";
        }
        strCallInfo += iterVal->first + ":" + iterVal->second;
    }
    LOGD("LBM请求:%s", strCallInfo.c_str());

    // 提交调用
    nRet = KCBPCLI_CallProgramAndCommit(m_hKCBPCli, (char *)strLbmNo.c_str());
    if (nRet != 0)
    {
        KCBPCLI_GetErrorMsg(m_hKCBPCli, szBuf);
        LOGE("调用KCBPCLI_CallProgramAndCommits失败. Code[%d] ErrMsg[%s]", nRet, szBuf);
        return nRet;
    }

    // 获取返回值
    KCBPCLI_GetErrorCode(m_hKCBPCli, &nRet);
    if (nRet != 0)
    {
        KCBPCLI_GetErrorMsg(m_hKCBPCli, szBuf);
        LOGE("函数调用失败，LBM[%s] Code[%d] ErrMsg[%s]", strLbmNo.c_str(), nRet, szBuf);
        return nRet;
    }

    nRet = KCBPCLI_RsOpen(m_hKCBPCli);
    if (nRet != 0 && nRet != 100)
    {
        KCBPCLI_GetErrorMsg(m_hKCBPCli, szBuf);
        LOGE("KCBPCLI_RsOpen 失败, Code[%d] ErrMsg[%s]", nRet, szBuf);
        return nRet;
    }

    int  nCol = 0;
    KCBPCLI_SQLNumResultCols(m_hKCBPCli, &nCol);
    KCBPCLI_SQLFetch(m_hKCBPCli);

    // 获取返回值代码
    nRet = KCBPCLI_RsGetColByName(m_hKCBPCli, "MSG_CODE", szBuf);
    if (nRet != 0)
    {
        KCBPCLI_GetErrorMsg(m_hKCBPCli, szBuf);
        LOGE("获取返回值代码字段[MSG_CODE]失败. Code[%d] ErrMsg[%s]", nRet, szBuf);
        return false;
    }

    // 调用失败
    nRet = atoi(szBuf);
    if (nRet != 0)
    {
        KCBPCLI_RsGetColByName(m_hKCBPCli, "MSG_TEXT", szBuf);
        LOGD("函数调用结果返回失败，Code[%d] ErrMsg[%s]", nRet, szBuf);
        return nRet;
    }

    // 调用成功，获取返回值
    nRet = KCBPCLI_SQLMoreResults(m_hKCBPCli);
    if (nRet == 0)
    {
        LOGD("返回值:");
        KCBPCLI_SQLNumResultCols(m_hKCBPCli, &nCol);
        while (!KCBPCLI_RsFetchRow(m_hKCBPCli))
        {
            strCallInfo = "";
            map<string, string> mapAns;
            string strName, strVal;
            for (int i = 1; i <= nCol; i++)
            {
                KCBPCLI_RsGetColName(m_hKCBPCli, i, szBuf, sizeof(m_szErrMsg)-1);
                strName = CMS::Trim(szBuf);
                KCBPCLI_RsGetCol(m_hKCBPCli, i, szBuf);
                strVal = CMS::Trim(szBuf);

                mapAns[strName] = strVal;
                if (strCallInfo.length() > 0)
                {
                    strCallInfo += ", ";
                }
                strCallInfo += strName + ":" + strVal;
            }
            vecAns.push_back(mapAns);
            LOGD("%s", strCallInfo.c_str());
        }
    }
    KCBPCLI_SQLCloseCursor(m_hKCBPCli);

    return nRet;
}

BOOL CCMSKCBPCli::SetKCBPPublicInput(map<string, string>& mapKV)
{
    BOOL bRet = FALSE;

    if (mapKV.size() <= 0)
    {
        char szBuf[128] = { 0 };
        string strTemp;

        mapKV["F_FUNCTION"] = "";
        mapKV["F_SUBSYS"] = "13";
        mapKV["F_OP_USER"] = "8888";
        mapKV["F_OP_ROLE"] = "2";
        mapKV["F_OP_SITE"] = "127.0.0.1";
        mapKV["F_CHANNEL"] = "0";
        mapKV["F_SESSION"] = "ZSOTCXTYZMU";

        CMS::GetDate(szBuf, "%Y-%m-%d");
        mapKV["F_RUNTIME"] = szBuf;
    }
    return TRUE;
}

void CCMSKCBPCli::GetLastError(int& nRet, string& strErrMsg)
{
    nRet = m_nLastError;
    strErrMsg = m_szErrMsg;
}
