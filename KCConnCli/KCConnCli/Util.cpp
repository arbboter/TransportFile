#include "stdafx.h"
#include "Util.h"
#include <Windows.h>
#include <time.h>
#include <algorithm>
#include <string.h>


std::string CMS::GetCurTime()
{
    time_t t = time(NULL); 
    char szBuf[64] = {0};

    strftime(szBuf, sizeof(szBuf), "%Y-%m-%d %H:%M:%S",localtime(&t) ); 

    return szBuf;
}
std::string CMS::GetCurDate()
{
    time_t t = time(NULL);
    char szBuf[64] = { 0 };

    strftime(szBuf, sizeof(szBuf), "%Y-%m-%d", localtime(&t));

    return szBuf;
}


string CMS::Formate(const char * pFmt, ...)
{
    va_list args;
    string  strRet;

    va_start(args, pFmt);
    strRet = Formate(pFmt, args);
    va_end(args);

    return strRet;
}

size_t CMS::CNLength(const string& strText, size_t nCnSize/*=2*/)
{
    size_t nSize = 0;
    for (size_t i = 0; i < strText.size(); )
    {
        if (strText[i] > 127)
        {
            nSize += nCnSize;
            i += nCnSize;
        }
        else
        {
            nSize++;
            i++;
        }
    }
    return nSize;
}

std::string CMS::Formate(const char * pFmt, va_list va)
{
    int     nLen = 0;
    string  strRet;

    nLen = _vscprintf(pFmt, va) + 1;

    char* pBuf = NULL;
    char* pNewBuf = NULL;
    try
    {
        static char pMiniBuf[1024] = { 0 };

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
        vsprintf_s(pBuf, nLen, pFmt, va);
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


bool CMS::StartsWith(const string& strText, const string& strPrefix)
{
    bool bRet = false;

    string& strFmtText = CMS::Lowwer(strText);
    string& strFmtFix = CMS::Lowwer(strPrefix);

    bRet = (strFmtText.substr(0, strFmtFix.length()) == strFmtFix);
    return bRet;
}

bool CMS::StartsWith(const string& strText, const vector<string>& vecFix)
{
    for (size_t i = 0; i < vecFix.size(); i++)
    {
        if (StartsWith(strText, vecFix[i]))
        {
            return true;
        }
    }
    return false;
}

bool CMS::EndsWith(const string& strText, const string& strPrefix)
{
    bool bRet = false;

    string& strFmtText = CMS::Lowwer(strText);
    string& strFmtFix = CMS::Lowwer(strPrefix);

    bRet = (strFmtText.substr(strFmtText.length() - strFmtFix.length()) == strFmtFix);
    return bRet;
}


std::string CMS::TextFill(const string& strText, const int& nMaxLen, const string& strFill /*= " "*/, bool bLeft /*= false*/)
{
    string strRet;
    string strAdd;

    int nAdd = nMaxLen - strText.length();
    int nFillPos = 0;
    while (nAdd > 0)
    {
        strAdd += strFill.at(nFillPos);
        nFillPos = (++nFillPos) % strFill.length();
        nAdd--;
    }
    
    if (bLeft)
    {
        strRet = strAdd + strText;
    }
    else
    {
        strRet = strText + strAdd;
    }
    return strRet;
}


std::string CMS::Trim(const string& strText)
{
    string str = strText;
    string strTrim = " \r\n\t";

    
    int nCur = -1;
    int nBeg = 0;
    int nEnd = strText.length()-1;
    int nTextLen = (int)strText.length();
    
    bool bFund = true;
    // 去头
    while (bFund)
    {
        // 到尾巴了
        if (nCur >= nTextLen)
        {
            break;
        }

        // 查找是否为空字符
        char c = strText[++nCur];
        bFund = false;
        for (size_t i = 0; i < strTrim.length(); i++)
        {
            if (c == strTrim[i])
            {
                bFund = true;
                break;
            }
        }

        // 空白字符
        if (bFund)
        {
            continue;
        }
        // 非空白字符
        else
        {
            nBeg = nCur;
        }
    }

    // 去尾巴
    bFund = true;
    nCur = strText.length();
    while (bFund)
    {
        // 到头了
        if (nCur <= nBeg)
        {
            break;
        }

        // 查找是否为空字符
        char c = strText[--nCur];
        bFund = false;
        for (size_t i = 0; i < strTrim.length(); i++)
        {
            if (c == strTrim[i])
            {
                bFund = true;
                break;
            }
        }

        // 空白字符
        if (bFund)
        {
            continue;
        }
        // 非空白字符
        else
        {
            nEnd = nCur;
        }
    }

    str = strText.substr(nBeg, nEnd - nBeg + 1);
    return str;
}

int CMS::Split(const string& strSrc, const string& strDim, vector<string>& vecItems, bool bTrim/*=false*/)
{
    size_t nPos = 0;
    size_t nLast = 0;
    string strBuf;
    while((nPos=strSrc.find(strDim,nPos)) != string::npos)
    {
        strBuf = strSrc.substr(nLast, nPos-nLast);
        if(bTrim)
        {
            strBuf = Trim(strBuf);
        }
        vecItems.push_back(strBuf);
        nPos += strDim.length();
        nLast =  nPos;
    }
    // 处理尾巴
    size_t nLen = strSrc.size();
    if(nLast <= nLen)
    {
        strBuf = strSrc.substr(nLast, nLen-nLast);
        if(bTrim)
        {
            strBuf = Trim(strBuf);
            if (strBuf.size() > 0)
            {
                vecItems.push_back(strBuf);
            }
        }
        else
        {
            vecItems.push_back(strBuf);
        }
    }
    return vecItems.size();
}

std::string CMS::Itoa(const int& n)
{
    char szBuf[32] = {0};

    sprintf_s(szBuf, sizeof(szBuf), "%d", n);
    return szBuf;
}

bool CMS::ElemOfName(char c)
{
    bool bRet = false;;
    
    if(c == '_')
    {
        bRet = true;
    }
    else if(c>='A' && c<='Z')
    {
        bRet = true;
    }
    else if(c>='a' && c<='a')
    {
        bRet = true;
    }
    else if(c>='0' && c<='9')
    {
        bRet = true;
    }

    return bRet;
}


std::string CMS::Lowwer(const string& strSrc)
{
    string strRet = strSrc;
    std::transform(strRet.begin(), strRet.end(), strRet.begin(), ::tolower);
    return strRet;
}

std::string CMS::Upper(const string& strSrc)
{
    string strRet = strSrc;
    std::transform(strRet.begin(), strRet.end(), strRet.begin(), ::toupper);
    return strRet;
}

string CMS::PrettyTable(const vector<vector<string>>& vecTab, const string& strColDim/* = " | "*/, const string& strLnDim/* = "\n"*/)
{
    string strTab;
    if (vecTab.size() <= 0)
    {
        return strTab;
    }

    size_t nMaxSize = 80;
    size_t nCurSize = 0;
    vector<size_t> vecSize(vecTab[0].size(), 0);

    // 计算最大每列最大宽长度
    for (size_t i = 0; i < vecTab.size(); i++)
    {
        // 每行列数都与标题列数相同
        if (vecTab[i].size() != vecSize.size())
        {
            return strTab;
        }

        for (size_t j = 0; j < vecTab[i].size(); j++)
        {
            nCurSize = vecTab[i][j].size();
            if (vecTab[i][j].size() > nMaxSize)
            {
                nCurSize = nMaxSize;
            }
            vecSize[j] = max(vecSize[j], nCurSize);
        }
    }

    // 构建返回值
    strTab = "";
    for (size_t i = 0; i < vecTab.size(); i++)
    {
        for (size_t j = 0; j < vecTab[i].size(); j++)
        {
            nCurSize = vecTab[i][j].size();

            // 分割连接符
            if (j > 0)
            {
                strTab += strColDim;
            }

            if (nCurSize > vecSize[j])
            {
                strTab += CutWordGBK(vecTab[i][j], vecSize[j]);
            }
            else
            {
                strTab +=  CMS::TextFill(vecTab[i][j], vecSize[j]);
            }
        }
        strTab += strLnDim;
    }
    return strTab;
}

std::string CMS::PrettyMap(const map<string, string>& mapData, const string& strColDim /*= " : "*/, const string& strLnDim /*= "\r\n"*/)
{
    string strInfo;
    size_t nMaxWidth = 0;
    for (auto i = mapData.begin(); i != mapData.end(); ++i)
    {
        nMaxWidth = max(i->first.size(), nMaxWidth);
    }
    for (auto i = mapData.begin(); i != mapData.end(); ++i)
    {
        strInfo += CMS::TextFill(i->first, nMaxWidth, " ", true) + strColDim;
        strInfo += i->second + strLnDim;
    }
    return strInfo;
}

string CMS::CutWordGBK(const string& strSrc, int nLen)
{
    string strRet = strSrc;

    if ((nLen < 0) || ((size_t)nLen >= strSrc.length()))
    {
        return strRet;
    }

    // 拷贝前n个字符，可能会拷贝到半个汉字
    strRet = strSrc.substr(0, nLen);

    /* 从截取后的字符串尾部依次往前看直到遇到英文字符并计数所查看的字符数，
    * 如果计数值为奇数，则表示不需要再截断做特殊处理，否则需要截掉最后一个字符 */
    char cCur = '\0';
    bool bForceEnd = false;
    for (int i = nLen - 1; i >= 0; i--)
    {
        cCur = strRet[i];
        if ((unsigned)cCur < 128)
        {
            break;
        }
        bForceEnd = !bForceEnd;
    }

    /* 如果需要，做再截断特殊处理 */
    if (bForceEnd)
    {
        strRet[nLen - 1] = '\0';
    }
    return strRet;
}

bool CMS::GetKV(const string& strKV, string& strK, string& strV, const string& strDim /*= "="*/)
{
    bool bRet = false;

    size_t nPos = strKV.find(strDim);
    if (nPos != string::npos)
    {
        strK = strKV.substr(0, nPos);
        strV = strKV.substr(nPos + strDim.length());
        bRet = true;
    }
    return bRet;
}

bool CMS::GetKV(const string& strKV, const string& strColDim, const string& strDim, map<string, string>& mapData)
{
    string k, v;
    vector<string> vecCol;
    Split(strKV, strColDim, vecCol);
    for (size_t i = 0; i < vecCol.size(); i++)
    {
        if (GetKV(vecCol[i], k, v, strDim))
        {
            mapData[k] = v;
        }
    }
    return !mapData.empty();
}

void CMS::GetDate(char *datestr, const char *format)
{
    time_t tCur;
    struct tm* tmCur;
    char buffer[80] = {0};

    time(&tCur);

    tmCur = localtime(&tCur);
    strftime(datestr, 80, format, tmCur);
}
string CMS::GetDate(const char *format)
{
    char szBuf[128] = { 0 };
    CMS::GetDate(szBuf, format);
    return szBuf;
}


void CMS::ReadIniSectionInfo(const string& strAppName, const string& strFilePath, map<string,string>& mapVals)
{
    string          strKey;
    string          strKeyValue;
    string          strValue;
    vector<string>  vecItems;
    char*pBuf = NULL;
    
    try
    {
        pBuf = new char[1024*1024];
        size_t tSize = sizeof(pBuf[0]) * 1024 * 1024;
        memset(pBuf, 0, tSize);
        size_t nLen = GetPrivateProfileSectionA(strAppName.c_str(), pBuf, tSize, strFilePath.c_str());
        size_t i = 0, nCur = 0;
        while (i <= nLen)
        {
            if (pBuf[i] == '\0')
            {
                strKeyValue = pBuf + nCur;
                vecItems.clear();
                if (CMS::GetKV(strKeyValue, strKey, strValue))
                {
                    mapVals[strKey] = strValue;
                }
                nCur = i + 1;
            }
            i++;
        }
    }
    catch(...)
    {

    }

    delete[] pBuf;
    pBuf = NULL;
}

void CMS::ReadIniSectionName(const string& strFilePath, vector<string>& vecName)
{
    char* pBuf = NULL;
    char* pCur = NULL;
    size_t tLen = sizeof(pBuf[0]) * 1024 * 512;
    try
    {
        pBuf = new char[tLen];
        memset(pBuf, 0, tLen);

        GetPrivateProfileSectionNamesA(pBuf, tLen, strFilePath.c_str());
        pCur = pBuf;
        while(*pCur != '\0')
        {
            vecName.push_back(pCur);
            pCur = strchr(pCur, '\0') + 1;
        }
    }
    catch(...)
    {

    }
    
}


std::string CMS::DelStrs(const string& strText, const string& strDel)
{
    string strRet;

    for (size_t i = 0; i < strText.length(); i++)
    {
        bool bDel = false;
        for (size_t j = 0; j < strDel.length(); j++)
        {
            if (strText[i] == strDel[j])
            {
                bDel = true;
                break;
            }
        }
        if (!bDel)
        {
            strRet += strText[i];
        }
    }
    return strText;
}

CMS::CUtil::CUtil(void)
{

}

CMS::CUtil::~CUtil(void)
{

}



/* MFC函数定义 */
#ifdef _MFC_VER
string CMS::GetWndText(CWnd& oWnd)
{
    // 获取
    CString strText;
    oWnd.GetWindowText(strText);

    return CSW2Cpps(strText);
}

std::string CMS::CSW2Cpps(const CString& strSrc)
{
    // 转换
    int len = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
    char *ptxtTemp = new char[len + 1];
    WideCharToMultiByte(CP_ACP, 0, strSrc, -1, ptxtTemp, len, NULL, NULL);
    string strRet = ptxtTemp;

    // 去尾
    delete[] ptxtTemp;
    ptxtTemp = 0;
    return strRet;
}

CString CMS::FileTime2Str(const FILETIME& tTime)
{
    CString strRet;
    SYSTEMTIME stLocal;
    FileTimeToSystemTime(&tTime, &stLocal);
    strRet.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
    return strRet;
}

std::string CMS::GetLocalHost()
{
    hostent* stHost = NULL;
    char szHostName[64] = { 0 };
    // 获得本机主机名
    gethostname(szHostName, sizeof(szHostName));
    // 根据本机主机名得到本机ip
    stHost = gethostbyname(szHostName);
    // 把ip换成字符串形式
    string strHost = inet_ntoa(*(struct in_addr *)stHost->h_addr_list[0]);
    return strHost;
}

int CMS::StrCaseCmp(const string& strA, const string& strB)
{
    string str1 = CMS::Lowwer(strA);
    string str2 = CMS::Lowwer(strB);
    return str1.compare(str2);
}

#endif
