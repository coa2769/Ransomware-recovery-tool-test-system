// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �Ǵ� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#include <tchar.h>
#include"..\..\Common\Common.h"

#include<WinSock2.h>
#include<WS2tcpip.h>
#pragma comment(lib,"ws2_32")
void ErrorHandler(char *str, bool isErrorCode);
