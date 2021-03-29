// GradientButton.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Manager_v1.0.h"
#include "GradientButton.h"

#include<iterator> //std::rotate()�Լ��� ���� ����

//�ʿ��� ���� ����

//�������� �������� ��ȯ
double AngleToRadian(double angle)
{
	static const double pi = 3.141592;
	return (pi / 180.0) * angle;
}

//������ ���� � ������ ��ȭ
void ColorTableRotate(COLORREF* colorTb, int angle)
{
	angle = angle % 360;
	if (angle <= 90)
	{
		//�ƹ��͵� ����.
	}
	else if (angle < 180)
	{
		//std::rotate
		//color Table�� ������ ������ ���� �� ĭ�� �������� �����
		//Ƣ�� ���� ���� �� �ݴ��� ���� ������.
		//ex) 12345 --(2ĭ)--> 34512
		//rotate(
		//	_FwdIt _First,	�迭�� ó��
		//	_FwdIt _Mid,	������ ������ �迭
		//	_FwdIt _Last)	�迭�� ������
		std::rotate(colorTb, colorTb + 4 - 1, colorTb + 4);
	}
	else if (angle <= 270)
	{
		std::rotate(colorTb, colorTb + 4 - 2, colorTb + 4);
	}
	else //a < 360
	{
		std::rotate(colorTb, colorTb + 4 - 3, colorTb + 4);
	}


}

void SetTriVertex(int x, int y, COLORREF color, TRIVERTEX* pTriVertex)
{
	pTriVertex->x = x;
	pTriVertex->y = y;
	//#define GetBValue(rgb)      (LOBYTE((rgb)>>16))
	//�̷��� Byte�� 3byte�� �÷��� ����Ѵ�.
	//�� �÷��� typedef USHORT COLOR16;�̹Ƿ� 2byte��
	//����Ʈ�������� �����ش�.
	pTriVertex->Red = GetRValue(color) << 8;
	pTriVertex->Green = GetGValue(color) << 8;
	pTriVertex->Blue = GetBValue(color) << 8;
}


void CalcSubColors(const CRect& rect, const int angle, const GradientColor& baseColor, COLORREF& subColorLeft, COLORREF& subColorRight)
{
	double alpha = rect.Width() * cos(AngleToRadian(angle));
	double beta = rect.Height() * sin(AngleToRadian(angle));
	//fabs() �ε��Ҽ��� ���밪
	double length = fabs(alpha) + fabs(beta);	
	double rateAlpha = fabs(alpha / length);	
	double rateBeta = fabs(beta / length);

	//left, right ���� �÷� ���� ����
	int redDiff = GetRValue(baseColor.end) - GetRValue(baseColor.begin);
	int greenDiff = GetGValue(baseColor.end) - GetGValue(baseColor.begin);
	int blueDiff = GetBValue(baseColor.end) - GetBValue(baseColor.begin);

	subColorRight = RGB(
		GetRValue(baseColor.begin) + (redDiff * rateAlpha),
		GetGValue(baseColor.begin) + (greenDiff * rateAlpha),
		GetBValue(baseColor.begin) + (blueDiff * rateAlpha));
	subColorLeft = RGB(
		GetRValue(baseColor.begin) + (redDiff * rateBeta),
		GetGValue(baseColor.begin) + (greenDiff * rateBeta),
		GetBValue(baseColor.begin) + (blueDiff * rateBeta));

}


void DrawLinearGradientFill(CDC *pDC, const CRect& rect, const GradientColor& color, const int angle)
{
	//typedef struct _TRIVERTEX
	//{
	//		LONG    x;
	//		LONG    y;
	//		COLOR16 Red;
	//		COLOR16 Green;
	//		COLOR16 Blue;
	//		COLOR16 Alpha;
	//}TRIVERTEX
	//�簢���� �� �������� ��ġ�� ����, ���İ��� ���� ������ �����Ѵ�.
	TRIVERTEX triVertex[4] = { 0 };
	//typedef struct _GRADIENT_TRIANGLE
	//{
	//		ULONG Vertex1;
	//		ULONG Vertex2;
	//		ULONG Vertex3;
	//}	GRADIENT_TRIANGLE
	//(0��) ��������������������������(1��)
	//	    ��		    ��
	//	    ��		    ��
	//	    ��		    ��
	//(3��) ��������������������������(2��)

	GRADIENT_TRIANGLE gTri[2] = { { 0,1,2 },{ 0,2,3 } }; //�� �ﰢ���� �������� TRIVERTEX�迭�� ���° index�ΰ�
	const int vertexCount = 4;
	const int mashCount = 2;

	COLORREF subRightColor, subLeftColor;
	//������ ���� left�� right�� �÷�(?)
	CalcSubColors(rect, angle, color, subLeftColor, subRightColor);

	//��,����,�Ʒ�,���� ���� table
	COLORREF colorTb[4] = { color.begin, subRightColor, color.end, subLeftColor };
	//������ ���� ���� table�� ������ �ٲ��.
	ColorTableRotate(colorTb, angle);

	//COLORREF�� COLOR16�� �����ִ� �Լ�
	SetTriVertex(rect.left, rect.top, colorTb[0], &triVertex[0]);
	SetTriVertex(rect.right, rect.top, colorTb[1], &triVertex[1]);
	SetTriVertex(rect.right, rect.bottom, colorTb[2], &triVertex[2]);
	SetTriVertex(rect.left, rect.bottom, colorTb[3], &triVertex[3]);

	//�׶��̼� ȿ���� �ִ� �Լ�
	//GradientFill(
	//	TRIVERTEX* pVertices,	TRIVERTEX �迭
	//	ULONG nVertices,		TRIVERTEX �迭 ����
   	//	void* pMesh,			����� ��� GRADIENT_TRIANGLE �迭, �簢�� ��� GRADIENT_RECT �迭
	//	ULONG nMeshElements,	pMesh �迭 ����
	//	DWORD dwMode)			�׶��̼� ��� 
	//							GRADIENT_FILL_RECT_V	: ����
	//							GRADIENT_FILL_RECT_H	: ����,
    //							RADIENT_FILL_TRIANGLE	: �ﰢ�� ���
	pDC->GradientFill(triVertex, vertexCount, &gTri, mashCount, GRADIENT_FILL_TRIANGLE);
}

void Draw3dBorder(CDC* pDC, const CRect& rect, const GradientColor& color)
{
	//(��� �Ӽ��� ����)
	//1. YCbCr���� �𵨷� ��ȯ��
	YCbCr topLeftColor(color.begin);
	YCbCr bottomRightColor(color.end);

	//2. ��� �Ӽ��� Y�� ���� +-40���� ��� ��
	topLeftColor.Y += 40;
	bottomRightColor.Y -= 40;

	if (topLeftColor.Y > 255) topLeftColor.Y = 255;
	if (bottomRightColor.Y < 0) bottomRightColor.Y = 0;

	//3. �ٽ� RGB���� �𵨷� ��ȯ�Ͽ� �׸��⸦ �����Ѵ�.
	COLORREF topLeftRGB = topLeftColor.GetRGB();
	COLORREF bottomRightRGB = bottomRightColor.GetRGB();

	pDC->Draw3dRect(rect, topLeftRGB, bottomRightRGB);
}

//CGradientRect class�� �Լ�
CGradientRect::CGradientRect()
{
}

CGradientRect::~CGradientRect()
{
	m_GradientBmp.DeleteObject();
}

void CGradientRect::SetGradientRect(CWnd* pParent, CRect& rect, COLORREF Begin, COLORREF end, int angle, BOOL b3DRect)
{
	m_color.begin = Begin;
	m_color.end = end;
	m_GradientRect = rect;
	m_angle = angle;
	m_b3DRect = b3DRect;

	CDC* pDC = pParent->GetDC();
	CreateGradientBitmap(pDC);
	pParent->ReleaseDC(pDC);
}

void CGradientRect::CreateGradientBitmap(CDC *pDC)
{
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	m_GradientBmp.CreateCompatibleBitmap(pDC, m_GradientRect.Width(), m_GradientRect.Height());
	CBitmap *pOldBmp = memDC.SelectObject(&m_GradientBmp);
	
	DrawLinearGradientFill(&memDC, CRect(0,0,m_GradientRect.Width(),m_GradientRect.Height()), m_color, m_angle);
	if (m_b3DRect == TRUE)
	{
		Draw3dBorder(&memDC, CRect(0, 0, m_GradientRect.Width(), m_GradientRect.Height()), m_color);
	}

	memDC.SelectObject(pOldBmp);
	memDC.DeleteDC();
}

void CGradientRect::DrawRect(CDC *pDC)
{
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(&m_GradientBmp);

	pDC->BitBlt(m_GradientRect.left, m_GradientRect.top,
		m_GradientRect.Width(), m_GradientRect.Height(),
		&memDC, 0, 0, SRCCOPY);

	memDC.DeleteDC();
}

// CGradientButton �Լ�

IMPLEMENT_DYNAMIC(CGradientButton, CButton)

CGradientButton::CGradientButton()
{

}

CGradientButton::~CGradientButton()
{
}


BEGIN_MESSAGE_MAP(CGradientButton, CButton)
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()



// CGradientButton �޽��� ó�����Դϴ�.

void CGradientButton::SetNormalGradient(CString text, COLORREF begin, COLORREF end, int angle)
{
	m_colorNormal.begin = begin;
	m_colorNormal.end = end;
	m_BtnText = text;
	CreateGradientBitmap(m_bmpNormal, m_colorNormal, angle);
}

void CGradientButton::SetOverGradient(CString text, COLORREF begin, COLORREF end, int angle)
{
	m_colorOver.begin = begin;
	m_colorOver.end = end;
	m_BtnText = text;
	CreateGradientBitmap(m_bmpOver, m_colorOver, angle);
}

void CGradientButton::SetSelectGradient(CString text, COLORREF begin, COLORREF end, int angle)
{
	m_colorSelect.begin = begin;
	m_colorSelect.end = end;
	m_BtnText = text;
	CreateGradientBitmap(m_bmpSelect, m_colorSelect, angle);
}

void CGradientButton::CreateGradientBitmap(CBitmap& bmp, GradientColor& color, int angle)
{
	CRect ClientRect;
	GetClientRect(ClientRect);

	CDC *pDC = GetDC();
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	bmp.DeleteObject();
	bmp.CreateCompatibleBitmap(pDC, ClientRect.Width(), ClientRect.Height());
	CBitmap *oldBmp = memDC.SelectObject(&bmp);

	DrawLinearGradientFill(&memDC, ClientRect, color, angle);
	//Draw3dBorder(&memDC, ClientRect, color);

	//���� ����
	LOGFONT lf;
	::ZeroMemory(&lf, sizeof(lf));

	lf.lfHeight = ClientRect.Height() / 3 * 2;
	lf.lfWeight = FW_EXTRABOLD;
	CFont NewFont;
	NewFont.CreateFontIndirectW(&lf);
	CFont *pOldFont = memDC.SelectObject(&NewFont);
	memDC.SetBkMode(TRANSPARENT);
	memDC.SetTextColor(RGB(255, 255, 255));
	memDC.TextOutW(ClientRect.Width() / (m_BtnText.GetLength() + 2), ClientRect.Height() / 6, m_BtnText);


	memDC.SelectObject(oldBmp);
	memDC.DeleteDC();
	ReleaseDC(pDC);
}

void CGradientButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	
	CRect ClientRect(lpDrawItemStruct->rcItem);
	if (m_bActive == TRUE)
	{
		if (lpDrawItemStruct->itemState & ODS_SELECTED) //itemStat : �׸��� ������ ���� �߻�
		{												//button�� ��� ODS_SELECTED�̸� ���콺�� button���� �ִ� ���̴�.
			memDC.SelectObject(m_bmpSelect);
		}
		else
		{
			if (m_bOver == TRUE)
				memDC.SelectObject(m_bmpOver);
			else
				memDC.SelectObject(m_bmpNormal);
		}
	}
	else
	{
		memDC.SelectObject(m_bmpSelect);
	}
	
	
	pDC->BitBlt(0, 0, ClientRect.Width(), ClientRect.Height(), &memDC, 0, 0, SRCCOPY);
}


void CGradientButton::OnDestroy()
{
	CButton::OnDestroy();

	m_bmpNormal.DeleteObject();
	m_bmpSelect.DeleteObject();
	m_bmpOver.DeleteObject();
}


void CGradientButton::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bOver == FALSE)
	{
		m_bOver = TRUE;

		TRACKMOUSEEVENT tme = { 0 };
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = 1;
		TrackMouseEvent(&tme);

		RedrawWindow();
	}

	CButton::OnMouseMove(nFlags, point);
}


void CGradientButton::OnMouseLeave()
{
	m_bOver = FALSE;
	RedrawWindow();

	CButton::OnMouseLeave();
}
