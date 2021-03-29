
// Manager_v1.0.cpp : ���� ���α׷��� ���� Ŭ���� ������ �����մϴ�.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"

#include "Manager_v1.0.h"

#include "MainFrm.h"
#include"ServerSocket.h"
#include"TestOSInfo.h"
#include"VBoxController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
/////////////////////////////////////////////////////////////////////////////
// $ : Info�� ���� Lock�̴�.
// $@ : Socket�� ���� Lock�̴�.
/////////////////////////////////////////////////////////////////////////////

// CManager_v10App

BEGIN_MESSAGE_MAP(CManager_v10App, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CManager_v10App::OnAppAbout)
END_MESSAGE_MAP()


// CManager_v10App ����

CManager_v10App::CManager_v10App()
{
	// �ٽ� ���� ������ ����
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
#ifdef _MANAGED
	// ���� ���α׷��� ���� ��� ��Ÿ�� ������ ����Ͽ� ������ ���(/clr):
	//     1) �� �߰� ������ �ٽ� ���� ������ ������ ����� �۵��ϴ� �� �ʿ��մϴ�.
	//     2) ������Ʈ���� �����Ϸ��� System.Windows.Forms�� ���� ������ �߰��ؾ� �մϴ�.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: �Ʒ� ���� ���α׷� ID ���ڿ��� ���� ID ���ڿ��� �ٲٽʽÿ�(����).
	// ���ڿ��� ���� ����: CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Manager_v1.0.AppID.NoVersion"));

	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.
}

// ������ CManager_v10App ��ü�Դϴ�.

CManager_v10App theApp;


// CManager_v10App �ʱ�ȭ


BOOL CManager_v10App::InitInstance()
{
	// ���� ���α׷� �Ŵ��佺Ʈ�� ComCtl32.dll ���� 6 �̻��� ����Ͽ� ���־� ��Ÿ����
	// ����ϵ��� �����ϴ� ���, Windows XP �󿡼� �ݵ�� InitCommonControlsEx()�� �ʿ��մϴ�. 
	// InitCommonControlsEx()�� ������� ������ â�� ���� �� �����ϴ�.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ���� ���α׷����� ����� ��� ���� ��Ʈ�� Ŭ������ �����ϵ���
	// �� �׸��� �����Ͻʽÿ�.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// OLE ���̺귯���� �ʱ�ȭ�մϴ�.
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// RichEdit ��Ʈ���� ����Ϸ���  AfxInitRichEdit2()�� �־�� �մϴ�.	
	// AfxInitRichEdit2();

	// ǥ�� �ʱ�ȭ
	// �̵� ����� ������� �ʰ� ���� ���� ������ ũ�⸦ ���̷���
	// �Ʒ����� �ʿ� ���� Ư�� �ʱ�ȭ
	// ��ƾ�� �����ؾ� �մϴ�.
	// �ش� ������ ����� ������Ʈ�� Ű�� �����Ͻʽÿ�.
	// TODO: �� ���ڿ��� ȸ�� �Ǵ� ������ �̸��� ����
	// ������ �������� �����ؾ� �մϴ�.
	SetRegistryKey(_T("���� ���� ���α׷� �����翡�� ������ ���� ���α׷�"));
	

	//�� 0. TestOS ������ �־�� class ����
	m_mapTestOsInfo.SetAt(WINDOW_7, new CTestOSInfo(WINDOW_7));
	m_mapTestOsInfo.SetAt(WINDOW_8, new CTestOSInfo(WINDOW_8));
	m_mapTestOsInfo.SetAt(WINDOW_10, new CTestOSInfo(WINDOW_10));

	//�� 1. ��� list�˻� UUID�߰�
	m_CVBoxController = new CVBoxController;
	m_CVBoxController->Initialize();
	m_CVBoxController->CheckVMList();
	//Take Snapshot
	//if(m_CVBoxController->HaveSnapshot(m_mapTestOsInfo[WINDOW_7]->GetUUID()))
	//	m_CVBoxController->TakeSnapshot(m_mapTestOsInfo[WINDOW_7]->GetUUID());
	//if(m_CVBoxController->HaveSnapshot(m_mapTestOsInfo[WINDOW_8]->GetUUID()))
	//	m_CVBoxController->TakeSnapshot(m_mapTestOsInfo[WINDOW_8]->GetUUID());
	//if(m_CVBoxController->HaveSnapshot(m_mapTestOsInfo[WINDOW_10]->GetUUID()))
	//	m_CVBoxController->TakeSnapshot(m_mapTestOsInfo[WINDOW_10]->GetUUID());

	//�� 1.1 ������ ������ �޴� Thread
	m_CServer = new CServerSocket;
	m_CServer->AcceptFuntion();
	

	// �� â�� ����� ���� �� �ڵ忡���� �� ������ â ��ü��
	// ���� ���� �̸� ���� ���α׷��� �� â ��ü�� �����մϴ�.
	CMainFrm* pFrame = new CMainFrm;
	if (!pFrame->CreateMainWindow())
		return FALSE;

	m_pMainWnd = pFrame;
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	return TRUE;
}

int CManager_v10App::ExitInstance()
{
	//TODO: �߰��� �߰� ���ҽ��� ó���մϴ�.
	delete m_mapTestOsInfo[WINDOW_7];
	delete m_mapTestOsInfo[WINDOW_8];
	delete m_mapTestOsInfo[WINDOW_10];

	m_mapTestOsInfo.RemoveAll();

	delete m_CServer;
	delete m_CVBoxController;

	AfxOleTerm(FALSE);

	return CWinApp::ExitInstance();
}

// CManager_v10App �޽��� ó����


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// ��ȭ ���ڸ� �����ϱ� ���� ���� ���α׷� ����Դϴ�.
void CManager_v10App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CManager_v10App �޽��� ó����



