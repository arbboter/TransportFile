
// VPlayerDlg.h : header file
//

#pragma once
#include <dshow.h>
#include <atlconv.h>
#include "afxcmn.h"
#include "TextProgressCtrl.h"
#include "PlayList.h"

// CVPlayerDlg dialog
class CVPlayerDlg : public CDialogEx
{
// Construction
public:
	CVPlayerDlg(CWnd* pParent = NULL);	// standard constructor
    virtual ~CVPlayerDlg();

// Dialog Data
	enum { IDD = IDD_VPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
    HICON                   m_hIcon;
    CToolBar                m_vToolBar;
    CMenu                   m_vMenu;
    CImageList              m_imgToolBar;
    CTextProgressCtrl       m_vTextProgress;
    CPlayList*              m_pPlayDlg;

private:
    IGraphBuilder*          m_pGraph;
    IMediaControl*          m_pControl;
    IMediaEvent*            m_pEvent; 
    IBasicVideo*            m_pVideo;
    IBasicAudio*            m_pAudio;
    IVideoWindow*           m_pWindow;
    IMediaSeeking*          m_pSeeking;

protected:
    void InitToolBar();
    BOOL InitDirectShow();
    void UnitDirectShow();

    BOOL OnToolNotify(UINT id,NMHDR* pNMHDR,LRESULT* pResult);
    // 获取视频画面窗口
    void GetVRect(CRect& rtV);
    void OnVideoWndSizeChange();
    void SetPlayerBackground();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnDropFiles(HDROP hDropInfo);
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnClose();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBtnBack();
    afx_msg void OnBtnForward();
    afx_msg void OnBtnFullscreen();
    afx_msg void OnBtnLast();
    afx_msg void OnBtnNext();
    afx_msg void OnBtnPause();
    afx_msg void OnBtnPlay();
    afx_msg void OnBtnStop();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnMove(int x, int y);
    afx_msg void OnFileOpen();
    afx_msg void OnWndPlaylist();
};
