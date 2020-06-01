
// TransFileDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "resource.h"
#include <string>
#include <vector>
#include <map>
#include "NetFile.h"

using namespace std;


#define RUN_MODE_CLI        1
#define RUN_MODE_SVR        2

// CTransFileDlg dialog
class CTransFileDlg : public CDialogEx
{
// Construction
public:
	CTransFileDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TRANSFILE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

public:
    CNetFile*                   m_pSocket;
    int                         m_nMode;
    CTask                       m_oTask;
    vector<CNetFileClient*>     m_vecCurClient;
    vector<string>              m_vecCurFile;

public:
    CComboBox                   m_vMode;
    CEdit                       m_vHost;
    CEdit                       m_vPort;
    CListCtrl                   m_vList;
    CEdit                       m_vLog;
    CButton                     m_vStart;

    // 输出日志信息到日志框
    void Log(const string& strLog);
    void Log(const char* pFormat, ...);
    void Log(const char * pFormat, va_list va);
    string Format(const char * pFormat, va_list va);

    // 网络通信处理
    void OnAccept();
    void SetConnectHostStatus(const CNetFile* pNet, const string& strStatus);
    void DelConnectHost(const CNetFile* pNet);
    CNetFileClient* GetClient(int nSocket);
    CNetFileClient* GetCurSelClient();
    bool AddClient(CNetFileClient* pCli);
    bool SendFile(CNetFile* pNet, const string& strFile);
    bool SendMsg(CNetFile* pNet, const string& strMsg);
    void OnSwitch(BOOL bEnable);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk(){}
    afx_msg void OnBnClickedCancel(){}
    afx_msg void OnClickedButtonStart();
    afx_msg void OnClose();
    afx_msg void OnRclickListHost(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnSendFile();
    afx_msg void OnSendMsg();
    afx_msg void OnBroadcastFile();
    afx_msg void OnBroadcastMsg();
    afx_msg LRESULT OnMyLog(WPARAM wParam,LPARAM lParam);
    afx_msg LRESULT OnChgSocket(WPARAM wParam,LPARAM lParam);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnSelchangeRunMode();
    afx_msg void OnSize(UINT nType, int cx, int cy);
};