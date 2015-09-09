// SvLdPropertyPage.cpp : 实现文件
//

#include "stdafx.h"
#include "ETApplication.h"
#include "SvLdPropertyPage.h"
#include "afxdialogex.h"

extern bool g_bPTANeedSaved;
// CSvLdPropertyPage 对话框
extern HINSTANCE hResourceHandle;
IMPLEMENT_DYNAMIC(CSvLdPropertyPage, CPropertyPage)

CSvLdPropertyPage::CSvLdPropertyPage()
	: CPropertyPage(CSvLdPropertyPage::IDD)
{
	pbutton = NULL;
	SavedPTAset = FALSE;
	memset(m_bufftime,0,sizeof(TCHAR)*8);
	memset(m_buffidnum,0,sizeof(TCHAR)*8);
	memset(m_bufffilesave,0,sizeof(TCHAR)*128);
	memset(m_buffhadsaved,0,sizeof(TCHAR)*128);

	memset(m_buffmorethan2,0,sizeof(TCHAR)*128);
	memset(m_buffmorethan3,0,sizeof(TCHAR)*128);
	memset(m_buffDelPTAWarn,0,sizeof(TCHAR)*128);
}

CSvLdPropertyPage::~CSvLdPropertyPage()
{
}

void CSvLdPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BOOL CSvLdPropertyPage::OnInitDialog()
{
	//IDC_LDPTA_LIST1 IDD_PROPPAGE_SAVE
	CRect rect;

	
    PROPSHEETPAGE psp;
	psp = this->GetPSP();

    HINSTANCE hinst = ::AfxGetInstanceHandle();
		if(hResourceHandle != NULL)
			hinst = hResourceHandle;

	    LoadString(hinst,IDS_STR_TIME,m_bufftime,8);
	    LoadString(hinst,IDS_STR_FILESAVED,m_bufffilesave,128);
	    LoadString(hinst,IDS_STR_FILEHADSAVED,m_buffhadsaved,128);
		LoadString(hinst,IDS_STR_IDNUM,m_buffidnum,8);

	    LoadString(hinst,IDS_STR_MORETHAN2,m_buffmorethan2,128);
		LoadString(hinst,IDS_STR_MORETHAN3,m_buffmorethan3,128);

		LoadString(hinst,IDS_DELPTA_PROMPT, m_buffDelPTAWarn,128);


	if(psp.pszTemplate == MAKEINTRESOURCE(IDD_PROPPAGE_LOAD)) 
	{


		CButton *pbtn = NULL;
		CETApplicationView *pldview = (CETApplicationView*)pm_storept;
		pm_loadlist = (CListCtrl*)GetDlgItem(IDC_LDPTA_LIST1);
		pm_loadlist->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES); 
		pm_loadlist->GetClientRect(rect);
		pm_loadlist->InsertColumn(0,m_buffidnum); //_T("ID"));
		pm_loadlist->InsertColumn(1,m_bufftime);  //_T("TIME"));
		pm_loadlist->SetColumnWidth(0,rect.Width()*2 /5);
		pm_loadlist->SetColumnWidth(1,rect.Width()*3 /5 );
		pldview->m_LDPage_T_isChecked = FALSE;
		pbtn = (CButton*)GetDlgItem(IDC_LD_DEL);
		pbtn->ModifyStyle(0,BS_PUSHBUTTON|WS_BORDER);

	//	LoadGeneralPTArecord();
	//	pm_loadlist->Invalidate();

	
	}
	if(psp.pszTemplate == MAKEINTRESOURCE(IDD_PROPPAGE_SAVE)) 
	{
		CString retTstr;
		CETApplicationView *psavedview = (CETApplicationView*)pm_storept;
	    CTime theTime = CTime::GetCurrentTime();
	    CString szTime = theTime.Format(" %Y-%m-%d_%H:%M ");

		pbutton = (CButton*)GetDlgItem(IDC_SPAG_SAVEBUTT);
		pstatic = (CStatic*)GetDlgItem(IDC_STATIC);
		pstatic->SetWindowTextW(szTime);

		retTstr = psavedview->plistctr[0]->GetItemText(3,1);
		if(retTstr.GetLength() == 0)
		{
			GetDlgItem(IDC_CHECK_T)->EnableWindow(FALSE);
		}
		else
			GetDlgItem(IDC_CHECK_T)->EnableWindow(TRUE);
		
	}
	return 0;

}
BOOL CSvLdPropertyPage::OnSetActive( )
{
	PROPSHEETPAGE psp;
	psp = this->GetPSP();
	
	if(psp.pszTemplate == MAKEINTRESOURCE(IDD_PROPPAGE_LOAD)) 
	{
		LoadGeneralPTArecord();
		pm_loadlist->Invalidate();
	}
	
	return CPropertyPage::OnSetActive();
}

void CSvLdPropertyPage::LoadGeneralPTArecord(BOOL general)
{
		CString strtime,strid;
		BOOL readstat = FALSE;
		int num = 0;
	    CETApplicationView *ploadview = (CETApplicationView*)pm_storept;
			
	    readstat = ploadview->ReadPtaFile(!general);

		pm_loadlist->DeleteAllItems();
		if(!readstat)   //audiomed connect
		{
				if(general)
				{
					num = ploadview->GetPtaRecordNum();
				}
				else
				{
					num = ploadview->GetPtaTTRecordNum();
				}
				for(int i = 0; i <num; i++)
				{
					strid.Format(_T("%d"),i+1);
					if(general)
					{
						strtime = ploadview->ReadSinglePtaDataRecord(i);
					}
					else
					{
						strtime = ploadview->ReadSingleTT_PtaDataRecord(i);
					}

					pm_loadlist->InsertItem(i,strid);
					pm_loadlist->SetItemText(i,1,strtime);
				}
		}
		else
		{
			ploadview->cldlist.RemoveAll();
			ploadview->cldT_list.RemoveAll();

		}

		pm_loadlist->Invalidate();

}
void CSvLdPropertyPage::OnOK( )
{
		int num = 0;
		int dat[3] = {-1,-1,-1};
		int idx = 0;
	
		PROPSHEETPAGE psp;
	    psp = this->GetPSP();

	    if(psp.pszTemplate == MAKEINTRESOURCE(IDD_PROPPAGE_LOAD)) 
		{
                
				num = pm_loadlist->GetItemCount();

				CETApplicationView *ploadview = (CETApplicationView*)this->pm_storept;

				for(int i =0; i<num; i++)
				{
					if(pm_loadlist->GetCheck(i))   //checked
					{  
						if(idx >= 3)
						{
							if(ploadview->m_LDPage_T_isChecked)
							{
								if(idx >= 2)
								   MessageBox(m_buffmorethan2);  //_T("Can't select more than 2 record!")); //fReadPta
							}
							else
							       MessageBox(m_buffmorethan3);  //_T("Can't select more than 3 record!")); //fReadPta

							if(ploadview->fReadPta.m_hFile == CFile::hFileNull)
                                 return  ;
							else
							{
								ploadview->fReadPta.Close();
								return;
							}
						}
						else
						{
							 
							if(ploadview->m_LDPage_T_isChecked)
							{
								if(idx >= 2)
								{
									 MessageBox(m_buffmorethan2);
									 return;
								}
							}
							dat[idx++] = i;
							
							 
							
						}
					}
				}

				ploadview->SavePTARecordIdx(dat[0],dat[1],dat[2]);
		}
	
	
	CPropertyPage::OnOK();

}
 

void CSvLdPropertyPage::OnPageSave()
{
   // CWnd *p;
	//p = AfxGetMainWnd();  IDC_CHECK_T
	CButton *pcheck = NULL;
	BOOL ret;
	BOOL checkstat = FALSE;
	SavedPTAset = TRUE;
	CETApplicationView *psaveview = (CETApplicationView*)pm_storept;
	
	pcheck = (CButton*)GetDlgItem(IDC_CHECK_T);

	if(pcheck->GetCheck() == BST_CHECKED)  //need to save T
	   checkstat = TRUE;
	
	/*if((checkstat == FALSE)&&(!g_bPTANeedSaved))
	{
		MessageBox(m_buffhadsaved); //_T("file already saved"));
		return;
	}*/

	ret =  psaveview->WritePtaFile(checkstat);
	if(!ret)
	{
		MessageBox(m_bufffilesave);   //_T("file saved"));
		if(checkstat == FALSE)
			g_bPTANeedSaved = false;
	}
}

void CSvLdPropertyPage::OnLDDelete()
{
	CETApplicationView *plddelview = (CETApplicationView*)pm_storept;
	int  num =0;
	BOOL *prec = NULL;
	BOOL ret = 0;

	int msgret = 0;

	msgret = MessageBox(m_buffDelPTAWarn,_T(""),MB_YESNO);

	if(msgret == IDYES)
	{
		num = pm_loadlist->GetItemCount();

		prec = new BOOL[num];

		memset(prec,0,sizeof(BOOL)*num);

		for(int i =0; i<num; i++)
		{
			if(pm_loadlist->GetCheck(i))   //checked
			{
				prec[i] = TRUE;
			}
		}

		ret  = plddelview->DelPTARecord(prec,plddelview->m_LDPage_T_isChecked);

		LoadGeneralPTArecord();
	}

}
void CSvLdPropertyPage::SaveViewPt(CWnd *pview)
{
	pm_storept = pview;
}
void CSvLdPropertyPage::OnLDTcheck()
{
	CETApplicationView *psaveview = (CETApplicationView*)pm_storept;
	CButton *pcheck = NULL;
	pcheck = (CButton*)GetDlgItem(IDC_CHECK_LOADT);

	pm_loadlist->DeleteAllItems();
	if(pcheck->GetCheck() == BST_CHECKED )
	{
	   LoadGeneralPTArecord(FALSE);	  //load TT
	   psaveview->m_LDPage_T_isChecked = TRUE;
	 
	}
	else if(pcheck->GetCheck() == BST_UNCHECKED  )
	{
		psaveview->m_LDPage_T_isChecked = FALSE;
		LoadGeneralPTArecord();
	}
		pm_loadlist->Invalidate();


}

BEGIN_MESSAGE_MAP(CSvLdPropertyPage, CPropertyPage)
	ON_BN_CLICKED(IDC_SPAG_SAVEBUTT,OnPageSave)
	ON_BN_CLICKED(IDC_CHECK_LOADT, OnLDTcheck)
	ON_BN_CLICKED(IDC_LD_DEL,OnLDDelete)
END_MESSAGE_MAP()


// CSvLdPropertyPage 消息处理程序
