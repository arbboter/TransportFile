#pragma once
#include "afxwin.h"


// CInputDlg dialog

class CInputDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInputDlg)

public:
	CInputDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInputDlg();

// Dialog Data
	enum { IDD = IDD_DLG_INPUT };

public:
    CEdit       m_vEdit;
    string      m_strText;
    string      m_strTitle;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedOk();
    virtual BOOL OnInitDialog();
};
