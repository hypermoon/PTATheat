#pragma once
#include "resource.h"
#include "ETApplicationView.h"

// CSvLdPropertyPage 对话框

class CSvLdPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSvLdPropertyPage)

public:
	CSvLdPropertyPage();
	void SaveViewPt(CWnd *pview);
	virtual ~CSvLdPropertyPage();
	void LoadGeneralPTArecord(BOOL general = TRUE);
	CButton *pbutton;
	CStatic *pstatic;
	BOOL    SavedPTAset;
	TCHAR   m_bufftime[8];
	TCHAR   m_buffidnum[8];
	TCHAR   m_bufffilesave[128];
	TCHAR   m_buffhadsaved[128];
	TCHAR   m_buffmorethan2[128];
	TCHAR   m_buffmorethan3[128];

	TCHAR        m_buffDelPTAWarn[128];
// 对话框数据
	enum { IDD = IDD_PROPPAGE_SAVE };
	virtual BOOL OnInitDialog();
	CListCtrl *pm_loadlist;
	CWnd      *pm_storept;
	virtual void OnOK( );
	virtual BOOL OnSetActive( );
protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg void OnPageSave();
	afx_msg void OnLDDelete();
	afx_msg void OnLDTcheck();
	DECLARE_MESSAGE_MAP()
};
