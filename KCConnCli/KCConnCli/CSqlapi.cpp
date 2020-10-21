#include "StdAfx.h"
#include "Sqlapi.h"
#include "CSqlapi.h"
#include "Slog.h"
#include "Util.h"

#define USE_SQLAPI_STATICLIB 1
//��SQLAPI++ 3.7.25�汾��ʹ�þ�̬��汾ʱ��Ҫ����user32.lib, version.lib, oleaut32.lib �� ole32.lib ���ļ�
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
        LOGI("���ݿ������ӣ��Զ��Ͽ�����");
        m_oConn.Disconnect();
    }
    try
    {
        m_strUser = strUser;
        m_strPass = strPass;

        LOGI("�����������ݿ�[%s]", strConnDesc.c_str());
        m_oConn.Connect(strConnDesc.c_str(), strUser.c_str(), strPass.c_str(), (SAClient_t)nType);
        bRet = m_oConn.isConnected();
    }
    catch(SAException& e)
    {
        LOGI("���ݿ�����ʧ�ܣ�������Ϣ[%s,%s,%s] ������Ϣ[%s]",
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
        LOGI("���ݿ�����δ������������");
        return bRet;
    }

    // ��ȡ��ѯ�ֶ���
    if(bHeader)
    {
        vector<string> vecHeader;
        if(!Sqlapi::GetSqlFields(strSql, vecHeader))
        {
            LOGE("��ȡSQL��ͷʧ��,SQL:%s", strSql.c_str());
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

        LOGD("ִ��SQL��ѯ:%s", strSql.c_str());
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
        LOGE("ִ��SQLʧ��[%s] ������Ϣ[%s]", strSql.c_str(), e.ErrText().GetMultiByteChars());
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
        LOGW("���ݿ�����δ������������");
        return bRet;
    }

    SACommand oCmd(&m_oConn);
    try
    {
        LOGD("ִ��SQL:%s", strSql.c_str());
        oCmd.setCommandText(strSql.c_str());
        oCmd.Execute();
        m_oConn.Commit();
        bRet = true;
    }
    catch(SAException& e)
    {
        LOGE("ִ��SQLʧ��[%s] ������Ϣ[%s]", strSql.c_str(), e.ErrText().GetMultiByteChars());
    }
    oCmd.Close();

    return bRet;
}

bool CSqlapi::Exec(const vector<string>& vecSql)
{
    bool bRet = false;
    if(!m_oConn.isConnected() || !m_oConn.isAlive())
    {
        LOGW("���ݿ�����δ������������");
        return bRet;
    }

    SACommand oCmd(&m_oConn);
    size_t i =0;
    try
    {
        for (i=0; i<vecSql.size(); i++)
        {
            LOGD("ִ��SQL:%s", vecSql[i].c_str());
            oCmd.setCommandText(vecSql[i].c_str());
            oCmd.Execute();
        }
        m_oConn.Commit();
        bRet = true;
    }
    catch(SAException& e)
    {
        LOGE("ִ��SQLʧ��[%s] ������Ϣ[%s]", vecSql[i].c_str(), e.ErrText().GetMultiByteChars());
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

    // ����,���Ӵ�,�û���,����,sql���
    if(vecPara.size() < 5)
    {
        return false;
    }

    int nType = atoi(vecPara[0].c_str());
    string strConnDesc = vecPara[1];
    string strUid = vecPara[2];
    string strPwd = vecPara[3];
    string strSql = vecPara[4];

    // �ж���������
    if(nType<1 || nType>SA_SQLite_Client)
    {
        return false;
    }

    // ���ݿ�����
    CSqlapi oSql;
    if(!oSql.Connect(strConnDesc, strUid, strPwd, nType))
    {
        return false;
    }
    // ��ȡ��ѯ�ֶ���
    if(bHeader)
    {
        vector<string> vecHeader;
        if(!GetSqlFields(strSql, vecHeader))
        {
            LOGE("��ȡSQL��ͷʧ��,SQL:%s", strSql.c_str());
        }
        else
        {
            vecRows.push_back(vecHeader);
        }
    }

    // ���ݿ��ѯ�ر�
    bRet = oSql.Query(strSql, vecRows);
    oSql.Disconnect();

    return bRet;
}

bool Sqlapi::ExecSql(const vector<string>& vecPara)
{
    bool bRet = false;

    // ����,���Ӵ�,�û���,����,sql���
    if(vecPara.size() < 5)
    {
        return false;
    }

    int nType = atoi(vecPara[0].c_str());
    string strConnDesc = vecPara[1];
    string strUid = vecPara[2];
    string strPwd = vecPara[3];
    string strSql = vecPara[4];

    // �ж���������
    if(nType<1 || nType>SA_SQLite_Client)
    {
        return false;
    }

    // ���ݿ�����
    CSqlapi oSql;
    if(!oSql.Connect(strConnDesc, strUid, strPwd, nType))
    {
        return false;
    }
    // ���ݿ��ѯ�ر�
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

    // �������
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
