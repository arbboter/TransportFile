#pragma once
#include <string>
#include <vector>
#include <map>
#include "KCBPCli.h"

using namespace std;
class CCMSKCBPCli
{
public:
    CCMSKCBPCli();
    ~CCMSKCBPCli();

    bool  Init();
    bool  Uninit();
    int   CallLbm(const string& strLbmNo, const map<string, string>& mapReq, vector<map<string, string>>& vecAns);
    BOOL  SetKCBPPublicInput(map<string, string>& mapKV);
    void  GetLastError(int& nRet, string& strErrMsg);
public:
    // ≈‰÷√ ˝æ›
    string      m_strUserName;
    string      m_strPassword;
    string      m_strServerName;
    string      m_strAddress;
    string      m_strSendQName;
    string      m_strReceiveQName;
    string      m_strPort;
    
protected:
    KCBPCLIHANDLE   m_hKCBPCli;
    int             m_nLastError;
    char            m_szErrMsg[2048];
};

