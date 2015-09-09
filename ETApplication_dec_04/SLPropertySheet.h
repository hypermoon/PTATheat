#pragma once



// CSLPropertySheet
//#include "SvLdPropertyPage.h"
class CSLPropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CSLPropertySheet)

public:
	CSLPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CSLPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CSLPropertySheet();
	virtual BOOL OnInitDialog( );

	TCHAR        msheet_buffcancel[12];
	TCHAR        msheet_buffapply[12];
 
	//CSvLdPropertyPage *pldpage;
protected:
	//afx_msg void OnOK();
	DECLARE_MESSAGE_MAP()
};


