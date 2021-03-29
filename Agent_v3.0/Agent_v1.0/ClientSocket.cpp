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
//※※ Thread ※※
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
		
		//수정 될 수 있다. AcceptEX 함수 찾아본다.
		//if (Client->m_bNoSendHealthCheck == FALSE)
		//{
			::EnterCriticalSection(&g_csSend);
			::send(Client->GetSocket(), (char*)&Head, sizeof(TCP_H), 0);
			::LeaveCriticalSection(&g_csSend);
			//printf("Send Health Check!!\n");
		//}


		//밀리어초 값이 들어간다.
		//1000ms = 1s
		::Sleep(1000); 
	}

	printf("********** END Health Check ************\n");

	return 0;
}

//결과 전송
DWORD WINAPI ThreadSendResult(LPVOID pParam)
{
	CClientSocket* Client = (CClientSocket*)pParam;
	Client->SendTestResult(); //내부에서 for로 전송

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Class함수

CClientSocket::CClientSocket(CTestFileInfo& DB)
	:m_CtfInfo(&DB),
	m_bCloseHealthCheck(FALSE)
	//m_bNoSendHealthCheck(FALSE)
{
	//다른 초기화 방법
	//https://girtowin.tistory.com/107
	//https://docs.microsoft.com/en-us/windows/desktop/sync/using-critical-section-objects
	::InitializeCriticalSection(&g_csSend);

	//1. 윈속 초기화
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

//Connect 함수
void CClientSocket::Connect(char* IP, unsigned short Port)
{
	printf("*****START Connect******\n");

	//※※※※※※※※※※※※ 소켓 생성 ※※※※※※※※※※※※
	//2. 서버에 연결할 소켓 생성
	m_hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (m_hSocket == INVALID_SOCKET)
	{
		::WSACleanup();
		ErrorHandler("Could Not Create Socket", false);
	}

	//3. 포트 바인딩 및 연결
	SOCKADDR_IN svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(Port);
	//https://techlog.gurucat.net/317
	//https://docs.microsoft.com/en-us/windows/desktop/api/ws2tcpip/nf-ws2tcpip-inetptonw
	//inet_addr()대신 inet_pton()함수를 사용해야 한다.(WS2tcpip.h 선언)
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
//Recive함수
void CClientSocket::Recive(void)
{
	printf("*****Start Recive*******\n");

	//Thread 생성 관련 변수
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

		//#1 접속된 것을 알려줌
		if (Head.type == ACCEPT_AGENT)
		{
			//OS를 종류를 알려준다.
			printf("Agent Connection Complete!!\n");
			//m_os = Head.os;

			//Thread생성
			dwThreadID = 0;
			hThread = ::CreateThread(NULL,
				0,
				ThreadSendHealthCheck,
				(LPVOID)this,
				0,
				&dwThreadID);
			::CloseHandle(hThread);
		}
		//#2 파일 받은(Ransom, recovery)
		else if (Head.type == FILE_RASOM)
		{
			printf("*********Receive Rasomware**********\n");
			//파일 받기
			memset(FilePath, 0, sizeof(TCHAR)*MAX_PATH);
			FileRecive(Head, FilePath); //여기서 파일 경로가 나온다.
			//hash값 검사
			if (FileHashCheck(Head.FileInfo.OriginalHash, FilePath) == TRUE)
			{
				//잘 받았으면 이름 저장
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
			//파일 받기
			memset(FilePath, 0, sizeof(TCHAR)*MAX_PATH);
			FileRecive(Head, FilePath); //여기서 파일 경로가 나온다.
			//hash값 검사
			if (FileHashCheck(Head.FileInfo.OriginalHash, FilePath) == TRUE)
			{
				//잘 받았으면 이름 저장
				m_CtfInfo->SetRecoveryFilePath(FilePath);
				m_CtfInfo->SetRecoveryFileName(Head.FileInfo.FileName);
				SendSignal(FILE_RECOVERY);
			}
			else
			{
				SendSignal(FILE_RECOVERY_ERROR);
			}
		}
		////#3 Test시작 신호를 받음
		else if (Head.type == START_RASOM)
		{
			printf("Start Rasomware!!\n");
			m_TestState = START_RASOM;
			//File 감시와 프로그램 실행(Thread)
			dwThreadID = 1;
			hThread = ::CreateThread(
				NULL,
				0,
				MonitoringFolder,
				arr,
				0,
				&dwThreadID);
			::CloseHandle(hThread);
			//시작 신호 보내기
			SendRasomStart();
		}
		//#4 Recovery 시작
		else if (Head.type == START_RECOVERY)
		{
			printf("Start Recovery!!\n");
			m_TestState = START_RECOVERY;
			//File 감시와 프로그램 실행(Thread)
			dwThreadID = 2;
			hThread = ::CreateThread(
				NULL,
				0,
				MonitoringFolder,
				arr,
				0,
				&dwThreadID);
			::CloseHandle(hThread);
			//시작 신호 보내기
			SendSignal(START_RECOVERY);
		}
		//Rasomware Hash를 저장한다.
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
		//Recovery Hash를 저장한다.
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
		//결과를 전송
		else if (Head.type == TEST_RESULT)
		{
			printf("Send Result!!\n");
			//결과 전송(Thread)
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
		//# Test 종료 신호를 받음(Test중 이건 상관없이 종료)[2]
		else if (Head.type == ALL_STOP)
		{
			//이 While문에서 나간다.
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

	//파일 생성 주소 만들기
	//https://docs.microsoft.com/ko-kr/windows/desktop/api/shlobj_core/nf-shlobj_core-shgetfolderpatha
	//https://msdn.microsoft.com/ko-kr/33d92271-2865-4ebd-b96c-bf293deb4310
	if (SHGetFolderPathW(NULL, CSIDL_DESKTOP, NULL, 0, FilePath) != S_OK)
	{
		ErrorHandler("Could Not Find Folder To Save", false);
	}
	//파일 경로
	StringCchCat(FilePath, MAX_PATH, _T("\\"));
	StringCchCat(FilePath, MAX_PATH, Head.FileInfo.FileName);

	//파일 만들기
	hFile = ::CreateFileW(
		FilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,	//언제나 파일을 생성한다.
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
			//서버에서 받은 크기만큼 데이터를 파일에 쓴다.
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
// Send함수 확인에 대한
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
//Send함수들
//#1 감염 시작 시간을 알림
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

//#2 진행률 알림[2]
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

//#3 결과 전송[2]
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
	//결과 전송
	printf("*****End SendTestResult()*******\n");
}

////////////////////////////////////////////////////////////////////////////////
//해제
void CClientSocket::ReleaseSocket(void)
{
	printf("*******ReleaseSocket() Start*********\n");

	m_bCloseHealthCheck = TRUE;
	//socket를 닫는다.
	if (m_hSocket)
	{
		::shutdown(m_hSocket, SD_BOTH);
		::closesocket(m_hSocket);
	}
	
	::WSACleanup();
	printf("*******ReleaseSocket() End*********\n");
}
//////////////////////////////////////////////////////////////////////////////////////
