// Agent_v1.0.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include"TestFileFution.h"
#include"ClientSocket.h"
#include<WinInet.h>
#pragma comment(lib, "WinInet.lib")

//!! console���� �����ڵ�� �ѱ��� ��� �Ϸ���
//_wsetlocale(LC_ALL, _T("korean"));
//_tprintf(_T("_tprintf()�Լ��̴�.\n"));
//�̿� ���� ó���� ���־�� �Ѵ�.

CTestFileInfo g_CTestFileInfo;
CClientSocket g_Socket(g_CTestFileInfo);

//#TRUE : WINDOWS 7, 8.1, 10���� ��� ��� ������ �Լ���� ��.

//������Ʈ ���� code
BOOL CtrlHandler(DWORD fdwCtrlType)
{
	//��� ���� �� ����� �̺�Ʈ
	//CTRL_C_EVENT
	//CTRL_BREAK_EVENT
	//CTRL_CLOSE_EVENT		//X�ڸ� �����ų� �۾������ڿ��� �����⸦ ������ ���
	//CTRL_LOGOFF_EVENT		//����ڰ� �α׿��� �� ���
	//CTRL_SHUTDOWN_EVENT	//��ǻ�͸� ���� �������� �ǰ�

	//�� ���ῡ ���� ��� �����ؾ��ϴ� ���� ���⼭ �ذ�
	_tprintf(_T("## Close Agnet ##\n"));

	//���� ���� Thread ����
	if (g_hEventEndMonitoring != 0)
	{
		SetEvent(g_hEventEndMonitoring);
	}
	
	//Health Check�� Thread ����
	g_Socket.ReleaseSocket();
	//���� ����
	g_CTestFileInfo.ReleaseTestFolderAndFile();

	Sleep(200); //���� ��ٷ��� ������


	return TRUE;
}


int main()
{
	//���� �� �Ҹ� �Լ� ���
	BOOL fSuccess = SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

	//Test�� ���ϵ� 
	g_CTestFileInfo.InitTestFolderAndFile();
	
	DWORD dwFlag = 0;
	//DWORD dwFlag = INTERNET_CONNECTION_LAN;

	TCHAR szName[256];
	while (true)
	{
		if (::InternetGetConnectedStateEx(&dwFlag, szName, 256, 0))
		{
			printf("Connet\n");
			break;
		}
		else
		{
			printf("No Connet\n");
		}

		Sleep(100);
	}

	//conect�� ����
	g_Socket.Connect("192.168.56.1", 25000);
	//g_Socket.Connect("127.0.0.1",25000);

	//recive
	g_Socket.Recive();

	g_CTestFileInfo.ReleaseTestFolderAndFile();

    return 0;
}

