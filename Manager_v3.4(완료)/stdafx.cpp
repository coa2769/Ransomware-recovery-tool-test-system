
// stdafx.cpp : ǥ�� ���� ���ϸ� ��� �ִ� �ҽ� �����Դϴ�.
// Manager_v1.0.pch�� �̸� �����ϵ� ����� �˴ϴ�.
// stdafx.obj���� �̸� �����ϵ� ���� ������ ���Ե˴ϴ�.

#include "stdafx.h"
#include"Manager_v1.0.h"

void ErrorHandler(LPCTSTR lpszText, BOOL exit)
{
	//Error ó��
	AfxMessageBox(lpszText);
	if (exit == TRUE)
	{
		theApp.m_pMainWnd->PostMessage(WM_CLOSE);
		//theApp.m_pMainWnd->PostMessage(WM_QUIT);
	}
}
