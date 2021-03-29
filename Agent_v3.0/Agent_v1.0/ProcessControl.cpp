#include"stdafx.h"
#include"ProcessControl.h"
#include<TlHelp32.h>
#include<string>

CProcessControl::CProcessControl()
	:m_dwRunProcessID(0)
{
}
CProcessControl::~CProcessControl()
{
}


void CProcessControl::RunChildProcess(TCHAR* filePath)
{
	BOOL bSuccess;

	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi = { 0 };
	
	//ERROR 193 : CreateProcess�� exe���ϸ��� �����Ѵ�. txt, png�� ������ ���� �Ϸ��� Shell�Լ��� ����Ѵ�.
	//�츮�� ���� txt, png���� ������ ���� Ŭ�� �� �� �� ������ �о���� �� �ִ� ���� ���α׷��� registry���� ã�´�.
	//Shell�� �̰� �ڵ����� �ذ����ְ� CreateProcess�� ������ �о�� �� �ִ� ���� ���α׷��� command���ڸ� �־��ָ� �����ϴ�.
	//ERROR 740 : admin������ �ʿ��� ���ø����̼��� ����.
	bSuccess = CreateProcess(
		filePath,
		NULL,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi);

	if (bSuccess == FALSE)
	{
		ErrorHandler("���μ��� ���� ����", true);
	}

	m_dwRunProcessID = pi.dwProcessId;
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

//���μ����� ���� ���ΰ�?
BOOL CProcessControl::IsProcessRunning(TCHAR* fileName)
{
	BOOL bNext;
	BOOL bIs = FALSE;

	//������ ���μ��� �������� �����´�.
	//TH32CS_SNAPPROCESS : �������� �ý��ۿ� �ִ� ��� ���μ����� �����´�.
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	std::wstring process;
	char buf[MAX_PATH] = { 0 };
	PROCESSENTRY32 ppe;
	ppe.dwSize = sizeof(PROCESSENTRY32);

	//ù ���μ��� ������ �����´�.
	bNext = Process32First(hSnapshot, &ppe);

	while (bNext)
	{
		if (_tcscmp(ppe.szExeFile, fileName) == 0)
		{
			bIs = TRUE;
			break;
		}
		bNext = Process32Next(hSnapshot, &ppe);
	}

	CloseHandle(hSnapshot);
	return bIs;
}

void CProcessControl::TerminateChildProcess(TCHAR* fileName)
{
	//�������̸� ����
	if (IsProcessRunning(fileName) == TRUE)
	{
		DWORD dwDesiredAccess = PROCESS_TERMINATE;
		BOOL bInHeritHandle = FALSE;
		//���� ���α׷��� ���� ���ܵǾ� ���� �� �ִ�.
		HANDLE hProcess = OpenProcess(dwDesiredAccess, bInHeritHandle, m_dwRunProcessID);
		if (hProcess == NULL)
			return;

		BOOL result = TerminateProcess(hProcess, 0);

		if (result == FALSE)
		{
			_tprintf(_T("TerminateProcess() ����"));
		}

		WaitForSingleObject(hProcess, INFINITE);
		m_dwRunProcessID = 0;

		CloseHandle(hProcess);
	}
}