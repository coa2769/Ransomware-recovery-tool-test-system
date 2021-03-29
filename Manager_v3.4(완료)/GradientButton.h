#pragma once

//��⸦ ���� ������ ���� ����ü��
//https://ko.wikipedia.org/wiki/YCbCr
//https://dic1224.blog.me/80180398481
//Ű���� : YCbCr, ���� �� ���� ��ȯ
//����ó���� ���� ���¼ҽ� ���̺귯�� -> OpenCV
struct YCbCr //���(Ȯ��) + ����(���)
{
	YCbCr() = default;
	~YCbCr() = default;
	YCbCr(COLORREF rgb) //RGB -> YCbCr
	{
		Y = 0.299 * GetRValue(rgb) + 0.587 * GetGValue(rgb) + 0.114 * GetBValue(rgb);
		Cb = -0.16874 * GetRValue(rgb) - 0.33126 * GetGValue(rgb) + 0.50 * GetBValue(rgb);
		Cr = 0.50 * GetRValue(rgb) - 0.41869 * GetGValue(rgb) - 0.08131 * GetBValue(rgb);
	}

	COLORREF GetRGB() //YCbCr -> RGB
	{
		double r = Y + 1.402 * Cr;
		double g = Y - 0.34414 * Cb - 0.71414 * Cr;
		double b = Y + 1.772 * Cb;
		return RGB(r, g, b);
	}

	double Y;	//���
	double Cb;	//���� ����
	double Cr;	//���� ����
};

//�׶��̼��� �׸��� ���� ����
struct GradientColor
{
	COLORREF begin;
	COLORREF end;
};

//�׶��̼��� ��� ��������
enum LinearGradientMode
{
	Horisontal = 0,
	Vertical = 90,
	ForwardDiagonal = 45,  //�տ��� �밢��
	BackwordDiagonal = 135 //�ڿ��� �밢��
};

void CalcSubColors(const CRect& rect, const int angle, const GradientColor& baseColor, COLORREF& subColorLeft, COLORREF& subColorRight);
double AngleToRadian(double angle);
void ColorTableRotate(COLORREF* colorTb, int angle);
void SetTriVertex(int x, int y, COLORREF color, TRIVERTEX* pTriVertex);

//CreateGradientBitmap�Լ� ���ο��� Bitmap�� �׸��⸦ �Ѵ�.
//DrawLinearGradientFill()������ ���� �׶��̼� �׸��⸦ �����ϴ� �Լ�
void DrawLinearGradientFill(CDC *pDC, const CRect& rect, const GradientColor& color, const int angle);
//3D�� ǥ���ϱ� ���� �׵θ� �׸��⸦ ����. 
//���� ����� �׵θ��� �׶��̼� ������ ���󺸴� ��Ⱑ ���� �������� �׸���
//������ �ϴ��� �׵θ��� �׶��̼� ���� ���󺸴� ��Ⱑ ���� �������� �׸���.
void Draw3dBorder(CDC* pDC, const CRect& rect, const GradientColor& color);

class CGradientRect
{
public:
	CGradientRect();
	~CGradientRect();
	void SetGradientRect(CWnd* pParent ,CRect& rect, COLORREF Begin, COLORREF end, int angle, BOOL b3DRect);
	

	void DrawRect(CDC *pDC);
	BOOL PtInRect(POINT& pt)
	{
		return m_GradientRect.PtInRect(pt);
	}
	CRect* GetRect(void) { return &m_GradientRect; }
	//void DisabledRect(void) { m_bActive = FALSE; }
	//void ActiveRect(void) { m_bActive = TRUE; }
	//BOOL GetActiveRect(void) { return m_bActive; }

private:
	CBitmap m_GradientBmp;
	GradientColor m_color;
	CRect m_GradientRect;
	int m_angle;
	BOOL m_b3DRect = TRUE;
	//BOOL m_bActive = TRUE;

	void CreateGradientBitmap(CDC *pDC);
};

// CGradientButton

class CGradientButton : public CButton
{
	DECLARE_DYNAMIC(CGradientButton)

public:
	CGradientButton();
	virtual ~CGradientButton();

	void SetNormalGradient(CString text, COLORREF begin, COLORREF end, int angle = Horisontal);
	void SetOverGradient(CString text, COLORREF begin, COLORREF end, int angle = Horisontal);
	void SetSelectGradient(CString text, COLORREF begin, COLORREF end, int angle = Horisontal);

	void DisabledButton(void) { m_bActive = FALSE; }
	void ActiveButton(void) { m_bActive = TRUE; }
	BOOL getActiveButton(void) { return m_bActive; }

private:
	BOOL m_bOver = FALSE;
	BOOL m_bActive = TRUE;
	CString m_BtnText;

	CBitmap m_bmpNormal;
	CBitmap m_bmpOver;
	CBitmap m_bmpSelect;

	GradientColor m_colorNormal;
	GradientColor m_colorOver;
	GradientColor m_colorSelect;

	void CreateGradientBitmap(CBitmap& bmp, GradientColor& color, int angle);

protected:
	//������ �ؼ� OnDrawItem��� �׷�����.(CButton�Լ��� ����Ǿ� �ִ�.)
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
};


