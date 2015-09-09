// EarButton.cpp : 
//

#include "stdafx.h"
#include "ETApplication.h"
#include "EarButton.h"
#include "ETApplicationView.h"

#define   BKCOLOR              RGB(150,150,150)
// CEarButton

IMPLEMENT_DYNAMIC(CEarButton, CButton)

CEarButton::CEarButton()
{
	pressed = FALSE;
	m_str = _T("");

}

CEarButton::~CEarButton()
{
}


BEGIN_MESSAGE_MAP(CEarButton, CButton)
	ON_WM_LBUTTONDOWN( )
END_MESSAGE_MAP()

 void CEarButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
 {
	 this->GetWindowTextW(m_str);
	 CDC* pDC = CDC::FromHandle(lpDIS->hDC); 
	 UINT state = lpDIS->itemState; 
     CRect focusRect, btnRect;
	 focusRect.CopyRect(&lpDIS->rcItem);
     btnRect.CopyRect(&lpDIS->rcItem); 

	 focusRect.left += 4; 

     focusRect.right -= 4; 

     focusRect.top += 4; 

     focusRect.bottom -= 4; 

	 DrawFilledRect(pDC, btnRect, BKCOLOR);//RGB(180,150,150)); 

	 DrawLine(pDC, btnRect.left, btnRect.top, btnRect.right, btnRect.top, RGB(255, 255, 255)); 
     DrawLine(pDC, btnRect.left,btnRect.top, btnRect.left, btnRect.bottom, RGB(255, 255, 255)); 
	 DrawLine(pDC, btnRect.left + 1, btnRect.bottom - 1, btnRect.right, btnRect.bottom - 1, RGB(1, 1, 1)); 
	 DrawLine(pDC, btnRect.right - 1, btnRect.top + 1, btnRect.right - 1,btnRect.bottom, RGB(1, 1, 1)); 

	 pDC->SetBkMode(TRANSPARENT);
	 pDC->DrawText(m_str,btnRect, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
 }

 void CEarButton::DrawFilledRect(CDC *pDC, CRect R, COLORREF color) 
{ 

	CBrush B; 

	B.CreateSolidBrush(color); 

	pDC->FillRect(R, &B); 

} 

 void CEarButton::DrawLine(CDC *pDC, long left, long top, long right, long bottom, COLORREF color)
 {
	CPen pen(PS_SOLID,1,color);
	pDC->SelectObject(&pen);
	pDC->MoveTo(left,top);
	pDC->LineTo(right,bottom);


 }
 void CEarButton::OnLButtonDown(UINT nFlags,CPoint point)
 {
	// MessageBox(_T("rrr"));
	 	//	buttupdate = TRUE;
		// Invalidate();
	 CETApplicationView *prev = (CETApplicationView*) this->GetParent();
	 int id =0 ;
	 id =  GetWindowLong(this->GetSafeHwnd(),GWL_ID); //operaID
	// if(id == 2000)
	// {
		 CETApplicationView *p = (CETApplicationView *)this->GetParent();

		 if(p->ampsideID == 0)
		 {
			 if((id >= IDC_SUB_BUTT_RT) && (id < IDC_SUB_BUTT_LF))
			 {
				 p->btnDown = TRUE;
				 //p->Invalidate();
				 p->operaID = id - IDC_SUB_BUTT_RT;
				// p->RedrawWindow();
			 }
		 }
		 if(p->ampsideID == 1)
		 {
			  if((id >= IDC_SUB_BUTT_LF) && (id < 3000))
			 {
				 p->btnDown = TRUE;
				 p->operaID = id - IDC_SUB_BUTT_LF;
			 }

		 }
	              //pPatientList[1]->SetItemText(1,2,_T("pressed"));
	                // pPatientList[1]->SetItemText(1,2,_T(" "));
		/* if(!prev->IsAmp)
		 {
		   if(id == 201 )
		     prev->pPatientList[1]->SetItemText(1,2,_T("pressed"));
		 }*/
	                 // }

	 CButton::OnLButtonDown( nFlags, point);
 }

// CEarButton


