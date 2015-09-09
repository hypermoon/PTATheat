// EarSliderCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "ETApplication.h"
#include "EarSliderCtrl.h"
#include "ETApplicationView.h"


// CEarSliderCtrl

IMPLEMENT_DYNAMIC(CEarSliderCtrl, CSliderCtrl)

CEarSliderCtrl::CEarSliderCtrl()
{
	m_dcBk = NULL;
	m_recthumb.left = 0;
	m_recthumb.right = 40;
	m_recthumb.top = 0;
	m_recthumb.bottom = 20;
	slidermoved = FALSE;
	appeared = FALSE;
	currpos = 0;
	ctlid = 0;
	m_focused = FALSE;
}

CEarSliderCtrl::~CEarSliderCtrl()
{
}


BEGIN_MESSAGE_MAP(CEarSliderCtrl, CSliderCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,OnReflectCustomDraw)
	ON_WM_LBUTTONUP( )
	ON_WM_LBUTTONDOWN( )
	ON_WM_KILLFOCUS()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CEarSliderCtrl::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

void CEarSliderCtrl::OnKillFocus(CWnd* pNewWnd )
{
	
	m_focused = FALSE;
	//this->Invalidate();
	CSliderCtrl::OnKillFocus(pNewWnd);
}

void CEarSliderCtrl::OnLButtonUp(UINT nFlags, CPoint point )
{
	//MessageBox(_T("tt"));
	int pos = 0;
	CETApplicationView *p = (CETApplicationView*) this->GetParent();
	RECT rect;
	p->sliderpressed = FALSE;
	//p->btnDown   = FALSE;
	this->GetThumbRect(&rect);
	ClientToScreen(&rect);
	p->displayrect = rect;
	slidermoved = TRUE;
	pos = this->GetPos(); //ctlid
	p->UpdateAMPListDbhr(ctlid,pos);
	p->m_pos = pos;
	//this->GetParent()->RedrawWindow();
	m_focused = TRUE;


	CSliderCtrl::OnLButtonUp( nFlags,  point);
}

 void CEarSliderCtrl::OnLButtonDown(UINT nFlags, CPoint point )
 {
	 CETApplicationView *p = (CETApplicationView*) this->GetParent();
	        //***********
	 int id =0 ;
	 id =  GetWindowLong(this->GetSafeHwnd(),GWL_ID); 

	// if(p->ampsideID == 0)
	// {
		 if((id >= IDC_SLIDER_RT) && (id < IDC_SLIDER_LF ))
		 {
			   p->ptamanualID = 0;
			   p->operaID = id -IDC_SLIDER_RT;
		 }
	// }
	// if(p->ampsideID == 1)
	// {
		 if((id >= IDC_SLIDER_LF ) && (id < 4000))
		 {
			   p->ptamanualID = 1;
			   p->operaID = id -IDC_SLIDER_LF ;
		 }
    //  }
	        //*************

	 ctlid = p->operaID;
	 p->sliderpressed = TRUE;
	 CSliderCtrl::OnLButtonDown( nFlags,  point);
 }

void CEarSliderCtrl::OnReflectCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMCUSTOMDRAW nmcd = *(LPNMCUSTOMDRAW)pNMHDR;
	LPNMCUSTOMDRAW lpcd = (LPNMCUSTOMDRAW)pNMHDR;

	UINT drawStage = nmcd.dwDrawStage;
	UINT itemSpec = nmcd.dwItemSpec;
	CDC *pDC = CDC::FromHandle(nmcd.hdc);
	
	switch(drawStage)
	{
		case CDDS_PREPAINT:
					*pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT ;
		            break;
		case CDDS_PREERASE:
		case CDDS_POSTERASE:
		case CDDS_ITEMPREERASE:
		case CDDS_ITEMPOSTERASE:
			       	*pResult = CDRF_DODEFAULT;
		            break;
            case CDDS_ITEMPREPAINT:
				   /* switch(itemSpec)
					{
						case TBCD_CHANNEL:
							 *pResult = CDRF_DODEFAULT| CDRF_NOTIFYPOSTPAINT;
			                 break;
						case TBCD_TICS:
						case TBCD_THUMB:
							 *pResult = CDRF_DODEFAULT;
			                 break;

					}
					break;*/
							switch(itemSpec)
							{
								case TBCD_TICS:
									*pResult = CDRF_DODEFAULT;
									break;
								case TBCD_THUMB:
									//DrawThumb(pDC, lpcd);
									
									if( m_focused ) 
									{
										CString str;
										DrawThumb(pDC, lpcd);
										*pResult =  CDRF_SKIPDEFAULT;
										break;
									}
									else
									{
										*pResult = CDRF_DODEFAULT ; //CDRF_SKIPDEFAULT;// CDRF_DODEFAULT;//CDRF_SKIPDEFAULT;//CDRF_SKIPDEFAULT;
										break;
									}

									
								//	str.Format(_T("%x"),lpcd->uItemState); CDIS_GRAYED
								//	DrawThumb(pDC, lpcd,str);
								//	*pResult =  CDRF_SKIPDEFAULT;
                                //    break;

								case TBCD_CHANNEL:
									if(this->IsWindowVisible())
									{
										DrawChannel(pDC,&nmcd);
										*pResult = CDRF_SKIPDEFAULT;//CDRF_SKIPDEFAULT;//CDRF_SKIPDEFAULT;// //CDRF_SKIPDEFAULT; //CDRF_SKIPDEFAULT ;//CDRF_SKIPDEFAULT;  //CDRF_SKIPDEFAULT;
									}
									else
                                    {
										*pResult = CDRF_DODEFAULT;
									}
									break;
							}
							break;
			case CDDS_ITEMPOSTPAINT:
					switch(itemSpec)
					{
						case TBCD_CHANNEL:
							/*  {
								    COLORREF m_color = RGB(128,128,128);
									COLORREF m_clr   =  ::GetSysColor(COLOR_BTNFACE);  //RGB(200,30,40);
									HDC  hbkdc  = this->GetParent()->GetDC()->GetSafeHdc();
									COLORREF bkcolor = ::GetBkColor(hbkdc);
									
									CDC* pDC = CDC::FromHandle( nmcd.hdc );
									
									CPen pen(PS_SOLID,nmcd.rc.right-nmcd.rc.left, bkcolor); //m_clr);
									//pDC->SelectObject(&pen);
									
									RECT rrc = {nmcd.rc.left, nmcd.rc.top+1, nmcd.rc.right, nmcd.rc.bottom-1};
								//	pDC->SetBkMode(TRANSPARENT);
								//	pDC->Draw3dRect( &rrc, bkcolor, bkcolor );

									pDC->SelectObject(&pen);
									pDC->MoveTo(nmcd.rc.left,nmcd.rc.top);
									pDC->LineTo(nmcd.rc.left,nmcd.rc.bottom); 

									pDC->MoveTo(nmcd.rc.right,nmcd.rc.top);
									pDC->LineTo(nmcd.rc.right,nmcd.rc.bottom); 
									//pDC->DrawEdge(&nmcd.rc,EDGE_BUMP,BF_RECT);
									pDC->Detach();
								}
								*pResult = CDRF_SKIPDEFAULT;
								break;*/
						case TBCD_TICS:
							        *pResult = CDRF_DODEFAULT;
			                        break;
						case TBCD_THUMB:
							  
									*pResult = CDRF_DODEFAULT ; // CDRF_DODEFAULT; // CDRF_SKIPDEFAULT; //CDRF_DODEFAULT;  //CDRF_DODEFAULT;//CDRF_SKIPDEFAULT;
									 break;
									
							 	

					}
					break;

			case CDDS_POSTPAINT:
				  	   *pResult = CDRF_DODEFAULT;
		               break;

		   default:
			        ASSERT( FALSE );

	}

}

void CEarSliderCtrl::SetThumbRect(RECT rect)
{
	m_recthumb = rect;
}
void  CEarSliderCtrl::DrawThumb(CDC *pDC, LPNMCUSTOMDRAW lpcd,CString str)
{

	//int len = lpcd->rc.right - lpcd->rc.left;
	//lpcd->rc.right = lpcd->rc.left + 3 * len/4;
	/*lpcd->rc = m_recthumb;
	lpcd->rc.left = m_recthumb.left;
	lpcd->rc.right = m_recthumb.right;
	lpcd->rc.bottom = m_recthumb.bottom;
	lpcd->rc.top = m_recthumb.top;*/
	RECT rects,channelrect;

	this->GetThumbRect(&rects);
	this->GetChannelRect(&channelrect);
	this->GetClientRect(&channelrect);
	
	//int len = lpcd->rc.right - lpcd->rc.left;
	//lpcd->rc.right = lpcd->rc.left+len*6/5;
	//m_recthumb.left +=2;
	//m_recthumb.right -=2;
	//m_recthumb.top +=1;
	//m_recthumb.bottom -=1;
	//lpcd->rc = m_recthumb;

	//pDC->Rectangle(&lpcd->rc);
	pDC->Draw3dRect(&lpcd->rc,RGB(192,192,192),RGB(128,128,128));

	CBrush brush;
	brush.CreateSolidBrush(SPICALCOLOR); //RGB(150,100,100));
	pDC->SelectObject(&brush);

	lpcd->rc.left += 1;
	lpcd->rc.right -= 1;
	lpcd->rc.top +=1;
	lpcd->rc.bottom -=1;

	pDC->FillRect(&lpcd->rc,&brush);

}

void  CEarSliderCtrl::DrawChannel(CDC *pDC, LPNMCUSTOMDRAW lpcd)
{
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
	hSaveHDC = ::CreateCompatibleDC(pDC->m_hDC);
	hSaveBmp = ::CreateCompatibleBitmap(hSaveHDC, iWidth, iHeight);
	HBITMAP hSaveCBmpOld = (HBITMAP)::SelectObject(hSaveHDC, hSaveBmp);			

	COLORREF crOldBack = ::SetBkColor(pDC->m_hDC, RGB(0,0,0));
	COLORREF crOldText = ::SetTextColor(pDC->m_hDC, RGB(255,255,255));	

	::BitBlt(hSaveHDC, 0, 0, iWidth, iHeight, pDC->m_hDC, crect.left, crect.top, SRCCOPY);
	::BitBlt(pDC->m_hDC, 0, 0, iWidth, iHeight, m_dcBk, 0, 0, SRCCOPY);
	::BitBlt(pDC->m_hDC, 0, 0, iWidth, iHeight, hSaveHDC, 0, 0, SRCAND);
	//restore and clean up

	::SetBkColor(pDC->m_hDC, crOldBack);
	::SetTextColor(pDC->m_hDC, crOldText);
	::SelectObject(hSaveHDC, hSaveCBmpOld);
	::DeleteObject(hSaveBmp);
	::DeleteDC(hSaveHDC);

/*	crect = lpcd->rc;
	if ((crect.bottom - crect.top) > (crect.right - crect.left))
		crect.InflateRect(1, 0, 1, 0);
	else								//is there a better way to know vert from horiz sliders??
		crect.InflateRect(0, 2, 0, 2);
	DrawEdge(pDC->m_hDC, &crect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
	*/
	
}
// CEarSliderCtrl 消息处理程序


