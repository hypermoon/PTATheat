// EarStatic.cpp : 实现文件
//

#include "stdafx.h"
#include "ETApplication.h"
#include "EarStatic.h"


// CEarStatic

IMPLEMENT_DYNAMIC(CEarStatic, CStatic)

CEarStatic::CEarStatic()
{
	m_backcolor = TRANS_BACK;
}

CEarStatic::~CEarStatic()
{
}

 HBRUSH   CEarStatic::CtlColor(CDC*   pDC,   UINT   nCtlColor)
 {
	 
	 m_brush.DeleteObject();
	 if(m_backcolor == TRANS_BACK)
	 {
		
		 //pDC->SetBkMode(TRANSPARENT);
		// pDC->SetBkColor(RGB(192,192,192));
		 // Invalidate();
		  m_brush.CreateStockObject(WHITE_BRUSH);//HOLLOW_BRUSH);
	 }
	 else
	 {
		 m_brush.CreateSolidBrush(m_backcolor);
		 pDC->SetBkColor(m_backcolor);
	 }

	     return (HBRUSH)m_brush;
 }
 BOOL CEarStatic::OnEraseBkgnd(CDC* pDC)
{

	return TRUE;
}
void CEarStatic::OnPaint( )
{
	/*HDC m_dcBk = NULL;
	HBITMAP m_bmpBk,m_bmpBkOld;

	CPaintDC dc(this);


	CClientDC clientDC(GetParent());
	CRect crect;
	CRect wrect;		
	GetClientRect(crect);
	GetWindowRect(wrect);
	GetParent()->ScreenToClient(wrect);
	if (m_dcBk == NULL)
	{
		m_dcBk = CreateCompatibleDC(clientDC.m_hDC);
		m_bmpBk = CreateCompatibleBitmap(clientDC.m_hDC, crect.Width(), crect.Height());
		m_bmpBkOld = (HBITMAP)::SelectObject(m_dcBk, m_bmpBk);
		::BitBlt(m_dcBk, 0, 0, crect.Width(), crect.Height(), clientDC.m_hDC, wrect.left, wrect.top, SRCCOPY);
	}

	HDC hSaveHDC;
	HBITMAP hSaveBmp;
	int iWidth = crect.Width();
	int iHeight = crect.Height();

	hSaveHDC = ::CreateCompatibleDC(dc.m_hDC);
	hSaveBmp = ::CreateCompatibleBitmap(hSaveHDC, iWidth, iHeight);
	HBITMAP hSaveCBmpOld = (HBITMAP)::SelectObject(hSaveHDC, hSaveBmp);			

	COLORREF crOldBack = ::SetBkColor(dc.m_hDC, RGB(0,0,0));
	COLORREF crOldText = ::SetTextColor(dc.m_hDC, RGB(255,255,255));	

	::BitBlt(hSaveHDC, 0, 0, iWidth, iHeight, dc.m_hDC, crect.left, crect.top, SRCCOPY);
	::BitBlt(dc.m_hDC, 0, 0, iWidth, iHeight, m_dcBk, 0, 0, SRCCOPY);
	//::BitBlt(dc.m_hDC, 0, 0, iWidth, iHeight, hSaveHDC, 0, 0, SRCAND);
	//restore and clean up

	::SetBkColor(dc.m_hDC, crOldBack);
	::SetTextColor(dc.m_hDC, crOldText);
	::SelectObject(hSaveHDC, hSaveCBmpOld);
	::DeleteObject(hSaveBmp);
	::DeleteDC(hSaveHDC);
	*/
	//CPaintDC dc(this);
	//dc.SetBkColor(RGB(200,200,20));
	//dc.SetBkMode(TRANSPARENT);

	//CPaintDC dc(this);
	//dc.SetBkMode(TRANSPARENT);
	//::SetTextColor(dc.m_hDC, RGB(210,20,20));
	CWnd::OnPaint();
}

void CEarStatic::OnMove( int x, int y)
{
	//this->ShowWindow(SW_HIDE);
	this->Invalidate();
	this->UpdateWindow();
	//this->ShowWindow(SW_SHOW);
	//this->RedrawWindow();
	//this->GetParent()->RedrawWindow();
	CWnd::OnMove(x,y);
}
BEGIN_MESSAGE_MAP(CEarStatic, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_ERASEBKGND()
	ON_WM_MOVE( )
	ON_WM_PAINT( )
END_MESSAGE_MAP()



// CEarStatic 消息处理程序


