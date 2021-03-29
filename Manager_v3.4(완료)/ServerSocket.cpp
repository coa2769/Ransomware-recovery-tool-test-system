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
//※※ CEvent ※※
CEvent g_CEventRecvEnd[WINDOW_10 + 1];
CEvent g_CEventConnet[WINDOW_10 + 1];
////////////////////////////////////////////////////////////////////////////////////////////
//※※ CCriticalSection ※※
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

	printf("%d OS recv진입\n", OS);

	//Recive 받음
	//키워드 : recv() 대기시간
	//https://kldp.org/node/113143
	//https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-setsockopt
	//또 다른 방법이 소켓 옵션에서 RCVTIMEO, 즉, receive time out 값을 설정하는 방법이다.
	//간단하게 소켓 생성 이후 소켓에 아래와 같이 설정함으로써 일정시간동안 recv 함수에서 대기가 발생하는 경우 타임아웃으로 처리할 수 있다.

	DWORD	nTime = 5000;	//5000 = 5초 동안 오지 않으면 에이전트의 종료로 인식
	if (setsockopt(hClient, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTime, sizeof(DWORD)) == SOCKET_ERROR)
	{
		OutputDebugString(_T("Health Check Socket의 설정을 실패했습니다."));
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
		//#1 랜섬 전달 완료
		if (Head.type == FILE_RASOM)
		{
			printf("## 랜섬 파일 전송 성공\n");
			UI->SetLogText(_T("복구 도구를 전송합니다."));
			UI->InvalidateRect(NULL);
			::Sleep(10);
			CWinThread* pThread01 = AfxBeginThread(TreadSendRecovery, (LPVOID)&OS);
		}
		//#1-1 랜섬 전달 ERROR
		else if (Head.type == FILE_RASOM_ERROR)
		{
			printf("ERROR : 랜섬 파일 전송 실패\n");
			//Error Box
			theApp.m_CServer->TermicationSession(OS, _T("랜섬파일 전송 실패"));
		}
		//#2 복구도구 전달 완료
		else if (Head.type == FILE_RECOVERY)
		{
			printf("## 복구도구 파일 전송 성공\n");
			theApp.m_CServer->SendSignal(OS, START_RASOM);
		}
		//#2-2 복구도구 전달 ERROR
		else if (Head.type == FILE_RECOVERY_ERROR)
		{
			printf("ERROR : 복구도구 파일 전송 실패\n");
			//Error Box
			theApp.m_CServer->TermicationSession(OS, _T("복구도구 파일 전송 실패"));
		}
		//#3 테스트 진행 정도
		else if (Head.type == TEST_PROGRESS)
		{
			//UITestOS에 %가 바뀐다.
			UI->SetProgressText(Head.TestFileCount, Head.ResultCount);
		}
		//#3-1 감염 시작
		else if (Head.type == START_RASOM)
		{
			printf("## 감염을 시작 합니다.(%d)\n", OS);
			//시간 기록
			//$
			g_csInfo.Lock();
			Info->SetInfectionStart();
			//상태 변화
			Info->SetState(STATE_TEST);
			g_csInfo.Unlock();

			//UI 변화
			UI->SetChangeStateText();
			UI->SetLogText(_T("감염중 입니다."));
			UI->SetProgressText(Head.TestFileCount, 0);
			UI->InvalidateRect(NULL);
			::Sleep(10);
		}
		//#3-2 감염 끝
		else if (Head.type == END_RASOM)
		{
			printf("## 감염이 끝났습니다.(%d)\n", OS);
			//시간 기록
			//$
			g_csInfo.Lock();
			Info->SetInfectionEnd();
			g_csInfo.Unlock();

			//UI변화
			UI->SetLogText(_T("감염끝. 결과를 저장중 입니다."));
			UI->InvalidateRect(NULL);
			::Sleep(10);
			//복구 시작
			theApp.m_CServer->SendSignal(OS, SAVE_RASOMWARE_HASH);
		}
		//#3-3 복구 시작
		else if (Head.type == START_RECOVERY)
		{
			printf("## 복구를 시작 합니다.(%d)\n", OS);
			//시간 기록
			//$
			g_csInfo.Lock();
			Info->SetRecoveryStart();
			g_csInfo.Unlock();

			//UI 변화
			UI->SetLogText(_T("복구중 입니다."));
			UI->SetProgressText(Head.TestFileCount, 0);
			UI->InvalidateRect(NULL);
			::Sleep(10);
		}
		//#3-4 복구 끝
		else if (Head.type == END_RECOVERY)
		{
			printf("## 복구가 끝났습니다.(%d)\n", OS);
			//시간 기록
			//$
			g_csInfo.Lock();
			Info->SetRecoveryEnd();
			g_csInfo.Unlock();

			//UI 변화
			UI->SetLogText(_T("복구끝. 결과를 저장중 입니다."));
			UI->InvalidateRect(NULL);
			::Sleep(10);
			//Hash를 저장 해라
			theApp.m_CServer->SendSignal(OS, SAVE_RECOVERY_HASH);
		}
		//#4 결과를 저장했다.
		else if (Head.type == SAVE_RASOMWARE_HASH)
		{
			printf("## rasomware 결과 저장(%d)\n", OS);

			//결과 전송
			theApp.m_CServer->SendSignal(OS, START_RECOVERY);
		}
		else if (Head.type == SAVE_RECOVERY_HASH)
		{
			printf("## recovery 결과 저장(%d)\n", OS);

			//결과 전송
			theApp.m_CServer->SendSignal(OS, TEST_RESULT);
		}
		//#5 결과를 받습니다.
		else if (Head.type == TEST_RESULT)
		{
			//결과를 저장한다.
			//$
			g_csInfo.Lock();
			Info->AddResult(Head.FileInfo);
			g_csInfo.Unlock();

			UI->SetProgressText(Head.TestFileCount, Head.ResultCount);
			UI->InvalidateRect(NULL);
			::Sleep(10);

			if (Head.ResultCount == 1)
			{
				printf("## 결과를 전송 받는다(%d)\n", OS);

				//상태 변화
				//$
				g_csInfo.Lock();
				Info->SetState(STATE_TEST_COMPLETE);
				g_csInfo.Unlock();

				//UI 변화
				UI->SetChangeStateText();
				UI->SetLogText(_T("결과 전송 받는 중 입니다."));
				UI->InvalidateRect(NULL);
				::Sleep(10);
			}
			//모두 받음
			else if (Head.TestFileCount == Head.ResultCount)
			{
				printf("## 결과를 전송 끝(%d)\n", OS);

				//복구된 파일 계산
				//$
				g_csInfo.Lock();
				Info->CountRecoveryFile();
				g_csInfo.Unlock();

				//UI변경
				UI->SetLogText(_T("보고서 저장중 입니다."));
				//UI->RedrawWindow(NULL);
				UI->InvalidateRect(NULL);
				::Sleep(10);

				//보고서 저장
				//$
				g_csInfo.Lock();
				Info->ExportData();
				Info->ReleaseResult();

				//상태 변화
				Info->SetState(STATE_CLOSE);
				g_csInfo.Unlock();

				UI->SetChangeStateText();
				UI->SetLogText(_T("에이전트 종료 중"));
				UI->InvalidateRect(NULL);
				::Sleep(10);
				//에이전트 종료와 vm종료, 스냅샷 복구(Thread)
				//일단 에이전트 종료, 나중에 변경된다.
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

	printf("## recv 빠져 나왔습니다.(%d)\n", OS);

	
	//잘못된 상황에서 종료
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
		ErrorHandler(_T("에이전트에 문제가 생겼습니다."), FALSE);
	}


	//정보를 해제
	//$@
	g_csUseClientSocek.Lock();
	theApp.m_CServer->RemoveClient(OS);
	g_csUseClientSocek.Unlock();

	//연결 종료
	::shutdown(hClient, SD_BOTH);
	::closesocket(hClient);

	//vm실행 중이다.
	//$
	g_csInfo.Lock();
	CString UUID = Info->GetUUID();
	g_csInfo.Unlock();

	if (theApp.m_CVBoxController->CheckBVMExecution(UUID) == TRUE)
	{
		UI->SetLogText(_T("GuestOS를 종료 중 입니다."));
		theApp.m_CVBoxController->QuitVM(UUID);
		theApp.m_CVBoxController->RestoreSnampshot(UUID);
		::Sleep(100);
	}

	//상태가 바뀐다.
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
		UI->SetLogText(_T("실험 결과 입니다."));
		UI->SetProgressText(0, -1);
		UI->InvalidateRect(NULL);
	}
	else if (state == STATE_ERROR)
	{
		UI->DrawActive();
	}
	
	((CMainFrm*)theApp.m_pMainWnd)->DrawDropFileRect();

	//socket종료 됨
	g_CEventRecvEnd[OS].SetEvent();

	return 0;
}
//Thread Accept 함수
UINT ThreadAccept(LPVOID pParam)
{
	SOCKADDR_IN clientaddr = { 0 };
	int nAddrLen = sizeof(clientaddr);

	SOCKET hClient = { 0 };
	CServerSocket *Server = theApp.m_CServer;

	//나중에 삭제
	//int temp = 0;
	while ((hClient = ::accept(Server->getSocket(), (SOCKADDR*)&clientaddr, &nAddrLen)) != INVALID_SOCKET)
	{
		TEST_OS os = Server->AddClnUser(clientaddr, hClient);
		//$
		g_csInfo.Lock();
		theApp.m_mapTestOsInfo[os]->SetState(STATE_TEST);
		g_csInfo.Unlock();
		
		printf("%d accept 테스트 상태로 변경\n", os);

		Server->SendSignal(os, ACCEPT_AGENT);

		//Thread시작
		//LPVOID = Long Point void
		//예전에 사용하던 형식 LP. 그냥 32bit 포인터라고 생각하면 된다.
		//typedef UINT_PTR        SOCKET;
		//UINT_PTR은 32bit & 64bit 상호 호환되는 주소값을 제공하기 위한 변수 타입이다.
		//지금은 32bit이므로 LPVOID에 그대로 대입가능하지만 64bit면 ERROR가 날 수 있다.
		
		//unsigned long       DWORD
		void* arr[2];
		arr[0] = &hClient;
		arr[1] = &os;

		//thread생성
		//CWinThread* pThread01 = AfxBeginThread(ThreadReceive, (LPVOID)hClient);
		CWinThread* pThread01 = AfxBeginThread(ThreadReceive, arr);
		if (pThread01 == NULL)
		{
			OutputDebugString(_T("Thread생성에 실패했습니다."));
			//Error 대화상자
			ErrorHandler(_T("Thread생성에 실패했습니다."), TRUE);
			break;
		}
		
		g_CEventRecvEnd[os].ResetEvent();
		g_CEventConnet[os].SetEvent();
		
	}

	OutputDebugString(_T("Communitication Accept Thread가 종료됩니다."));

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////
//class 함수
CServerSocket::CServerSocket()
	:m_hSocket(0)
	//m_ConnectEnd(FALSE)
{
	g_CEventRecvEnd[WINDOW_7].SetEvent();
	g_CEventRecvEnd[WINDOW_8].SetEvent();
	g_CEventRecvEnd[WINDOW_10].SetEvent();

	//1. 윈속 초기화
	m_wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
		OutputDebugString(_T("윈속을 초기화 할 수 없습니다."));
}


CServerSocket::~CServerSocket()
{
	//Release();
}

void CServerSocket::InitAcceptSocket(SOCKET& hSocket, unsigned short Port, BOOL isHealth)
{
	//2. 접속대기 소켓 생성
	hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		OutputDebugString(_T("접속대기 소켓을 생성할 수 없습니다."));
		//ErrorBox
		ErrorHandler(_T("접속대기 소켓을 생성할 수 없습니다."), TRUE);
	}
	//3. 포트 바인딩
	SOCKADDR_IN svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(Port);
	svraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (::bind(hSocket, (SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
	{
		OutputDebugString(_T("소켓에 IP주소와 포트를 바인드 할 수 없습니다."));
		//ErrorBox
		ErrorHandler(_T("소켓에 IP주소와 포트를 바인드 할 수 없습니다."), TRUE);
	}
	//4. 접속대기 상태로 전환
	if (::listen(hSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		OutputDebugString(_T("리슨 상태로 전환할 수 없습니다."));
		//ErrorBox
		ErrorHandler(_T("리슨 상태로 전환할 수 없습니다."), TRUE);
	}

}

//접속 함수
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
		printf("window7 접속\n");
	}
	else if (strcmp(clnIP, "192.168.56.102") == 0)
	{
		os = WINDOW_8;
		printf("window8 접속\n");
	}
	else if (strcmp(clnIP, "192.168.56.103") == 0)
	{
		os = WINDOW_10;
		printf("window10 접속\n");
	}

	//$@
	g_csUseClientSocek.Lock();
	m_mapClnSocket.SetAt(os, hSocekt);
	g_csUseClientSocek.Unlock();

	return os;
}

//send 함수
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
		OutputDebugString(_T("Ransomware 파일의 Hash값을 받지 못했습니다."));
	}

	//파일 열기
	HANDLE hFile = ::CreateFile(((CMainFrm*)theApp.m_pMainWnd)->m_CRansomwarePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(_T("Ransomware 파일 Open실패"));
		//Error code 넣기
		TermicationSession(os, _T("랜섬웨어 파일 open실패"));
		return;
	}

	Head.FileInfo.FileSize = ::GetFileSize(hFile, NULL);
	
	TRANSMIT_FILE_BUFFERS tfb = { 0 };
	tfb.Head = &Head;
	tfb.HeadLength = sizeof(TCP_H);

	//파일 송신
	//g_csUseClientSocek.Lock();
	if (::TransmitFile(
		m_mapClnSocket[os],	//파일을 전송할 소켓 핸들.
		hFile,		//전송할 파일 핸들
		0,			//전송할 크기, 0이면 전체.
		65535,		//한 번에 전송할 버퍼 크기,MAX_PACKET_SIZE
		NULL,		//비동기 입/출력에 대한 OVERLAPPED구조체
		&tfb,		//파일 전송에 앞서 먼저 전송할 데이터
		0			//기타 옵션
		) == FALSE)
	{
		OutputDebugString(_T("파일 전송에 실패했습니다."));
		//ERROR code 넣기
		TermicationSession(os,_T("랜섬웨어 파일 전송 실패"));
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
		OutputDebugString(_T("복구도구 파일의 Hash값을 받지 못했습니다."));
	}

	//파일 열기
	HANDLE hFile = ::CreateFile(((CMainFrm*)theApp.m_pMainWnd)->m_CRecoveryPath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(_T("복구도구 파일 Open실패"));
		//Error code 넣기
		TermicationSession(os, _T("복구도구 파일 Open실패"));
		return;
	}

	Head.FileInfo.FileSize = ::GetFileSize(hFile, NULL);

	TRANSMIT_FILE_BUFFERS tfb = { 0 };
	tfb.Head = &Head;
	tfb.HeadLength = sizeof(TCP_H);

	//파일 송신
	//g_csUseClientSocek.Lock();
	if (::TransmitFile(
		m_mapClnSocket[os],	//파일을 전송할 소켓 핸들.
		hFile,		//전송할 파일 핸들
		0,			//전송할 크기, 0이면 전체.
		65535,		//한 번에 전송할 버퍼 크기,MAX_PACKET_SIZE
		NULL,		//비동기 입/출력에 대한 OVERLAPPED구조체
		&tfb,		//파일 전송에 앞서 먼저 전송할 데이터
		0			//기타 옵션
	) == FALSE)
	{
		OutputDebugString(_T("파일 전송에 실패했습니다."));
		//Error code 넣기
		TermicationSession(os, _T("파일 전송에 실패했습니다."));
	}

	//g_csUseClientSocek.Unlock();
	CloseHandle(hFile);
	//g_csSendRecoveryFile.Unlock();
	g_csUseClientSocek.Unlock();

}

//Error로 인한 종료
void CServerSocket::TermicationSession(TEST_OS os, LPCTSTR lpszText)
{
	//Error code 넣기
	ErrorHandler(lpszText, FALSE);
	////$
	//g_csInfo.Lock();
	//theApp.m_mapTestOsInfo[os]->SetState(STATE_ERROR);
	//g_csInfo.Unlock();

	//((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os]->SetChangeStateText();
	//((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os]->SetLogText(_T("GeustOS를 종료합니다."));
	//((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os]->InvalidateRect(NULL);
	//::Sleep(10);
	SendSignal(os, ALL_STOP);

}

void CServerSocket::Release()
{
	//에이전트와의 연결 종료
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

	//모두 종료되길 기다린다.
	HANDLE hArr[3];
	hArr[0] = g_CEventRecvEnd[WINDOW_7];
	hArr[1] = g_CEventRecvEnd[WINDOW_8];
	hArr[2] = g_CEventRecvEnd[WINDOW_10];

	WaitForMultipleObjects(3, hArr, TRUE, INFINITE);

	//Accept 종료
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
		//소켓 해제
		closesocket(hSocket);
	}
	//map 전체 삭제
	m_mapClnSocket.RemoveAll();
	g_csUseClientSocek.Unlock();

	OutputDebugString(_T("모든 클라이언트 연결을 종료\n"));

	::Sleep(100);
	if (m_hSocket != 0) ::closesocket(m_hSocket);
	
	//윈속 해제
	::WSACleanup();

}
