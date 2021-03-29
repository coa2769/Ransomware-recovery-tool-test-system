#pragma once
class CVBoxController
{
private:
	FILE* RunShellCommand(const TCHAR* command);

	CString m_CVBoxManagerPath;
public:
	CVBoxController();
	~CVBoxController();

	//VBoxManager�� �ִ°�? Ȯ�� �� path����
	void Initialize(void);

	//list�˻� & �� TestOSInfo�� UUID���� ����
	void CheckVMList();
	//���� list�˻�
	BOOL CheckBVMExecution(CString& UUID);
	//VM����
	void ExevuteVM(CString& UUID);
	//VM����
	void QuitVM(CString& UUID);
	//�������� �ִ°�?
	BOOL HaveSnapshot(CString& UUID);
	//������ ���
	void TakeSnapshot(CString& UUID);
	//������ ����
	void RestoreSnampshot(CString& UUID);

	void Release(void);
};

