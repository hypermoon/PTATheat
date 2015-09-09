#pragma once


// CEarSliderCtrl

class CEarSliderCtrl : public CSliderCtrl
{
	DECLARE_DYNAMIC(CEarSliderCtrl)

public:
	CEarSliderCtrl();
	virtual ~CEarSliderCtrl();

	HDC			m_dcBk;
	HBITMAP		m_bmpBk;
	HBITMAP     m_bmpBkOld;

	BOOL        m_focused;

	int    currpos;
	int    ctlid;
	BOOL   slidermoved;
	BOOL   appeared;
	COLORREF bkcolor;
	RECT     m_recthumb;
	CBrush   thumbrush;
	void  SetThumbRect(RECT rect);
	void  DrawThumb(CDC *pDC, LPNMCUSTOMDRAW lpcd,CString str = _T(""));
	void  DrawChannel(CDC *pDC, LPNMCUSTOMDRAW lpcd);

protected:
	afx_msg void OnReflectCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point );
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point );
	afx_msg void OnKillFocus(CWnd* pNewWnd );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};


