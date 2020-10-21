
// KCConnCliDlg.h : 头文件
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

// CKCConnCliDlg 对话框
class CKCConnCliDlg : public CDialogEx
{
// 构造
public:
	CKCConnCliDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_KCCONNCLI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
    // 连接KCXP
    BOOL ConnectKcxp();
    // 获取证券行情信息
    BOOL GetInstCodeInfo(const string& strInstCode, map<string, string>& mapInfo);
    BOOL GetInstCodeInfo(map<string, string>& mapReq, map<string, string>& mapInstInfo, map<string, string>& mapHqInfo);
    // 设置公参
    BOOL SetKCBPPublicInput(map<string, string>& mapKV);
    // 解析命令
    BOOL ParseCmd(const string& strCmd, string& strName, string& strLBM, map<string, string>& mapReq);
    // 获取命令参数
    BOOL GetCmdPara(const string& strCmd, STRunBatchTaskProcPara* pPara);
    // 组装成命令
    BOOL MakeCmd(const string& strName, const string& strLBM, const map<string, string>& mapReq, string& strCmd);
    // 自动下单指令处理
    BOOL MakeCmd(const int nCmd, const vector<vector<string>>& vecTask, vector<vector<string>>& vecCmd);
    // 获取指令控制参数
    bool GetCmdPara(const string& strPara, map<string, string>& mapKV);
    // 调用接口
    BOOL CallKxcpLbm(const string& strLbmNo, const map<string, string>& mapReq, vector<map<string, string>>& vecAns, bool bRst = true);
    // 命令模式调用接口
    BOOL CallKxcpByCmd(const string& strCmd);
    // 非命令模式
    BOOL CallWithNoneCmd(const string& strCall);
    // 文件模式
    BOOL CallByFile(const string& strFile);
    // 自定义接口
    BOOL CallMyFunc(const string& strLbmNo, const map<string, string>& mapReq);

public:
    // 加载连接配置
    BOOL LoadServerConfig();
    void AddOut(const CString& strText, BOOL bNewLine = TRUE);
    void AddOut(const string& strText, BOOL bNewLine = TRUE);
    int  TimeRunStat(); // 时间没到返回-1，时间已过返回1，否则返回0
    bool GetDatabaseTaskData(vector<string>& vecData);

protected:
    void GetDefaultRunPara(map<string, string>& mapRunPara);
    bool IsNQAgreeOrder(const string& strLBM);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
