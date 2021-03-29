#include "stdafx.h"
#include "VBoxController.h"
#include<io.h>
#include<iostream>	
using namespace std;

#include"Manager_v1.0.h"
#include"TestOSInfo.h"

CVBoxController::CVBoxController()
{
}


CVBoxController::~CVBoxController()
{
}

FILE* CVBoxController::RunShellCommand(const TCHAR* command)
{
	BOOL success = FALSE;

	//1. ������ ����
	//1-1. ���� �Ӽ� ����(������ �ڵ� ��� ����)
	//https://wonjayk.tistory.com/271
	SECURITY_ATTRIBUTES securityAttr = { 0 };
	securityAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttr.bInheritHandle = TRUE;			//��Ӱ����� Handle�̶�� ��.
	securityAttr.lpSecurityDescriptor = NULL;	//���� ����

	/*=======================================================*
	*                                                        *
	*  �θ� �ڽ� ���μ����� ����Ǵ� ������ ����               *
	*                                                        *
	*  Parent                                         Child  *
	*  +------+                                    +------+  *
	*  |  hParentWriteToChild  ---> hChildReadFromParent  |  *
	*  |      |                                    |      |  *
	*  |  hParentReadFromChild <--- hChildWriteToParent   |  *
	*  +------+                                    +------+  *
	*                                                        *
	*========================================================*/

	//1-2. �ܹ��� ������ ����
	//https://artisticbit.tistory.com/entry/08%EC%9E%A5-%ED%94%84%EB%A1%9C%EC%84%B8%EC%8A%A4%EA%B0%84-%ED%86%B5%EC%8B%A0IPC-2-%E2%91%A1%ED%8C%8C%EC%9D%B4%ED%94%84-%EB%B0%A9%EC%8B%9D%EC%9D%98-IPC
	HANDLE hParentReadFromChild = NULL;
	HANDLE hChildWriteToParent = NULL;
	success = CreatePipe(&hParentReadFromChild, &hChildWriteToParent, &securityAttr, 0);
	if (success == FALSE)
	{
		printf("ERROR : ������ ���� ����\n");
		return NULL;
	}

	//1-3. �ڵ��� Ư�� �Ӽ��� �����մϴ�.
	//�ڽ� ���μ������� ��ü �ڵ��� ����� �� �ִ�.
	//https://docs.microsoft.com/ko-kr/windows/desktop/api/handleapi/nf-handleapi-sethandleinformation
	success = SetHandleInformation(hParentReadFromChild, HANDLE_FLAG_INHERIT, 0);
	if (success == FALSE)
	{
		printf("ERROR : �ڵ� �Ӽ� ���濡 ����\n");
		return NULL;
	}

	//2. �ڽ� ���μ��� ����
	//2-1. �ڽ� ���μ����� ǥ�� �Է�, ���, ������� �ڵ� ����
	STARTUPINFO startupInfo = { 0 };
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.hStdError = hChildWriteToParent;
	startupInfo.hStdOutput = hChildWriteToParent;
	startupInfo.hStdInput = hChildWriteToParent;
	startupInfo.dwFlags |= STARTF_USESTDHANDLES;

	PROCESS_INFORMATION processInfo = { 0 };

	//2-2. �ڽ� ���μ��� ����
	success = CreateProcess(
		NULL,
		(LPTSTR)command,
		NULL,
		NULL,
		TRUE,
		DETACHED_PROCESS,
		NULL,
		NULL,
		&startupInfo,
		&processInfo);
	//�ڽ� ���μ�����console�� �������� �ʴ� ���
	//DWORD dwCreationFlags		DETACHED_PROCESS�� flag�� �ش�.

	CloseHandle(hChildWriteToParent);

	if (success == FALSE)
	{
		printf("ERROR : ���μ��� ���� ����\n");
		return NULL;
	}

	//3. �ڵ� ��ȯ
	//https://docs.microsoft.com/ko-kr/cpp/c-runtime-library/reference/open-osfhandle?view=vs-2017
	//������ �ڵ� -> ���� �ڵ�
	const int fd = _open_osfhandle((intptr_t)hParentReadFromChild, 0);
	return _wfdopen(fd, _T("r"));

	//CloseHandle(hParentReadFromChild);
}

//VBoxManager�� �ִ°�? Ȯ�� �� path����
void CVBoxController::Initialize(void)
{
	//VBoxManage �ּ�
	CString Findfile;
	TCHAR path[MAX_PATH] = { 0 };
	ExpandEnvironmentStrings(_T("%ProgramW6432%"), path, sizeof(TCHAR)*MAX_PATH);
	Findfile += path;
	Findfile += _T("\\Oracle\\VirtualBox\\VBoxManage.exe");

	//���� �ִ��� Ȯ��
	//exists �����ϴ�.
	const int existsFile = _taccess(Findfile, 0x00);
	if (existsFile != 0)
	{
		printf("ERROR : VBoxManage�� �����ϴ�.\n");
		ErrorHandler(_T("ERROR : VBoxManage�� �����ϴ�. Virtualbox�� ��ġ �ƴ��� Ȯ���� �ּ���.\n"), TRUE);
	}

	m_CVBoxManagerPath = _T("\"");
	m_CVBoxManagerPath += Findfile;
	m_CVBoxManagerPath += _T("\"");

}

//fget
//https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fgets-fgetws?view=vs-2017
//readfile�Լ�
//https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-readfile
//https://blog.naver.com/power2845/50144349557
//�� ��
//https://wwwi.tistory.com/218
//CString �Ľ̿� ���� �Լ�
//https://shaeod.tistory.com/331

//list�˻� & �� TestOSInfo�� UUID���� ����
void CVBoxController::CheckVMList()
{
	//��ɾ� ����
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" list -s vms");

	//���
	_tprintf(_T("%s\n"), command.GetBuffer(0));
	//��ɾ� ����
	FILE* fp = RunShellCommand(command);
	
	//��� ��� & ��
	BOOL success = FALSE;
	char buffer[1024] = { 0 };
	int index = 0;
	CString UUID;
	CString Name;
	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);

		UUID = buffer;
		index = UUID.Find('{');
		Name = UUID.Left(index - 1);
		UUID = UUID.Mid(index);
		index = UUID.Find('}');
		UUID = UUID.Left(index + 1);

		if (Name == _T("\"TestWin7\""))
		{
			theApp.m_mapTestOsInfo[WINDOW_7]->SetUUID(UUID);
			theApp.m_mapTestOsInfo[WINDOW_7]->SetState(STATE_ACTIVE);
		}
		else if (Name == _T("\"TestWin81\""))
		{
			theApp.m_mapTestOsInfo[WINDOW_8]->SetUUID(UUID);
			theApp.m_mapTestOsInfo[WINDOW_8]->SetState(STATE_ACTIVE);
		}
		else if (Name == _T("\"TestWin10\""))
		{
			theApp.m_mapTestOsInfo[WINDOW_10]->SetUUID(UUID);
			theApp.m_mapTestOsInfo[WINDOW_10]->SetState(STATE_ACTIVE);
		}
	}
	
	
	fclose(fp);
	cout << endl;
}

//���� list�˻�
BOOL CVBoxController::CheckBVMExecution(CString& UUID)
{
	//��ɾ� ����
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" list runningvms");

	//���
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//��ɾ� ����
	FILE* fp = RunShellCommand(command);

	//��� ��� & ��
	BOOL success = FALSE;
	char buffer[1024] = { 0 };
	int index = 0;
	CString CStrUUID;

	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);

		CStrUUID = buffer;
		index = CStrUUID.Find('{');
		CStrUUID = CStrUUID.Mid(index);
		index = CStrUUID.Find('}');
		CStrUUID = CStrUUID.Left(index + 1);
		if (CStrUUID == UUID)
		{
			success = TRUE;
			break;
		}
	}

	cout << endl;

	fclose(fp);
	return success;

}

//VM����
void CVBoxController::ExevuteVM(CString& UUID)
{
	//��ɾ� ����
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" startvm ");
	command += UUID;
	command += _T(" --type headless");

	//���
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//��ɾ� ����
	FILE* fp = RunShellCommand(command);

	char buffer[1024] = { 0 };
	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);
	}

}

void CVBoxController::QuitVM(CString& UUID)
{
	//��ɾ� ����
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" controlvm ");
	command += UUID;
	command += _T(" poweroff");

	//���
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//��ɾ� ����
	FILE* fp = RunShellCommand(command);

	char buffer[1024] = { 0 };
	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);
	}
}

//������ ���
void CVBoxController::TakeSnapshot(CString& UUID)
{
	//��ɾ� ����
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" snapshot ");
	command += UUID;
	command += _T(" take \"first\"");

	//���
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//��ɾ� ����
	FILE* fp = RunShellCommand(command);

	char buffer[1024] = { 0 };
	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);
	}

}

//������ ����
void CVBoxController::RestoreSnampshot(CString& UUID)
{
	//��ɾ� ����
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" snapshot ");
	command += UUID;
	command += _T(" restorecurrent");

	//���
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//��ɾ� ����
	FILE* fp = RunShellCommand(command);

	char buffer[1024] = { 0 };
	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);
	}
}

//�������� �ִ°�?
BOOL CVBoxController::HaveSnapshot(CString& UUID)
{
	//��ɾ� ����
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" snapshot ");
	command += UUID;
	command += _T(" list");

	//���
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//��ɾ� ����
	FILE* fp = RunShellCommand(command);
	
	size_t count = 0;
	unsigned char buffer[1024 + 1] = { 0 };
	while (!feof(fp))
	{
		count = fread(buffer, 1, sizeof(char) * 1024, fp);
		buffer[count] = '\0';
		cout << buffer;
	}

	return TRUE;
}

//����
void CVBoxController::Release(void)
{
	//��ɾ� ����
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" list runningvms");

	//���
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//��ɾ� ����
	FILE* fp = RunShellCommand(command);

	//��� ��� & ��
	BOOL success = FALSE;
	char buffer[1024] = { 0 };
	int index = 0;
	int count = 0;
	CString CStrUUID[WINDOW_10 + 1];
	CStrUUID[0] = _T("");
	CStrUUID[1] = _T("");
	CStrUUID[2] = _T("");

	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);

		CStrUUID[count] = buffer;
		index = CStrUUID[count].Find('{');
		CStrUUID[count] = CStrUUID[count].Mid(index);
		index = CStrUUID[count].Find('}');
		CStrUUID[count] = CStrUUID[count].Left(index + 1);

		count++;
	}

	cout << endl;

	fclose(fp);

	for (int i = 0; i < (WINDOW_10 + 1); i++)
	{
		if (CStrUUID[i] != _T(""))
		{
			this->QuitVM(CStrUUID[i]);
			this->RestoreSnampshot(CStrUUID[i]);
		}
	}
}