// StableListCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "ETApplication.h"
#include "StableListCtrl.h"
#include "ETApplicationView.h"


// CStableListCtrl

IMPLEMENT_DYNAMIC(CStableListCtrl, CListCtrl)

CStableListCtrl::CStableListCtrl()
{
	ptateststat = FALSE;
	isubitem = -1;
	m_side = 0;

}

CStableListCtrl::~CStableListCtrl()
{
}

BOOL   CStableListCtrl::OnNotify(WPARAM   wParam,   LPARAM   lParam,   LRESULT*   pResult)   
{ 
	switch   (((NMHDR*)lParam)-> code) 
	{ 
		case   HDN_BEGINTRACKW: 
		case   HDN_BEGINTRACKA: 
		*pResult   =   TRUE;                                 //   disable   tracking 
		return   TRUE;                                         //   Processed   message 
	} 

	return   CListCtrl::OnNotify(wParam,   lParam,   pResult); 
}
void   CStableListCtrl::OnCustomdrawMyList(NMHDR   *pNMHDR,   LRESULT   *pResult) 
{ 
     NMLVCUSTOMDRAW*   pLVCD   =   reinterpret_cast <NMLVCUSTOMDRAW*> (   pNMHDR   ); 
	  CETApplicationView *p = (CETApplicationView*) this->GetParent();
	 NMCUSTOMDRAW nmcd =  pLVCD->nmcd;
	 *pResult   =   CDRF_DODEFAULT; 
	 CDC *pDC = CDC::FromHandle(nmcd.hdc);
	 double iscal  =  p->iscal;
	 double yiscal =  p->yiscal;
	 int side = 0 ;

	 LONG ret =  GetWindowLong(this->GetSafeHwnd(),GWL_ID);  // IDC_LIST_PATIENTDATA :IDC_LIST_PATIENTHST
	 if(ret == 500)
		   side = 0;          //right
	 if(ret == 501)
		   side = 1;          //left

	 int  nItem   =   static_cast <int> (   pLVCD-> nmcd.dwItemSpec   ); 

	if(CDDS_PREPAINT   ==   pLVCD-> nmcd.dwDrawStage) 
	{ 
		*pResult   =   CDRF_NOTIFYITEMDRAW; 
	} 
    else if(CDDS_ITEMPREPAINT   ==   pLVCD-> nmcd.dwDrawStage) 
	{ 
        *pResult   =  CDRF_NOTIFYSUBITEMDRAW; 
	} 
    else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD-> nmcd.dwDrawStage   ) 
	{ 
		//COLORREF   clrNewTextColor,   clrNewBkColor; 
		CPen *poldpen = NULL;
		CRect rtBound;
		int x,y;
		

			LOGBRUSH logBrush1 ,logBrush2;   
			logBrush1.lbStyle=BS_SOLID;   
			logBrush1.lbColor=RGB(160,40,30);   

			logBrush2.lbStyle=BS_SOLID;   
			logBrush2.lbColor=RGB(40,30,160); 

			CPen pen[2]={CPen(PS_SOLID,2,RGB(208,39,28)),CPen(PS_SOLID,2,RGB(28,28,208))};

			CPen dashpen[2]={CPen(PS_DASH,1,&logBrush1),CPen(PS_DASH,1,&logBrush2)};

			CPen dotpen[2]= {CPen(PS_DOT,1,&logBrush1),CPen(PS_DOT,1,&logBrush2)};

			pDC->SetBkMode(TRANSPARENT);

		

			if(nItem == 0 && pLVCD->iSubItem==0 )
			{
			   this->GetSubItemRect( nItem, pLVCD->iSubItem ,LVIR_LABEL, rtBound );
			   rect1  = rtBound;
			}
			  
			   x = rect1.left + (rect1.right - rect1.left)/2;
			   y = rect1.top + (rect1.bottom - rect1.top)/2;
			   poldpen = pDC->SelectObject(&pen[side]);
    
			  if(!p->IsAmp && !p->m_LDPage_T_isChecked)
			  {
			   pDC->MoveTo(x,y);    //(x*iscal,y*yiscal);
			   pDC->LineTo(rect1.right,y);    //(rect1.right *iscal,y *yiscal);
			  }

			//pDC->MoveTo(40*iscal,34*yiscal);
			//pDC->LineTo(82*iscal,34*yiscal);
										
			if(nItem == 1 && pLVCD->iSubItem==0 )
			{
			   this->GetSubItemRect( nItem, pLVCD->iSubItem ,LVIR_LABEL, rtBound );
			   rect2  = rtBound;
			}
			   x = rect2.left + (rect2.right - rect2.left)/2;
			   y = rect2.top + (rect2.bottom - rect2.top)/2;

			pDC->SelectObject(&dashpen[side]);
			if(!p->IsAmp  && !p->m_LDPage_T_isChecked)
			{
				pDC->MoveTo(x,y);  //          (x*iscal,y*yiscal);
				pDC->LineTo(rect2.right,y);                     //(rect2.right*iscal,y*yiscal);
			}


			if(nItem == 2 && pLVCD->iSubItem==0 )
			{
			   this->GetSubItemRect( nItem, pLVCD->iSubItem ,LVIR_LABEL, rtBound );
			   rect3  = rtBound;
			}
			   x = rect3.left + (rect3.right - rect3.left)/2;
			   y = rect3.top + (rect3.bottom - rect3.top)/2;

			pDC->SelectObject(&dotpen[side]);
			if(!p->IsAmp  && !p->m_LDPage_T_isChecked)
			{
			pDC->MoveTo(x,y);//(x*iscal,y*yiscal);
			pDC->LineTo(rect3.right,y);       //(rect3.right*iscal,y*yiscal);
			}


			if(ptateststat)
			{
				if(nItem == 0 && pLVCD->iSubItem == isubitem)
				{
		
					if(m_side ==0)
					   pLVCD->clrText = RGB(200,0,10);
					if(m_side == 1)
                       pLVCD->clrText = RGB(10,0,250);

				}
				else
				{
					pLVCD->clrText = RGB(0,0,0);
				}
			}
			else
			{
				pLVCD->clrText = RGB(0,0,0);
			}

			*pResult   = CDRF_DODEFAULT; 
                
       }

}

void CStableListCtrl::DrawSubItemText(BOOL ptatesting ,int column,int side)
{
	ptateststat = ptatesting;
	isubitem = column;
	m_side = side;
	this->Invalidate();

}

BEGIN_MESSAGE_MAP(CStableListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomdrawMyList)
END_MESSAGE_MAP()



// CStableListCtrl 消息处理程序


