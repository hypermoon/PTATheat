// SLPropertySheet.cpp : 实现文件
//

#include "stdafx.h"
#include "ETApplication.h"
#include "ETApplicationView.h"
#include "SLPropertySheet.h"
#include "SvLdPropertyPage.h"

// CSLPropertySheet

IMPLEMENT_DYNAMIC(CSLPropertySheet, CPropertySheet)
extern HINSTANCE hResourceHandle;
CSLPropertySheet::CSLPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{

		 memset(msheet_buffcancel,0,sizeof(TCHAR)*12);
	     memset(msheet_buffapply,0,sizeof(TCHAR)*12);
}

CSLPropertySheet::CSLPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	    memset(msheet_buffcancel,0,sizeof(TCHAR)*12);
	     memset(msheet_buffapply,0,sizeof(TCHAR)*12);
}

BOOL CSLPropertySheet::OnInitDialog()
{
	BOOL bresult  = CPropertySheet::OnInitDialog();

	HINSTANCE hinst = ::AfxGetInstanceHandle();
	if(hResourceHandle != NULL)
		hinst = hResourceHandle;

	LoadString(hinst,IDS_STR_CANCEL,msheet_buffcancel,12);
	LoadString(hinst,IDS_STR_APPLY,msheet_buffapply,12);

	GetDlgItem(IDOK)->SetWindowTextW(_T("OK"));
	GetDlgItem(IDCANCEL)->SetWindowTextW(msheet_buffcancel);
	GetDlgItem(ID_APPLY_NOW)->SetWindowTextW(msheet_buffapply);

	return bresult;

}
CSLPropertySheet::~CSLPropertySheet()
{
}
/*
void CSLPropertySheet::OnOK()
{
	
	int num = 0;
	int dat[3] = {0,0,0};
	int idx = 0;
	CSvLdPropertyPage *pldpage = (CSvLdPropertyPage*)GetPage(1);
	CListCtrl *plist = pldpage->pm_loadlist;
	num = plist->GetItemCount();

	CETApplicationView *ploadview = (CETApplicationView*)pldpage->pm_storept;

	for(int i =0; i<num; i++)
	{
		if(plist->GetCheck(i))   //checked
		{  
			if(idx >= 3)
			{
				MessageBox(_T("Can't select more than 3 record!"));
				return  ;
			}
			else
			{
                dat[idx++] = i;
			}
		}
	}

	ploadview->SavePTARecordIdx(dat[0],dat[1],dat[2]);

	//PressButton(PSBTN_OK );
}*/

//ID_APPLY_NOW 
BEGIN_MESSAGE_MAP(CSLPropertySheet, CPropertySheet)
	//ON_BN_CLICKED(IDOK,OnOK)
END_MESSAGE_MAP()


// CSLPropertySheet 消息处理程序
