
// KCConnCliDlg.h : ͷ�ļ�
//

#pragma once
#include "KCBPCli.h"
#include <map>
#include <string>
#include <vector>
#include "afxwin.h"

using namespace std;

enum
{
    MODE_CMD,
    MODE_FILE_CMD,
    MODE_ORA_TASK,
};

class CKCConnCliDlg;
typedef struct __st_run_batch_task_proc_para__
{
    vector<string>  vecTasks;
    CKCConnCliDlg*  pThis;
    size_t          nRunTimes;
    int             nMode;
}STRunBatchTaskProcPara;

typedef struct __db_task_info__
{
    int                    m_nRunTimes;
    string                 m_strDbUid;
    string                 m_strDbPwd;
    string                 m_strDbConn;
    vector<string>         m_vecSql;
    vector<vector<string>> m_vecHeader;
    vector<string>         m_vecPara;
    vector<string>         m_vecName;
    vector<string>         m_vecData;
}STDbRunTaskInfo;

// CKCConnCliDlg �Ի���
class CKCConnCliDlg : public CDialogEx
{
// ����
public:
	CKCConnCliDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_KCCONNCLI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
    // ����KCXP
    BOOL ConnectKcxp();
    // ��ȡ֤ȯ������Ϣ
    BOOL GetInstCodeInfo(const string& strInstCode, map<string, string>& mapInfo);
    BOOL GetInstCodeInfo(map<string, string>& mapReq, map<string, string>& mapInstInfo, map<string, string>& mapHqInfo);
    // ���ù���
    BOOL SetKCBPPublicInput(map<string, string>& mapKV);
    // ��������
    BOOL ParseCmd(const string& strCmd, string& strName, string& strLBM, map<string, string>& mapReq);
    // ��ȡ�������
    BOOL GetCmdPara(const string& strCmd, STRunBatchTaskProcPara* pPara);
    // ��װ������
    BOOL MakeCmd(const string& strName, const string& strLBM, const map<string, string>& mapReq, string& strCmd);
    // �Զ��µ�ָ���
    BOOL MakeCmd(const int nCmd, const vector<vector<string>>& vecTask, vector<vector<string>>& vecCmd);
    // ��ȡָ����Ʋ���
    bool GetCmdPara(const string& strPara, map<string, string>& mapKV);
    // ���ýӿ�
    BOOL CallKxcpLbm(const string& strLbmNo, const map<string, string>& mapReq, vector<map<string, string>>& vecAns, bool bRst = true);
    // ����ģʽ���ýӿ�
    BOOL CallKxcpByCmd(const string& strCmd);
    // ������ģʽ
    BOOL CallWithNoneCmd(const string& strCall);
    // �ļ�ģʽ
    BOOL CallByFile(const string& strFile);
    // �Զ���ӿ�
    BOOL CallMyFunc(const string& strLbmNo, const map<string, string>& mapReq);

public:
    // ������������
    BOOL LoadServerConfig();
    void AddOut(const CString& strText, BOOL bNewLine = TRUE);
    void AddOut(const string& strText, BOOL bNewLine = TRUE);
    int  TimeRunStat(); // ʱ��û������-1��ʱ���ѹ�����1�����򷵻�0
    bool GetDatabaseTaskData(vector<string>& vecData);

protected:
    void GetDefaultRunPara(map<string, string>& mapRunPara);
    bool IsNQAgreeOrder(const string& strLBM);

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
    map<string, string>    m_mapKcbpPublicPara;
    STDbRunTaskInfo        m_oDbTask;

public:
    CEdit m_vServerName;
    CEdit m_vUid;
    CEdit m_vPwd;
    CEdit m_vHost;
    CEdit m_vPort;
    CEdit m_vReqQName;
    CEdit m_vAnsQName;
    CComboBox m_vCmdList;
    CEdit m_vOutput;
    CButton m_vConnectServerBtn;
    CEdit m_vBegTime;
    CEdit m_vEndTime;
    CButton m_vSwitchBtn;
    CComboBox m_vTask;

    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnClose();
    afx_msg void OnBnClickedBtnServer();
    afx_msg void OnBnClickedBtnLoadCmd();
    afx_msg void OnBnClickedBtnSendCmd();
    afx_msg void OnBnClickedBtnLoadClear();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedBtnSaveCmd();
    afx_msg void OnBnClickedBtnEditCmd();
    afx_msg void OnClickedBtnEditRunPara();
};
