
// KCConnCli.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CKCConnCliApp: 
// �йش����ʵ�֣������ KCConnCli.cpp
//

class CKCConnCliApp : public CWinApp
{
public:
	CKCConnCliApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CKCConnCliApp theApp;