
// stdafx.cpp : source file that includes just the standard includes
// TransFile.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include <tlhelp32.h>
#include"psapi.h"

#pragma comment(lib,"psapi.lib")

bool HasSameProcRun()
{
    // ��ȡ��ǰ������
    TCHAR szCurName[MAX_PATH] = {0};
    GetModuleFileName(NULL, szCurName, sizeof(szCurName));

    return FindProcess(szCurName) != 0;
}


//����ָ������
DWORD FindProcess(TCHAR *strProcessName)
{
    DWORD aProcesses[1024], cbNeeded, cbMNeeded;
    HMODULE hMods[1024];
    HANDLE hProcess;
    TCHAR szProcessName[MAX_PATH];
    DWORD dwCurId = GetCurrentProcessId();
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))  return 0;
    for(int i=0; i< (int) (cbNeeded / sizeof(DWORD)); i++)
    {
        if(aProcesses[i] == dwCurId)
        {
            continue;
        }
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
        EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbMNeeded);
        GetModuleFileNameEx(hProcess, hMods[0], szProcessName,sizeof(szProcessName));

        CString strPrcFullName(szProcessName);
        CString strPrcName(strProcessName);
        if(_tcsstr(strPrcFullName, strPrcName) || _tcsstr(strPrcFullName, strPrcName.MakeLower()))
        {
            return(aProcesses[i]);
        }
    }
    return 0;
}

// �˺������������ FindProcess ����������Ŀ����̵�ID
// ��WIN API OpenPorcess ��ô˽��̵ľ��������TerminateProcessǿ�ƽ����������
VOID KillProcess(TCHAR *strProcessName)
{
    // When the all operation fail this function terminate the "winlogon" Process for force exit the system.
    HANDLE hYourTargetProcess = OpenProcess(PROCESS_QUERY_INFORMATION |   // Required by Alpha
        PROCESS_CREATE_THREAD     |   // For CreateRemoteThread
        PROCESS_VM_OPERATION      |   // For VirtualAllocEx/VirtualFreeEx
        PROCESS_VM_WRITE          |  // For WriteProcessMemory
        PROCESS_TERMINATE,           //Required to terminate a process using TerminateProcess function
        FALSE, FindProcess(strProcessName));

    if(hYourTargetProcess == NULL)
    {
        DWORD ulErrCode = GetLastError();
        CString strError;
    }

    BOOL result = TerminateProcess(hYourTargetProcess, 0);
    if(!result)
    {
        DWORD ulErrCode = GetLastError();
    }
    return;
}

// �� Windows NT/2000/XP �п�����Ȩ�޲����������Ϻ���ʧ��
BOOL GetDebugPriv()
{
    HANDLE hToken;
    LUID sedebugnameValue;
    TOKEN_PRIVILEGES tkp;

    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        return FALSE;
    }

    if(!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))
    {
        CloseHandle( hToken );
        return FALSE;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = sedebugnameValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges( hToken, FALSE, &tkp, sizeof tkp, NULL, NULL))
    {
        CloseHandle(hToken);
        return FALSE;
    }

    return TRUE;
}
