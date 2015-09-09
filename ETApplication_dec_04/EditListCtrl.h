#pragma once

#include "LvEdit.h"
// CEditListCtrl

class CEditListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CEditListCtrl)

public:
	CEditListCtrl();
	virtual ~CEditListCtrl();

	CLvEdit  m_ctrledit,m_sysinfoedit;
	BOOL bShow ,bsysShow; //= FALSE;
	TCHAR m_bufstrpmpt[64];
protected:
	afx_msg void OnLButtonDown( UINT, CPoint );
	afx_msg void OnEditCtlLostFocus();
	afx_msg void OnGetDisInfoFn( NMHDR * pNotifyStruct , LRESULT* result ); 
	DECLARE_MESSAGE_MAP()
};


