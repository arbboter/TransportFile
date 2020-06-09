// InputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TransFile.h"
#include "InputDlg.h"
#include "afxdialogex.h"


// CInputDlg dialog

IMPLEMENT_DYNAMIC(CInputDlg, CDialogEx)

CInputDlg::CInputDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInputDlg::IDD, pParent)
{

}

CInputDlg::~CInputDlg()
{
}

void CInputDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT2, m_vEdit);
}


BEGIN_MESSAGE_MAP(CInputDlg, CDialogEx)
    ON_BN_CLICKED(IDCANCEL, &CInputDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDOK, &CInputDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CInputDlg message handlers


void CInputDlg::OnBnClickedCancel()
{
    m_strText = "";
    CDialogEx::OnCancel();
}


void CInputDlg::OnBnClickedOk()
{
    CString strBuf;
    m_vEdit.GetWindowText(strBuf);
    m_strText = strBuf;
    CDialogEx::OnOK();
}


BOOL CInputDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    if(m_strTitle.empty())
    {
        SetWindowText("«Î ‰»Î:");
    }
    else
    {
        SetWindowText(m_strTitle.c_str());
    }
    return TRUE;
}
