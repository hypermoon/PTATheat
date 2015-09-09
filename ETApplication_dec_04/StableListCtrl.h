#pragma once


// CStableListCtrl

class CStableListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CStableListCtrl)

public:
		BOOL   OnNotify(WPARAM   wParam,   LPARAM   lParam,   LRESULT*   pResult); 
	CStableListCtrl();
	virtual ~CStableListCtrl();
	void  DrawSubItemText(BOOL ptatesting ,int column,int side);
	void   OnCustomdrawMyList(NMHDR   *pNMHDR,   LRESULT   *pResult); 
	BOOL   ptateststat;
	int    isubitem;
	int    m_side;
	CRect rect1,rect2,rect3;
protected:
	DECLARE_MESSAGE_MAP()
};


