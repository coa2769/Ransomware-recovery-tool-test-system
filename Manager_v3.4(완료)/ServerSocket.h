#pragma once
////////////////////////////////////////////////////////////////////////////////////////////
//※※ CEvent ※※
extern CEvent g_CEventRecvEnd[WINDOW_10 + 1];
extern CEvent g_CEventConnet[WINDOW_10 + 1];

//extern CCriticalSection g_csServer;

class CServerSocket
{
private:
	WSADATA				m_wsa;
	SOCKET				m_hSocket;
	//BOOL				m_ConnectEnd;

	CMap<TEST_OS, TEST_OS, SOCKET, SOCKET>	m_mapClnSocket;

	void InitAcceptSocket(SOCKET& hSocket, unsigned short Port, BOOL isHealth);

public:
	CServerSocket();
	~CServerSocket();
	CEvent m_SuccessReceiveEvent[(int)WINDOW_10 + 1];

	SOCKET getSocket(void) { return m_hSocket; }
	//BOOL getConnetEnd(void) { return m_ConnectEnd; }
	//void setConnetEnd(void) { m_ConnectEnd = TRUE; }

	//접속 함수
	void AcceptFuntion(void);

	//client추가(내부에서 OS를 나눈다.)
	TEST_OS AddClnUser(SOCKADDR_IN& clnaddr, SOCKET& hSocekt);

	//client삭제
	void RemoveClient(TEST_OS os) { m_mapClnSocket.RemoveKey(os); }

	//Error로 인한 종료
	void TermicationSession(TEST_OS os, LPCTSTR lpszText);

	//send 함수
	void SendSignal(TEST_OS os, HEAD_TYPE signal);
	void SendRansomware(TEST_OS os); 
	void SendRecovery(TEST_OS os);	  
	
	void Release(void);
};

