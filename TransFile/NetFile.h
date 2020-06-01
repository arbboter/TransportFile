#pragma once
#include <string>
#include <vector>
#include <map>
#include <queue>
#include "Task.h"
using namespace std;

// 消息类型
#define MSG_VERSION     202005
#define MSG_ID_FLAG     0X12345678
#define MSG_TYPE_TEXT   100
#define MSG_TYPE_FILE   101
#define MSG_TYPE_CMDS   102

// 消息指令
#define MSG_CMD_FBEGIN  1000
#define MSG_CMD_FEND    1001

// 消息包类型
#define MSG_PACK_TYPE_REQ   1000
#define MSG_PACK_TYPE_ACK   2000

// 内存大小
#define FSIZE_M             (1024*1024)
#define FSIZE_G             (1024*FSIZE_M)

// 网络句柄变化状态
#define SOCK_CHG_ACCEPT      (0)
#define SOCK_CHG_RECV        (1)
#define SOCK_CHG_SEND        (2)
#define SOCK_CHG_CLOSE       (3)

// 通信报文头
class CMsgPackHeader
{
public:
    int         nSize;          // 包大小：包头+包体（必填）
    int         nIdFlag;        // 标识号（必填）
    int         nVersion;       // 版本号: 版本号（必填）
    int         nPackType;      // 消息包类型
    int         nDataType;      // 数据类型: 100-文本 101-文件 102-指令
    char        pSender[32];    // 发送者（可选）
    char        pRecver[32];    // 接收者（可选）
    char        pName[64];      // 文件名（可选）
    int         nSeq;           // 包号（可选）
    int         nFlag;          // 标志
    INT64       nTotalSize;     // 总大小

    CMsgPackHeader()
    {
        nSize = sizeof(CMsgPackHeader);
        nIdFlag = MSG_ID_FLAG;
        nVersion = MSG_VERSION;
        memset(pSender, 0, sizeof(pSender));
        memset(pSender, 0, sizeof(pRecver));
        nSeq = 0;
        nDataType = MSG_TYPE_TEXT;
        nPackType = MSG_PACK_TYPE_REQ;
        nFlag = 0;
        nTotalSize = 0;
    }
};
// 通信报文
class CMsgPack
{
public:
    CMsgPackHeader      m_oHeader;  // 消息头
    char*               m_pPack;    // 消息：头+体

    // 拷贝构造函数
    CMsgPack(const CMsgPack& oPack)
    {
        Init(oPack);
    }
    CMsgPack& operator=(const CMsgPack& oPack)
    {
        Init(oPack);
        return *this;
    }
    void Init(const CMsgPack& oPack)
    {
        m_oHeader = oPack.m_oHeader;
        m_pPack = new char[m_oHeader.nSize+1];
        memcpy(m_pPack, oPack.m_pPack, m_oHeader.nSize+1);
    }

    CMsgPack()
    {
        m_pPack = NULL;
    }
    ~CMsgPack()
    {
        delete m_pPack;
        m_pPack = NULL;
    }

    // 数据地址
    char* Data()
    {
        return m_pPack+sizeof(m_oHeader);
    }
    int DataSize()
    {
        return m_oHeader.nSize - sizeof(m_oHeader);
    }
    // 完整数据包
    char* Pack()
    {
        if(m_pPack == NULL)
        {
            SetBody(NULL, 0);
        }
        return m_pPack;
    }
    int PackSize()
    {
        if(m_pPack == NULL)
        {
            SetBody(NULL, 0);
        }
        return m_oHeader.nSize;
    }

    // 设置数据
    void SetBody(const void* pData, int nLen)
    {
        delete m_pPack;
        m_pPack = NULL;

        // 无数据时，发1字节内容
        char* pBody = "1";
        if(nLen <= 0)
        {
            nLen = 1;
        }
        else
        {
            pBody = (char*)pData;
        }
        m_oHeader.nSize = sizeof(m_oHeader) + nLen;
        m_pPack = new char[m_oHeader.nSize+1];
        memset(m_pPack, 0, m_oHeader.nSize+1);
        memcpy(m_pPack, &m_oHeader, sizeof(m_oHeader));
        memcpy(m_pPack+sizeof(m_oHeader), pBody, nLen);
    }

    // 解析数据
    bool LoadPack(const void* pData, int nLen)
    {
        if(!IsThisPack(pData, nLen))
        {
            return false;
        }
        CMsgPackHeader* pHeader = (CMsgPackHeader*)pData;
        // 大小校验
        if(pHeader->nSize > nLen)
        {
            return false;
        }
        m_oHeader = *pHeader;
        const char* pBody = (char*)pData + sizeof(m_oHeader);
        SetBody(pBody, pHeader->nSize - sizeof(m_oHeader));
        return true;
    }

    // 判断是否为当前包内容
    static bool IsThisPack(const void* pData, int nLen)
    {
        // 包头大小判断
        if(nLen < sizeof(CMsgPackHeader))
        {
            return false;
        }
        CMsgPackHeader* pHeader = (CMsgPackHeader*)pData;
        // 大小校验
        if(pHeader->nIdFlag != MSG_ID_FLAG)
        {
            return false;
        }
        // 版本校验
        if(pHeader->nVersion < MSG_VERSION)
        {
            return false;
        }
        return true;
    }

    // 拆包
    static bool SplitPack(const void* pData, int nLen, CMsgPack& oPack)
    {
        // 格式校验
        if(!IsThisPack(pData, nLen))
        {
            return false;
        }
        // 拆包
        return oPack.LoadPack(pData, nLen);
    }
};

class CNetFileClient;
class CNetFile
{
public:
    CNetFile();
    ~CNetFile();

protected:
    SOCKET              m_hSocket;      // 网络句柄
    string              m_strHost;      // IP地址
    int                 m_nPort;        // 端口
    bool                m_bRun;         // 运行状态

public:
    // SOCKET句柄
    SOCKET Socket() const { return m_hSocket; }

public:
    // 发送文件
    virtual int SendFile(const string& strFile) = NULL;
    virtual int SendMsg(const string& strMsg) = NULL;
    // 发送报文
    virtual int Send(const char* pBuf, int nLen) = NULL;
    // 地址信息
    virtual string GetAddr() const;
    // 关闭
    virtual void Close() = NULL;
    // 获取客户端列表
    virtual void GetClient(vector<CNetFileClient*>& vecClient) = NULL;
    
};

// 接收缓存区
class CRecvBuf
{
public:
    char*   m_pBuf;         // 内存地址
    int     m_nBufLen;      // 内存大小
    int     m_nDataLen;     // 数据大小

    CRecvBuf(int nSize)
    {
        m_nBufLen = nSize;
        m_pBuf = new char[m_nBufLen];
        memset(m_pBuf, 0, m_nBufLen);
        m_nDataLen = 0;
    }
    // 追加数据
    char* Append(const char* pData, int nLen)
    {
        if(nLen+m_nDataLen > m_nBufLen)
        {
            return NULL;
        }
        memcpy(m_pBuf+m_nDataLen, pData, nLen);
        m_nDataLen += nLen;
        return m_pBuf;
    }
    // 删除已用数据
    void Pop(int nLen)
    {
        memmove(m_pBuf, m_pBuf+nLen, m_nDataLen-nLen);
        m_nDataLen -= nLen;
    }
    // 清除数据
    void Clear()
    {
        m_nDataLen = 0;
    }
    ~CRecvBuf()
    {
        delete[] m_pBuf;
        m_pBuf = NULL;
    }
};

class CNetFileClient : public CNetFile
{
public:
    CNetFileClient();
    ~CNetFileClient(void);

protected:
    CTask                           m_oRecvProc;    // 数据接收线程
    const unsigned int              m_nBlockSize;   // 块大小
    const unsigned int              m_nBlockNum;    // 块数
    queue<CMsgPack>                 m_qAck;         // 接收应答ACK
    CRITICAL_SECTION                m_csRecv;       // 数据接收区锁
    INT64                           m_nTotalSize;   // 总大小
    INT64                           m_nCurRecvSize; // 当前接收大小
    int                             m_nSeq;         // 包序号
    FILE*                           m_fpFile;       // 文件句柄

public:
    // 是否有效
    bool IsInvalid();
    // 内存接收缓冲区锁操作
    void LockRecv();
    void UnlockRecv();
    // 连接服务器
    int Connect(const string& strHost, int nPort);
    // 设置连接句柄
    int Init(SOCKET& socket, const string& strHost, int nPort);
    // 关闭
    int DisConnect();
    int CreateFile(const CMsgPackHeader& oHeader);
    // 关闭文件
    int CloseFile();
    // 处理数据
    int DealRecvPack(CMsgPack& oPack);
    int DealRecvPack(const char* pBuf, int nLen);
    int DealRecvCmd(CMsgPack& oPack);
    // 接收报文到本地接收缓存区
    virtual int OnRecv();
    // 从接收缓存区获取接收数据,每次获取块大小，不足全部获取
    virtual int GetRecvBlock(char* pBuf, unsigned int nBufSize);
    // 发送文件
    virtual int SendFile(const string& strFile);
    // 发送消息
    virtual int SendMsg(const string& strMsg);
    // 发送数据
    virtual int Send(const char* pBuf, int nLen);
    // 接收
    virtual int Recv(CMsgPack& oPack, int nMiSeconds=0);
    virtual int Recv(int nMiSeconds=0);
    // 关闭
    virtual void Close();
    // 获取客户端列表
    virtual void GetClient(vector<CNetFileClient*>& vecClient);
};

class CNetFileServer : public CNetFile
{
public:
    CNetFileServer();
    ~CNetFileServer(void);

private:
    CTask                           m_oAcceptProc;  // 接收子线程
    map<SOCKET, CNetFileClient*>    m_mapClient;    // 客户端

protected:
    // 更新客户端列表，删除已失效的
    void UpdateClient();

public:
    // 接收客户端请求，不需要手动调用，创建服务端已经启动
    void OnAccept();
    // 获取客户端
    CNetFileClient* GetClient(SOCKET nSocket);

public:
    // 创建服务端
    int Create(int nPort);
    // 关闭
    virtual void Close();
    // 发送文件
    virtual int SendFile(const string& strFile);
    // 发送消息
    virtual int SendMsg(const string& strMsg);
    // 发送数据
    virtual int Send(const char* pBuf, int nLen);
    // 获取客户端列表
    virtual void GetClient(vector<CNetFileClient*>& vecClient);
};

