#include "stdafx.h"
#include "ServerSocket.h"

#include"Manager_v1.0.h"
#include"MainFrm.h"
#include"UITestOS.h"
#include"TestOSInfo.h"
#include"HashFuntion.h"
#include"VBoxController.h"

#include<WinSock2.h>
#include<ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

#include<MSWSock.h>
#pragma comment(lib,"Mswsock")

////////////////////////////////////////////////////////////////////////////////////////////
//�ء� CEvent �ء�
CEvent g_CEventRecvEnd[WINDOW_10 + 1];
CEvent g_CEventConnet[WINDOW_10 + 1];
////////////////////////////////////////////////////////////////////////////////////////////
//�ء� CCriticalSection �ء�
CCriticalSection g_csUseClientSocek;
//CCriticalSection g_csServer;
/////////////////////////////////////////////////////////////////////////////////////////////
UINT TreadSendRecovery(LPVOID pParam)
{
	TEST_OS* os = (TEST_OS*)pParam;
	theApp.m_CServer->SendRecovery(*os);

	return 0;
}

UINT ThreadReceive(LPVOID pParam)
{
	SOCKET	hClient = *((SOCKET**)pParam)[0];
	TEST_OS OS = *((TEST_OS**)pParam)[1];

	TCP_H Head;
	memset(&Head, 0, sizeof(TCP_H));

	//g_csUI.Lock();
	CUITestOS* UI = ((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[OS];
	//g_csUI.Unlock();

	g_csInfo.Lock();
	CTestOSInfo* Info = theApp.m_mapTestOsInfo[OS];
	g_csInfo.Unlock();

	printf("%d OS recv����\n", OS);

	//Recive ����
	//Ű���� : recv() ���ð�
	//https://kldp.org/node/113143
	//https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-setsockopt
	//�� �ٸ� ����� ���� �ɼǿ��� RCVTIMEO, ��, receive time out ���� �����ϴ� ����̴�.
	//�����ϰ� ���� ���� ���� ���Ͽ� �Ʒ��� ���� ���������ν� �����ð����� recv �Լ����� ��Ⱑ �߻��ϴ� ��� Ÿ�Ӿƿ����� ó���� �� �ִ�.

	DWORD	nTime = 5000;	//5000 = 5�� ���� ���� ������ ������Ʈ�� ����� �ν�
	if (setsockopt(hClient, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTime, sizeof(DWORD)) == SOCKET_ERROR)
	{
		OutputDebugString(_T("Health Check Socket�� ������ �����߽��ϴ�."));
		shutdown(hClient, SD_BOTH);
		closesocket(hClient);
		return 0;
	}

	//https://m.blog.naver.com/PostView.nhn?blogId=skywood1&logNo=100128756613&proxyReferer=https%3A%2F%2Fwww.google.com%2F
	int count = 0;
	int nRecevie;
	while ((nRecevie = ::recv(hClient,(char*)&Head,sizeof(TCP_H),0)) > 0)
	{
		//g_csServer.Lock();
		//if (theApp.m_CServer->getConnetEnd() == TRUE)
		//{
		//	break;
		//}
		//g_csServer.Unlock();

		//g_csInfo.Lock();
		//g_csUI.Lock();
		//#1 ���� ���� �Ϸ�
		if (Head.type == FILE_RASOM)
		{
			printf("## ���� ���� ���� ����\n");
			UI->SetLogText(_T("���� ������ �����մϴ�."));
			UI->InvalidateRect(NULL);
			::Sleep(10);
			CWinThread* pThread01 = AfxBeginThread(TreadSendRecovery, (LPVOID)&OS);
		}
		//#1-1 ���� ���� ERROR
		else if (Head.type == FILE_RASOM_ERROR)
		{
			printf("ERROR : ���� ���� ���� ����\n");
			//Error Box
			theApp.m_CServer->TermicationSession(OS, _T("�������� ���� ����"));
		}
		//#2 �������� ���� �Ϸ�
		else if (Head.type == FILE_RECOVERY)
		{
			printf("## �������� ���� ���� ����\n");
			theApp.m_CServer->SendSignal(OS, START_RASOM);
		}
		//#2-2 �������� ���� ERROR
		else if (Head.type == FILE_RECOVERY_ERROR)
		{
			printf("ERROR : �������� ���� ���� ����\n");
			//Error Box
			theApp.m_CServer->TermicationSession(OS, _T("�������� ���� ���� ����"));
		}
		//#3 �׽�Ʈ ���� ����
		else if (Head.type == TEST_PROGRESS)
		{
			//UITestOS�� %�� �ٲ��.
			UI->SetProgressText(Head.TestFileCount, Head.ResultCount);
		}
		//#3-1 ���� ����
		else if (Head.type == START_RASOM)
		{
			printf("## ������ ���� �մϴ�.(%d)\n", OS);
			//�ð� ���
			//$
			g_csInfo.Lock();
			Info->SetInfectionStart();
			//���� ��ȭ
			Info->SetState(STATE_TEST);
			g_csInfo.Unlock();

			//UI ��ȭ
			UI->SetChangeStateText();
			UI->SetLogText(_T("������ �Դϴ�."));
			UI->SetProgressText(Head.TestFileCount, 0);
			UI->InvalidateRect(NULL);
			::Sleep(10);
		}
		//#3-2 ���� ��
		else if (Head.type == END_RASOM)
		{
			printf("## ������ �������ϴ�.(%d)\n", OS);
			//�ð� ���
			//$
			g_csInfo.Lock();
			Info->SetInfectionEnd();
			g_csInfo.Unlock();

			//UI��ȭ
			UI->SetLogText(_T("������. ����� ������ �Դϴ�."));
			UI->InvalidateRect(NULL);
			::Sleep(10);
			//���� ����
			theApp.m_CServer->SendSignal(OS, SAVE_RASOMWARE_HASH);
		}
		//#3-3 ���� ����
		else if (Head.type == START_RECOVERY)
		{
			printf("## ������ ���� �մϴ�.(%d)\n", OS);
			//�ð� ���
			//$
			g_csInfo.Lock();
			Info->SetRecoveryStart();
			g_csInfo.Unlock();

			//UI ��ȭ
			UI->SetLogText(_T("������ �Դϴ�."));
			UI->SetProgressText(Head.TestFileCount, 0);
			UI->InvalidateRect(NULL);
			::Sleep(10);
		}
		//#3-4 ���� ��
		else if (Head.type == END_RECOVERY)
		{
			printf("## ������ �������ϴ�.(%d)\n", OS);
			//�ð� ���
			//$
			g_csInfo.Lock();
			Info->SetRecoveryEnd();
			g_csInfo.Unlock();

			//UI ��ȭ
			UI->SetLogText(_T("������. ����� ������ �Դϴ�."));
			UI->InvalidateRect(NULL);
			::Sleep(10);
			//Hash�� ���� �ض�
			theApp.m_CServer->SendSignal(OS, SAVE_RECOVERY_HASH);
		}
		//#4 ����� �����ߴ�.
		else if (Head.type == SAVE_RASOMWARE_HASH)
		{
			printf("## rasomware ��� ����(%d)\n", OS);

			//��� ����
			theApp.m_CServer->SendSignal(OS, START_RECOVERY);
		}
		else if (Head.type == SAVE_RECOVERY_HASH)
		{
			printf("## recovery ��� ����(%d)\n", OS);

			//��� ����
			theApp.m_CServer->SendSignal(OS, TEST_RESULT);
		}
		//#5 ����� �޽��ϴ�.
		else if (Head.type == TEST_RESULT)
		{
			//����� �����Ѵ�.
			//$
			g_csInfo.Lock();
			Info->AddResult(Head.FileInfo);
			g_csInfo.Unlock();

			UI->SetProgressText(Head.TestFileCount, Head.ResultCount);
			UI->InvalidateRect(NULL);
			::Sleep(10);

			if (Head.ResultCount == 1)
			{
				printf("## ����� ���� �޴´�(%d)\n", OS);

				//���� ��ȭ
				//$
				g_csInfo.Lock();
				Info->SetState(STATE_TEST_COMPLETE);
				g_csInfo.Unlock();

				//UI ��ȭ
				UI->SetChangeStateText();
				UI->SetLogText(_T("��� ���� �޴� �� �Դϴ�."));
				UI->InvalidateRect(NULL);
				::Sleep(10);
			}
			//��� ����
			else if (Head.TestFileCount == Head.ResultCount)
			{
				printf("## ����� ���� ��(%d)\n", OS);

				//������ ���� ���
				//$
				g_csInfo.Lock();
				Info->CountRecoveryFile();
				g_csInfo.Unlock();

				//UI����
				UI->SetLogText(_T("���� ������ �Դϴ�."));
				//UI->RedrawWindow(NULL);
				UI->InvalidateRect(NULL);
				::Sleep(10);

				//���� ����
				//$
				g_csInfo.Lock();
				Info->ExportData();
				Info->ReleaseResult();

				//���� ��ȭ
				Info->SetState(STATE_CLOSE);
				g_csInfo.Unlock();

				UI->SetChangeStateText();
				UI->SetLogText(_T("������Ʈ ���� ��"));
				UI->InvalidateRect(NULL);
				::Sleep(10);
				//������Ʈ ����� vm����, ������ ����(Thread)
				//�ϴ� ������Ʈ ����, ���߿� ����ȴ�.
				theApp.m_CServer->SendSignal(OS, ALL_STOP);
			}
		}
		//#5 Health Check
		else if (Head.type == HEALTH_CHECK)
		{
			//count++;
			//if (count > 2000)
			//{
				printf("Health Check!!(%d)\n", OS);
				//count = 0;
			//}
		}
		//g_csInfo.Unlock();
		//g_csUI.Unlock();

	}

	printf("## recv ���� ���Խ��ϴ�.(%d)\n", OS);

	
	//�߸��� ��Ȳ���� ����
	//$
	g_csInfo.Lock();
	TEST_OS_STATE state = Info->getState();
	g_csInfo.Unlock();

	if (state != STATE_CLOSE)
	{
		g_csInfo.Unlock();
		Info->SetState(STATE_ERROR);
		g_csInfo.Unlock();

		UI->SetChangeStateText();
		UI->InvalidateRect(NULL);
		ErrorHandler(_T("������Ʈ�� ������ ������ϴ�."), FALSE);
	}


	//������ ����
	//$@
	g_csUseClientSocek.Lock();
	theApp.m_CServer->RemoveClient(OS);
	g_csUseClientSocek.Unlock();

	//���� ����
	::shutdown(hClient, SD_BOTH);
	::closesocket(hClient);

	//vm���� ���̴�.
	//$
	g_csInfo.Lock();
	CString UUID = Info->GetUUID();
	g_csInfo.Unlock();

	if (theApp.m_CVBoxController->CheckBVMExecution(UUID) == TRUE)
	{
		UI->SetLogText(_T("GuestOS�� ���� �� �Դϴ�."));
		theApp.m_CVBoxController->QuitVM(UUID);
		theApp.m_CVBoxController->RestoreSnampshot(UUID);
		::Sleep(100);
	}

	//���°� �ٲ��.
	//$
	g_csInfo.Lock();
	state = Info->getState();
	g_csInfo.Unlock();

	if (state == STATE_CLOSE)
	{
		//$
		g_csInfo.Lock();
		Info->SetState(STATE_SHOW_REPORT);
		g_csInfo.Unlock();

		UI->SetChangeStateText();
		UI->SetLogText(_T("���� ��� �Դϴ�."));
		UI->SetProgressText(0, -1);
		UI->InvalidateRect(NULL);
	}
	else if (state == STATE_ERROR)
	{
		UI->DrawActive();
	}
	
	((CMainFrm*)theApp.m_pMainWnd)->DrawDropFileRect();

	//socket���� ��
	g_CEventRecvEnd[OS].SetEvent();

	return 0;
}
//Thread Accept �Լ�
UINT ThreadAccept(LPVOID pParam)
{
	SOCKADDR_IN clientaddr = { 0 };
	int nAddrLen = sizeof(clientaddr);

	SOCKET hClient = { 0 };
	CServerSocket *Server = theApp.m_CServer;

	//���߿� ����
	//int temp = 0;
	while ((hClient = ::accept(Server->getSocket(), (SOCKADDR*)&clientaddr, &nAddrLen)) != INVALID_SOCKET)
	{
		TEST_OS os = Server->AddClnUser(clientaddr, hClient);
		//$
		g_csInfo.Lock();
		theApp.m_mapTestOsInfo[os]->SetState(STATE_TEST);
		g_csInfo.Unlock();
		
		printf("%d accept �׽�Ʈ ���·� ����\n", os);

		Server->SendSignal(os, ACCEPT_AGENT);

		//Thread����
		//LPVOID = Long Point void
		//������ ����ϴ� ���� LP. �׳� 32bit �����Ͷ�� �����ϸ� �ȴ�.
		//typedef UINT_PTR        SOCKET;
		//UINT_PTR�� 32bit & 64bit ��ȣ ȣȯ�Ǵ� �ּҰ��� �����ϱ� ���� ���� Ÿ���̴�.
		//������ 32bit�̹Ƿ� LPVOID�� �״�� ���԰��������� 64bit�� ERROR�� �� �� �ִ�.
		
		//unsigned long       DWORD
		void* arr[2];
		arr[0] = &hClient;
		arr[1] = &os;

		//thread����
		//CWinThread* pThread01 = AfxBeginThread(ThreadReceive, (LPVOID)hClient);
		CWinThread* pThread01 = AfxBeginThread(ThreadReceive, arr);
		if (pThread01 == NULL)
		{
			OutputDebugString(_T("Thread������ �����߽��ϴ�."));
			//Error ��ȭ����
			ErrorHandler(_T("Thread������ �����߽��ϴ�."), TRUE);
			break;
		}
		
		g_CEventRecvEnd[os].ResetEvent();
		g_CEventConnet[os].SetEvent();
		
	}

	OutputDebugString(_T("Communitication Accept Thread�� ����˴ϴ�."));

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////
//class �Լ�
CServerSocket::CServerSocket()
	:m_hSocket(0)
	//m_ConnectEnd(FALSE)
{
	g_CEventRecvEnd[WINDOW_7].SetEvent();
	g_CEventRecvEnd[WINDOW_8].SetEvent();
	g_CEventRecvEnd[WINDOW_10].SetEvent();

	//1. ���� �ʱ�ȭ
	m_wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
		OutputDebugString(_T("������ �ʱ�ȭ �� �� �����ϴ�."));
}


CServerSocket::~CServerSocket()
{
	//Release();
}

void CServerSocket::InitAcceptSocket(SOCKET& hSocket, unsigned short Port, BOOL isHealth)
{
	//2. ���Ӵ�� ���� ����
	hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		OutputDebugString(_T("���Ӵ�� ������ ������ �� �����ϴ�."));
		//ErrorBox
		ErrorHandler(_T("���Ӵ�� ������ ������ �� �����ϴ�."), TRUE);
	}
	//3. ��Ʈ ���ε�
	SOCKADDR_IN svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(Port);
	svraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (::bind(hSocket, (SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
	{
		OutputDebugString(_T("���Ͽ� IP�ּҿ� ��Ʈ�� ���ε� �� �� �����ϴ�."));
		//ErrorBox
		ErrorHandler(_T("���Ͽ� IP�ּҿ� ��Ʈ�� ���ε� �� �� �����ϴ�."), TRUE);
	}
	//4. ���Ӵ�� ���·� ��ȯ
	if (::listen(hSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		OutputDebugString(_T("���� ���·� ��ȯ�� �� �����ϴ�."));
		//ErrorBox
		ErrorHandler(_T("���� ���·� ��ȯ�� �� �����ϴ�."), TRUE);
	}

}

//���� �Լ�
void CServerSocket::AcceptFuntion(void)
{
	this->InitAcceptSocket(m_hSocket, 25000, FALSE);
	CWinThread *pThread = AfxBeginThread(ThreadAccept, NULL);
}

//Add Client
TEST_OS CServerSocket::AddClnUser(SOCKADDR_IN& clnaddr, SOCKET& hSocekt)
{
	TEST_OS os;
	char clnIP[254];
	memset(clnIP, 0, sizeof(char) * 254);
	inet_ntop(AF_INET, &clnaddr.sin_addr, clnIP, sizeof(char) * 254);
	
	//if (strcmp(clnIP, "127.0.0.1") == 0)
	//{
	//	os = WINDOW_7;
	//}

	if (strcmp(clnIP, "192.168.56.104") == 0)
	{
		os = WINDOW_7;
		printf("window7 ����\n");
	}
	else if (strcmp(clnIP, "192.168.56.102") == 0)
	{
		os = WINDOW_8;
		printf("window8 ����\n");
	}
	else if (strcmp(clnIP, "192.168.56.103") == 0)
	{
		os = WINDOW_10;
		printf("window10 ����\n");
	}

	//$@
	g_csUseClientSocek.Lock();
	m_mapClnSocket.SetAt(os, hSocekt);
	g_csUseClientSocek.Unlock();

	return os;
}

//send �Լ�
void CServerSocket::SendSignal(TEST_OS os, HEAD_TYPE signal)
{
	TCP_H Head;
	memset(&Head, 0, sizeof(TCP_H));
	int size = sizeof(TCP_H);
	Head.type = signal;
	//Head.os = os;

	//$@
	g_csUseClientSocek.Lock();
	::send(m_mapClnSocket[os], (char*)&Head, size, 0);
	g_csUseClientSocek.Unlock();
}

void CServerSocket::SendRansomware(TEST_OS os)
{
	TCP_H Head;
	memset(&Head, 0, sizeof(TCP_H));
	Head.type = FILE_RASOM;
	//Head.os = os;
	memcpy(Head.FileInfo.FileName, ((CMainFrm*)theApp.m_pMainWnd)->m_CRansomwareName, sizeof(TCHAR) * MAX_PATH);
	
	//$@
	g_csUseClientSocek.Lock();
	//g_csSendRasomwareFile.Lock();

	if (FileHashCalution(((CMainFrm*)theApp.m_pMainWnd)->m_CRansomwarePath.GetBuffer(0), Head.FileInfo.OriginalHash) == FALSE)
	{
		OutputDebugString(_T("Ransomware ������ Hash���� ���� ���߽��ϴ�."));
	}

	//���� ����
	HANDLE hFile = ::CreateFile(((CMainFrm*)theApp.m_pMainWnd)->m_CRansomwarePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(_T("Ransomware ���� Open����"));
		//Error code �ֱ�
		TermicationSession(os, _T("�������� ���� open����"));
		return;
	}

	Head.FileInfo.FileSize = ::GetFileSize(hFile, NULL);
	
	TRANSMIT_FILE_BUFFERS tfb = { 0 };
	tfb.Head = &Head;
	tfb.HeadLength = sizeof(TCP_H);

	//���� �۽�
	//g_csUseClientSocek.Lock();
	if (::TransmitFile(
		m_mapClnSocket[os],	//������ ������ ���� �ڵ�.
		hFile,		//������ ���� �ڵ�
		0,			//������ ũ��, 0�̸� ��ü.
		65535,		//�� ���� ������ ���� ũ��,MAX_PACKET_SIZE
		NULL,		//�񵿱� ��/��¿� ���� OVERLAPPED����ü
		&tfb,		//���� ���ۿ� �ռ� ���� ������ ������
		0			//��Ÿ �ɼ�
		) == FALSE)
	{
		OutputDebugString(_T("���� ���ۿ� �����߽��ϴ�."));
		//ERROR code �ֱ�
		TermicationSession(os,_T("�������� ���� ���� ����"));
	}
	//g_csUseClientSocek.Unlock();
	CloseHandle(hFile);
	//g_csSendRasomwareFile.Unlock();
	g_csUseClientSocek.Unlock();
}

void CServerSocket::SendRecovery(TEST_OS os)
{
	TCP_H Head;
	memset(&Head, 0, sizeof(TCP_H));
	Head.type = FILE_RECOVERY;
	//Head.os = os;
	
	memcpy(Head.FileInfo.FileName, ((CMainFrm*)theApp.m_pMainWnd)->m_CRecoveryName, sizeof(TCHAR) * MAX_PATH);
	
	//$@
	g_csUseClientSocek.Lock();
	//g_csSendRecoveryFile.Lock();
	if (FileHashCalution(((CMainFrm*)theApp.m_pMainWnd)->m_CRecoveryPath.GetBuffer(0), Head.FileInfo.OriginalHash) == FALSE)
	{
		OutputDebugString(_T("�������� ������ Hash���� ���� ���߽��ϴ�."));
	}

	//���� ����
	HANDLE hFile = ::CreateFile(((CMainFrm*)theApp.m_pMainWnd)->m_CRecoveryPath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(_T("�������� ���� Open����"));
		//Error code �ֱ�
		TermicationSession(os, _T("�������� ���� Open����"));
		return;
	}

	Head.FileInfo.FileSize = ::GetFileSize(hFile, NULL);

	TRANSMIT_FILE_BUFFERS tfb = { 0 };
	tfb.Head = &Head;
	tfb.HeadLength = sizeof(TCP_H);

	//���� �۽�
	//g_csUseClientSocek.Lock();
	if (::TransmitFile(
		m_mapClnSocket[os],	//������ ������ ���� �ڵ�.
		hFile,		//������ ���� �ڵ�
		0,			//������ ũ��, 0�̸� ��ü.
		65535,		//�� ���� ������ ���� ũ��,MAX_PACKET_SIZE
		NULL,		//�񵿱� ��/��¿� ���� OVERLAPPED����ü
		&tfb,		//���� ���ۿ� �ռ� ���� ������ ������
		0			//��Ÿ �ɼ�
	) == FALSE)
	{
		OutputDebugString(_T("���� ���ۿ� �����߽��ϴ�."));
		//Error code �ֱ�
		TermicationSession(os, _T("���� ���ۿ� �����߽��ϴ�."));
	}

	//g_csUseClientSocek.Unlock();
	CloseHandle(hFile);
	//g_csSendRecoveryFile.Unlock();
	g_csUseClientSocek.Unlock();

}

//Error�� ���� ����
void CServerSocket::TermicationSession(TEST_OS os, LPCTSTR lpszText)
{
	//Error code �ֱ�
	ErrorHandler(lpszText, FALSE);
	////$
	//g_csInfo.Lock();
	//theApp.m_mapTestOsInfo[os]->SetState(STATE_ERROR);
	//g_csInfo.Unlock();

	//((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os]->SetChangeStateText();
	//((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os]->SetLogText(_T("GeustOS�� �����մϴ�."));
	//((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os]->InvalidateRect(NULL);
	//::Sleep(10);
	SendSignal(os, ALL_STOP);

}

void CServerSocket::Release()
{
	//������Ʈ���� ���� ����
	BOOL bFind = FALSE;
	SOCKET hSocket;

	for (int i = 0; i < (WINDOW_10 + 1); i++)
	{
		//$@
		g_csUseClientSocek.Lock();
		bFind = m_mapClnSocket.Lookup((TEST_OS)i, hSocket);
		g_csUseClientSocek.Unlock();

		if (bFind == TRUE)
		{
			g_csInfo.Lock();
			theApp.m_mapTestOsInfo[(TEST_OS)i]->SetState(STATE_CLOSE);
			g_csInfo.Unlock();
			SendSignal((TEST_OS)i, ALL_STOP);
		}
	}

	//��� ����Ǳ� ��ٸ���.
	HANDLE hArr[3];
	hArr[0] = g_CEventRecvEnd[WINDOW_7];
	hArr[1] = g_CEventRecvEnd[WINDOW_8];
	hArr[2] = g_CEventRecvEnd[WINDOW_10];

	WaitForMultipleObjects(3, hArr, TRUE, INFINITE);

	//Accept ����
	::shutdown(m_hSocket, SD_BOTH);

	POSITION pos;
	TEST_OS os;

	//!!communication socket
	//$@
	g_csUseClientSocek.Lock();
	pos = m_mapClnSocket.GetStartPosition();
	while (pos != NULL)
	{
		m_mapClnSocket.GetNextAssoc(pos, os, hSocket);
		//���� ����
		closesocket(hSocket);
	}
	//map ��ü ����
	m_mapClnSocket.RemoveAll();
	g_csUseClientSocek.Unlock();

	OutputDebugString(_T("��� Ŭ���̾�Ʈ ������ ����\n"));

	::Sleep(100);
	if (m_hSocket != 0) ::closesocket(m_hSocket);
	
	//���� ����
	::WSACleanup();

}
