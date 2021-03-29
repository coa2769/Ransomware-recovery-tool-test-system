#include "stdafx.h"
#include "ClientSocket.h"
#include"TestFileFution.h"
#include"HashFuntion.h"

#include<strsafe.h>
#include<ShlObj.h>
#pragma comment(lib,"Shell32") 

////////////////////////////////////////////////////////////////////////////
CRITICAL_SECTION g_csSend;
////////////////////////////////////////////////////////////////////////////
//�ء� Thread �ء�
////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ThreadSendHealthCheck(LPVOID pParam)
{
	printf("********** Start Health Check **********\n");

	CClientSocket* Client = (CClientSocket*)pParam;
	TCP_H Head;
	int count = 0;
	while (true)
	{
		if (Client->m_bCloseHealthCheck == TRUE)
		{
			break;
		}

		memset(&Head, 0, sizeof(Head));
		Head.type = HEALTH_CHECK;
		//Head.os = Client->GetOS();
		
		//���� �� �� �ִ�. AcceptEX �Լ� ã�ƺ���.
		//if (Client->m_bNoSendHealthCheck == FALSE)
		//{
			::EnterCriticalSection(&g_csSend);
			::send(Client->GetSocket(), (char*)&Head, sizeof(TCP_H), 0);
			::LeaveCriticalSection(&g_csSend);
			//printf("Send Health Check!!\n");
		//}


		//�и����� ���� ����.
		//1000ms = 1s
		::Sleep(1000); 
	}

	printf("********** END Health Check ************\n");

	return 0;
}

//��� ����
DWORD WINAPI ThreadSendResult(LPVOID pParam)
{
	CClientSocket* Client = (CClientSocket*)pParam;
	Client->SendTestResult(); //���ο��� for�� ����

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Class�Լ�

CClientSocket::CClientSocket(CTestFileInfo& DB)
	:m_CtfInfo(&DB),
	m_bCloseHealthCheck(FALSE)
	//m_bNoSendHealthCheck(FALSE)
{
	//�ٸ� �ʱ�ȭ ���
	//https://girtowin.tistory.com/107
	//https://docs.microsoft.com/en-us/windows/desktop/sync/using-critical-section-objects
	::InitializeCriticalSection(&g_csSend);

	//1. ���� �ʱ�ȭ
	if (::WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
	{
		::WSACleanup();
		ErrorHandler("Not Initalized Winsock", false);
	}

}

CClientSocket::~CClientSocket()
{
	::DeleteCriticalSection(&g_csSend);

	ReleaseSocket();
}

//Connect �Լ�
void CClientSocket::Connect(char* IP, unsigned short Port)
{
	printf("*****START Connect******\n");

	//�ءءءءءءءءءءء� ���� ���� �ءءءءءءءءءءء�
	//2. ������ ������ ���� ����
	m_hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (m_hSocket == INVALID_SOCKET)
	{
		::WSACleanup();
		ErrorHandler("Could Not Create Socket", false);
	}

	//3. ��Ʈ ���ε� �� ����
	SOCKADDR_IN svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(Port);
	//https://techlog.gurucat.net/317
	//https://docs.microsoft.com/en-us/windows/desktop/api/ws2tcpip/nf-ws2tcpip-inetptonw
	//inet_addr()��� inet_pton()�Լ��� ����ؾ� �Ѵ�.(WS2tcpip.h ����)
	inet_pton(AF_INET, IP, &svraddr.sin_addr.S_un.S_addr);
	//while (::connect(m_hSocket, (SOCKADDR*)&svraddr, sizeof(svraddr)) != SOCKET_ERROR)
	//{
	//	printf("Unable To Connect To Server\n");
	//}

	if (::connect(m_hSocket, (SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
	{
		::WSACleanup();
		ErrorHandler("Unable To Connect To Server", false);
	}

	printf("*****End Connect******\n");
}

////////////////////////////////////////////////////////////////////////////////
//Recive�Լ�
void CClientSocket::Recive(void)
{
	printf("*****Start Recive*******\n");

	//Thread ���� ���� ����
	DWORD dwThreadID;
	HANDLE hThread;

	//File Path
	TCHAR FilePath[MAX_PATH];

	TCP_H Head;
	memset(&Head, 0, sizeof(TCP_H));
	
	void* arr[2];
	arr[0] = m_CtfInfo;
	arr[1] = this;

	int size = sizeof(TCP_H);
	int nRecv = 0;

	while ((nRecv = ::recv(m_hSocket, (char*)&Head, size, 0)) > 0)
	{

		//#1 ���ӵ� ���� �˷���
		if (Head.type == ACCEPT_AGENT)
		{
			//OS�� ������ �˷��ش�.
			printf("Agent Connection Complete!!\n");
			//m_os = Head.os;

			//Thread����
			dwThreadID = 0;
			hThread = ::CreateThread(NULL,
				0,
				ThreadSendHealthCheck,
				(LPVOID)this,
				0,
				&dwThreadID);
			::CloseHandle(hThread);
		}
		//#2 ���� ����(Ransom, recovery)
		else if (Head.type == FILE_RASOM)
		{
			printf("*********Receive Rasomware**********\n");
			//���� �ޱ�
			memset(FilePath, 0, sizeof(TCHAR)*MAX_PATH);
			FileRecive(Head, FilePath); //���⼭ ���� ��ΰ� ���´�.
			//hash�� �˻�
			if (FileHashCheck(Head.FileInfo.OriginalHash, FilePath) == TRUE)
			{
				//�� �޾����� �̸� ����
				m_CtfInfo->SetRasomwareFilePath(FilePath);
				m_CtfInfo->SetRansomwareFileName(Head.FileInfo.FileName);
				SendSignal(FILE_RASOM);
			}
			else
			{
				SendSignal(FILE_RASOM_ERROR);
			}
		}
		else if (Head.type == FILE_RECOVERY)
		{
			printf("*********Receive Recovery**********\n");
			//���� �ޱ�
			memset(FilePath, 0, sizeof(TCHAR)*MAX_PATH);
			FileRecive(Head, FilePath); //���⼭ ���� ��ΰ� ���´�.
			//hash�� �˻�
			if (FileHashCheck(Head.FileInfo.OriginalHash, FilePath) == TRUE)
			{
				//�� �޾����� �̸� ����
				m_CtfInfo->SetRecoveryFilePath(FilePath);
				m_CtfInfo->SetRecoveryFileName(Head.FileInfo.FileName);
				SendSignal(FILE_RECOVERY);
			}
			else
			{
				SendSignal(FILE_RECOVERY_ERROR);
			}
		}
		////#3 Test���� ��ȣ�� ����
		else if (Head.type == START_RASOM)
		{
			printf("Start Rasomware!!\n");
			m_TestState = START_RASOM;
			//File ���ÿ� ���α׷� ����(Thread)
			dwThreadID = 1;
			hThread = ::CreateThread(
				NULL,
				0,
				MonitoringFolder,
				arr,
				0,
				&dwThreadID);
			::CloseHandle(hThread);
			//���� ��ȣ ������
			SendRasomStart();
		}
		//#4 Recovery ����
		else if (Head.type == START_RECOVERY)
		{
			printf("Start Recovery!!\n");
			m_TestState = START_RECOVERY;
			//File ���ÿ� ���α׷� ����(Thread)
			dwThreadID = 2;
			hThread = ::CreateThread(
				NULL,
				0,
				MonitoringFolder,
				arr,
				0,
				&dwThreadID);
			::CloseHandle(hThread);
			//���� ��ȣ ������
			SendSignal(START_RECOVERY);
		}
		//Rasomware Hash�� �����Ѵ�.
		else if (Head.type == SAVE_RASOMWARE_HASH)
		{
			printf("Save Rasomware Hash!!\n");
			dwThreadID = 3;
			hThread = ::CreateThread(
				NULL,
				0,
				ThreadSaveHash,
				arr,
				0,
				&dwThreadID);
			::CloseHandle(hThread);
		}
		//Recovery Hash�� �����Ѵ�.
		else if (Head.type == SAVE_RECOVERY_HASH)
		{
			printf("Save Recovery Hash!!\n");
			dwThreadID = 4;
			hThread = ::CreateThread(
				NULL,
				0,
				ThreadSaveHash,
				arr,
				0,
				&dwThreadID);
			::CloseHandle(hThread);
		}
		//����� ����
		else if (Head.type == TEST_RESULT)
		{
			printf("Send Result!!\n");
			//��� ����(Thread)
			dwThreadID = 5;
			hThread = ::CreateThread(
				NULL,
				0,
				ThreadSendResult,
				(LPVOID)this,
				0,
				&dwThreadID);
			::CloseHandle(hThread);
		}
		//# Test ���� ��ȣ�� ����(Test�� �̰� ������� ����)[2]
		else if (Head.type == ALL_STOP)
		{
			//�� While������ ������.
			break;
		}
		else
		{
			printf("Unknown Packet Arrived\n");
		}
	}

	printf("*****End Recive(%d)*******\n", nRecv);
}

void CClientSocket::FileRecive(TCP_H& Head, TCHAR* FilePath)
{
	BYTE byBuffer[65535];
	int nRecv;
	DWORD dwTotalRecv = 0, dwRead = 0;
	HANDLE hFile;

	//���� ���� �ּ� �����
	//https://docs.microsoft.com/ko-kr/windows/desktop/api/shlobj_core/nf-shlobj_core-shgetfolderpatha
	//https://msdn.microsoft.com/ko-kr/33d92271-2865-4ebd-b96c-bf293deb4310
	if (SHGetFolderPathW(NULL, CSIDL_DESKTOP, NULL, 0, FilePath) != S_OK)
	{
		ErrorHandler("Could Not Find Folder To Save", false);
	}
	//���� ���
	StringCchCat(FilePath, MAX_PATH, _T("\\"));
	StringCchCat(FilePath, MAX_PATH, Head.FileInfo.FileName);

	//���� �����
	hFile = ::CreateFileW(
		FilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,	//������ ������ �����Ѵ�.
		0,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		ErrorHandler("Failed To Create File", false);
	}

	while (dwTotalRecv < Head.FileInfo.FileSize)
	{
		if ((nRecv = ::recv(m_hSocket, (char*)byBuffer, 65535, 0)) > 0)
		{
			dwTotalRecv += nRecv;
			//�������� ���� ũ�⸸ŭ �����͸� ���Ͽ� ����.
			::WriteFile(hFile, byBuffer, nRecv, &dwRead, NULL);
			printf("Receive : %d/%d\n", dwTotalRecv, Head.FileInfo.FileSize);
			fflush(stdout);
		}
		else
		{
			ErrorHandler("Error Receiving File", true);
			break;
		}
	}

	::CloseHandle(hFile);

}

BOOL CClientSocket::FileHashCheck(BYTE* OriHash, TCHAR *FilePath)
{
	BYTE Hash[20];
	if (FileHashCalution(FilePath, Hash) == FALSE)
	{
		ErrorHandler("Failed Hash Value Calculation", false);
	}
	return SameValue(OriHash, Hash);
}

//////////////////////////////////////////////////////////////////////////////////////
// Send�Լ� Ȯ�ο� ����
void CClientSocket::SendSignal(HEAD_TYPE signal)
{
	printf("*****Start SendSignal(%d)*******\n", signal);

	TCP_H Head;
	memset(&Head, 0, sizeof(Head));
	Head.type = signal;
	//Head.os = m_os;

	//m_bNoSendHealthCheck = TRUE;
	::EnterCriticalSection(&g_csSend);
	::send(m_hSocket, (char*)&Head, sizeof(TCP_H), 0);
	::LeaveCriticalSection(&g_csSend);
	//m_bNoSendHealthCheck = FALSE;

	printf("*****End SendSignal(%d)*******\n", signal);
}

//////////////////////////////////////////////////////////////////////////////////////
//Send�Լ���
//#1 ���� ���� �ð��� �˸�
void CClientSocket::SendRasomStart(void)
{
	printf("*****Start SendRasomStart()*******\n");

	TCP_H Head;
	memset(&Head, 0, sizeof(Head));
	Head.type = START_RASOM;
	//Head.os = m_os;
	Head.TestFileCount = m_CtfInfo->GetTestFileCount();

	//m_bNoSendHealthCheck = TRUE;
	::EnterCriticalSection(&g_csSend);
	::send(m_hSocket, (char*)&Head, sizeof(TCP_H), 0);
	::LeaveCriticalSection(&g_csSend);
	//m_bNoSendHealthCheck = FALSE;

	printf("*****End SendRasomStart()*******\n");
}

//#2 ����� �˸�[2]
void CClientSocket::SendTestProgress(int Count)
{
	printf("*****Start SendTestProgress()*******\n");

	TCP_H Head;
	memset(&Head, 0, sizeof(Head));
	Head.type = TEST_PROGRESS;
	//Head.os = m_os;
	Head.TestFileCount = m_CtfInfo->GetTestFileCount();
	Head.ResultCount = m_CtfInfo->GetChageTestFileCount();

	//m_bNoSendHealthCheck = TRUE;
	::EnterCriticalSection(&g_csSend);
	::send(m_hSocket, (char*)&Head, sizeof(TCP_H), 0);
	::LeaveCriticalSection(&g_csSend);
	//m_bNoSendHealthCheck = FALSE;
	
	printf("*****End SendTestProgress()*******\n");
}

//#3 ��� ����[2]
void CClientSocket::SendTestResult(void)
{
	printf("*****Start SendTestResult()*******\n");

	TCP_H Head;
	for (unsigned int i = 0; i < m_CtfInfo->GetTestFileCount(); i++)
	{
		memset(&Head, 0, sizeof(Head));
		Head.type = TEST_RESULT;
		//Head.os = m_os;
		Head.TestFileCount = m_CtfInfo->GetTestFileCount();
		Head.ResultCount = i + 1;
		m_CtfInfo->GetTestFileInfo(i, Head.FileInfo);

		//m_bNoSendHealthCheck = TRUE;
		::EnterCriticalSection(&g_csSend);
		::send(m_hSocket, (char*)&Head, sizeof(TCP_H), 0);
		::LeaveCriticalSection(&g_csSend);
		//m_bNoSendHealthCheck = FALSE;

		//::Sleep(60);

	}
	//��� ����
	printf("*****End SendTestResult()*******\n");
}

////////////////////////////////////////////////////////////////////////////////
//����
void CClientSocket::ReleaseSocket(void)
{
	printf("*******ReleaseSocket() Start*********\n");

	m_bCloseHealthCheck = TRUE;
	//socket�� �ݴ´�.
	if (m_hSocket)
	{
		::shutdown(m_hSocket, SD_BOTH);
		::closesocket(m_hSocket);
	}
	
	::WSACleanup();
	printf("*******ReleaseSocket() End*********\n");
}
//////////////////////////////////////////////////////////////////////////////////////
