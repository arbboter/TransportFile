#include "StdAfx.h"
#include "NetFile.h"
#include "Task.h"

CNetFileServer::CNetFileServer()
{
}


CNetFileServer::~CNetFileServer(void)
{
}

// 服务接收处理子线程
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
            GLog("收到了错误的连接请求，已忽略");
            continue;
        }
        // 获取连接地址信息
        pHost = inet_ntoa(addrCli.sin_addr);
        nPort = ntohs(addrCli.sin_port);
        GLog("收到[%s:%d]连接请求", pHost, nPort);

        // 保存客户端连接
        CNetFileClient* pCli = new CNetFileClient();
        pCli->Init(socketCli, pHost, nPort);
        m_mapClient[socketCli] = pCli;
        NotifySocketChange(pCli, SOCK_CHG_ACCEPT);
        GLog("已处理[%s:%d]连接请求", pHost, nPort);

        // 更新客户端
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
    // 更新客户端列表
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

    // 检查是否已启动
    if(m_hSocket != INVALID_SOCKET)
    {
        return 0;
    }

    m_hSocket = socket(AF_INET,SOCK_STREAM,0);
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port=htons(nPort);

    // 绑定端口
    nRet = bind(m_hSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    if(nRet)
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        nRet = WSAGetLastError();
        GLog("绑定到端口[%d]失败，请检查端口是否已被占用", nPort);
        return nRet;
    }

    // 监听端口
    nRet =  listen(m_hSocket, 10);
    if(nRet)
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        nRet = WSAGetLastError();
        GLog("服务端口[%d]监听失败，请检查端口是否已被占用", nPort);
        return nRet;
    }
    // 创建接收子线程
    if(!m_oAcceptProc.RunTask(ServerAcceptProc, this, &m_bRun))
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        nRet = -1;
        GLog("创建接收线程失败");
        return nRet;
    }
    GLog("服务启动成功，监听端口号:%d", nPort);
    m_nPort = nPort;

    return nRet;
}

void CNetFileServer::Close()
{
    m_bRun = false;
    m_oAcceptProc.Stop();
    // 关闭已连接的客户端
    map<SOCKET, CNetFileClient*>::iterator e = m_mapClient.end();
    for (e=m_mapClient.begin(); e!=m_mapClient.end(); ++e)
    {
        e->second->Close();
        delete e->second;
    }
    m_mapClient.clear();

    // 关闭监听SOCKET
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
    // 初始化变量
    m_nTotalSize = 0;
    m_nCurRecvSize = 0;
    m_nSeq = 0;
    m_fpFile = NULL;
    InitializeCriticalSection(&m_csRecv);
}

CNetFileClient::~CNetFileClient(void)
{
    // 删除临界区
    DeleteCriticalSection(&m_csRecv);
}

void CNetFileClient::Close()
{
    m_bRun = false;
    m_oRecvProc.Stop();
    if(m_hSocket)
    {
        GLog("断开主机[%s:%d]的连接", m_strHost.c_str(), m_nPort);
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
    // 接收数据
    CRecvBuf* pRecv = new CRecvBuf(m_nBlockSize);
    CRecvBuf* pData = new CRecvBuf(m_nBlockSize*2);
    int nRet = 0;
    while(m_oRecvProc.IsRunning())
    {
        // 分配接收内存
        pRecv->m_nDataLen = ::recv(m_hSocket, pRecv->m_pBuf, pRecv->m_nBufLen, 0);
        if(pRecv->m_nDataLen <= 0)
        {
            nRet = WSAGetLastError();
            // SOCKET关闭
            if(nRet==10054 || nRet==0) 
            {
                NotifySocketChange(this, SOCK_CHG_CLOSE);
                Close();
                return 0;
            }
            GLog("主机[%s:%d]数据接收失败，错误码:%d", m_strHost.c_str(), m_nPort, nRet);
            Sleep(1000);
            continue;
        }
        NotifySocketChange(this, SOCK_CHG_RECV);
        // GLog("收到主机[%s:%d]数据，数据大小:%d字节", m_strHost.c_str(), m_nPort, pRecv->m_nDataLen);

        // 数据拼接
        if(!pData->Append(pRecv->m_pBuf, pRecv->m_nDataLen))
        {
            GLog("主机[%s:%d]数据处理出错，数据太大:%d", m_strHost.c_str(), m_nPort, pRecv->m_nDataLen);
        }

        // 处理接收的数据
        // 本程序通信报文
        if(CMsgPack::IsThisPack(pData->m_pBuf, pData->m_nDataLen))
        {
            // 拆包
            char* pCur = pData->m_pBuf;
            int nCurLen = pData->m_nDataLen;
            CMsgPack oPack;
            while(CMsgPack::SplitPack(pCur, nCurLen, oPack))
            {
                // 数据移位
                pCur += oPack.m_oHeader.nSize;
                nCurLen -= oPack.m_oHeader.nSize;

                // 处理收包
                nRet = DealRecvPack(oPack);
                if(!nRet)
                {
                    break;
                }
            }
            // 数据重置
            pData->Pop(pData->m_nDataLen - nCurLen);
        }
        // 不识别的报文
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
    // 加载解析消息包
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
    
    // ACK先缓存起来
    if(oPack.m_oHeader.nPackType == MSG_PACK_TYPE_ACK)
    {
        LockRecv();
        m_qAck.push(oPack);
        UnlockRecv();
        return 0;
    }

    // 只允许处理请求包
    if(oPack.m_oHeader.nPackType != MSG_PACK_TYPE_REQ)
    {
        GLog("处理[%s:%d]主机错误，非请求包", m_strHost.c_str(), m_nPort);
        return nRet;
    }

    // 处理消息包
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
                GLog("文件[%s]未打开，主机[%s:%d]数据保存失败", oHeader.pName, m_strHost.c_str(), m_nPort);
                break;
            }
            if(oHeader.nSeq == m_nSeq)
            {
                // 保存数据到文件
                GLog("主机[%s:%d]数据写入包序号[%d]，大小[%d]字节", 
                    m_strHost.c_str(), m_nPort, oHeader.nSeq, oHeader.nSize-sizeof(oHeader));
                fwrite(oPack.Data(), oPack.DataSize(), 1, m_fpFile);

                // 更新大小和包号
                m_nCurRecvSize += oHeader.nSize-sizeof(oHeader);
                m_nSeq++;
                nRet = 0;
            }
            else
            {
                GLog("文件包序号错误，请求包序号[%d],等待包序号[%d]，主机[%s:%d]数据保存失败", 
                    oHeader.nSeq, m_nSeq, oHeader.pName, m_strHost.c_str(), m_nPort);
            }
        }
        break;
    default:
        GLog("RU[%s:%d]%s", m_strHost.c_str(), m_nPort, pData);
        break;
    }
    // 处理失败，直接返回
    if(nRet)
    {
        return nRet;
    }
    // 会应答包
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
    // GLog("发送ACK[%d]至[%s:%d]", oAck.m_oHeader.nSeq, m_strHost.c_str(), m_nPort);
    return nRet;
}

int CNetFileClient::DealRecvCmd(CMsgPack& oPack)
{
    int nRet = -1;
    CMsgPackHeader& oHeader = oPack.m_oHeader;
    int nFlag = oPack.m_oHeader.nFlag;
    // 开始发送文件
    if(nFlag == MSG_CMD_FBEGIN)
    {
        GLog("收到文件开始发送指令");
        return CreateFile(oHeader);
    }
    // 文件发送结束
    else if(nFlag == MSG_CMD_FEND)
    {
        GLog("收到文件停止发送指令");
        return CloseFile();
    }
    return nRet;
}

int CNetFileClient::GetRecvBlock(char* pBuf, unsigned int nBufSize)
{
    //// 参数校验
    //if(pBuf==NULL || nBufSize<=m_nBlockSize)
    //{
    //    return -1;
    //}

    //// 取缓存数据
    //CRecvBuf* pRecv = NULL;
    //LockRecv();
    //if(!m_qRecvBuf.empty())
    //{
    //    // 从队列中取数据
    //   pRecv  = m_qRecvBuf.front();
    //   m_qRecvBuf.pop();
    //}
    //UnlockRecv();
    //// 如果数据不为空
    //if(pRecv)
    //{
    //    // 拷贝数据
    //    memcpy(pBuf, pRecv->m_pBuf, pRecv->m_nDataLen);
    //    nBufSize = pRecv->m_nDataLen;

    //    // 清理内存
    //    delete pRecv;
    //    return nBufSize;
    //}
    return 0;
}

int CNetFileClient::SendFile(const string& strFile)
{
    int nRet = 0;
    // 获取文件名
    string strName, strExt;
    GetFileNameExtByPath(strFile, strName, strExt);
    strName += "." + strExt;

    // 打开文件
    FILE* fp = NULL;
    fp = fopen(strFile.c_str(), "rb");
    if(fp == NULL)
    {
        GLog("打开文件[%s]失败", strFile.c_str());
        return false;
    }
    // 获取本机IP
    string strHost = GetLocalHost();

    // 设置包头信息
    CMsgPack oPack;
    CMsgPackHeader& oHeader = oPack.m_oHeader;
    strncpy(oHeader.pName, strName.c_str(), sizeof(oHeader.pName)-1);
    strncpy(oHeader.pSender, strHost.c_str(), sizeof(oHeader.pSender)-1);
    strncpy(oHeader.pRecver, m_strHost.c_str(), sizeof(oHeader.pRecver)-1);
    oHeader.nTotalSize = MGetFileSize(fp);

    int nNum = 0;
    int nSend = 0;
    INT64 nRawSendSize = 0;

    // 发送文件开始指令
    oHeader.nDataType = MSG_TYPE_CMDS;
    oHeader.nFlag = MSG_CMD_FBEGIN;
    oHeader.nPackType = MSG_PACK_TYPE_REQ;
    oPack.SetBody(NULL, 0);
    GLog("开始发送文件:%s, 总大小:%I64d字节", strFile.c_str(), oHeader.nTotalSize);
    nNum = this->Send(oPack.Pack(), oPack.PackSize());
    nRawSendSize += oPack.PackSize();

    // 接收文件开始ACK数据
    CMsgPack oRecvPack;
    nRet = this->Recv(oRecvPack, 5000);
    if(nRet)
    {
        GLog("接收文件开始指令ACK失败");
        fclose(fp);
        return nRet;
    }
    CMsgPackHeader* pHeader = &oRecvPack.m_oHeader;
    if(pHeader->nPackType != MSG_PACK_TYPE_ACK 
        || (pHeader->nSeq != oHeader.nSeq+1)
        )
    {
        GLog("接收文件开始指令ACK数据非法");
        fclose(fp);
        return nRet;
    }

    // 发送文件
    char* pBuf = new char[m_nBlockSize];
    oHeader.nDataType = MSG_TYPE_FILE;
    oHeader.nFlag = 0;
    INT64 nFileSize = 0;
    while((nNum = fread(pBuf, 1, m_nBlockSize-sizeof(CMsgPackHeader), fp)) > 0)
    {
        // 设置包号和数据
        oHeader.nSeq++;
        oPack.SetBody(pBuf, nNum);

        // 发送数据
        nFileSize += nNum;
        nSend += Send(oPack.Pack(), oPack.PackSize());
        nRawSendSize += oPack.PackSize();
        GLog("已发送[%s]数据：%d 字节 ，包号:%d 数据包大小:%I64d 字节", oHeader.pRecver, nSend, oHeader.nSeq, nRawSendSize);
        
        // 接收应答包
        nRet = this->Recv(oRecvPack, 5000);
        if(nRet)
        {
            GLog("接收文件数据发送ACK失败");
            break;
        }
    }
    fclose(fp);

    // 发送文件结束指令
    oHeader.nDataType = MSG_TYPE_CMDS;
    oHeader.nFlag = MSG_CMD_FEND;
    oPack.SetBody(NULL, 0);
    nNum = Send(oPack.Pack(), oPack.PackSize());
    nRawSendSize += oPack.PackSize();
    GLog("结束发送文件[%s] 文件大小[%I64d]字节 总发送数据大小[%I64d]字节", strFile.c_str(), nFileSize, nRawSendSize);
    // 接收应答包
    nRet = this->Recv(oRecvPack, 5000);
    if(nRet)
    {
        GLog("接收文件结束指令ACK失败");
    }

    return 0;
}

int CNetFileClient::SendMsg(const string& strMsg)
{
    int nRet = 0;
    // 获取本机IP
    string strHost = GetLocalHost();

    // 设置包头信息
    CMsgPack oPack;
    CMsgPackHeader& oHeader = oPack.m_oHeader;
    strncpy(oHeader.pName, "Message Send", sizeof(oHeader.pName)-1);
    strncpy(oHeader.pSender, strHost.c_str(), sizeof(oHeader.pSender)-1);
    strncpy(oHeader.pRecver, m_strHost.c_str(), sizeof(oHeader.pRecver)-1);
    // 发送消息
    oHeader.nDataType = MSG_TYPE_TEXT;
    oHeader.nPackType = MSG_PACK_TYPE_REQ;
    oPack.SetBody(strMsg.c_str(), strMsg.size());
    this->Send(oPack.Pack(), oPack.PackSize());
    GLog("发送消息:%s, 总大小:%d字节", strMsg.c_str(), oHeader.nSize);
    
    return 0;
}

int CNetFileClient::Send(const char* pBuf, int nLen)
{
    int nRet = 0;

    // 发送数据
    nRet = ::send(m_hSocket, pBuf, nLen, 0);

    return nRet;
}

int CNetFileClient::Recv(CMsgPack& oPack, int nMiSeconds/*=0*/)
{
    // 取数据
    int nRet = -1;
    const int nSleep = 100;
    do
    {
        // 检查是否有数据
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
            // 取数据
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
    //// 判断是否有数据，无数据直接返回
    //if(m_qRecvBuf.empty())
    //{
    //    return 0;
    //}

    //// 内存处理
    //unsigned int nRecvLen = m_nBlockSize*2;
    //char* pBuf = new char[nRecvLen];
    //int nLen = 0;
    //memset(pBuf, 0, nRecvLen);
    //char* pRecvBuf = new char[m_nBlockSize];
    //int nFlag = MSG_CMD_FEND;

    //// 处理接收数据会话
    //CMsgPack oRecv;
    //CMsgPack oSend;
    //int nRet = 0;
    //FILE* fp = NULL;
    //const int nSleep = 100;
    //int nWait = nMiSeconds;

    //// 设置发送数据包
    //strncpy(oSend.m_oHeader.pSender, GetLocalHost().c_str(), sizeof(oSend.m_oHeader.pSender));
    //oSend.m_oHeader.nPackType = MSG_PACK_TYPE_REQ;

    //while(true)
    //{
    //    // 取数据
    //    int nRet = GetRecvBlock(pRecvBuf, m_nBlockSize);
    //    if(nRet <= 0)
    //    {
    //        nWait -= nSleep;
    //        // 未结束或未超时，继续等待
    //        if(nFlag!=MSG_CMD_FEND || nWait>0)
    //        {
    //            Sleep(nSleep);
    //            continue;
    //        }
    //        break;
    //    }
    //    nWait = nMiSeconds;

    //    // 数据拼接
    //    memcpy(pBuf+nLen, pRecvBuf, nRet);
    //    nLen += nRet;

    //    // 数据解析
    //    if(!oRecv.LoadPack(pBuf, nLen))
    //    {
    //        GLog("收到非法数据");
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
    //        // 开始接收文件
    //        if(oHeader.nFlag == MSG_CMD_FBEGIN)
    //        {
    //            // 文件名检查
    //            m_nCurRecvSize = 0;
    //            if(oHeader.pName[0] == '\0')
    //            {
    //                GLog("收到[%s:%d]文件消息，包名字为空，已忽略", m_strHost.c_str(), m_nPort);
    //                return false;
    //            }
    //            // 关闭文件
    //            fclose(fp);
    //            fp = fopen(oHeader.pName, "wb");
    //            if(fp == NULL)
    //            {
    //                GLog("新建主机[%s:%d]文件[%s]失败，", m_strHost.c_str(), m_nPort, oHeader.pName);
    //                break;
    //            }
    //            else
    //            {
    //                GLog("新建主机[%s:%d]文件[%s]成功，文件大小[%I64d]字节，准备接收文件数据",
    //                    m_strHost.c_str(), m_nPort, oHeader.pName, oHeader.nTotalSize);
    //                oSend.m_oHeader.nSeq = oRecv.m_oHeader.nSeq;
    //            }
    //        }
    //        // 结束接收文件
    //        else if(oHeader.nFlag == MSG_CMD_FEND)
    //        {
    //            GLog("关闭主机[%s:%d]文件[%s]，已收文件大小[%I64d]字节 接收结果:%s", 
    //                m_strHost.c_str(), m_nPort, oHeader.pName, 
    //                m_nCurRecvSize, (m_nCurRecvSize==oHeader.nTotalSize ? "成功":"失败"));
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
    //            GLog("新建文件[%s]失败，主机[%s:%d]数据保存失败", oHeader.pName, m_strHost.c_str(), m_nPort);
    //            break;
    //        }
    //        if(oHeader.nSeq == oSend.m_oHeader.nSeq)
    //        {
    //            // 保存数据到文件
    //            GLog("主机[%s:%d]数据写入包序号[%d]，大小[%d]字节", 
    //                m_strHost.c_str(), m_nPort, oHeader.nSeq, oHeader.nSize-sizeof(oHeader));
    //            fwrite(oRecv.Data(), oRecv.DataSize(), 1, fp);
    //            
    //            // 更新大小和包号
    //            m_nCurRecvSize += oHeader.nSize-sizeof(oHeader);
    //            oSend.m_oHeader.nSeq++;
    //        }
    //        else
    //        {
    //            GLog("文件包序号错误，请求包序号[%d],等待包序号[%d]，主机[%s:%d]数据保存失败", 
    //                oHeader.nSeq, oSend.m_oHeader.nSeq,
    //                oHeader.pName, m_strHost.c_str(), m_nPort);
    //            break;
    //        }
    //    }

    //    // 发送ACK
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
    //初始化连接与端口号
    m_hSocket = socket(AF_INET,SOCK_STREAM,0);
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr=inet_addr(strHost.c_str());
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(nPort);

    // 连接服务器
    nRet = ::connect(m_hSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    if(nRet)
    {
        nRet = WSAGetLastError();
        GLog("连接服务器[%s:%d]失败，错误代码：%d", strHost.c_str(), nPort, nRet);
        return nRet;
    }
    GLog("连接服务器[%s:%d]成功", strHost.c_str(), nPort);

    // 启用接收线程
    if(!m_oRecvProc.RunTask(NetFileRecvProc, this, &m_bRun))
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        GLog("创建[%s:%d]数据接收线程失败", strHost.c_str(), nPort);
        return -1;
    }
    GLog("创建[%s:%d]数据接收线程成功", strHost.c_str(), nPort);

    // 初始化成员变量
    m_strHost = strHost;
    m_nPort = nPort;

    return nRet;
}

int CNetFileClient::Init(SOCKET& socket, const string& strHost, int nPort)
{
    m_hSocket = socket;
    m_strHost = strHost;
    m_nPort = nPort;

    // 启用接收线程
    if(!m_oRecvProc.RunTask(NetFileRecvProc, this, &m_bRun))
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        GLog("创建[%s:%d]数据接收线程失败", strHost.c_str(), nPort);
        return -1;
    }
    GLog("创建[%s:%d]数据接收线程成功", strHost.c_str(), nPort);

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
    // 设置停止运行
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
    // 文件名检查
    m_nCurRecvSize = 0;
    if(oHeader.pName[0] == '\0')
    {
        GLog("收到[%s:%d]文件消息，包名字为空，已忽略", m_strHost.c_str(), m_nPort);
        return -1;
    }
    // 关闭文件
    CloseFile();
    string strFilePath = "./";
    strFilePath += oHeader.pName;
    m_fpFile = fopen(strFilePath.c_str(), "wb");
    if(m_fpFile == NULL)
    {
        GLog("新建主机[%s:%d]文件[%s]失败，", m_strHost.c_str(), m_nPort, strFilePath.c_str());
        return -1;
    }
    else
    {
        GLog("新建主机[%s:%d]文件[%s]成功，文件大小[%I64d]字节，准备接收文件数据",
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
        GLog("关闭接收主机[%s:%d]文件，已收文件大小[%I64d]字节 接收结果:%s", 
            m_strHost.c_str(), m_nPort, m_nCurRecvSize, (m_nCurRecvSize==m_nTotalSize ? "成功":"失败"));
        fclose(m_fpFile);
        m_fpFile = NULL;
    }
    m_nSeq = 0;
    m_nCurRecvSize = 0;
    m_nTotalSize = 0;
    return 0;
}
