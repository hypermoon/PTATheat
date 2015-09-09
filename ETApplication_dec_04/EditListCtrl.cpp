// EditListCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "ETApplication.h"
#include "EditListCtrl.h"
#include "ETApplicationView.h"


// CEditListCtrl

IMPLEMENT_DYNAMIC(CEditListCtrl, CListCtrl)

extern HINSTANCE hResourceHandle;

CEditListCtrl::CEditListCtrl()
{
	 bShow = FALSE;
	 bsysShow = FALSE;
	 memset(m_bufstrpmpt,0,64*sizeof(TCHAR));

}

CEditListCtrl::~CEditListCtrl()
{
}

void CEditListCtrl::OnLButtonDown( UINT in, CPoint pt )
{
	
	CListCtrl::OnLButtonDown(in,pt);
	CRect rtBound,lvRect;
	LONG len=0,high=0;
	LVHITTESTINFO lvhti;
	lvhti.pt = pt;

	HINSTANCE hinst = ::AfxGetInstanceHandle();
	if(hResourceHandle != NULL)
		hinst = hResourceHandle;
	LoadString(hinst,IDS_STRING_PROMPT,m_bufstrpmpt,64);
	
	CETApplicationView *prev = (CETApplicationView*) this->GetParent();

	LONG ret =  GetWindowLong(this->GetSafeHwnd(),GWL_ID);  
    if(ret == IDC_LIST_PATIENTDATA) 
    {
		if(prev->m_LinkState != LINKED && prev->pbutt[0]->pressed )
	   {
			this->SubItemHitTest(&lvhti);
			if(lvhti.iItem > -1 && (lvhti.iSubItem == 1 || lvhti.iSubItem == 3 || (lvhti.iSubItem == 2 && lvhti.iItem == 2) ))
			{
					this->GetSubItemRect( lvhti.iItem, lvhti.iSubItem ,LVIR_LABEL, rtBound );
					this->SetItemState(lvhti.iItem,LVIS_SELECTED,LVIS_SELECTED);

					::GetWindowRect(this->GetSafeHwnd(),lvRect);
				    ScreenToClient(lvRect);

					len = rtBound.Width();
					high = rtBound.Height();
					rtBound.left += lvRect.left;
					rtBound.top += lvRect.top;
					rtBound.right = rtBound.left + len;
					rtBound.bottom = rtBound.top + high;

					if(m_ctrledit.m_hWnd   ==   NULL)
					{
					  m_ctrledit.Create(ES_AUTOHSCROLL|WS_CHILD|ES_LEFT|ES_WANTRETURN|WS_BORDER,CRect(0,0,0,0),this,IDC_LST_EDIT);
					  m_ctrledit.ShowWindow(SW_HIDE);
					  CFont   tpFont;
					  tpFont.CreateStockObject(DEFAULT_GUI_FONT);
					  m_ctrledit.SetFont(&tpFont);
					  tpFont.DeleteObject();
					}

					if(lvhti.iItem ==0 && lvhti.iSubItem ==1)
					{
			
					   bShow = TRUE;
					}

					if(bShow   ==   TRUE)
					{
						CString   strpmp =  m_bufstrpmpt; // _T("Pls Input ID ...");
						CString   strItem   =   this->GetItemText( lvhti.iItem,lvhti.iSubItem);
						m_ctrledit.MoveWindow(rtBound);
						m_ctrledit.ShowWindow(SW_SHOW);
						if(lvhti.iItem ==0 && lvhti.iSubItem ==1)
						{
							if(!strItem.CompareNoCase(strpmp))
								m_ctrledit.SetWindowText(_T(""));
							else
							{
								prev->m_ShowWarning = FALSE;
								prev->m_warncount = 15;
								prev->m_strwarnct = _T("15");
								prev->KillTimer(prev->m_nwarnTime);
								m_ctrledit.SetWindowText(strItem);
								prev->Invalidate();
							}
						}
						else
								m_ctrledit.SetWindowText(strItem);

						::SetFocus(m_ctrledit.GetSafeHwnd());
						m_ctrledit.SetSel(-1);
					 }
				  else
					  m_ctrledit.ShowWindow(SW_HIDE);
				}
						   
			    if(m_ctrledit.m_hWnd != NULL)
			{

			    if((lvhti.iItem == 0 && (lvhti.iSubItem == 1 || lvhti.iSubItem ==3)) || (lvhti.iItem ==1 && lvhti.iSubItem ==1) )
				{
					m_ctrledit.SetLimitText(15);
				}
				else
				{
					m_ctrledit.SetLimitText(26);
				}
				
			    m_ctrledit.SaveCurrentPos(lvhti.iItem,lvhti.iSubItem);
			 }
	     }

    }
	
}
  /*if(ret == IDC_LIST_PATIENTHST)
  {
	  this->SubItemHitTest(&lvhti);
	  if(lvhti.iItem > -1 && (lvhti.iSubItem == 2))
	  {
		  this->GetSubItemRect( lvhti.iItem, lvhti.iSubItem ,LVIR_LABEL, rtBound );
	      this->SetItemState(lvhti.iItem,LVIS_SELECTED,LVIS_SELECTED);
		  ::GetWindowRect(this->GetSafeHwnd(),lvRect);
		  ScreenToClient(lvRect);

			len = rtBound.Width();
			high = rtBound.Height();
			rtBound.left += lvRect.left;
			rtBound.top += lvRect.top;
			rtBound.right = rtBound.left + len;
			rtBound.bottom = rtBound.top + high;

			if(m_sysinfoedit.m_hWnd   ==   NULL)  //IDC_LST_SYSEDIT
			{
				  m_sysinfoedit.Create(ES_AUTOHSCROLL|WS_CHILD|ES_LEFT|ES_WANTRETURN|WS_BORDER,CRect(0,0,0,0),this,IDC_LST_SYSEDIT);
				  m_sysinfoedit.ShowWindow(SW_HIDE);

			}
			if(lvhti.iItem == 1 && lvhti.iSubItem ==2)
			{
			   bsysShow = TRUE;
			}
			if(bsysShow == TRUE)
			{
				 m_sysinfoedit.MoveWindow(rtBound);
			     m_sysinfoedit.ShowWindow(SW_SHOW);

			}
	  }
  }
  */
	

void CEditListCtrl::OnEditCtlLostFocus()
{
	int item =  m_ctrledit.GetnItemidx();  //Pls input ...
	int subitem = m_ctrledit.GetsubItemidx();
	CString str;
	BOOL Idret=FALSE;
	CETApplicationView *prev = (CETApplicationView*) this->GetParent();
	m_ctrledit.GetWindowTextW(str);

	if(item == -1 && subitem == -1)
	    return;
	else
	{
      /*  if((item == 0 && (subitem == 1 || subitem ==3)) || (item ==1 && subitem ==1) )
		{
			m_ctrledit.SetLimitText(15);
		}
		*/
		m_ctrledit.ShowWindow(FALSE);
		this->SetItemText(item,subitem,str);  
	}

	if((item == 0 && subitem ==1) )
	{
		if(!str.GetLength())
		{  
			this->SetItemText(0,1, m_bufstrpmpt); //_T("Pls Input ID ..."));                            //no input
	        bShow = FALSE;
		}
		else
		{
			Idret = prev->TestIfUserExists(str);
			if(Idret)
			{
				bShow = FALSE;
			}
			else
			{
				prev->pPatientList[0]->SetItemText(0,3,_T(""));        //name
				prev->pPatientList[0]->SetItemText(1,1,_T(""));        //age
				prev->pPatientList[0]->SetItemText(1,3,_T(""));        //gender
				prev->pPatientList[0]->SetItemText(2,1,_T(""));        //address

				bShow = TRUE;
			}
		}
	}

	this->UpdateWindow();

}

void CEditListCtrl::OnGetDisInfoFn( NMHDR * pNotifyStruct , LRESULT* result )
{
	 LV_DISPINFO  *pinfo =  (LV_DISPINFO*) pNotifyStruct;
	 int item,isubitem, ipushbtnitem =2;
	 CRect rtBound,lvRect;
	 BOOL  pushedstat = FALSE;
	 CString  str;
	 LONG len=0,high=0;
	 item     = pinfo->item.iItem;
	 isubitem = pinfo->item.iSubItem; 
	 LONG ret =  GetWindowLong(this->GetSafeHwnd(),GWL_ID);  
	 CETApplicationView *prev = (CETApplicationView*) this->GetParent();

	 if(ret == IDC_LIST_PATIENTHST ) //sysinfo
	 {


	   if((item == 1) && (isubitem  == 3))
	    {
			ipushbtnitem = isubitem -1 ;
			//isubitem = isubitem+ 1; //add for no sense test
			str = this->GetItemText(item,ipushbtnitem );
			if(!str.CompareNoCase(_T("pressed")))
			{
				  this->GetSubItemRect( item, ipushbtnitem ,LVIR_LABEL, rtBound );
				  this->SetItemState(item,LVIS_SELECTED,LVIS_SELECTED);
				  ::GetWindowRect(this->GetSafeHwnd(),lvRect);
				  ScreenToClient(lvRect);

				len = rtBound.Width();
				high = rtBound.Height();
				rtBound.left += lvRect.left+2;
				rtBound.top += lvRect.top+2;
				rtBound.right = rtBound.left + len-2;
				rtBound.bottom = rtBound.top + high-2;

					if(m_sysinfoedit.m_hWnd   ==   NULL)  //IDC_LST_SYSEDIT
					{
						  m_sysinfoedit.Create(ES_AUTOHSCROLL|WS_CHILD|ES_LEFT|ES_WANTRETURN|WS_BORDER,CRect(0,0,0,0),this,IDC_LST_SYSEDIT);
						  m_sysinfoedit.ShowWindow(SW_HIDE);

					}

					  bsysShow = TRUE;
					
					if(bsysShow == TRUE)
					{  
						 m_sysinfoedit.SetBKcolors(SPICALCOLOR); //RGB(150,100,100));
						 m_sysinfoedit.MoveWindow(rtBound);
						 m_sysinfoedit.ShowWindow(SW_SHOW);
						 m_sysinfoedit.EnableWindow(FALSE);
						 //m_sysinfoedit.ShowWindow(SW_HIDE);
						 m_sysinfoedit.SaveCurrentPos(item,ipushbtnitem);
					}

			}
			else
			{
				      if(m_sysinfoedit.m_hWnd   !=   NULL)
				         m_sysinfoedit.ShowWindow(SW_HIDE);
			}
	    }
	 }
}

BEGIN_MESSAGE_MAP(CEditListCtrl, CListCtrl)
	ON_WM_LBUTTONDOWN( )
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO,OnGetDisInfoFn)
	ON_EN_KILLFOCUS(IDC_LST_EDIT,OnEditCtlLostFocus)
END_MESSAGE_MAP()



// CEditListCtrl 消息处理程序


