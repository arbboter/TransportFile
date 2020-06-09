#pragma once
#include "afxwin.h"

// CPlayList dialog

class CPlayItem
{
public:
    // ����
    CString m_strName;
    // ·��
    CString m_strPath;
};

enum
{
    PLAY_MODE_ORDER,
    PLAY_MODE_RAND,
    PLAY_MODE_CYCLE
};

class CVPlayerDlg;
class CPlayList : public CDialogEx
{
	DECLARE_DYNAMIC(CPlayList)

public:
    // ����ģʽ
    int                     m_nPlayMode;
    // ������
    CVPlayerDlg*            m_pMainView;

private:
    // �����б�
    CArray<CPlayItem>       m_arItems;
    // ��ǰ������
    int                     m_nCurSel;
public:
    // �����
    int Add(const CPlayItem& oItem);
    // ɾ����
    void Remove(int nSeq);
    // ��ȡ��ǰ��
    int GetCurSel();
    
    // ָ��������
    void SetCurSel(int nSeq);
    // ��һ��
    int Next();
    // ��һ��
    int Last();
    // ��ȡ��ǰ��
    int GetCurSel(CPlayItem& oItem);

public:
	CPlayList(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPlayList();

// Dialog Data
	enum { IDD = IDD_DLG_PLAYLIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnClose();
    CListBox m_vList;
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSelchangeList();
    afx_msg void OnDblclkList();
};
