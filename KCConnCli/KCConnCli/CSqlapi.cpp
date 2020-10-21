#include "StdAfx.h"
#include "Sqlapi.h"
#include "CSqlapi.h"
#include "Slog.h"
#include "Util.h"

#define USE_SQLAPI_STATICLIB 1
//自SQLAPI++ 3.7.25版本后，使用静态库版本时需要输入user32.lib, version.lib, oleaut32.lib 和 ole32.lib 库文件
#if USE_SQLAPI_STATICLIB
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "version.lib")
#pragma comment (lib, "oleaut32.lib")
#pragma comment (lib, "ole32.lib")
#ifdef _DEBUG
#pragma comment (lib, "sqlapisd.lib")
#else
#pragma comment (lib, "sqlapis.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment (lib, "sqlapid.lib")
#else
#pragma comment (lib, "sqlapi.lib")
#endif
#endif

CSqlapi::CSqlapi(void)
{
}


CSqlapi::~CSqlapi(void)
{
    Disconnect();
}

bool CSqlapi::Connect(const string& strHost, const string& strName, const string& strUser, const string& strPass)
{
    m_strHost = strHost;
    m_strName = strName;
    m_strUser = strUser;
    m_strPass = strPass;

    string strConnDesc = strHost + "@" + strName;
    return Connect(strConnDesc, strUser, strPass, SA_SQLServer_Client);
}

bool CSqlapi::Connect(const string& strConnDesc, const string& strUser, const string& strPass, int nType)
{
    bool bRet = false;

    if(m_oConn.isConnected())
    {
        LOGI("数据库已连接，自动断开重连");
        m_oConn.Disconnect();
    }
    try
    {
        m_strUser = strUser;
        m_strPass = strPass;

        LOGI("正在连接数据库[%s]", strConnDesc.c_str());
        m_oConn.Connect(strConnDesc.c_str(), strUser.c_str(), strPass.c_str(), (SAClient_t)nType);
        bRet = m_oConn.isConnected();
    }
    catch(SAException& e)
    {
        LOGI("数据库连接失败，连接信息[%s,%s,%s] 错误信息[%s]",
            strConnDesc.c_str(), strUser.c_str(), strPass.c_str(),
            e.ErrText().GetMultiByteChars());
    }
    return bRet;
}

bool CSqlapi::Query(const string& strSql, vector<vector<string>>& vecRows, bool bHeader/*=false*/)
{
    bool bRet = false;
    if(!m_oConn.isConnected() || !m_oConn.isAlive())
    {
        LOGI("数据库连接未就绪，请重连");
        return bRet;
    }

    // 获取查询字段名
    if(bHeader)
    {
        vector<string> vecHeader;
        if(!Sqlapi::GetSqlFields(strSql, vecHeader))
        {
            LOGE("获取SQL表头失败,SQL:%s", strSql.c_str());
        }
        else
        {
            vecRows.push_back(vecHeader);
        }
    }

    SACommand oCmd(&m_oConn, strSql.c_str());
    try
    {
        int nNum = 0;
        vector<string> vecRow;
        string strTemp;
        string strDebug;

        LOGD("执行SQL查询:%s", strSql.c_str());
        oCmd.Execute();
        while(oCmd.FetchNext())
        {
            vecRow.clear();
            nNum = oCmd.FieldCount();
            for (int i=1; i<=nNum; i++)
            {
                strTemp = oCmd.Field(i).asString().GetMultiByteChars();
                vecRow.push_back(strTemp);
            }
            vecRows.push_back(vecRow);
        }
        bRet = true;
    }
    catch(SAException& e)
    {
        LOGE("执行SQL失败[%s] 错误信息[%s]", strSql.c_str(), e.ErrText().GetMultiByteChars());
    }
    oCmd.Close();

    return bRet;
}

bool CSqlapi::Disconnect()
{
    if(m_oConn.isConnected())
    {
        m_oConn.Disconnect();
    }
    return true;
}

bool CSqlapi::Exec(const string& strSql)
{
    bool bRet = false;
    if(!m_oConn.isConnected() || !m_oConn.isAlive())
    {
        LOGW("数据库连接未就绪，请重连");
        return bRet;
    }

    SACommand oCmd(&m_oConn);
    try
    {
        LOGD("执行SQL:%s", strSql.c_str());
        oCmd.setCommandText(strSql.c_str());
        oCmd.Execute();
        m_oConn.Commit();
        bRet = true;
    }
    catch(SAException& e)
    {
        LOGE("执行SQL失败[%s] 错误信息[%s]", strSql.c_str(), e.ErrText().GetMultiByteChars());
    }
    oCmd.Close();

    return bRet;
}

bool CSqlapi::Exec(const vector<string>& vecSql)
{
    bool bRet = false;
    if(!m_oConn.isConnected() || !m_oConn.isAlive())
    {
        LOGW("数据库连接未就绪，请重连");
        return bRet;
    }

    SACommand oCmd(&m_oConn);
    size_t i =0;
    try
    {
        for (i=0; i<vecSql.size(); i++)
        {
            LOGD("执行SQL:%s", vecSql[i].c_str());
            oCmd.setCommandText(vecSql[i].c_str());
            oCmd.Execute();
        }
        m_oConn.Commit();
        bRet = true;
    }
    catch(SAException& e)
    {
        LOGE("执行SQL失败[%s] 错误信息[%s]", vecSql[i].c_str(), e.ErrText().GetMultiByteChars());
    }
    oCmd.Close();

    return bRet;
}

bool CSqlapi::First(const vector<vector<string>>& vecRows, string& strVal)
{
    if(vecRows.size() > 0)
    {
        if(vecRows[0].size() > 0)
        {
            strVal = vecRows[0][0];
            return true;
        }
    }
    return false;
}

bool CSqlapi::IsConnected()
{
    return m_oConn.isConnected();
}

bool Sqlapi::FetchWithSql(const vector<string>& vecPara, vector<vector<string>>& vecRows, bool bHeader/*=false*/)
{
    bool bRet = false;

    // 类型,连接串,用户名,密码,sql语句
    if(vecPara.size() < 5)
    {
        return false;
    }

    int nType = atoi(vecPara[0].c_str());
    string strConnDesc = vecPara[1];
    string strUid = vecPara[2];
    string strPwd = vecPara[3];
    string strSql = vecPara[4];

    // 判断连接类型
    if(nType<1 || nType>SA_SQLite_Client)
    {
        return false;
    }

    // 数据库连接
    CSqlapi oSql;
    if(!oSql.Connect(strConnDesc, strUid, strPwd, nType))
    {
        return false;
    }
    // 获取查询字段名
    if(bHeader)
    {
        vector<string> vecHeader;
        if(!GetSqlFields(strSql, vecHeader))
        {
            LOGE("获取SQL表头失败,SQL:%s", strSql.c_str());
        }
        else
        {
            vecRows.push_back(vecHeader);
        }
    }

    // 数据库查询关闭
    bRet = oSql.Query(strSql, vecRows);
    oSql.Disconnect();

    return bRet;
}

bool Sqlapi::ExecSql(const vector<string>& vecPara)
{
    bool bRet = false;

    // 类型,连接串,用户名,密码,sql语句
    if(vecPara.size() < 5)
    {
        return false;
    }

    int nType = atoi(vecPara[0].c_str());
    string strConnDesc = vecPara[1];
    string strUid = vecPara[2];
    string strPwd = vecPara[3];
    string strSql = vecPara[4];

    // 判断连接类型
    if(nType<1 || nType>SA_SQLite_Client)
    {
        return false;
    }

    // 数据库连接
    CSqlapi oSql;
    if(!oSql.Connect(strConnDesc, strUid, strPwd, nType))
    {
        return false;
    }
    // 数据库查询关闭
    bRet = oSql.Exec(strSql);
    oSql.Disconnect();

    return bRet;
}

bool Sqlapi::GetSqlFields(const string& strSql, vector<string>& vecFields)
{
    // select ... from ...
    string strLowerSql = CMS::Upper(strSql);
    string strBegTag = "SELECT ";
    string strEndTag = " FROM ";
    size_t nBeg = strLowerSql.find(strBegTag);
    size_t nEnd = strLowerSql.find(strEndTag);
    if(nBeg==string::npos || nEnd==string::npos)
    {
        return false;
    }

    // 处理别名
    int nNum = nEnd - nBeg - strBegTag.size();
    string strFields = strSql.substr(nBeg+strBegTag.size(), nNum);
    strFields = CMS::Trim(strFields);
    if(strFields == "*")
    {
        return false;
    }

    vector<string> vecSqlFileds;
    vector<string> vecFieldItem;
    CMS::Split(strFields, ",", vecSqlFileds, true);
    for (size_t i=0; i<vecSqlFileds.size(); i++)
    {
        vecFieldItem.clear();
        CMS::Split(vecSqlFileds[i], " ", vecFieldItem, true);
        vecFields.push_back(*(--vecFieldItem.end()));
    }
    return true;
}
