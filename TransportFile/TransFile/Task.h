#pragma once
#include <string>

using namespace std;
#define HOUR_SEC  (60*60)
#define DAY_SECS  (24*HOUR_SEC)

#define TASK_STATUS_READY           0   // δ����
#define TASK_STATUS_RUNNING         1   // ������
#define TASK_STATUS_RUNSTOP         2   // ������ֹ
#define TASK_STATUS_SUCC            3   // ִ�гɹ�
#define TASK_STATUS_FAIL            4   // ִ��ʧ��

class CTask
{
public:
    CTask(void);
    ~CTask(void);

    bool RunTask(unsigned (__stdcall * pTaskProc) (void *), void * pPara, bool* bRun);
    bool IsRunning();
    void Stop();

protected:
    bool*   m_pRun;
    HANDLE  m_hTask;
    bool    m_bRun;
};

