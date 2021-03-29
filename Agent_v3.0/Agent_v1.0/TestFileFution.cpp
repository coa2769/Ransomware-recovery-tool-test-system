#include"stdafx.h"
#include"TestFileFution.h"
#include<ShlObj.h>
#include<strsafe.h>
#pragma comment(lib,"User32.lib")

#include"HashFuntion.h"
#include"ProcessControl.h"
#include"ClientSocket.h"

/////////////////////////////////////////////////////////////////////////////
//�ء� Thread �ء�
/////////////////////////////////////////////////////////////////////////////
HANDLE g_hEventEndMonitoring; //������ �߰��� �����Ű�� ���� Event
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI MonitoringFolder(LPVOID pParam)
{
	printf("************* MonitoringFolder() **************\n");
	//LPVOID == void* ���⿡ void* �迭�� �ּҸ� �־���.
	//�� ����� LPVOID = void**
	//(void**)[0] = void* <-�ٵ� [0]�� [1]�� ���� CTestFileInfo*�� CClientSocekt*�̴�.
	CTestFileInfo*	CTFInfo = ((CTestFileInfo**)pParam)[0]; //�� �ּ� ���� �� �Ѿ���� Ȯ��
	CClientSocket* Client = ((CClientSocket**)pParam)[1];

	CProcessControl CProcControl;
	DWORD			dwWaitState;	//wait�Լ��� ��ȯ ����
	HANDLE			hArr[2];		//[0] : ������ �ڵ�
									//[1] : ������ Event
	printf("Create a Stop Event\n");
	g_hEventEndMonitoring = CreateEvent(NULL, TRUE, FALSE, _T("EndMonitoring"));
	hArr[1] = g_hEventEndMonitoring;

	//�����ϴ� ���� ����
	//FILE_NOTIFY_CHANGE_LAST_WRITE
	//�� flag�� ������ ��� �����Ҷ�, ������ ���� �� �����Ѵ�.
	printf("C:\\Users\\cC_er\\Documents Start Monitoring\n");
	if ((hArr[0] = FindFirstChangeNotification(CTFInfo->GetFolderPath(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE)) == INVALID_HANDLE_VALUE)
	{
		ErrorHandler("FindFirstChangeNotification() Fail!!", true);
	}

	if (hArr[0] == NULL)
	{
		ErrorHandler("The Handle Value is NULL.", true);
	}

	//���α׷� ����
	if (Client->GetTestState() == START_RASOM)
	{
		printf("!!RUN Rasomware!!\n");
		//CProcControl.RunChildProcess(_T("C:\\Windows\\System32\\notepad.exe"));
		CProcControl.RunChildProcess(CTFInfo->GetRasomwarePath());
	}
	else if (Client->GetTestState() == START_RECOVERY)
	{
		printf("!!RUN Recovery!!\n");
		//CProcControl.RunChildProcess(_T("C:\\Windows\\System32\\notepad.exe"));
		CProcControl.RunChildProcess(CTFInfo->GetRecoveryPath());
	}

	DWORD milliSec = INFINITE;
	//BOOL bSignal;
	//���� ����
	while (true)
	{
		printf("Waiting for notification.......\n\n");

		//WaitForSingleObject()
		//�ּ� ���� : windowXP
		//dwWaitStatus = WaitForSingleObject(hArr[0], INFINITE);
		dwWaitState = WaitForMultipleObjects(2, hArr, FALSE, milliSec);

		if (dwWaitState == WAIT_OBJECT_0)
		{
			printf("[ Folder Change Detection ]\n");
			if (FindNextChangeNotification(hArr[0]) == FALSE)
			{
				ErrorHandler("FindNextChangeNotification() Fail !!", true);
				break;
			}
			
			//��� ���Ͽ� ��ȭ�� �־��� ��?
			//FILE_NOTIFY_CHANGE_LAST_WRITE�� ������ ������ �� ������ �� �� ��� ���� �ϹǷ� �ʿ�
			CTFInfo->FindfilesChange();

			//��ȭ�� ���� ���� ����
			Client->SendTestProgress(CTFInfo->GetChageTestFileCount());
			printf("=====(%d / %d)\n", CTFInfo->GetChageTestFileCount(), CTFInfo->GetTestFileCount());
			
			//��� ��ȭ�� �Ͼ�°�?
			if (CTFInfo->SameChangecountAndTestFileCount() == TRUE)
			{
				printf("ALL Changed\n");
				milliSec = 2000; //2��				
			}
		}
		//�ι�° Event�� �߻�
		else if (dwWaitState == (WAIT_OBJECT_0 + 1))
		{
			printf("g_hEventEndMonitoring Signal\n");
			break;
		}
		//��� ��ȭ �߱� Ư�� �ð����� ���浵 ������
		else if (dwWaitState == WAIT_TIMEOUT)
		{
			//������ ����
			break;
		}
	}

	//���� MAC time �ʱ�ȭ
	CTFInfo->InitFileMACtime();

	//���� ���� ���α׷� ����
	printf("!! Exit The Executable Program !!\n");
	if (Client->GetTestState() == START_RASOM)
	{
		//CProcControl.TerminateChildProcess(_T("notepad.exe"));
		CProcControl.TerminateChildProcess(CTFInfo->GetRasomwareName());
	}
	else if (Client->GetTestState() == START_RECOVERY)
	{
		//CProcControl.TerminateChildProcess(_T("notepad.exe"));
		CProcControl.TerminateChildProcess(CTFInfo->GetRecoveryName());
	}

	printf("Close Folder Detection Handle\n");
	//���� handl �ݱ�
	FindCloseChangeNotification(hArr[0]);
	//������ Evnet
	CloseHandle(g_hEventEndMonitoring);
	g_hEventEndMonitoring = 0;


	if (dwWaitState == WAIT_TIMEOUT)
	{
		//���� �Ϸ��� �ð� ����
		if (Client->GetTestState() == START_RASOM)
		{
			printf("!! Send Infection End Time !!\n");
			Client->SendSignal(END_RASOM);
		}
		else if (Client->GetTestState() == START_RECOVERY)
		{
			printf("!! Send Recovery End Time !!\n");
			Client->SendSignal(END_RECOVERY);
		}
	}

	return 0;
}
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ThreadSaveHash(LPVOID pParam)
{
	CTestFileInfo* CTFInfo = ((CTestFileInfo**)pParam)[0];
	CClientSocket* Client = ((CClientSocket**)pParam)[1];

	if (Client->GetTestState() == START_RASOM)
	{
		CTFInfo->HashAfterRasomware();
		Client->SendSignal(SAVE_RASOMWARE_HASH);


	}
	else if (Client->GetTestState() == START_RECOVERY)
	{
		CTFInfo->HashAfterRecovery();
		Client->SendSignal(SAVE_RECOVERY_HASH);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////


CTestFileInfo::CTestFileInfo()
	:m_nChangeFileCount(0)
{
	//CSIDL_PERSONAL  = CSIDL_MYDOCUMENTS
	if (SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, 0, m_FolderPath) != S_OK)
	{
		ErrorHandler("Not Found Folder To Be Monitored", false);
	}

	g_hEventEndMonitoring = 0;
}

CTestFileInfo::~CTestFileInfo()
{
	ReleaseTestFolderAndFile();
}

//���� �̸� �Ľ�
void CTestFileInfo::ParsingFileName(TCHAR* ori, TCHAR* Name)
{
	//int size;
	TCHAR* temp;
	_tcsncpy_s(Name, sizeof(TCHAR)*MAX_PATH, ori, sizeof(TCHAR)*MAX_PATH);

	while (true)
	{
		temp = _tcsrchr(Name, _T('.'));
		if (temp == NULL)break;

		(*temp) = _T('\0');
		//size = temp - ori;
	}
	
	//_tcsncpy_s(Name, sizeof(TCHAR)*MAX_PATH, ori, size);
}

//HANDLE& hFind		: ó������ INVALID_HANDLE_VALUE ���� ���� handl�� �־��ְ� ���� ������ ���� ���� �Լ� ���ο���
//					  ���� hFind�� �״�� �Ű������� �־��ش�.
//WIN32_FIND_DATA& fileData : ���ϴ� ����� �������� ������ �����͸� �о�´�.
//BOOL ��� �� �о��ų� �����ϸ� FALSE�� ��ȯ�ȴ�.
BOOL CTestFileInfo::FindFilesInAFolder(HANDLE& hFind, WIN32_FIND_DATA& fileData)
{
	TCHAR szDir[MAX_PATH]; //���� ���� ���
	size_t length_of_arg;
	BOOL isComplete = TRUE;

	//��� file�� ��Ÿ���� ���� \*�� ���̰� \n�� �ٿ����ϹǷ� 3ĭ�� ���� �־���Ѵ�.
	//���ڿ��� ������ ���̸� �ʰ��ϴ��� ���� �Ǻ�
	//#TRUE
	StringCchLength(m_FolderPath, MAX_PATH, &length_of_arg);

	if (length_of_arg > (MAX_PATH - 3))
	{
		ErrorHandler("���� ��ΰ� �ʹ� ���.", false);
	}
	//#TRUE
	StringCchCopy(szDir, MAX_PATH, m_FolderPath);
	//#TRUE
	StringCchCat(szDir, MAX_PATH, _T("\\*"));

	//�˻��� ������ ù��°�� WIN32_FIND_DATA�� ������ ��ƿ´�.
	if (hFind == INVALID_HANDLE_VALUE)
	{
		//#TRUE
		hFind = FindFirstFile(szDir, &fileData);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			ErrorHandler("������ handl�� �������⸦ �����߽��ϴ�.", false);
		}
	}
	else
	{
		//#TRUE
		isComplete = FindNextFile(hFind, &fileData);

		//�� �����̸� �������� �Ѿ��.
		if (_tcscmp(fileData.cFileName, _T("desktop.ini")) == 0)
		{
			isComplete = FindNextFile(hFind, &fileData);
		}
		
		if (isComplete == FALSE)
		{
			if (GetLastError() == ERROR_NO_MORE_FILES)
			{
				printf("No More File\n");
			}
			//#TRUE
			FindClose(hFind);
		}

	}

	return isComplete;
}


void CTestFileInfo::InitTestFolderAndFile(void)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE; //handle�ʱ�ȭ
	TCHAR szDir[MAX_PATH];
	TCHAR FileName[MAX_PATH];

	_tprintf(_T("<Target Directory is %s >\n"), m_FolderPath);

	while (FindFilesInAFolder(hFind, ffd) != FALSE)
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0x00000000)
		{
			//�� �ȿ��� file list�� ����� ��
			FILE_INFO *temp = new FILE_INFO;
			//Hash ���
			StringCchCopy(temp->FileName, FILENAME_MAX, ffd.cFileName);
			StringCchCopy(szDir, MAX_PATH, m_FolderPath);
			StringCchCat(szDir, MAX_PATH, _T("\\"));
			StringCchCat(szDir, MAX_PATH, temp->FileName);
			FileHashCalution(szDir, temp->OriginalHash);

			//���� �̸�
			ParsingFileName(ffd.cFileName, FileName);
			m_mapTestFile.insert(std::pair<std::wstring, FILE_INFO*>(FileName, temp));
			m_vecTestFile.push_back(temp);
			//�ʱ� ftLastAccessTime����
			m_mapFileLastWrite.insert(std::pair<std::wstring, _FILETIME>(FileName, ffd.ftLastWriteTime));
			_tprintf(_T("File : %s \n"), temp->FileName);
		}
	}
}

void CTestFileInfo::ReleaseTestFolderAndFile(void)
{
	for (unsigned int i = 0; i < m_vecTestFile.size(); i++)
	{
		delete m_vecTestFile[i];
	}
	m_vecTestFile.clear();
	m_mapTestFile.clear();
	m_mapFileLastWrite.clear();
}


//4. ���ϵ��� Mac�ð��� �����ͼ� ��ȭ ����
void CTestFileInfo::FindfilesChange(void)
{
	_tprintf(_T("## Monitoring File Last Write Time"));
	HANDLE hFind = INVALID_HANDLE_VALUE; //handle�ʱ�ȭ
	WIN32_FIND_DATA ffd;
	TCHAR FileName[MAX_PATH];

	m_nChangeFileCount = 0;
	while (FindFilesInAFolder(hFind, ffd) != FALSE)
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0x00000000)
		{
			ParsingFileName(ffd.cFileName, FileName);
			if ((m_mapFileLastWrite[FileName].dwHighDateTime != ffd.ftLastWriteTime.dwHighDateTime) ||
				(m_mapFileLastWrite[FileName].dwLowDateTime != ffd.ftLastWriteTime.dwLowDateTime))
			{
				m_nChangeFileCount++;
			}
		}
	}
}

//���� �ð� ��� ������Ʈ
void CTestFileInfo::InitFileMACtime(void)
{
	m_mapFileLastWrite.clear();

	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR FileName[MAX_PATH];

	while (FindFilesInAFolder(hFind, ffd) != FALSE)
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0x00000000)
		{
			ParsingFileName(ffd.cFileName, FileName);
			m_mapFileLastWrite.insert(std::pair<std::wstring, _FILETIME>(FileName, ffd.ftLastWriteTime));
		}
	}
}

//5. ���� Hash�� ���� ����
//void CTestFileInfo::HashOriginal(void)
//{
//	TCHAR szDir[MAX_PATH];
//	for (unsigned int i = 0; i < m_vecTestFile.size(); i++)
//	{
//		StringCchCopy(szDir, MAX_PATH, m_FolderPath);
//		StringCchCat(szDir, MAX_PATH, _T("\\"));
//		StringCchCat(szDir, MAX_PATH, m_vecTestFile[i]->FileName);
//		FileHashCalution(szDir, m_vecTestFile[i]->OriginalHash);
//	}
//}

void CTestFileInfo::HashAfterRasomware(void)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR FileName[MAX_PATH];
	TCHAR szDir[MAX_PATH];

	while (FindFilesInAFolder(hFind, ffd) != FALSE)
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0x00000000)
		{
			StringCchCopy(szDir, MAX_PATH, m_FolderPath);
			StringCchCat(szDir, MAX_PATH, _T("\\"));
			StringCchCat(szDir, MAX_PATH, ffd.cFileName);
			ParsingFileName(ffd.cFileName, FileName);
			FileHashCalution(szDir, m_mapTestFile[FileName]->InfectionHash);
		}
	}
}

void CTestFileInfo::HashAfterRecovery(void)
{
	TCHAR szDir[MAX_PATH];
	for (unsigned int i = 0; i < m_vecTestFile.size(); i++)
	{
		StringCchCopy(szDir, MAX_PATH, m_FolderPath);
		StringCchCat(szDir, MAX_PATH, _T("\\"));
		StringCchCat(szDir, MAX_PATH, m_vecTestFile[i]->FileName);
		FileHashCalution(szDir, m_vecTestFile[i]->RecoveryHash);
	}
}