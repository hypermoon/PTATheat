#pragma once


// CLvEdit

class CLvEdit : public CEdit
{
	DECLARE_DYNAMIC(CLvEdit)

public:
	CLvEdit();
	virtual ~CLvEdit();
	
	
    LONG  m_leftpos,m_rightpos;
	int item,subitem;
	int  GetnItemidx(){return item;};
	int  GetsubItemidx(){return subitem;};
	void SaveCurrentPos(int lvitem, int lvsubitem);
	void SetBKcolors(COLORREF col);
	HBRUSH m_br;
	COLORREF m_col;

protected:
	  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
      afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
	DECLARE_MESSAGE_MAP()
};


