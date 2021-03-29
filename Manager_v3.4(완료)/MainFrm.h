#pragma once

#include"GradientButton.h"
#include"DrawButton.h"
// CMainFrm

#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 800

#define FILE_DROP_WIDTH 300
#define FILE_DROP_HEIGHT 300

class CUITestOS;

//Frame부분 색깔
//RGB(27, 28, 30), RGB(0, 0, 0)

//view부분 색깔
//RGB(46, 47, 51), RGB(30, 31, 33)

//file 이름 출력 rect
//RGB(35, 36, 40), RGB(23, 24, 26)

//file 이름 취소 버튼
//RGB(61, 66, 67), RGB(51, 54, 56) },
//RGB(216, 218, 218), RGB(141, 144, 146)
//RGB(103, 107, 108), RGB(157, 163, 165)

//file input rect color 이 중 하나 && 비활성화 상태 색깔
//RGB(105, 110, 113), RGB(90, 94, 97) },
//RGB(227, 229, 229), RGB(141, 144, 146)
//RGB(103, 107, 108), RGB(157, 163, 165)


class CMainFrm : public CWnd
{
	DECLARE_DYNAMIC(CMainFrm)

public:
	CMainFrm();
	virtual ~CMainFrm();
	BOOL CreateMainWindow(); //등록 함수

	CString m_CRansomwarePath;
	CString m_CRansomwareName;
	CString m_CRecoveryPath;
	CString m_CRecoveryName;

private:
	CRect m_CaptionRect;
	CRect m_FrameRect;

	CGradientRect m_RectBk;
	CGradientRect m_RectBkContainer;

	//http://www.tipssoft.com/bulletin/board.php?bo_table=FAQ&wr_id=299
	CEdit m_EditRansom;
	CEdit m_EditRecovery;
	CGradientRect m_RectRansomInput;
	CGradientRect m_RectRansomDisabledInput;
	CGradientRect m_RectRecoveryInput;
	CGradientRect m_RectRecoveryDisabledInput;
	CImage m_CImageRansom;
	CImage m_CImageRecovery;

	CDrawButton m_btnClose;
	CDrawButton m_btnMaximize;
	CDrawButton m_btnMinimize;


	BOOL PossibleFileInput(void);
protected:
	DECLARE_MESSAGE_MAP()
public:
	CUITestOS* m_CUIWIndow[(int)WINDOW_10 + 1];

	void OnBnClickedButtonMinimize();
	void OnBnClickedButtonClose();

	void DrawDropFileRect(void);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnDestroy();
};


