// PlayList.cpp : implementation file
//

#include "stdafx.h"
#include "VPlayer.h"
#include "PlayList.h"
#include "afxdialogex.h"
#include "VPlayerDlg.h"

// 生成随机数[nMin, nMax)
int RandInt(int nMin, int nMax)
{
    // 初始化随机数种子，单次
    static bool bInitRand = false;
    if(!bInitRand)
    {
        srand(GetTickCount());
        bInitRand = true;
    }
    if(nMax <= nMin) return nMin;
    return rand()%(nMax-nMin) + nMin;
}

int CPlayList::Add(const CPlayItem& oItem)
{
    m_arItems.Add(oItem);
    int nSeq = m_arItems.GetCount();
    CString strInfo;
    strInfo.Format(L"%03d ", nSeq);
    m_vList.AddString(strInfo + oItem.m_strName);
    if(m_nCurSel < 0)
    {
        m_nCurSel = nSeq - 1;
    }
    return nSeq;
}

int CPlayList::Add(const CString& strPath)
{
    CPlayItem oItem;

    oItem.m_strName = PathFindFileName(strPath);
    oItem.m_strPath = strPath;
    return Add(oItem);
}

void CPlayList::Remove(int nSeq)
{
    if(nSeq<0 || nSeq>=m_arItems.GetCount())
    {
        return;
    }
    m_arItems.RemoveAt(nSeq);
    m_vList.DeleteString(nSeq);
    if(nSeq==m_nCurSel || m_arItems.IsEmpty())
    {
        Next();
    }
}

void CPlayList::SetCurSel(int nSeq)
{
    if(nSeq<0 || nSeq>=m_arItems.GetCount())
    {
        return;
    }
    m_nCurSel = nSeq;
    m_vList.SetCurSel(nSeq);
}

int CPlayList::GetCurSel(CPlayItem& oItem)
{
    int nCurSel = GetCurSel();
    if(nCurSel < 0)
    {
        return nCurSel;
    }
    oItem = m_arItems.GetAt(nCurSel);
    return nCurSel;
}

int CPlayList::GetCurSel()
{
    return m_nCurSel;
}


int CPlayList::Next()
{
    switch(m_nPlayMode)
    {
    case PLAY_MODE_ORDER:
        // 播放尾部后停止
        m_nCurSel += 1;
        if(m_nCurSel >= m_arItems.GetCount())
        {
            m_nCurSel -= 1;
        }
        break;
    case PLAY_MODE_RAND:
        m_nCurSel = (m_arItems.IsEmpty() ? -1 : RandInt(0, m_arItems.GetCount()));
        break;
    case PLAY_MODE_CYCLE:
        // 播放尾部后重新开始
        m_nCurSel += 1;
        if(m_nCurSel >= m_arItems.GetCount())
        {
            m_nCurSel = 0;
        }
        break;
    }
    SetCurSel(m_nCurSel);
    return m_nCurSel;
}

int CPlayList::Last()
{
    switch(m_nPlayMode)
    {
    case PLAY_MODE_ORDER:
        // 播放头部后停止
        m_nCurSel -= 1;
        if(m_nCurSel < 0)
        {
            m_nCurSel += 1;
        }
        break;
    case PLAY_MODE_RAND:
        m_nCurSel = (m_arItems.IsEmpty() ? -1 : RandInt(0, m_arItems.GetCount()));
        break;
    case PLAY_MODE_CYCLE:
        // 播放尾部后重新开始
        m_nCurSel -= 1;
        if(m_nCurSel < 0)
        {
            m_nCurSel = m_arItems.GetCount() - 1;
        }
        break;
    }
    SetCurSel(m_nCurSel);
    return m_nCurSel;
}

int CPlayList::ItemCount()
{
    return m_arItems.GetCount();
}

// CPlayList dialog

IMPLEMENT_DYNAMIC(CPlayList, CDialogEx)

CPlayList::CPlayList(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPlayList::IDD, pParent)
{
    m_nCurSel = -1;
    m_nPlayMode = PLAY_MODE_ORDER;
    m_pMainView = NULL;
}

CPlayList::~CPlayList()
{
}

void CPlayList::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_vList);
}


BEGIN_MESSAGE_MAP(CPlayList, CDialogEx)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDOK, &CPlayList::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CPlayList::OnBnClickedCancel)
    ON_WM_SIZE()
    ON_LBN_SELCHANGE(IDC_LIST1, &CPlayList::OnSelchangeList)
    ON_LBN_DBLCLK(IDC_LIST1, &CPlayList::OnDblclkList)
END_MESSAGE_MAP()


// CPlayList message handlers


void CPlayList::OnBnClickedOk()
{
}


void CPlayList::OnBnClickedCancel()
{
}

void CPlayList::OnClose()
{
    // 菜单选中关闭
    SendMessage(WM_COMMAND, MAKEWPARAM(ID_WND_PLAYLIST,BN_CLICKED), NULL);
    EndDialog(IDCLOSE);
}


void CPlayList::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);
    CDialogEx::OnSize(nType, cx, cy);
    if(!IsWindow(m_vList.GetSafeHwnd()))
    {
        return;
    }

    // 主窗口
    CRect rtMain;
    GetClientRect(&rtMain);
    m_vList.MoveWindow(rtMain);
}


void CPlayList::OnSelchangeList()
{
   
}


void CPlayList::OnDblclkList()
{
    if(m_pMainView == NULL)
    {
        return;
    }
    SetCurSel(m_vList.GetCurSel());
    m_pMainView->OnBtnPlay();
}
