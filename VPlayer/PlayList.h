#pragma once
#include "afxwin.h"

// CPlayList dialog

class CPlayItem
{
public:
    // 名字
    CString m_strName;
    // 路径
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
    // 播放模式
    int                     m_nPlayMode;
    // 主窗口
    CVPlayerDlg*            m_pMainView;

private:
    // 播放列表
    CArray<CPlayItem>       m_arItems;
    // 当前播放项
    int                     m_nCurSel;
public:
    // 添加项
    int Add(const CPlayItem& oItem);
    // 删除项
    void Remove(int nSeq);
    // 获取当前项
    int GetCurSel();
    
    // 指定播放项
    void SetCurSel(int nSeq);
    // 下一个
    int Next();
    // 上一个
    int Last();
    // 获取当前项
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
