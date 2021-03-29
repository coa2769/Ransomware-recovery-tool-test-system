// UITestOS.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Manager_v1.0.h"
#include "UITestOS.h"

#include"GradientButton.h"
#include"ServerSocket.h"
#include"TestOSInfo.h"
#include"VBoxController.h"
#include"MainFrm.h"

////////////////////////////////////////////////////////////////////////////////////////////
//�ء� CCriticalSection �ء�
//CCriticalSection g_csUI;

//�ء� CEvent
//CEvent g_CEventReadyEnd;

// CUITestOS
IMPLEMENT_DYNAMIC(CUITestOS, CWnd)

CUITestOS::CUITestOS()
{
	//m_CStrState = _T("�׽�Ʈ ��");
	//m_CStrLog = _T("������. ����� ������ �Դϴ�.");

	//m_CStrProgress.Format(_T("����� : %d / %d"), 240, 240);
}

CUITestOS::~CUITestOS()
{
}


BEGIN_MESSAGE_MAP(CUITestOS, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(BTN_ID_START, &CUITestOS::OnBnClickedButtonStart)
END_MESSAGE_MAP()



// CUITestOS �޽��� ó�����Դϴ�.

BOOL CUITestOS::CreateUITestOS(CWnd* CWndParent, TEST_OS os, int x, int y, int ID)
{
	const int Obj_dis = 10;
	const DWORD dwStyle = WS_CHILD | WS_VISIBLE;
	m_os = os;
	if (os == WINDOW_7) m_CStrOSName.Format(_T(" 7"));
	else if (os == WINDOW_8) m_CStrOSName.Format(_T(" 8"));
	else if (os == WINDOW_10) m_CStrOSName.Format(_T("10"));

	return Create(NULL, _T(""), dwStyle, CRect(x, y, x + VERSION_UI_WIDTH + LOG_UI_WIDTH, y + TEST_OS_HEIGHT), CWndParent, ID);
}

int CUITestOS::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	//rect
	m_DisabledRect = new CGradientRect;
	m_DisabledRect->SetGradientRect(this, CRect(0, 0, VERSION_UI_WIDTH, TEST_OS_HEIGHT), RGB(103, 107, 108), RGB(157, 163, 165), LinearGradientMode::Vertical, FALSE);
	
	m_ActivationRect = new CGradientRect;
	m_ActivationRect->SetGradientRect(this, CRect(0, 0, VERSION_UI_WIDTH, TEST_OS_HEIGHT), RGB(43, 197, 90), RGB(22, 160, 63), LinearGradientMode::Vertical, FALSE);
	
	m_LogRect = new CGradientRect;
	m_LogRect->SetGradientRect(this, CRect(VERSION_UI_WIDTH + 10, 0, VERSION_UI_WIDTH + LOG_UI_WIDTH, TEST_OS_HEIGHT), RGB(61, 67, 67), RGB(51, 55, 56), LinearGradientMode::Vertical, FALSE);

	const DWORD dwStyle = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
	const int obj_dis = 20;

	//button
	CRect* rect = m_LogRect->GetRect();
	m_StartButton = new CGradientButton;
	m_StartButton->Create(NULL, dwStyle, CRect(CPoint(VERSION_UI_WIDTH + 10 + obj_dis, obj_dis), CSize(BTN_WIDTH, BTN_HEIGHT)), this, BTN_ID_START);
	m_StartButton->SetNormalGradient(_T("��  ��"), RGB(105, 110, 113), RGB(90, 94, 97), LinearGradientMode::Vertical);
	m_StartButton->SetOverGradient(_T("��  ��"), RGB(227, 229, 229), RGB(141, 144, 146), LinearGradientMode::ForwardDiagonal);
	m_StartButton->SetSelectGradient(_T("��  ��"), RGB(103, 107, 108), RGB(157, 163, 165), LinearGradientMode::Vertical);
	//��ư ����
	if (theApp.m_mapTestOsInfo[m_os]->getState() == STATE_DISABLE)
	{
		DrawDisable();
	}


	return 0;
}

BOOL CUITestOS::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////////////
//Draw Funtion
void CUITestOS::SetChangeStateText(void)
{
	//$
	g_csInfo.Lock();
	TEST_OS_STATE state = theApp.m_mapTestOsInfo[m_os]->getState();
	g_csInfo.Unlock();

	if (state == STATE_READY)
	{
		m_CStrState = _T("�غ���");
	}
	else if (state == STATE_TEST)
	{
		m_CStrState = _T("�׽�Ʈ��");
	}
	else if (state == STATE_TEST_COMPLETE)
	{
		m_CStrState = _T("��� ������");
	}
	else if (state == STATE_DISABLE)
	{
		m_CStrState = _T("��Ȱ��ȭ");
	}
	else if (state == STATE_ERROR)
	{
		m_CStrState = _T("ERROR ����");
	}
	else if (state == STATE_CLOSE)
	{
		m_CStrState = _T("������Ʈ ������");
	}
	else if (state == STATE_SHOW_REPORT)
	{
		m_CStrState = _T("���� ���");
	}
	else if (state == STATE_ACTIVE)
	{
		m_CStrState = _T("");
	}
}

void CUITestOS::DrawDisable(void)
{
	m_StartButton->ShowWindow(SW_HIDE);
	
	//$
	g_csInfo.Lock();
	theApp.m_mapTestOsInfo[m_os]->SetState(STATE_DISABLE);
	g_csInfo.Unlock();

	SetChangeStateText();
	SetLogText(_T("GuestOS�� �����ϴ�."));
}

void CUITestOS::DrawActive(void)
{
	//$
	g_csInfo.Lock();
	theApp.m_mapTestOsInfo[m_os]->SetState(STATE_ACTIVE);
	theApp.m_mapTestOsInfo[m_os]->InitResult();
	g_csInfo.Unlock();

	SetChangeStateText();
	SetProgressText(0, -1);
	SetLogText(_T(""));
	m_StartButton->ShowWindow(SW_SHOW);
	InvalidateRect(NULL);
}

void CUITestOS::DrawStateText(CDC* pDC)
{
	LOGFONT lf;
	CFont smallFont, bigFont;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfWeight = FW_BOLD;
	lf.lfHeight = 80;
	bigFont.CreateFontIndirectW(&lf);
	CFont *pOldFont = pDC->SelectObject(&bigFont);
	CRect *pRect = m_LogRect->GetRect();
	pDC->TextOutW(pRect->left + 10, pRect->top + 10, m_CStrState);
	
	pDC->SetTextColor(RGB(0, 0, 0));
	//���� �۾�
	lf.lfHeight = 30;
	smallFont.CreateFontIndirectW(&lf);
	pDC->SelectObject(&smallFont);
	pDC->TextOutW(pRect->left + 10, pRect->top + 100, m_CStrLog);
	pDC->TextOutW(pRect->left + 10, pRect->top + 130, m_CStrProgress);

	pDC->SelectObject(pOldFont);
}

void CUITestOS::DrawReportCount(CDC* pDC)
{
	LOGFONT lf;
	CFont smallFont, bigFont;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfWeight = FW_BOLD;
	//���� �۾�
	lf.lfHeight = 25;
	bigFont.CreateFontIndirectW(&lf);
	CFont *pOldFont = pDC->SelectObject(&bigFont);
	CRect *pRect = m_LogRect->GetRect();
	
	pDC->SetTextColor(RGB(192, 192, 192));

	CPoint pos(pRect->left + LOG_UI_WIDTH / 2 + 40, pRect->top + 20);
	CString str;

	//$
	g_csInfo.Lock();
	CTestOSInfo * Info = theApp.m_mapTestOsInfo[m_os];

	str.Format(_T("�������� : %hd�� %hd�� %hd��"),
		Info->getTestTime().InfecionStart.wHour, Info->getTestTime().InfecionStart.wMinute, Info->getTestTime().InfecionStart.wSecond);
	pDC->TextOutW(pos.x, pos.y, str);

	str.Format(_T("�������� : %hd�� %hd�� %hd��"),
		Info->getTestTime().InfectionEnd.wHour, Info->getTestTime().InfectionEnd.wMinute, Info->getTestTime().InfectionEnd.wSecond);
	pDC->TextOutW(pos.x, pos.y + 30, str);
	
	str.Format(_T("�������� : %hd�� %hd�� %hd��"),
		Info->getTestTime().RecoveryStart.wHour, Info->getTestTime().RecoveryStart.wMinute, Info->getTestTime().RecoveryStart.wSecond);
	pDC->TextOutW(pos.x, pos.y + 60, str);
	
	str.Format(_T("�������� : %hd�� %hd�� %hd��"),
		Info->getTestTime().RecoveryEnd.wHour, Info->getTestTime().RecoveryEnd.wMinute, Info->getTestTime().RecoveryEnd.wSecond);
	pDC->TextOutW(pos.x, pos.y + 90, str);

	str.Format(_T("����/�� : %d / %d"),Info->GetRecoveryFileCount(),Info->GetResultCount());
	pDC->TextOutW(pos.x, pos.y + 120, str);
	g_csInfo.Unlock();

}

void CUITestOS::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
					   // �׸��� �޽����� ���ؼ��� CWnd::OnPaint()��(��) ȣ������ ���ʽÿ�.
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap memBmp, *pOldBmp;

	CRect clnRect;
	GetClientRect(&clnRect);
	memBmp.CreateCompatibleBitmap(&dc, clnRect.Width(), clnRect.Height());
	pOldBmp = memDC.SelectObject(&memBmp);

	m_LogRect->DrawRect(&memDC);

	//$
	g_csInfo.Lock();
	TEST_OS_STATE state = theApp.m_mapTestOsInfo[m_os]->getState();
	g_csInfo.Unlock();

	if (state == STATE_DISABLE)
	{
		m_DisabledRect->DrawRect(&memDC);
	}
	else
	{
		m_ActivationRect->DrawRect(&memDC);
	}

	//window ����
	//https://blog.naver.com/iamcloud/50146656660
	LOGFONT lf;
	CFont smallFont, bigFont;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfWeight = FW_BOLD;
	//���� �۾�
	lf.lfHeight = 50;
	smallFont.CreateFontIndirectW(&lf);
	CFont *pOldFont = memDC.SelectObject(&smallFont);
	memDC.SetBkMode(TRANSPARENT);
	memDC.SetTextColor(RGB(255, 255, 255));
	memDC.TextOutW(10, 10, _T("Windows"));
	//ū �۾�
	lf.lfHeight = 100;
	bigFont.CreateFontIndirectW(&lf);
	memDC.SelectObject(&bigFont);
	memDC.TextOutW(50, 80, m_CStrOSName);
	memDC.SelectObject(pOldFont);

	DrawStateText(&memDC);

	//$
	g_csInfo.Lock();
	state = theApp.m_mapTestOsInfo[m_os]->getState();
	g_csInfo.Unlock();

	if (state == STATE_TEST || state == STATE_TEST_COMPLETE || state == STATE_SHOW_REPORT)
	{
		DrawReportCount(&memDC);
	}

	dc.BitBlt(0, 0, clnRect.Width(), clnRect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

///////////////////////////////////////////////////////////////////////////////////
//Thread
UINT ThreadReady(LPVOID pParam)
{
	CUITestOS* UI = (CUITestOS*)pParam;
	//�� Thread����
	//{
	//1. VM����
	//$
	g_csInfo.Lock();
	theApp.m_mapTestOsInfo[UI->GetOS()]->SetState(STATE_READY);
	g_csInfo.Unlock();

	UI->SetChangeStateText();
	UI->SetLogText(_T("������Ʈ ������ ��ٸ��ϴ�."));
	((CMainFrm*)theApp.m_pMainWnd)->DrawDropFileRect();
	UI->InvalidateRect(NULL);
	::Sleep(10);
	
	//$
	g_csInfo.Lock();
	CString UUID = theApp.m_mapTestOsInfo[UI->GetOS()]->GetUUID();
	g_csInfo.Unlock();

	theApp.m_CVBoxController->ExevuteVM(UUID);

	//!! ��ٸ��鼭 UI�� ������ OnTime�Լ� ���
	//2. ������Ʈ�� Accept�� ��ٸ��� UI�� ����.
	HANDLE hArr[2];
	hArr[0] = g_CEventConnet[UI->GetOS()];
	hArr[1] = theApp.m_ExitEvent;
	DWORD dwWaitState;
	//1000 = 1��, 120 x 1000(2�� ���ȸ� ��ٸ���.)
	dwWaitState = WaitForMultipleObjects(2, hArr, FALSE, 120000);

	//������Ʈ ����
	if (dwWaitState == WAIT_OBJECT_0)
	{
		//3.1. Ransomware ���� ����
		UI->SetLogText(_T("Ransomware�� �����մϴ�."));
		UI->InvalidateRect(NULL);
		theApp.m_CServer->SendRansomware(UI->GetOS());
	}
	//3.2. Geust OS ����
	else if(dwWaitState == WAIT_TIMEOUT)
	{
		ErrorHandler(_T("������Ʈ�� Connect�� �����ϴ�."), FALSE);
			
		//$
		g_csInfo.Lock();
		theApp.m_mapTestOsInfo[UI->GetOS()]->SetState(STATE_CLOSE);
		g_csInfo.Unlock();

		UI->SetChangeStateText();
		UI->SetLogText(_T("GeustOS�� �����մϴ�."));
		UI->InvalidateRect(NULL);
		::Sleep(10);

		theApp.m_CVBoxController->QuitVM(UUID);
		theApp.m_CVBoxController->RestoreSnampshot(UUID);

		//$
		UI->DrawActive();
	}
	
	g_CEventConnet[UI->GetOS()].ResetEvent();

	return 0;
}

void CUITestOS::OnBnClickedButtonStart()
{
	if (((CMainFrm*)theApp.m_pMainWnd)->m_CRansomwarePath != _T("") &&
		((CMainFrm*)theApp.m_pMainWnd)->m_CRecoveryPath != _T(""))
	{
		//State�� �غ������� ����
		m_StartButton->ShowWindow(SW_HIDE);
		//Test�� �غ��ϱ� ���� Thread
		CWinThread* pThread = AfxBeginThread(ThreadReady, (LPVOID)this);
	}
	else
	{
		ErrorHandler(_T("���� ���� �Է��� �Ϸ���� �ʾҽ��ϴ�."), FALSE);
	}

}

//Recvie�� ������ ���� UI��ȭ
void CUITestOS::OnDestroy()
{
	CWnd::OnDestroy();

	if (m_StartButton != NULL)
	{
		delete m_StartButton;
		m_StartButton = NULL;
	}
	if (m_LogRect != NULL)
	{
		delete m_LogRect;
		m_LogRect = NULL;
	}
	if (m_DisabledRect != NULL)
	{
		delete m_DisabledRect;
		m_DisabledRect = NULL;
	}
	if (m_ActivationRect != NULL)
	{
		delete m_ActivationRect;
		m_ActivationRect = NULL;
	}
}	

