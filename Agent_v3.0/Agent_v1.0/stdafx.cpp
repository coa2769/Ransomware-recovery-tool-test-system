// stdafx.cpp : ǥ�� ���� ���ϸ� ��� �ִ� �ҽ� �����Դϴ�.
// Agent_v1.0.pch�� �̸� �����ϵ� ����� �˴ϴ�.
// stdafx.obj���� �̸� �����ϵ� ���� ������ ���Ե˴ϴ�.

#include "stdafx.h"


void ErrorHandler(char *str, bool isErrorCode)
{
	if (isErrorCode == true)
	{
		DWORD code = GetLastError();
		printf("ERROR : %s(code : %d)\n", str, code);
		exit(0);
		
	}
	else
	{
		printf("ERROR : %s\n", str);

	}
}
// TODO: �ʿ��� �߰� �����
// �� ������ �ƴ� STDAFX.H���� �����մϴ�.
