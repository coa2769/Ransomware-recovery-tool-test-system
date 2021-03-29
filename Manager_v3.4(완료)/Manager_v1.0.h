
// Manager_v1.0.h : Manager_v1.0 ���� ���α׷��� ���� �� ��� ����
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"       // �� ��ȣ�Դϴ�.

#include"TextParsing.h"

// CManager_v10App:
// �� Ŭ������ ������ ���ؼ��� Manager_v1.0.cpp�� �����Ͻʽÿ�.
//

class CServerSocket;
class CTestOSInfo;
class CVBoxController;


class CManager_v10App : public CWinApp
{
public:
	CManager_v10App();


// �������Դϴ�.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// �����Դϴ�.

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	
public:
	CTextParsing m_CParser;

	CServerSocket *m_CServer;
	CVBoxController *m_CVBoxController;

	CMap<TEST_OS, TEST_OS, CTestOSInfo*, CTestOSInfo*> m_mapTestOsInfo;
	CEvent m_ExitEvent;

};

extern CManager_v10App theApp;
