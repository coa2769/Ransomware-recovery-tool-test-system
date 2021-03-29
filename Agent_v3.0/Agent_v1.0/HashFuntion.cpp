#include"stdafx.h"
#include"HashFuntion.h"


BOOL SameValue(BYTE* fir, BYTE* scn)
{
	bool is = TRUE;
	for (int i = 0; i < 20; i++)
	{
		if (fir[i] != scn[i])
		{
			is = FALSE;
			break;
		}
	}
	return is;
}

BOOL FileHashCalution(TCHAR *pFilePath, BYTE *Hash)
{
	//���� ����
	//CreateFile() : �ּ� ���� WindowXP, ���� �ڵ��� �Ҵ��ϴ� �Լ�
	//https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-createfilea
	//HANDLE CreateFileA(
	//	 LPCSTR lpFileName,									//���� ���
	//	 DWORD dwDesiredAccess,								//������ �׼��� Ÿ��
	//	 DWORD dwShareMode,									//������ ���� ���
	//	 LPSECURITY_ATTRIBUTES lpSecurityAttributes,		//���ȿ� ���õ� ����ü
	//	 DWORD dwCreationDisposition,						//������ � ������� �����ΰ�?
	//	 DWORD dwFlagsAndAttributes,						//���Ͽ� ���� �Ӽ�
	//	 HANDLE hTemplateFile								//������ �߰� �Ӽ�, �� ������ ����
	//);

	HANDLE hFile;
	if ((hFile = CreateFile(pFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		ErrorHandler("Failed To Open File", true);
		return FALSE;
	}

	//���� ���� �˾ƿ���
	//GetFileSize() : �ּ� ���� WindowXP, ������ ����� �����´�. 4GB�̻��� �������� ���Ѵ�.
	//������Ʈ�� 32bit�̰� ū ���Ͽ� ���� ó���� �������� ���� ���̴�.
	DWORD nLength = 0;
	DWORD nResult = 0;
	if ((nLength = GetFileSize(hFile, NULL)) == INVALID_FILE_SIZE)
	{
		CloseHandle(hFile);
		ErrorHandler("Not Know The File Size", true);
		return FALSE;
	}

	BYTE *pBuffer = new BYTE[nLength];
	//���� �о����
	if (ReadFile(hFile, pBuffer, sizeof(BYTE) * nLength, &nResult, NULL) == FALSE)
	{
		CloseHandle(hFile);
		ErrorHandler("Failure File Read", true);
		return FALSE;
	}

	//winCrypto api
	HCRYPTPROV hProv;
	HCRYPTHASH hHash;
	DWORD dwHash = 0;

	//SHA-1 �ִ� 160bit
	if (CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)
	{
		CloseHandle(hFile);
		ErrorHandler("Failed To Create Object <CryptAcquire>", true);
		return FALSE;
	}

	if (CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash) == FALSE)
	{
		CloseHandle(hFile);
		ErrorHandler("Failed To Create Object <Hash>", true);
		return FALSE;
	}

	if (CryptHashData(hHash, pBuffer, nLength, 0) == FALSE)
	{
		CloseHandle(hFile);
		ErrorHandler("failed Hash Calculation", true);
		return FALSE;
	}

	//�ִ� 20byte, ������ ũ�Ⱑ �۴ٸ� �� ���� 20byte���� ���� �� �ִ�.
	dwHash = 20;
	if (CryptGetHashParam(hHash, HP_HASHVAL, Hash, &dwHash, 0) == FALSE)
	{
		CloseHandle(hFile);
		ErrorHandler("Failed To Get Value",true);
		return FALSE;
	}

	if (hHash) CryptDestroyHash(hHash);
	if (hProv) CryptReleaseContext(hProv, 0);

	//���� �ݱ�
	CloseHandle(hFile);

	return TRUE;
}