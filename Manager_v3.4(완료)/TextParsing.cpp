#include "stdafx.h"
#include "TextParsing.h"


CTextParsing::CTextParsing()
{
}


CTextParsing::~CTextParsing()
{
}

void CTextParsing::FileNameParsing(CString& fileName)
{
	int count;

	//�������� �� ���ڰ� �ִ� �ε����� ã�� �ش�.
	count = fileName.ReverseFind('\\');
	//index�� ���� ���� ���ڿ��� ���� �� �ִ�.
	fileName = fileName.Mid(count + 1);

}