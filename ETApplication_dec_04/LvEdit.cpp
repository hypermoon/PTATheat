// LvEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "ETApplication.h"
#include "LvEdit.h"


// CLvEdit

IMPLEMENT_DYNAMIC(CLvEdit, CEdit)

CLvEdit::CLvEdit()
{
	m_leftpos = 0;
	m_rightpos =0;
	item = -1;
	subitem = -1;
	m_br = CreateSolidBrush(RGB(255,255,255));
	m_col = RGB(255,255,255);
}

CLvEdit::~CLvEdit()
{
}

void CLvEdit::SaveCurrentPos(int lvitem, int lvsubitem)
{
	item = lvitem;
	subitem = lvsubitem;

}
 BOOL CLvEdit::OnEraseBkgnd(CDC* pDC)
 {
	 return true;
 }

 void CLvEdit::SetBKcolors(COLORREF col)
 {
	 m_br = CreateSolidBrush(col);
	 m_col = col;
 }
  HBRUSH CLvEdit::CtlColor(CDC* pDC, UINT nCtlColor)
 {
	  
	 //m_br = CreateSolidBrush(RGB(80,40,130));
	 //pDC->SetTextColor(RGB(100,100,100));
	 pDC->SetBkColor(m_col);
	// pDC->SetBkMode(TRANSPARENT);
	 return m_br;

 }

BEGIN_MESSAGE_MAP(CLvEdit, CEdit)
	ON_WM_ERASEBKGND()
    ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()



// CLvEdit 消息处理程序


