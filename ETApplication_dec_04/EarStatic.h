#pragma once


// CEarStatic

class CEarStatic : public CStatic
{
	DECLARE_DYNAMIC(CEarStatic)

public:
	CEarStatic();
	virtual ~CEarStatic();
	CBrush  m_brush;
	COLORREF   m_backcolor;

protected:
	  afx_msg   HBRUSH   CtlColor(CDC*   pDC,   UINT   nCtlColor);
	  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	  afx_msg void OnPaint( );
	  afx_msg void OnMove( int x, int y);
	DECLARE_MESSAGE_MAP()
};
#define TRANS_BACK  -1


