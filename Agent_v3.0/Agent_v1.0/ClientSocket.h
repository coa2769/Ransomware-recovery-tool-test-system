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

	BOOL m_bCloseHealthCheck;	//Health Check Thread�� ���� ��
	//BOOL m_bNoSendHealthCheck;	//���� �ʿ� ���� ��

	//get �Լ�
	//TEST_OS GetOS(void) { return m_os; }
	SOCKET GetSocket(void) { return m_hSocket; }
	HEAD_TYPE GetTestState(void) { return m_TestState; }

	//Connet �Լ�
	void Connect(char* IP, unsigned short Port);

	//Recive(�� �Լ���)
	void Recive(void);
	void FileRecive(TCP_H& Head, TCHAR* FilePath);
	BOOL FileHashCheck(BYTE* OriHash, TCHAR *FilePath);

	// Send�Լ�
	void SendSignal(HEAD_TYPE signal);
	//#1 ���� ���� �ð��� �˸�
	void SendRasomStart(void);
	//#2 ����� �˸�
	void SendTestProgress(int Count);
	//#3 ��� ����
	void SendTestResult(void);

	//����
	void ReleaseSocket(void);

};

