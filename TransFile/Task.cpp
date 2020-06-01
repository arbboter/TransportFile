#include "StdAfx.h"
#include "Task.h"


CTask::CTask(void)
{
    m_pRun = NULL;
    m_hTask = INVALID_HANDLE_VALUE;
    m_bRun = false;
}


CTask::~CTask(void)
{
    Stop();
}

bool CTask::RunTask(unsigned (__stdcall * pTaskProc) (void *), void* pPara, bool* bRun)
{
    if(bRun == NULL)
    {
        bRun = &m_bRun;
    }
    if(m_pRun && *m_pRun)
    {
        return false;
    }

    m_pRun = bRun;
    *m_pRun = true;
    m_hTask = (HANDLE)_beginthreadex(NULL, 0, pTaskProc, pPara, 0, NULL);
    if(m_hTask == INVALID_HANDLE_VALUE)
    {
        *m_pRun = false;
        return false;
    }
    return true;
}

bool CTask::IsRunning()
{
    if(m_hTask != INVALID_HANDLE_VALUE)
    {
        DWORD dwExitCode = 0;
        GetExitCodeThread(m_hTask, &dwExitCode);
        if(dwExitCode == STILL_ACTIVE)
        {
            return true;
        }
        m_hTask = INVALID_HANDLE_VALUE;
        *m_pRun = false;
    }
    return false;
}

void CTask::Stop()
{
    if(m_hTask != INVALID_HANDLE_VALUE)
    {
        if (WaitForSingleObject(m_hTask, 100) == WAIT_TIMEOUT) 
        {
            TerminateThread(m_hTask, 0);
        }
    }
}
