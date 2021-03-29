#pragma once
#include<stdio.h>

typedef enum _TEST_OS
{
	WINDOW_7,
	WINDOW_8,
	WINDOW_10
}TEST_OS;

typedef struct FILE_INFO
{
	wchar_t FileName[FILENAME_MAX];
	unsigned long FileSize;				//���� ������
	unsigned char OriginalHash[20];		//BYTE
	unsigned char InfectionHash[20];	//BYTE
	unsigned char RecoveryHash[20];		//BYTE
}FILE_INFO;

typedef enum Head_Type
{
	ACCEPT_AGENT,
	FILE_RASOM,
	FILE_RASOM_ERROR,
	FILE_RECOVERY,
	FILE_RECOVERY_ERROR,
	SAVE_RASOMWARE_HASH,
	SAVE_RECOVERY_HASH,
	START_RASOM,
	END_RASOM,
	START_RECOVERY,
	END_RECOVERY,
	TEST_PROGRESS,
	TEST_RESULT,
	HEALTH_CHECK,
	ALL_STOP
}HEAD_TYPE;

typedef struct Tcp_H
{
	HEAD_TYPE type;
	//TEST_OS os;
	int TestFileCount;	//��ü ���� ����
	int ResultCount;	//���� �Ǵ� ������ �Ϸ�� ���� ����
	FILE_INFO  FileInfo;//���� ������ ���� 			
}TCP_H;

