#pragma once
#include "afxcmn.h"
#include <map>
#include <string>
#include "afxwin.h"
#include "resource.h"

using namespace std;
// CEditList 对话框

class CEditList : public CDialogEx
{
	DECLARE_DYNAMIC(CEditList)

public:
	CEditList(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CEditList();

// 对话框数据
	enum { IDD = IDD_DIALOG1 };

private:
    int     m_nCurCol;
    int     m_nCurRow;

    void    LoadData();

public:
    CString m_strTitle;
    map<string, string> m_mapData;

    void SetData(const map<string, string>& mapData);
    void GetData(map<string, string>& mapData);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    CListCtrl m_vList;
    afx_msg void OnSize(UINT nType, int cx, int cy);
    virtual BOOL OnInitDialog();
    void    AdjustFilesWidth();
    afx_msg void OnDblclkListMain(NMHDR *pNMHDR, LRESULT *pResult);
    CEdit m_vInput;
    afx_msg void OnKillfocusEditInput();
    afx_msg void OnClose();
    afx_msg void OnBnClickedBtnOk();
    afx_msg void OnBnClickedBtnCancel();
    CButton m_vBtnOk;
    CButton m_vBtnCancel;
    afx_msg void OnRclickListMain(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDelCur();
    afx_msg void OnApend();
};
