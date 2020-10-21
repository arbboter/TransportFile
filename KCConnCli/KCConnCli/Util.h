#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <deque>
#include <windows.h>
#include <stdarg.h>
#include <sstream>
#include <assert.h>

#pragma warning(disable:4996)
using namespace std;

namespace CMS{

class CUtil
{
public:
    CUtil(void);
    ~CUtil(void);

public:
    
};

// 时间函数
string  GetCurTime();
string  GetCurDate();
void    GetDate(char *datestr, const char *format);
string  GetDate(const char *format);

// 字符串函数
bool    StartsWith(const string& strText, const string& strPrefix);
bool    StartsWith(const string& strText, const vector<string>& vecFix);
bool    EndsWith(const string& strText, const string& strPrefix);
string  Formate(const char * pFmt, ...);
string  Formate(const char * pFmt, va_list va);
size_t  CNLength(const string& strText, size_t nCnSize=2);
string  TextFill(const string& strText, const int& nMaxLen, const string& strFill = " ", bool bLeft = false);
string  Trim(const string& strText);
string  DelStrs(const string& strText, const string& strDel);
int     Split(const string& strSrc, const string& strDim, vector<string>& vecItems,bool bTrim=false);
string  Itoa(const int& n);
int     StrCaseCmp(const string& strA, const string& strB);
bool    ElemOfName(char c);
string  Lowwer(const string& strSrc);
string  Upper(const string& strSrc);
string  CutWordGBK(const string& strSrc, int nLen);
bool    GetKV(const string& strKV, string& strK, string& strV, const string& strDim = "=");
bool    GetKV(const string& strKV, const string& strColDim, const string& strDim, map<string, string>& mapData);
string  PrettyTable(const vector<vector<string>>& vecTab, const string& strColDim = " | ", const string& strLnDim = "\n");
string  PrettyMap(const map<string, string>& mapData, const string& strColDim = " : ", const string& strLnDim = "\r\n");

// MFC函数
#ifdef _MFC_VER
CString FileTime2Str(const FILETIME& tTime);
string  CSW2Cpps(const CString& strSrc);
string  GetWndText(CWnd& oWnd);
string  GetLocalHost();
#endif




// Windows文件配置函数
// 枚举读取小节信息
void ReadIniSectionInfo(const string& strAppName, const string& strFilePath, map<string,string>& mapVals);
// 枚举小节名
void ReadIniSectionName(const string& strFilePath, vector<string>& vecName);


// 调试函数
template<typename T>
void ShowVector(const vector<T>& vecInfo)
{
    for (size_t i=0; i< vecInfo.size(); i++)
    {
        cout << i << "\t" << vecInfo[i] << endl;
    }
}
template<typename T1, typename T2>
void ShowMap(const map<T1, T2>& mapInfo)
{
    map<T1, T2>::const_iterator iteInfo = mapInfo.begin();
    while(iteInfo != mapInfo.end())
    {
        cout << iteInfo->first << "\t : " << iteInfo->second << endl;
        iteInfo++;
    }
}

template<typename T1, typename T2>
bool Get(const map<T1, T2>& mapDat, const T1& tKey ,T2& tVal)
{
    bool bRet = false;
    map<T1, T2>::const_iterator iteMap = mapDat.find(tKey);
    if (iteMap != mapDat.end())
    {
        tVal = iteMap->second;
        bRet = true;
    }
    return bRet;
}

template<typename T1, typename T2>
T2 CastFrom(const T1& t1)
{
    stringstream ss;
    T2 t2;

    try
    {
        ss << t1;
        ss >> t2;
    }
    catch (...)
    {
        assert(0);
    }
    return t2;
}

}