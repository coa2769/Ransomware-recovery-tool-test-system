#pragma once

class CTestFileInfo;

class CClientSocket
{
private:
	WSADATA m_wsa = { 0 };
	SOCKET m_hSocket;

	CTestFileInfo* m_CtfInfo;

	//TEST_OS m_os;
	HEAD_TYPE m_TestState;

public:
	explicit CClientSocket(CTestFileInfo& DB);
	~CClientSocket();

	BOOL m_bCloseHealthCheck;	//Health Check Thread를 나올 때
	//BOOL m_bNoSendHealthCheck;	//보낼 필요 없을 때

	//get 함수
	//TEST_OS GetOS(void) { return m_os; }
	SOCKET GetSocket(void) { return m_hSocket; }
	HEAD_TYPE GetTestState(void) { return m_TestState; }

	//Connet 함수
	void Connect(char* IP, unsigned short Port);

	//Recive(한 함수에)
	void Recive(void);
	void FileRecive(TCP_H& Head, TCHAR* FilePath);
	BOOL FileHashCheck(BYTE* OriHash, TCHAR *FilePath);

	// Send함수
	void SendSignal(HEAD_TYPE signal);
	//#1 감염 시작 시간을 알림
	void SendRasomStart(void);
	//#2 진행률 알림
	void SendTestProgress(int Count);
	//#3 결과 전송
	void SendTestResult(void);

	//해제
	void ReleaseSocket(void);

};

