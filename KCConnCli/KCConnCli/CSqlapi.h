#pragma once
#include "Sqlapi.h"
#include <vector>
#include <string>

using namespace std;
class CSqlapi
{
public:
    CSqlapi(void);
    ~CSqlapi(void);

    bool Connect(const string& strHost, const string& strName, const string& strUser, const string& strPass);
    bool Connect(const string& strConnDesc, const string& strUser, const string& strPass, int nType);
    bool Query(const string& strSql, vector<vector<string>>& vecRows, bool bHeader=false);
    bool Disconnect();
    bool Exec(const string& strSql);
    bool Exec(const vector<string>& vecSql);
    bool First(const vector<vector<string>>& vecRows, string& strVal);
    bool IsConnected();

public:
    SAConnection        m_oConn;
public:
    string              m_strHost;
    string              m_strName;
    string              m_strUser;
    string              m_strPass;
};

namespace Sqlapi
{
    bool FetchWithSql(const vector<string>& vecPara, vector<vector<string>>& vecRows, bool bHeader=false);
    bool ExecSql(const vector<string>& vecPara);
    bool GetSqlFields(const string& strSql, vector<string>& vecFields);
}