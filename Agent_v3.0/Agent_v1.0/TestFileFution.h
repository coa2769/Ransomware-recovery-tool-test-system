#pragma once
#include<map>
#include<vector>
#include<string>

//���� event
extern HANDLE g_hEventEndMonitoring;


//�ء� ���� ���� �Լ�(thread�� �ȴ�.)
//���� ��� : https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-findfirstchangenotificationa
//���� : https://docs.microsoft.com/ko-kr/windows/desktop/FileIO/obtaining-directory-change-notifications
//����Լ��� ���� : https://docs.microsoft.com/ko-kr/windows/desktop/Sync/wait-functions
//�̷� ����� �ִ�.
//https://www.benjaminlog.com/entry/ReadDirectoryChangesW
DWORD WINAPI MonitoringFolder(LPVOID pParam);
DWORD WINAPI ThreadSaveHash(LPVOID pParam);

class CTestFileInfo
{
private:
	//hash_map -> ������ hash_map ��� unordered_map�� ���ȴ�.
	//C++��� ������ �ٲ�鼭 �̷��� �Ǿ��µ� �̷� �κ��� OS������ ��� ������ ����̴�.
	//http://www.hanbit.co.kr/channel/category/category_view.html?cms_code=CMS4230438179
	//�ϴ� map�� ����Ѵ�.


	//�� Test������ ���ϵ� ����
	std::vector<FILE_INFO*>				m_vecTestFile;
	std::map<std::wstring, FILE_INFO*>	m_mapTestFile;
	std::map<std::wstring, _FILETIME>	m_mapFileLastWrite; //���� ���� �ð� Accet�� window vista���� ������ �ȵȴ�.  

	//<���� ����>
	//C:\Users\cC_er\Documents

	TCHAR m_FolderPath[MAX_PATH];	//�׽�Ʈ�� ����
	int	m_nChangeFileCount = 0;		//����� ���� ����
	
	TCHAR m_RasomwareName[MAX_PATH];
	TCHAR m_RecoveryName[MAX_PATH];
	TCHAR m_RansomwarePath[MAX_PATH];
	TCHAR m_RecoveryPath[MAX_PATH];
public:
	//�Լ�
	CTestFileInfo();
	~CTestFileInfo();
	TCHAR* GetFolderPath(void) { return m_FolderPath; }
	TCHAR* GetRasomwarePath(void) { return m_RansomwarePath; }
	TCHAR* GetRasomwareName(void) { return m_RasomwareName; }
	TCHAR* GetRecoveryPath(void) { return m_RecoveryPath; }
	TCHAR* GetRecoveryName(void) { return m_RecoveryName; }
	size_t GetTestFileCount(void) { return m_vecTestFile.size(); }
	int GetChageTestFileCount(void) { return m_nChangeFileCount; }

	void GetTestFileInfo(int index, FILE_INFO& info)
	{
		FILE_INFO *temp = m_vecTestFile[index];
		memcpy_s(info.FileName, sizeof(wchar_t) * (MAX_PATH),temp->FileName, sizeof(wchar_t) * (MAX_PATH));
		memcpy_s(info.OriginalHash, sizeof(unsigned char) * 20, temp->OriginalHash, sizeof(unsigned char) * 20);
		memcpy_s(info.InfectionHash, sizeof(unsigned char) * 20, temp->InfectionHash, sizeof(unsigned char) * 20);
		memcpy_s(info.RecoveryHash, sizeof(unsigned char) * 20, temp->RecoveryHash, sizeof(unsigned char) * 20);
	}

	BOOL SameChangecountAndTestFileCount(void)
	{
		BOOL is = FALSE;
		if (m_nChangeFileCount == m_vecTestFile.size())
		{
			is = TRUE;
		}
		return is;
	}

	void SetRasomwareFilePath(TCHAR* path)
	{
		memcpy_s(m_RansomwarePath, sizeof(TCHAR) * MAX_PATH, path, sizeof(TCHAR) * MAX_PATH);
	}
	void SetRecoveryFilePath(TCHAR* path)
	{
		memcpy_s(m_RecoveryPath, sizeof(TCHAR) * MAX_PATH, path, sizeof(TCHAR) * MAX_PATH);

	}
	void SetRansomwareFileName(TCHAR* name)
	{
		memcpy_s(m_RasomwareName,sizeof(TCHAR) * MAX_PATH, name, sizeof(TCHAR) * MAX_PATH);
	}

	void SetRecoveryFileName(TCHAR* name)
	{
		memcpy_s(m_RecoveryName, sizeof(TCHAR) * MAX_PATH, name, sizeof(TCHAR) * MAX_PATH);
	}

	//���� �̸� �Ľ�
	void ParsingFileName(TCHAR* ori, TCHAR* Name);

	//1. Ư�� ���� ���� ���� ������ �����´�.
	BOOL FindFilesInAFolder(HANDLE& hFind, WIN32_FIND_DATA& fileData);
	//2. ������ ���ϵ� �̸��� ���� ����
	void InitTestFolderAndFile(void);
	//3. ��� ������ ����
	void ReleaseTestFolderAndFile(void);
	//4. ������ ��� ����Ǿ��°�? (MAC�ð����� �Ǻ�)
	//http://www.tipssoft.com/bulletin/board.php?bo_table=FAQ&wr_id=14
	void FindfilesChange(void);
	//���� �ð� ��� ������Ʈ
	void InitFileMACtime(void);
	//5. ���� Hash�� ���� ����
	//void HashOriginal(void);
	void HashAfterRasomware(void);
	void HashAfterRecovery(void);

};
