#pragma once
//790
#define VERSION_UI_WIDTH	210
#define LOG_UI_WIDTH		580
#define TEST_OS_HEIGHT		215

#define BTN_WIDTH 210
#define BTN_HEIGHT 100


////////////////////////////////////////////////////////////////////////////////////////////
//※※ CCriticalSection ※※
//extern CCriticalSection g_csUI;

class CGradientButton;
class CGradientRect;


//file input rect color 이 중 하나 && 비활성화 상태 색깔
//RGB(105, 110, 113), RGB(90, 94, 97) },
//RGB(227, 229, 229), RGB(141, 144, 146)
//RGB(103, 107, 108), RGB(157, 163, 165)

// CUITestOS
//활성 상태 색깔
//RGB(22, 157, 62), RGB(45, 198, 90)
//rect

//button
//RGB(55, 183, 10), RGB(3, 153, 44)
//RGB(43, 167, 1), RGB(2, 128, 36)
//RGB(3, 136, 40), RGB(40, 162, 2)

//Test 시작
//RGB(0, 155, 254), RGB(3, 142, 231)


//준비 상태 색깔(노랑)
//RGB(239, 141, 8), RGB(245, 185, 12)

//rect

//button
//RGB(254, 234, 20), RGB(249, 190, 1)
//RGB(247, 223, 0), RGB(242, 171, 0)
//RGB(241, 168, 0), RGB(251, 206, 7)


//문제 상태 색깔(빨강)
//RGB(241, 64, 0), RGB(253, 93, 1)

//rect

//button
//RGB(255, 137, 0), RGB(242, 67, 0)
//RGB(246, 107, 0), RGB(213, 55, 0)
//RGB(207, 49, 0), RGB(246, 110, 0)


class CUITestOS : public CWnd
{
	DECLARE_DYNAMIC(CUITestOS)

public:
	CUITestOS();
	virtual ~CUITestOS();

private:
	CGradientButton* m_StartButton;
	CGradientRect* m_LogRect;
	CGradientRect* m_DisabledRect;
	CGradientRect* m_ActivationRect;
	
	TEST_OS m_os;
	CString m_CStrOSName;
	CString m_CStrState;
	CString m_CStrLog;
	CString m_CStrProgress;

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();

	//Create function
	BOOL CreateUITestOS(CWnd* CWndParent, TEST_OS os, int x, int y, int ID);

	//Button
	void OnBnClickedButtonStart();
	
	TEST_OS GetOS(void) { return m_os; }

	//Progress
	void SetProgressText(int max, int current)
	{
		if (current == -1)
		{
			m_CStrProgress = _T("");
		}
		else
		{
			m_CStrProgress.Format(_T("진행률 : %d / %d"), current, max);
		}
		InvalidateRect(NULL);
	}

	void SetLogText(TCHAR* log)
	{
		m_CStrLog = log;
	}

	void SetChangeStateText(void);
	void DrawStateText(CDC* pDC);
	void DrawReportCount(CDC* pDC);
	void DrawDisable(void);
	void DrawActive(void);


};


