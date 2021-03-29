#pragma once
#include<time.h>

////////////////////////////////////////////////////////////////////////////////////////////
//�ء� CCriticalSection �ء�
extern CCriticalSection g_csInfo;

typedef struct TEST_TIME
{
	SYSTEMTIME InfecionStart;
	SYSTEMTIME InfectionEnd;
	SYSTEMTIME RecoveryStart;
	SYSTEMTIME RecoveryEnd;
}TEST_TIME;

typedef enum TEST_OS_STATE
{
	STATE_READY,
	STATE_ACTIVE,
	STATE_TEST,
	STATE_TEST_COMPLETE,
	STATE_CLOSE,
	STATE_SHOW_REPORT,
	STATE_DISABLE,
	STATE_ERROR
}TEST_OS_STATE;

class CTestOSInfo
{
private:
	TEST_OS			m_os;
	TEST_OS_STATE	m_state;
	TEST_TIME		m_TestTime;
	CString			m_strUUID;

	CList<FILE_INFO*> m_listResult;
	int m_nTestFileCount;
	int m_nRecovery;
	int m_nReportCount;	//���° �����ΰ�?
public:
	CTestOSInfo(TEST_OS os);
	~CTestOSInfo();

	//get
	TEST_OS getOS(void) { return m_os; }
	TEST_TIME getTestTime(void) { return m_TestTime; }
	TEST_OS_STATE getState(void) { return m_state; }
	int GetRecoveryFileCount(void) { return m_nRecovery; }
	int GetResultCount(void) { return m_nTestFileCount; }
	CString GetUUID(void) { return m_strUUID; }

	//set
	void SetUUID(CString& str) { m_strUUID = str; }
	void SetState(TEST_OS_STATE state) { m_state = state; }

	//���� ����
	void SetInfectionStart(void)
	{
		GetLocalTime(&m_TestTime.InfecionStart);
	}
	//���� ��
	void SetInfectionEnd(void)
	{
		GetLocalTime(&m_TestTime.InfectionEnd);
	}
	//���� ����
	void SetRecoveryStart(void)
	{
		GetLocalTime(&m_TestTime.RecoveryStart);
	}
	//���� ��
	void SetRecoveryEnd(void)
	{
		GetLocalTime(&m_TestTime.RecoveryEnd);
	}

	void InitResult(void)
	{
		m_nRecovery = 0;
		m_nTestFileCount = 0;
		m_nReportCount = 0;
		m_TestTime.InfecionStart = { 0 };
		m_TestTime.InfectionEnd = { 0 };
		m_TestTime.RecoveryEnd = { 0 };
		m_TestTime.RecoveryStart = { 0 };
		//��� ����
		ReleaseResult();
	}

	void AddResult(FILE_INFO& file)
	{
		FILE_INFO *temp = new FILE_INFO;
		memcpy(temp->FileName, file.FileName, sizeof(TCHAR)*(FILENAME_MAX + 1));
		memcpy(temp->OriginalHash, file.OriginalHash, sizeof(BYTE) * 20);
		memcpy(temp->InfectionHash, file.InfectionHash, sizeof(BYTE) * 20);
		memcpy(temp->RecoveryHash, file.RecoveryHash, sizeof(BYTE) * 20);
		m_listResult.AddTail(temp);
	}

	void ReleaseResult(void)
	{
		POSITION rPos = m_listResult.GetHeadPosition();
		FILE_INFO *file;
		while (rPos)
		{
			file = m_listResult.GetNext(rPos);
			delete file;
		}

		m_listResult.RemoveAll();
	}

	void CountRecoveryFile(void);
	void ExportData(void);
};

