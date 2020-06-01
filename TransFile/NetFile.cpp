#include "StdAfx.h"
#include "NetFile.h"
#include "Task.h"

CNetFileServer::CNetFileServer()
{
}


CNetFileServer::~CNetFileServer(void)
{
}

// ������մ������߳�
unsigned int __stdcall ServerAcceptProc(void* pPara)
{
    CNetFileServer* pSer = (CNetFileServer*)pPara;
    pSer->OnAccept();
    return 0;
}

void CNetFileServer::UpdateClient()
{
    map<SOCKET, CNetFileClient*>::iterator e = m_mapClient.begin();
    while(e != m_mapClient.end())
    {
        if(!e->second->IsInvalid())
        {
            e = m_mapClient.erase(e);
        }
        else
        {
            ++e;
        }
    }
}

void CNetFileServer::OnAccept()
{
    if(m_hSocket == INVALID_SOCKET)
    {
        return;
    }

    char* pHost = NULL;
    int nPort;
    SOCKADDR_IN addrCli;
    SOCKET socketCli;
    int nAddrLen = sizeof(addrCli);
    while(true)
    {
        socketCli = ::accept(m_hSocket, (SOCKADDR*)&addrCli, &nAddrLen);
        if(socketCli == INVALID_SOCKET)
        {
            GLog("�յ��˴�������������Ѻ���");
            continue;
        }
        // ��ȡ���ӵ�ַ��Ϣ
        pHost = inet_ntoa(addrCli.sin_addr);
        nPort = ntohs(addrCli.sin_port);
        GLog("�յ�[%s:%d]��������", pHost, nPort);

        // ����ͻ�������
        CNetFileClient* pCli = new CNetFileClient();
        pCli->Init(socketCli, pHost, nPort);
        m_mapClient[socketCli] = pCli;
        NotifySocketChange(pCli, SOCK_CHG_ACCEPT);
        GLog("�Ѵ���[%s:%d]��������", pHost, nPort);

        // ���¿ͻ���
        UpdateClient();
    }
}

CNetFileClient* CNetFileServer::GetClient(SOCKET nSocket)
{
    map<SOCKET, CNetFileClient*>::iterator e = m_mapClient.find(nSocket);
    if(e != m_mapClient.end())
    {
        return e->second;
    }
    return NULL;
}

void CNetFileServer::GetClient(vector<CNetFileClient*>& vecClient)
{
    // ���¿ͻ����б�
    UpdateClient();
    map<SOCKET, CNetFileClient*>::iterator e = m_mapClient.begin();
    for (; e!=m_mapClient.end(); ++e)
    {
        vecClient.push_back(e->second);
    }
}

int CNetFileServer::Create(int nPort)
{
    int nRet = 0;

    // ����Ƿ�������
    if(m_hSocket != INVALID_SOCKET)
    {
        return 0;
    }

    m_hSocket = socket(AF_INET,SOCK_STREAM,0);
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port=htons(nPort);

    // �󶨶˿�
    nRet = bind(m_hSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    if(nRet)
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        nRet = WSAGetLastError();
        GLog("�󶨵��˿�[%d]ʧ�ܣ�����˿��Ƿ��ѱ�ռ��", nPort);
        return nRet;
    }

    // �����˿�
    nRet =  listen(m_hSocket, 10);
    if(nRet)
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        nRet = WSAGetLastError();
        GLog("����˿�[%d]����ʧ�ܣ�����˿��Ƿ��ѱ�ռ��", nPort);
        return nRet;
    }
    // �����������߳�
    if(!m_oAcceptProc.RunTask(ServerAcceptProc, this, &m_bRun))
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        nRet = -1;
        GLog("���������߳�ʧ��");
        return nRet;
    }
    GLog("���������ɹ��������˿ں�:%d", nPort);
    m_nPort = nPort;

    return nRet;
}

void CNetFileServer::Close()
{
    m_bRun = false;
    m_oAcceptProc.Stop();
    // �ر������ӵĿͻ���
    map<SOCKET, CNetFileClient*>::iterator e = m_mapClient.end();
    for (e=m_mapClient.begin(); e!=m_mapClient.end(); ++e)
    {
        e->second->Close();
        delete e->second;
    }
    m_mapClient.clear();

    // �رռ���SOCKET
    closesocket(m_hSocket);
    m_hSocket = INVALID_SOCKET;
}

int CNetFileServer::SendFile(const string& strFile)
{
    return 0;
}

int CNetFileServer::SendMsg(const string& strMsg)
{
    return 0;
}

int CNetFileServer::Send(const char* pBuf, int nLen)
{
    return 0;
}

CNetFileClient::CNetFileClient()
    :m_nBlockSize(4*FSIZE_M), m_nBlockNum(32)
{
    // ��ʼ������
    m_nTotalSize = 0;
    m_nCurRecvSize = 0;
    m_nSeq = 0;
    m_fpFile = NULL;
    InitializeCriticalSection(&m_csRecv);
}

CNetFileClient::~CNetFileClient(void)
{
    // ɾ���ٽ���
    DeleteCriticalSection(&m_csRecv);
}

void CNetFileClient::Close()
{
    m_bRun = false;
    m_oRecvProc.Stop();
    if(m_hSocket)
    {
        GLog("�Ͽ�����[%s:%d]������", m_strHost.c_str(), m_nPort);
        NotifySocketChange(this, SOCK_CHG_CLOSE);
        closesocket(m_hSocket);
    }
    CloseFile();
}

void CNetFileClient::GetClient(vector<CNetFileClient*>& vecClient)
{
    if(IsInvalid())
    {
        vecClient.push_back(this);
    }
}

int CNetFileClient::OnRecv()
{
    // ��������
    CRecvBuf* pRecv = new CRecvBuf(m_nBlockSize);
    CRecvBuf* pData = new CRecvBuf(m_nBlockSize*2);
    int nRet = 0;
    while(m_oRecvProc.IsRunning())
    {
        // ��������ڴ�
        pRecv->m_nDataLen = ::recv(m_hSocket, pRecv->m_pBuf, pRecv->m_nBufLen, 0);
        if(pRecv->m_nDataLen <= 0)
        {
            nRet = WSAGetLastError();
            // SOCKET�ر�
            if(nRet==10054 || nRet==0) 
            {
                NotifySocketChange(this, SOCK_CHG_CLOSE);
                Close();
                return 0;
            }
            GLog("����[%s:%d]���ݽ���ʧ�ܣ�������:%d", m_strHost.c_str(), m_nPort, nRet);
            Sleep(1000);
            continue;
        }
        NotifySocketChange(this, SOCK_CHG_RECV);
        // GLog("�յ�����[%s:%d]���ݣ����ݴ�С:%d�ֽ�", m_strHost.c_str(), m_nPort, pRecv->m_nDataLen);

        // ����ƴ��
        if(!pData->Append(pRecv->m_pBuf, pRecv->m_nDataLen))
        {
            GLog("����[%s:%d]���ݴ����������̫��:%d", m_strHost.c_str(), m_nPort, pRecv->m_nDataLen);
        }

        // ������յ�����
        // ������ͨ�ű���
        if(CMsgPack::IsThisPack(pData->m_pBuf, pData->m_nDataLen))
        {
            // ���
            char* pCur = pData->m_pBuf;
            int nCurLen = pData->m_nDataLen;
            CMsgPack oPack;
            while(CMsgPack::SplitPack(pCur, nCurLen, oPack))
            {
                // ������λ
                pCur += oPack.m_oHeader.nSize;
                nCurLen -= oPack.m_oHeader.nSize;

                // �����հ�
                nRet = DealRecvPack(oPack);
                if(!nRet)
                {
                    break;
                }
            }
            // ��������
            pData->Pop(pData->m_nDataLen - nCurLen);
        }
        // ��ʶ��ı���
        else
        {
            DealRecvPack(pData->m_pBuf, pData->m_nDataLen);
            pData->Clear();
        }
    }
    return 0;
}

int CNetFileClient::DealRecvPack(const char* pBuf, int nLen)
{
    if(pBuf==NULL || nLen<=4)
    {
        return false;
    }
    // ���ؽ�����Ϣ��
    CMsgPack oPack;
    if(!oPack.LoadPack(pBuf, nLen))
    {
        GLog("RU[%s:%d]%s", m_strHost.c_str(), m_nPort, pBuf);
        return false;
    }
    return DealRecvPack(oPack);
}

int CNetFileClient::DealRecvPack(CMsgPack& oPack)
{
    int nRet = -1;
    
    // ACK�Ȼ�������
    if(oPack.m_oHeader.nPackType == MSG_PACK_TYPE_ACK)
    {
        LockRecv();
        m_qAck.push(oPack);
        UnlockRecv();
        return 0;
    }

    // ֻ�����������
    if(oPack.m_oHeader.nPackType != MSG_PACK_TYPE_REQ)
    {
        GLog("����[%s:%d]�������󣬷������", m_strHost.c_str(), m_nPort);
        return nRet;
    }

    // ������Ϣ��
    CMsgPackHeader& oHeader = oPack.m_oHeader;
    const char* pData = oPack.Data();
    switch(oHeader.nDataType)
    {
    case MSG_TYPE_TEXT:
        GLog("RT[%s:%d]%s", m_strHost.c_str(), m_nPort, pData);
        break;
    case MSG_TYPE_CMDS:
        {
            nRet = DealRecvCmd(oPack);
        }
        break;
    case MSG_TYPE_FILE:
        {
            if(m_fpFile == NULL)
            {
                GLog("�ļ�[%s]δ�򿪣�����[%s:%d]���ݱ���ʧ��", oHeader.pName, m_strHost.c_str(), m_nPort);
                break;
            }
            if(oHeader.nSeq == m_nSeq)
            {
                // �������ݵ��ļ�
                GLog("����[%s:%d]����д������[%d]����С[%d]�ֽ�", 
                    m_strHost.c_str(), m_nPort, oHeader.nSeq, oHeader.nSize-sizeof(oHeader));
                fwrite(oPack.Data(), oPack.DataSize(), 1, m_fpFile);

                // ���´�С�Ͱ���
                m_nCurRecvSize += oHeader.nSize-sizeof(oHeader);
                m_nSeq++;
                nRet = 0;
            }
            else
            {
                GLog("�ļ�����Ŵ�����������[%d],�ȴ������[%d]������[%s:%d]���ݱ���ʧ��", 
                    oHeader.nSeq, m_nSeq, oHeader.pName, m_strHost.c_str(), m_nPort);
            }
        }
        break;
    default:
        GLog("RU[%s:%d]%s", m_strHost.c_str(), m_nPort, pData);
        break;
    }
    // ����ʧ�ܣ�ֱ�ӷ���
    if(nRet)
    {
        return nRet;
    }
    // ��Ӧ���
    CMsgPack oAck;
    strncpy(oAck.m_oHeader.pSender, GetLocalHost().c_str(), sizeof(oAck.m_oHeader.pSender));
    oAck.m_oHeader.nPackType = MSG_PACK_TYPE_ACK;
    strcpy(oAck.m_oHeader.pRecver, oPack.m_oHeader.pSender);
    oAck.m_oHeader.nSeq = oPack.m_oHeader.nSeq + 1;
    oAck.m_oHeader.nFlag = oPack.m_oHeader.nFlag;
    oAck.m_oHeader.nDataType = oPack.m_oHeader.nDataType;
    oAck.m_oHeader.nFlag = oPack.m_oHeader.nFlag;
    oAck.m_oHeader.nTotalSize = oPack.m_oHeader.nTotalSize;
    Send(oAck.Pack(), oAck.PackSize());
    // GLog("����ACK[%d]��[%s:%d]", oAck.m_oHeader.nSeq, m_strHost.c_str(), m_nPort);
    return nRet;
}

int CNetFileClient::DealRecvCmd(CMsgPack& oPack)
{
    int nRet = -1;
    CMsgPackHeader& oHeader = oPack.m_oHeader;
    int nFlag = oPack.m_oHeader.nFlag;
    // ��ʼ�����ļ�
    if(nFlag == MSG_CMD_FBEGIN)
    {
        GLog("�յ��ļ���ʼ����ָ��");
        return CreateFile(oHeader);
    }
    // �ļ����ͽ���
    else if(nFlag == MSG_CMD_FEND)
    {
        GLog("�յ��ļ�ֹͣ����ָ��");
        return CloseFile();
    }
    return nRet;
}

int CNetFileClient::GetRecvBlock(char* pBuf, unsigned int nBufSize)
{
    //// ����У��
    //if(pBuf==NULL || nBufSize<=m_nBlockSize)
    //{
    //    return -1;
    //}

    //// ȡ��������
    //CRecvBuf* pRecv = NULL;
    //LockRecv();
    //if(!m_qRecvBuf.empty())
    //{
    //    // �Ӷ�����ȡ����
    //   pRecv  = m_qRecvBuf.front();
    //   m_qRecvBuf.pop();
    //}
    //UnlockRecv();
    //// ������ݲ�Ϊ��
    //if(pRecv)
    //{
    //    // ��������
    //    memcpy(pBuf, pRecv->m_pBuf, pRecv->m_nDataLen);
    //    nBufSize = pRecv->m_nDataLen;

    //    // �����ڴ�
    //    delete pRecv;
    //    return nBufSize;
    //}
    return 0;
}

int CNetFileClient::SendFile(const string& strFile)
{
    int nRet = 0;
    // ��ȡ�ļ���
    string strName, strExt;
    GetFileNameExtByPath(strFile, strName, strExt);
    strName += "." + strExt;

    // ���ļ�
    FILE* fp = NULL;
    fp = fopen(strFile.c_str(), "rb");
    if(fp == NULL)
    {
        GLog("���ļ�[%s]ʧ��", strFile.c_str());
        return false;
    }
    // ��ȡ����IP
    string strHost = GetLocalHost();

    // ���ð�ͷ��Ϣ
    CMsgPack oPack;
    CMsgPackHeader& oHeader = oPack.m_oHeader;
    strncpy(oHeader.pName, strName.c_str(), sizeof(oHeader.pName)-1);
    strncpy(oHeader.pSender, strHost.c_str(), sizeof(oHeader.pSender)-1);
    strncpy(oHeader.pRecver, m_strHost.c_str(), sizeof(oHeader.pRecver)-1);
    oHeader.nTotalSize = MGetFileSize(fp);

    int nNum = 0;
    int nSend = 0;
    INT64 nRawSendSize = 0;

    // �����ļ���ʼָ��
    oHeader.nDataType = MSG_TYPE_CMDS;
    oHeader.nFlag = MSG_CMD_FBEGIN;
    oHeader.nPackType = MSG_PACK_TYPE_REQ;
    oPack.SetBody(NULL, 0);
    GLog("��ʼ�����ļ�:%s, �ܴ�С:%I64d�ֽ�", strFile.c_str(), oHeader.nTotalSize);
    nNum = this->Send(oPack.Pack(), oPack.PackSize());
    nRawSendSize += oPack.PackSize();

    // �����ļ���ʼACK����
    CMsgPack oRecvPack;
    nRet = this->Recv(oRecvPack, 5000);
    if(nRet)
    {
        GLog("�����ļ���ʼָ��ACKʧ��");
        fclose(fp);
        return nRet;
    }
    CMsgPackHeader* pHeader = &oRecvPack.m_oHeader;
    if(pHeader->nPackType != MSG_PACK_TYPE_ACK 
        || (pHeader->nSeq != oHeader.nSeq+1)
        )
    {
        GLog("�����ļ���ʼָ��ACK���ݷǷ�");
        fclose(fp);
        return nRet;
    }

    // �����ļ�
    char* pBuf = new char[m_nBlockSize];
    oHeader.nDataType = MSG_TYPE_FILE;
    oHeader.nFlag = 0;
    INT64 nFileSize = 0;
    while((nNum = fread(pBuf, 1, m_nBlockSize-sizeof(CMsgPackHeader), fp)) > 0)
    {
        // ���ð��ź�����
        oHeader.nSeq++;
        oPack.SetBody(pBuf, nNum);

        // ��������
        nFileSize += nNum;
        nSend += Send(oPack.Pack(), oPack.PackSize());
        nRawSendSize += oPack.PackSize();
        GLog("�ѷ���[%s]���ݣ�%d �ֽ� ������:%d ���ݰ���С:%I64d �ֽ�", oHeader.pRecver, nSend, oHeader.nSeq, nRawSendSize);
        
        // ����Ӧ���
        nRet = this->Recv(oRecvPack, 5000);
        if(nRet)
        {
            GLog("�����ļ����ݷ���ACKʧ��");
            break;
        }
    }
    fclose(fp);

    // �����ļ�����ָ��
    oHeader.nDataType = MSG_TYPE_CMDS;
    oHeader.nFlag = MSG_CMD_FEND;
    oPack.SetBody(NULL, 0);
    nNum = Send(oPack.Pack(), oPack.PackSize());
    nRawSendSize += oPack.PackSize();
    GLog("���������ļ�[%s] �ļ���С[%I64d]�ֽ� �ܷ������ݴ�С[%I64d]�ֽ�", strFile.c_str(), nFileSize, nRawSendSize);
    // ����Ӧ���
    nRet = this->Recv(oRecvPack, 5000);
    if(nRet)
    {
        GLog("�����ļ�����ָ��ACKʧ��");
    }

    return 0;
}

int CNetFileClient::SendMsg(const string& strMsg)
{
    int nRet = 0;
    // ��ȡ����IP
    string strHost = GetLocalHost();

    // ���ð�ͷ��Ϣ
    CMsgPack oPack;
    CMsgPackHeader& oHeader = oPack.m_oHeader;
    strncpy(oHeader.pName, "Message Send", sizeof(oHeader.pName)-1);
    strncpy(oHeader.pSender, strHost.c_str(), sizeof(oHeader.pSender)-1);
    strncpy(oHeader.pRecver, m_strHost.c_str(), sizeof(oHeader.pRecver)-1);
    // ������Ϣ
    oHeader.nDataType = MSG_TYPE_TEXT;
    oHeader.nPackType = MSG_PACK_TYPE_REQ;
    oPack.SetBody(strMsg.c_str(), strMsg.size());
    this->Send(oPack.Pack(), oPack.PackSize());
    GLog("������Ϣ:%s, �ܴ�С:%d�ֽ�", strMsg.c_str(), oHeader.nSize);
    
    return 0;
}

int CNetFileClient::Send(const char* pBuf, int nLen)
{
    int nRet = 0;

    // ��������
    nRet = ::send(m_hSocket, pBuf, nLen, 0);

    return nRet;
}

int CNetFileClient::Recv(CMsgPack& oPack, int nMiSeconds/*=0*/)
{
    // ȡ����
    int nRet = -1;
    const int nSleep = 100;
    do
    {
        // ����Ƿ�������
        if(m_qAck.empty())
        {
            nMiSeconds -= nSleep;
            if(nMiSeconds > 0)
            {
                Sleep(nSleep);
                continue;
            }
            return -1;
        }
        else
        {
            // ȡ����
            LockRecv();
            if(!m_qAck.empty())
            {
                oPack = m_qAck.front();
                m_qAck.pop();
                nRet = 0;
            }
            UnlockRecv();
        }
    }while(nRet);
    return nRet;
}

int CNetFileClient::Recv(int nMiSeconds/*=0*/)
{
    //// �ж��Ƿ������ݣ�������ֱ�ӷ���
    //if(m_qRecvBuf.empty())
    //{
    //    return 0;
    //}

    //// �ڴ洦��
    //unsigned int nRecvLen = m_nBlockSize*2;
    //char* pBuf = new char[nRecvLen];
    //int nLen = 0;
    //memset(pBuf, 0, nRecvLen);
    //char* pRecvBuf = new char[m_nBlockSize];
    //int nFlag = MSG_CMD_FEND;

    //// ����������ݻỰ
    //CMsgPack oRecv;
    //CMsgPack oSend;
    //int nRet = 0;
    //FILE* fp = NULL;
    //const int nSleep = 100;
    //int nWait = nMiSeconds;

    //// ���÷������ݰ�
    //strncpy(oSend.m_oHeader.pSender, GetLocalHost().c_str(), sizeof(oSend.m_oHeader.pSender));
    //oSend.m_oHeader.nPackType = MSG_PACK_TYPE_REQ;

    //while(true)
    //{
    //    // ȡ����
    //    int nRet = GetRecvBlock(pRecvBuf, m_nBlockSize);
    //    if(nRet <= 0)
    //    {
    //        nWait -= nSleep;
    //        // δ������δ��ʱ�������ȴ�
    //        if(nFlag!=MSG_CMD_FEND || nWait>0)
    //        {
    //            Sleep(nSleep);
    //            continue;
    //        }
    //        break;
    //    }
    //    nWait = nMiSeconds;

    //    // ����ƴ��
    //    memcpy(pBuf+nLen, pRecvBuf, nRet);
    //    nLen += nRet;

    //    // ���ݽ���
    //    if(!oRecv.LoadPack(pBuf, nLen))
    //    {
    //        GLog("�յ��Ƿ�����");
    //        break;
    //    }
    //    nLen -= oRecv.PackSize();

    //    CMsgPackHeader& oHeader = oRecv.m_oHeader;
    //    if(oHeader.nPackType == MSG_PACK_TYPE_ACK)
    //    {
    //        continue;
    //    }
    //    if(oHeader.nDataType = MSG_TYPE_CMDS)
    //    {
    //        // ��ʼ�����ļ�
    //        if(oHeader.nFlag == MSG_CMD_FBEGIN)
    //        {
    //            // �ļ������
    //            m_nCurRecvSize = 0;
    //            if(oHeader.pName[0] == '\0')
    //            {
    //                GLog("�յ�[%s:%d]�ļ���Ϣ��������Ϊ�գ��Ѻ���", m_strHost.c_str(), m_nPort);
    //                return false;
    //            }
    //            // �ر��ļ�
    //            fclose(fp);
    //            fp = fopen(oHeader.pName, "wb");
    //            if(fp == NULL)
    //            {
    //                GLog("�½�����[%s:%d]�ļ�[%s]ʧ�ܣ�", m_strHost.c_str(), m_nPort, oHeader.pName);
    //                break;
    //            }
    //            else
    //            {
    //                GLog("�½�����[%s:%d]�ļ�[%s]�ɹ����ļ���С[%I64d]�ֽڣ�׼�������ļ�����",
    //                    m_strHost.c_str(), m_nPort, oHeader.pName, oHeader.nTotalSize);
    //                oSend.m_oHeader.nSeq = oRecv.m_oHeader.nSeq;
    //            }
    //        }
    //        // ���������ļ�
    //        else if(oHeader.nFlag == MSG_CMD_FEND)
    //        {
    //            GLog("�ر�����[%s:%d]�ļ�[%s]�������ļ���С[%I64d]�ֽ� ���ս��:%s", 
    //                m_strHost.c_str(), m_nPort, oHeader.pName, 
    //                m_nCurRecvSize, (m_nCurRecvSize==oHeader.nTotalSize ? "�ɹ�":"ʧ��"));
    //            fclose(fp);
    //            fp = NULL;
    //            break;
    //        }
    //        else
    //        {
    //            continue;
    //        }
    //    }
    //    else
    //    {
    //        if(fp == NULL)
    //        {
    //            GLog("�½��ļ�[%s]ʧ�ܣ�����[%s:%d]���ݱ���ʧ��", oHeader.pName, m_strHost.c_str(), m_nPort);
    //            break;
    //        }
    //        if(oHeader.nSeq == oSend.m_oHeader.nSeq)
    //        {
    //            // �������ݵ��ļ�
    //            GLog("����[%s:%d]����д������[%d]����С[%d]�ֽ�", 
    //                m_strHost.c_str(), m_nPort, oHeader.nSeq, oHeader.nSize-sizeof(oHeader));
    //            fwrite(oRecv.Data(), oRecv.DataSize(), 1, fp);
    //            
    //            // ���´�С�Ͱ���
    //            m_nCurRecvSize += oHeader.nSize-sizeof(oHeader);
    //            oSend.m_oHeader.nSeq++;
    //        }
    //        else
    //        {
    //            GLog("�ļ�����Ŵ�����������[%d],�ȴ������[%d]������[%s:%d]���ݱ���ʧ��", 
    //                oHeader.nSeq, oSend.m_oHeader.nSeq,
    //                oHeader.pName, m_strHost.c_str(), m_nPort);
    //            break;
    //        }
    //    }

    //    // ����ACK
    //    strcpy(oSend.m_oHeader.pRecver, oRecv.m_oHeader.pSender);
    //    oSend.m_oHeader.nSeq++;
    //    oSend.m_oHeader.nFlag = oRecv.m_oHeader.nFlag;
    //    oSend.m_oHeader.nDataType = oRecv.m_oHeader.nDataType;
    //    oSend.m_oHeader.nPackType = oRecv.m_oHeader.nPackType;
    //    oSend.m_oHeader.nFlag = oRecv.m_oHeader.nFlag;
    //    oSend.m_oHeader.nTotalSize = oRecv.m_oHeader.nTotalSize;
    //    Send(oSend.Pack(), oSend.PackSize());
    //}

    return 0;
}

bool CNetFileClient::IsInvalid()
{
    return m_oRecvProc.IsRunning();
}

void CNetFileClient::LockRecv()
{
    EnterCriticalSection(&m_csRecv);
}

void CNetFileClient::UnlockRecv()
{
    LeaveCriticalSection(&m_csRecv);
}

unsigned int __stdcall NetFileRecvProc(void* pPara)
{
    CNetFileClient* pNet = (CNetFileClient*)pPara;
    pNet->OnRecv();
    return 0;
}

int CNetFileClient::Connect(const string& strHost, int nPort)
{
    int nRet = 0;
    //��ʼ��������˿ں�
    m_hSocket = socket(AF_INET,SOCK_STREAM,0);
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr=inet_addr(strHost.c_str());
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(nPort);

    // ���ӷ�����
    nRet = ::connect(m_hSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    if(nRet)
    {
        nRet = WSAGetLastError();
        GLog("���ӷ�����[%s:%d]ʧ�ܣ�������룺%d", strHost.c_str(), nPort, nRet);
        return nRet;
    }
    GLog("���ӷ�����[%s:%d]�ɹ�", strHost.c_str(), nPort);

    // ���ý����߳�
    if(!m_oRecvProc.RunTask(NetFileRecvProc, this, &m_bRun))
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        GLog("����[%s:%d]���ݽ����߳�ʧ��", strHost.c_str(), nPort);
        return -1;
    }
    GLog("����[%s:%d]���ݽ����̳߳ɹ�", strHost.c_str(), nPort);

    // ��ʼ����Ա����
    m_strHost = strHost;
    m_nPort = nPort;

    return nRet;
}

int CNetFileClient::Init(SOCKET& socket, const string& strHost, int nPort)
{
    m_hSocket = socket;
    m_strHost = strHost;
    m_nPort = nPort;

    // ���ý����߳�
    if(!m_oRecvProc.RunTask(NetFileRecvProc, this, &m_bRun))
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        GLog("����[%s:%d]���ݽ����߳�ʧ��", strHost.c_str(), nPort);
        return -1;
    }
    GLog("����[%s:%d]���ݽ����̳߳ɹ�", strHost.c_str(), nPort);

    return 0;
}

CNetFile::CNetFile()
{
    m_strHost = "127.0.0.1";
    m_nPort = 0;
    m_hSocket = INVALID_SOCKET;
    m_bRun = false;
}

CNetFile::~CNetFile()
{
    // ����ֹͣ����
    m_bRun = false;
}

std::string CNetFile::GetAddr() const
{
    char szBuf[128] = {0};
    sprintf(szBuf, "%s:%d", m_strHost.c_str(), m_nPort);
    return szBuf;
}

int CNetFileClient::DisConnect()
{
    if(m_hSocket)
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        m_strHost = "";
        m_nPort = 0;
    }
    return 0;
}

int CNetFileClient::CreateFile(const CMsgPackHeader& oHeader)
{
    // �ļ������
    m_nCurRecvSize = 0;
    if(oHeader.pName[0] == '\0')
    {
        GLog("�յ�[%s:%d]�ļ���Ϣ��������Ϊ�գ��Ѻ���", m_strHost.c_str(), m_nPort);
        return -1;
    }
    // �ر��ļ�
    CloseFile();
    string strFilePath = "./";
    strFilePath += oHeader.pName;
    m_fpFile = fopen(strFilePath.c_str(), "wb");
    if(m_fpFile == NULL)
    {
        GLog("�½�����[%s:%d]�ļ�[%s]ʧ�ܣ�", m_strHost.c_str(), m_nPort, strFilePath.c_str());
        return -1;
    }
    else
    {
        GLog("�½�����[%s:%d]�ļ�[%s]�ɹ����ļ���С[%I64d]�ֽڣ�׼�������ļ�����",
            m_strHost.c_str(), m_nPort, strFilePath.c_str(), oHeader.nTotalSize);
    }
    m_nSeq = oHeader.nSeq + 1;
    m_nCurRecvSize = 0;
    m_nTotalSize = oHeader.nTotalSize;
    return 0;
}

int CNetFileClient::CloseFile()
{
    if(m_fpFile)
    {
        GLog("�رս�������[%s:%d]�ļ��������ļ���С[%I64d]�ֽ� ���ս��:%s", 
            m_strHost.c_str(), m_nPort, m_nCurRecvSize, (m_nCurRecvSize==m_nTotalSize ? "�ɹ�":"ʧ��"));
        fclose(m_fpFile);
        m_fpFile = NULL;
    }
    m_nSeq = 0;
    m_nCurRecvSize = 0;
    m_nTotalSize = 0;
    return 0;
}
