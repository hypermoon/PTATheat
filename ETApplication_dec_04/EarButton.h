#pragma once


// CEarButton

class CEarButton : public CButton
{
	DECLARE_DYNAMIC(CEarButton)

public:
	CEarButton();
	CString m_str;
	BOOL     pressed;
	virtual ~CEarButton();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	void DrawFilledRect(CDC *pDC, CRect R, COLORREF color);
	void DrawLine(CDC *DC, long left, long top, long right, long bottom, COLORREF color);
protected:
	afx_msg void OnLButtonDown(UINT nFlags,CPoint point);
	DECLARE_MESSAGE_MAP()
};


