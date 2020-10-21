// EditList.cpp : 实现文件
//

#include "stdafx.h"
#include "KCConnCli.h"
#include "EditList.h"
#include "afxdialogex.h"
#include "Util.h"


// CEditList 对话框

IMPLEMENT_DYNAMIC(CEditList, CDialogEx)

CEditList::CEditList(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEditList::IDD, pParent)
{

}

CEditList::~CEditList()
{
}

void CEditList::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_MAIN, m_vList);
    DDX_Control(pDX, IDC_EDIT_INPUT, m_vInput);
    DDX_Control(pDX, IDC_BTN_OK, m_vBtnOk);
    DDX_Control(pDX, IDC_BTN_CANCEL, m_vBtnCancel);
}


BEGIN_MESSAGE_MAP(CEditList, CDialogEx)
    ON_BN_CLICKED(IDOK, &CEditList::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CEditList::OnBnClickedCancel)
    ON_WM_SIZE()
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_MAIN, &CEditList::OnDblclkListMain)
    ON_EN_KILLFOCUS(IDC_EDIT_INPUT, &CEditList::OnKillfocusEditInput)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BTN_OK, &CEditList::OnBnClickedBtnOk)
    ON_BN_CLICKED(IDC_BTN_CANCEL, &CEditList::OnBnClickedBtnCancel)
    ON_NOTIFY(NM_RCLICK, IDC_LIST_MAIN, &CEditList::OnRclickListMain)
    ON_COMMAND(ID_DEL_CUR, &CEditList::OnDelCur)
    ON_COMMAND(ID_APEND, &CEditList::OnApend)
END_MESSAGE_MAP()


// CEditList 消息处理程序


void CEditList::OnBnClickedOk()
{
    m_vList.SetFocus();
}


void CEditList::OnBnClickedCancel()
{
    
}


void CEditList::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    CRect rtCtrl;
    CRect rtMain;

    if (this->IsWindowVisible())
    {
        // 主窗口大小
        GetClientRect(&rtMain);

        m_vList.GetClientRect(&rtCtrl);
        ScreenToClient(&rtCtrl);

        DWORD dwEdgeSize = 20;

        // 按钮框
        m_vBtnCancel.GetClientRect(&rtCtrl);
        ScreenToClient(&rtCtrl);
        int nBtnWidth = rtCtrl.Width();
        int nBtnHeight = rtCtrl.Height();

        rtCtrl.right = rtMain.right - dwEdgeSize;
        rtCtrl.bottom = rtMain.bottom - dwEdgeSize;
        rtCtrl.left = rtCtrl.right - nBtnWidth;
        rtCtrl.top = rtCtrl.bottom - nBtnHeight;
        m_vBtnCancel.MoveWindow(rtCtrl);

        rtCtrl.right = rtCtrl.left - dwEdgeSize;
        rtCtrl.left = rtCtrl.right - nBtnWidth;
        m_vBtnOk.MoveWindow(rtCtrl);

        // 列表框
        m_vList.GetWindowRect(&rtCtrl);
        ScreenToClient(&rtCtrl);
        rtCtrl.right = rtMain.right - dwEdgeSize;
        rtCtrl.bottom = rtMain.bottom - dwEdgeSize - 30;
        m_vList.MoveWindow(rtCtrl);

        AdjustFilesWidth();
    }
}


BOOL CEditList::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 设置列表框
    LONG lListStyle;
    lListStyle = GetWindowLong(m_vList.m_hWnd, GWL_STYLE);
    lListStyle &= ~LVS_TYPEMASK;
    lListStyle |= LVS_REPORT;
    SetWindowLong(m_vList.m_hWnd, GWL_STYLE, lListStyle);
    m_vList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);

    //初始化列表的标题栏  
    CRect rt;
    m_vList.GetClientRect(&rt);
    TCHAR pColName[][16] = {
        _T("字段名"), _T("字段值")
    };

    int nLen = sizeof(pColName) / sizeof(pColName[0]);
    for (int i = 0; i < nLen; i++)
    {
        m_vList.InsertColumn(i, pColName[i]);
    }
    AdjustFilesWidth();

    SetWindowText(m_strTitle);
    LoadData();

    m_mapData.clear();
    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常:  OCX 属性页应返回 FALSE
}

void CEditList::AdjustFilesWidth()
{
    CRect rtWnd;
    double pWidth[] = { 0.5,0.5 };
    int nCol = sizeof(pWidth) / sizeof(pWidth[0]);

    m_vList.GetClientRect(&rtWnd);
    for (int i = 0; i < nCol; i++)
    {
        m_vList.SetColumnWidth(i, (DWORD)(rtWnd.Width()*pWidth[i]));
    }
}

void CEditList::SetData(const map<string, string>& mapData)
{
    m_mapData = mapData;
}

void CEditList::LoadData()
{
    m_vList.DeleteAllItems();
    m_nCurCol = -1;
    m_nCurRow = -1;
    int nIdx = 0;
    map<string, string>::const_iterator iteMap = m_mapData.begin();
    for (; iteMap != m_mapData.end(); iteMap++)
    {
        m_vList.InsertItem(nIdx, CString(iteMap->first.c_str()));
        m_vList.SetItemText(nIdx, 1, CString(iteMap->second.c_str()));
    }
}


void CEditList::OnDblclkListMain(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    *pResult = 0;

    // 未选择行
    if (pNMItemActivate->iItem < 0)
    {
        return;
    }

    // 获取当前选择单元
    m_nCurCol = pNMItemActivate->iSubItem;
    m_nCurRow = pNMItemActivate->iItem;

    // 编辑框大小和位置
    CRect rtCol;
    m_vList.GetSubItemRect(m_nCurRow, m_nCurCol, LVIR_LABEL, rtCol);
    int nOutLen = 4;
    rtCol.top -= nOutLen;
    rtCol.bottom += nOutLen;
    rtCol.left -= nOutLen;
    rtCol.right += nOutLen;

    m_vInput.SetParent(&m_vList);
    m_vInput.MoveWindow(rtCol);
    m_vInput.SetWindowText(m_vList.GetItemText(m_nCurRow, m_nCurCol));
    m_vInput.ShowWindow(SW_SHOW);
    m_vInput.SetFocus();
    m_vInput.ShowCaret();
    m_vInput.SetSel(-1);
}


void CEditList::OnKillfocusEditInput()
{
    CString strTxt;
    m_vInput.GetWindowText(strTxt);
    m_vList.SetItemText(m_nCurRow, m_nCurCol, strTxt);
    m_vInput.ShowWindow(SW_HIDE);
    m_vList.SetFocus();
}

void CEditList::GetData(map<string, string>& mapData)
{
    CString strName;
    CString strValue;
    for (int i = 0; i < m_vList.GetItemCount(); i++)
    {
        strName = m_vList.GetItemText(i, 0);
        strValue = m_vList.GetItemText(i, 1);

        mapData[CMS::CSW2Cpps(strName)] = CMS::CSW2Cpps(strValue);
    }
}


void CEditList::OnClose()
{
    EndDialog(IDCLOSE);
}


void CEditList::OnBnClickedBtnOk()
{
    m_mapData.clear();
    GetData(m_mapData);
    EndDialog(IDOK);
}


void CEditList::OnBnClickedBtnCancel()
{
    EndDialog(IDCANCEL);
}


void CEditList::OnRclickListMain(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    *pResult = 0;

    m_nCurRow = pNMItemActivate->iItem;
    m_nCurCol = pNMItemActivate->iSubItem;
    if (m_nCurRow < 0)
    {
        return;
    }

    // 右键菜单
    CMenu menu, *pPopup;
    menu.LoadMenu(IDR_MENU1);
    pPopup = menu.GetSubMenu(0);
    CPoint myPoint;
    ClientToScreen(&myPoint);
    GetCursorPos(&myPoint);
    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);
}


void CEditList::OnDelCur()
{
    if (m_nCurRow < 0)
    {
        return;
    }
    m_vList.DeleteItem(m_nCurRow);
    m_nCurRow = -1;
}


void CEditList::OnApend()
{
    int nItemCount = m_vList.GetItemCount();
    m_vList.InsertItem(nItemCount, _T("Filed1"));
}
