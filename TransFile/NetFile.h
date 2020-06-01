#pragma once
#include <string>
#include <vector>
#include <map>
#include <queue>
#include "Task.h"
using namespace std;

// ��Ϣ����
#define MSG_VERSION     202005
#define MSG_ID_FLAG     0X12345678
#define MSG_TYPE_TEXT   100
#define MSG_TYPE_FILE   101
#define MSG_TYPE_CMDS   102

// ��Ϣָ��
#define MSG_CMD_FBEGIN  1000
#define MSG_CMD_FEND    1001

// ��Ϣ������
#define MSG_PACK_TYPE_REQ   1000
#define MSG_PACK_TYPE_ACK   2000

// �ڴ��С
#define FSIZE_M             (1024*1024)
#define FSIZE_G             (1024*FSIZE_M)

// �������仯״̬
#define SOCK_CHG_ACCEPT      (0)
#define SOCK_CHG_RECV        (1)
#define SOCK_CHG_SEND        (2)
#define SOCK_CHG_CLOSE       (3)

// ͨ�ű���ͷ
class CMsgPackHeader
{
public:
    int         nSize;          // ����С����ͷ+���壨���
    int         nIdFlag;        // ��ʶ�ţ����
    int         nVersion;       // �汾��: �汾�ţ����
    int         nPackType;      // ��Ϣ������
    int         nDataType;      // ��������: 100-�ı� 101-�ļ� 102-ָ��
    char        pSender[32];    // �����ߣ���ѡ��
    char        pRecver[32];    // �����ߣ���ѡ��
    char        pName[64];      // �ļ�������ѡ��
    int         nSeq;           // ���ţ���ѡ��
    int         nFlag;          // ��־
    INT64       nTotalSize;     // �ܴ�С

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
// ͨ�ű���
class CMsgPack
{
public:
    CMsgPackHeader      m_oHeader;  // ��Ϣͷ
    char*               m_pPack;    // ��Ϣ��ͷ+��

    // �������캯��
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

    // ���ݵ�ַ
    char* Data()
    {
        return m_pPack+sizeof(m_oHeader);
    }
    int DataSize()
    {
        return m_oHeader.nSize - sizeof(m_oHeader);
    }
    // �������ݰ�
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

    // ��������
    void SetBody(const void* pData, int nLen)
    {
        delete m_pPack;
        m_pPack = NULL;

        // ������ʱ����1�ֽ�����
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

    // ��������
    bool LoadPack(const void* pData, int nLen)
    {
        if(!IsThisPack(pData, nLen))
        {
            return false;
        }
        CMsgPackHeader* pHeader = (CMsgPackHeader*)pData;
        // ��СУ��
        if(pHeader->nSize > nLen)
        {
            return false;
        }
        m_oHeader = *pHeader;
        const char* pBody = (char*)pData + sizeof(m_oHeader);
        SetBody(pBody, pHeader->nSize - sizeof(m_oHeader));
        return true;
    }

    // �ж��Ƿ�Ϊ��ǰ������
    static bool IsThisPack(const void* pData, int nLen)
    {
        // ��ͷ��С�ж�
        if(nLen < sizeof(CMsgPackHeader))
        {
            return false;
        }
        CMsgPackHeader* pHeader = (CMsgPackHeader*)pData;
        // ��СУ��
        if(pHeader->nIdFlag != MSG_ID_FLAG)
        {
            return false;
        }
        // �汾У��
        if(pHeader->nVersion < MSG_VERSION)
        {
            return false;
        }
        return true;
    }

    // ���
    static bool SplitPack(const void* pData, int nLen, CMsgPack& oPack)
    {
        // ��ʽУ��
        if(!IsThisPack(pData, nLen))
        {
            return false;
        }
        // ���
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
    SOCKET              m_hSocket;      // ������
    string              m_strHost;      // IP��ַ
    int                 m_nPort;        // �˿�
    bool                m_bRun;         // ����״̬

public:
    // SOCKET���
    SOCKET Socket() const { return m_hSocket; }

public:
    // �����ļ�
    virtual int SendFile(const string& strFile) = NULL;
    virtual int SendMsg(const string& strMsg) = NULL;
    // ���ͱ���
    virtual int Send(const char* pBuf, int nLen) = NULL;
    // ��ַ��Ϣ
    virtual string GetAddr() const;
    // �ر�
    virtual void Close() = NULL;
    // ��ȡ�ͻ����б�
    virtual void GetClient(vector<CNetFileClient*>& vecClient) = NULL;
    
};

// ���ջ�����
class CRecvBuf
{
public:
    char*   m_pBuf;         // �ڴ��ַ
    int     m_nBufLen;      // �ڴ��С
    int     m_nDataLen;     // ���ݴ�С

    CRecvBuf(int nSize)
    {
        m_nBufLen = nSize;
        m_pBuf = new char[m_nBufLen];
        memset(m_pBuf, 0, m_nBufLen);
        m_nDataLen = 0;
    }
    // ׷������
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
    // ɾ����������
    void Pop(int nLen)
    {
        memmove(m_pBuf, m_pBuf+nLen, m_nDataLen-nLen);
        m_nDataLen -= nLen;
    }
    // �������
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
    CTask                           m_oRecvProc;    // ���ݽ����߳�
    const unsigned int              m_nBlockSize;   // ���С
    const unsigned int              m_nBlockNum;    // ����
    queue<CMsgPack>                 m_qAck;         // ����Ӧ��ACK
    CRITICAL_SECTION                m_csRecv;       // ���ݽ�������
    INT64                           m_nTotalSize;   // �ܴ�С
    INT64                           m_nCurRecvSize; // ��ǰ���մ�С
    int                             m_nSeq;         // �����
    FILE*                           m_fpFile;       // �ļ����

public:
    // �Ƿ���Ч
    bool IsInvalid();
    // �ڴ���ջ�����������
    void LockRecv();
    void UnlockRecv();
    // ���ӷ�����
    int Connect(const string& strHost, int nPort);
    // �������Ӿ��
    int Init(SOCKET& socket, const string& strHost, int nPort);
    // �ر�
    int DisConnect();
    int CreateFile(const CMsgPackHeader& oHeader);
    // �ر��ļ�
    int CloseFile();
    // ��������
    int DealRecvPack(CMsgPack& oPack);
    int DealRecvPack(const char* pBuf, int nLen);
    int DealRecvCmd(CMsgPack& oPack);
    // ���ձ��ĵ����ؽ��ջ�����
    virtual int OnRecv();
    // �ӽ��ջ�������ȡ��������,ÿ�λ�ȡ���С������ȫ����ȡ
    virtual int GetRecvBlock(char* pBuf, unsigned int nBufSize);
    // �����ļ�
    virtual int SendFile(const string& strFile);
    // ������Ϣ
    virtual int SendMsg(const string& strMsg);
    // ��������
    virtual int Send(const char* pBuf, int nLen);
    // ����
    virtual int Recv(CMsgPack& oPack, int nMiSeconds=0);
    virtual int Recv(int nMiSeconds=0);
    // �ر�
    virtual void Close();
    // ��ȡ�ͻ����б�
    virtual void GetClient(vector<CNetFileClient*>& vecClient);
};

class CNetFileServer : public CNetFile
{
public:
    CNetFileServer();
    ~CNetFileServer(void);

private:
    CTask                           m_oAcceptProc;  // �������߳�
    map<SOCKET, CNetFileClient*>    m_mapClient;    // �ͻ���

protected:
    // ���¿ͻ����б�ɾ����ʧЧ��
    void UpdateClient();

public:
    // ���տͻ������󣬲���Ҫ�ֶ����ã�����������Ѿ�����
    void OnAccept();
    // ��ȡ�ͻ���
    CNetFileClient* GetClient(SOCKET nSocket);

public:
    // ���������
    int Create(int nPort);
    // �ر�
    virtual void Close();
    // �����ļ�
    virtual int SendFile(const string& strFile);
    // ������Ϣ
    virtual int SendMsg(const string& strMsg);
    // ��������
    virtual int Send(const char* pBuf, int nLen);
    // ��ȡ�ͻ����б�
    virtual void GetClient(vector<CNetFileClient*>& vecClient);
};

