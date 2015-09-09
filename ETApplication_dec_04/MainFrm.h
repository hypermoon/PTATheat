
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "ETApplicationView.h"
typedef struct freznode
{
	float band;
	float dbhl;
}FREZNODE;
typedef CList<FREZNODE,FREZNODE&> CDocList;

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();
	BOOL        m_IsPTAtested;
	CDocList    m_Lf_list[3], m_Rt_list[3];
// Attributes
public:

// Operations
public:
	CETApplicationView *pm_save;
// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	
	/*virtual void OnUpdateFrameMenu(HMENU hMenuAlt);
	virtual BOOL LoadFrame(
						   UINT nIDResource,
						   DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
						   CWnd* pParentWnd = NULL,
						   CCreateContext* pContext = NULL 
						   );
						   
	virtual BOOL OnCreateClient(
						   LPCREATESTRUCT lpcs,
						   CCreateContext* pContext 
						   );
						   */
 
 
 


// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
		void StayOnTop(bool toTop);
protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	//CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;

// Generated message map functions
protected:
	afx_msg LRESULT OnNcHitTest(CPoint point );
	afx_msg void OnNcLButtonDown(UINT nHitTest,CPoint point );
	afx_msg void OnNcLButtonDblClk(UINT nHitTest,CPoint point );
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};


