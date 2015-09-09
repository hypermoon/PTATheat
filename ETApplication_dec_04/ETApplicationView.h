//-----------------------------------------------------------------------------
// File: ETApplicationView.h
//
// Desc: interface of the CETApplicationView class
//
// Copyright (c) 2011 Eartronic Ltd. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#include "EarButton.h"
#include "EditListCtrl.h"
#include "EarSliderCtrl.h"
#include "ETApplicationDoc.h"
#include "EarStatic.h"
#include "SLPropertySheet.h"
#include "StableListCtrl.h"
#include  <endpointvolume.h>
#include "ETCore//ETCoreLibInc.h"


DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE,  
            0xA5DCBF10L,   0x6530,   0x11D2,   0x90,   0x1F,   0x00,   0xC0,   0x4F,   0xB9,   0x51,   0xED);  
#define   GUID_CLASS_USB_DEVICE                       GUID_DEVINTERFACE_USB_DEVICE 

#ifdef _UNICODE
    #define tmemset wmemset
#else
    #define tmemset memset
#endif

#define    SLIDERGAP               11
#define    HIGHFREQNUM             10
#define    DIARYNUM                30
//typedef  PVOID           HDEVNOTIFY;

typedef struct linenode
{
	float x;
	float y;
	//float bandval;
}LNNODE;
typedef struct Ampnodes
{
	float xband;
	float ydbhr;
	BOOL  IsSet;
}AMPNODE;

typedef struct LDNODE
{
	TCHAR  time[20];
	UINT    idx;           //idx is based from 1
	int     sizeofbuff;
	LPTSTR  pstrpos;
}LDPTARECNODE;

typedef struct FocusPara
{
		ETSessionID  id; 
		ETLeftRight  side; 
		long freq; 
		float value;
}FOCUSPARAMETER;


typedef CList<LDPTARECNODE,LDPTARECNODE&> CLDPTARDList;
typedef CList<LNNODE,LNNODE&> CNewList;
typedef CList<AMPNODE,AMPNODE&>CAMPList;
typedef CList<CString, CString&>CDiaryTimeList;

#define SPICALCOLOR   RGB(127,127,127)     //(150,100,100)

class CETApplicationView : public CView
{
protected: // create from serialization only
	CETApplicationView();
	DECLARE_DYNCREATE(CETApplicationView)

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	void OnInitalPTAMP();
	void OnInitalHighRangeArray();
// Attributes
public:
	CETApplicationDoc* GetDocument() const;

	ETSessionID m_sessionID;
	ETLeftRight m_leftright;


    CNewList     m_postL_list[3],m_postR_list[3];
	CAMPList     m_AMP_List[2];
	 
	CLDPTARDList cldlist, cldT_list;

	CDiaryTimeList  m_d_TimeList;

	bool m_isPTATesting;
	bool m_PTAPaused;
	ETLinkingStatus m_LinkState;
	bool m_LDPage_T_isChecked;
	bool m_diaryMod;
	int m_screenxLogPixPerInch;
	int m_screenyLogPixPerInch;
	CString     strptapath;
	CString     strdiarypcpath;
	CString     m_stridxpath;
	CBitmap     bmplogo;
	CString     ptalistctltime[3];

	FOCUSPARAMETER m_current;

	int   m_rcdvalarr[2][11];
// Operations
public:
	LPTSTR strButt[8];
	CString strprt;
	BOOL         m_highRangeSet;
	//CETApplicationDoc  *pdocstore;
// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	void    UpdateAMPListDbhr(int index, int val);
	void    LoadResourceStr(HINSTANCE hins,BOOL mod);
	void    SavePTARecordIdx(int one,int two,int three);
	int    AnalysisPTARecord(int idx ,LPTSTR pstrbuff); 
	void    OnInitalDiaryList();
	BOOL    ReadDiaryFile(int id);
	CString    GetDiaryHead(char *pheadbuff, BOOL localinvoke = TRUE);
	void    GetTreatDiaryHeadStruct(char *pheadbuff , BOOL& nomatch);
	void    SetDefaultTreatSetting();
	void    GetSingleDiaryRecord(CString strtime,char *pbuff,int offset);
	CString GetDiaryTime(char *p);

	void  PTAQuickCalibrate();
	BOOL  JudgeLowLevelAMPset();
	int  IfCurrentSelectedBand(int side,int band);
	BOOL  CutPcFileToAudm();
	BOOL  TestIfUserExists(CString str);
	BOOL  ReadExistsIDinfo(CString str);
	BOOL  SetMuteSysVol(BOOL mute);

	int DownloadProgress();

	void SetPTAProgressInd(int LR, long frequency, double db);
	
protected:

	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	CString GetCorresponseMonth(CString mth);

	CWinThread *	    m_pPatientLinkingThread;

	BOOL BtnSelectBand(bool,int,int);

	void StartManualPTA();
	void GoToManualMod();
	void RemoveManualMod();
	void UpdateCtrlListData();
	void UpdateCtrlTabFromList(int idx = 0);
	void InitalFrezArray();
	void ReSetPTACtrlListLabel();
	void StoreDataInList();
	void DrawWaveTable(CDC* pDC);
	void PTAcharToUnic(CString &str);
	void ReSetAmpRightStat();
	BOOL GetMicPhoneStat();
	BOOL JudgeETFfileGen();
	void ReSetAmpLeftStat();
	void StartNameSearch();
	BOOL LoadAmpTreatHeadSetting();
	void RegisterNewPTAandIdx();
	void GeneratePCdiary(ETSessionID sessionID, int limitPlay = 9, int leftVol= 0, int rightVol=0); 
	BOOL CompareNoSameDayTreatBands(ETSessionID m_sessionID);
	void WriteSingleRecordinPcDiary(LPCSTR strFilename);
	bool GetAMPResultbyTable(ETSessionID session, ETAMPResult& result);
	IAudioEndpointVolume* SetMicPhoneStat(BOOL firststart = FALSE);
	void GetUpdateOfHighRange();
	void AdjustMicVol(UINT nchar, UINT rep);
	void DrawHighRangeLine(CDC* pDC, int nsideId);
	void AdjustHighRangeLine(CDC *pDC,int startidx, int nlen,UINT optType);
	void SetMainRect(int l,int t,int r,int b);
	void DrawBkLine(CDC *pDC);
	BOOL FindIfDiaryFile(CString& diarystr);
	void DisplayAMPElement(int x,int y,int len);
	void SetHighRangPara(int startidx, int nlen,UINT optType);
	virtual BOOL PreTranslateMessage(MSG* pMsg );
	bool DoRegisterDeviceInterfaceToHwnd(GUID InterfaceClassGuid, HWND hWnd, HDEVNOTIFY *hDeviceNotify);
	void CopyScreenToBitmap(int  xStartPt,int  yStartPt, int width, int height,int  xToCopy, int yToCopy);

	CEarButton *pRveLeft, *pRveRight;
	CEarSliderCtrl *pSliderLf, *pSliderRt;
	CEarStatic *pfontLf, *pfontRt;
	CSLPropertySheet *psheet;
	CRect    m_storelistrect;
	CDC      m_listdc;
	CString  strTime, strpauseTime ,strendTime;

	BOOL     m_manualmod;

	CString m_strId,m_strName,m_strAge,m_strGender,m_strDate;

	ETFrequencyRangeArray m_highRange[2];
	ETFrequencyArray      m_initalexchangearry;

	CDialog    *pdlgprogress;
	float  currmicrvol;
	CProgressCtrl *pctrlprogress;
	CString      m_lowfreqRTWarning,m_lowfreqLFWarning;
	//CNewList *plistL,*plistR;
	HDEVNOTIFY   m_hDeviceNotify;
	BOOL         m_audiomedpluged;
	BOOL         m_usbhidleved;
	
	BOOL         m_freqsetArry[2][HIGHFREQNUM];
	BOOL         m_keypressed[2];

	int          m_startidx;
	int          m_nlen[2];
	UINT         m_keytype;
	int          *pfrez[2];
	CString      strWarning;

	int          m_fontlen;

	TCHAR        m_buffarr[8][32];
	TCHAR        m_buffprtbotm[256];
	TCHAR        m_buffstrside[2][20];
	TCHAR        m_buffprtbotm2[256];
	TCHAR        m_buffstrname[10];
	TCHAR        m_buffstrage[10];
	TCHAR        m_buffstrid[10];
	TCHAR        m_buffstradd[10];
	TCHAR        m_buffstrgender[12];
	TCHAR        m_bufstrprompt[64];
	TCHAR        m_buffstrSysstat[20];
	TCHAR        m_buffstrpushbutt[20];
	TCHAR        m_buffstrMic[20];
	TCHAR        m_buffstrunlink[20];
	TCHAR        m_buffstrlinking[20];
	TCHAR        m_buffstrlinked[20];
	TCHAR        m_buffstroff[20];
	TCHAR        m_buffstron[20];
	TCHAR        m_buffassign[256];
	TCHAR        m_buffave[18];
	TCHAR        m_buffave1[18];
	TCHAR        m_buffave2[18];
	TCHAR        m_delta[18];
	TCHAR        m_buffcontrol[12];
	TCHAR        m_buffresume[12];
	TCHAR        m_buffcancel[12];
	TCHAR        m_buffapply[12];
	TCHAR        m_bufstopta[128];
	TCHAR        m_buffnopta[32];
	TCHAR        m_buffassfail[32];
	TCHAR        m_buffwarnid[32];
	TCHAR        m_buffstpgen[24];

	TCHAR		 m_buffNewUserSaved[64];

	TCHAR        m_buffTreatmentStop[128];
	TCHAR        m_buffTreatmentPlay[128];

	TCHAR        m_buffAMDlinked[32];
	TCHAR		 m_buffFreSelLimit[256];
	TCHAR		 m_buffProfileSuccess[128];
	TCHAR        m_buffPatData[24];
	TCHAR        m_buffStop[12];
	TCHAR        m_buffamptreat[32];

	TCHAR        m_buffManCancel[12];
	TCHAR        m_buffManFinish[12];

	TCHAR        m_buffAMPGood1[256];
	TCHAR        m_buffAMPGood2[256];
	TCHAR        m_buffAMPGood3[256];
	TCHAR        m_buffAMPBad1[256];
	TCHAR        m_buffAMPBad2[256];
	TCHAR        m_buffAMPBad3[256];

	//TCHAR        m_buffAMPAsk[128];
	TCHAR        m_buffTimer[12];
	
	TCHAR        m_buffWarning[12];

	TCHAR		 m_buffplaystop[32];

	TCHAR		 m_buffPTANoSaveWarning[256];
	TCHAR		 m_buffNoFinishWarning[256];

	TCHAR        m_buffgenfirst[128];
	TCHAR        m_buffcantloadmul[128];

	int          m_set_highside;
	int          m_PTArecordChecked[3];
	BOOL         m_grayHighLine[2];

	int          linewidth;
	IAudioEndpointVolume  *pmic;

	CFont *psysfont;
	LOGFONT font,storefont;
	AMPNODE     m_RtNode[2],m_LfNode[2];
	//BOOL   slidermoved;
	
	int  defposval;
	POINT defsliderPt;


	ETFrequencyArray m_FreArray;
	int m_LR;
	long m_frequency;
	double m_db_intermediate;

public:
	BOOL   btnDown;
	BOOL   IsAmp;
	int    operaID;
	BOOL   sliderpressed;
	int    m_RightCount;
	int    m_diaryarr[2][DIARYNUM];
	int    m_warncount;
	CString m_strwarnct;
	BOOL         m_ShowWarning;
	BOOL         m_narrowWarningRT, m_narrowWarningLF;

	 DWORD   dwStartTickTime,dwPauseTime,dwResumeTime, dwEndTime;

	 UINT_PTR  m_nTimer, m_nwarnTime;

	RECT  displayrect;

	virtual void OnInitialUpdate( );
	CRect   AutoAdjust(CRect currRect);
	CRect   m_MainDispRect;
	CEarButton*   pbutt[8];       //*pOn,*pBand_L,*pMod_R,*pStar_ETF,*pAuse_Lk,*pData_Copy,*pRt_Test,*pXxx_Back; CStableListCtrl
	CEarButton*   pHighRangeBtn[2];
	//CListCtrl*  plistctr[2];
	CStableListCtrl*  plistctr[2];
	CListCtrl*  pdiaryctl[2];
	CEditListCtrl*  pPatientList[2];
	
	float m_exchangex,m_exchangey, m_ex2,m_ey2,m_ex3,m_ey3;
	int xoffset,yoffset;
	long gap[2];
	long hightmpgap[2];
	double iscal, yiscal;
	int  m_pos;
	int  ampsideID;
	int  ptamanualID;
	BOOL m_treatstage; 

	BOOL m_micstat;
	BOOL m_manualcheckboxstat;

	void FillPTAResult();
	void PTAProgressIndication();
		
//	CFile fpta;
	CFile fReadPta;
	BOOL ReadPtaFile(BOOL  isloadT = FALSE);
	BOOL DelPTARecord(BOOL arrRec[],BOOL isloadT = FALSE);
	CString ReadSinglePtaDataRecord(UINT idx);
	CString ReadSingleTT_PtaDataRecord(UINT idx);
	int  GetPtaRecordNum();
	int  GetPtaTTRecordNum();
	BOOL WritePtaFile(BOOL saveT = FALSE);
//	BOOL WritePtaData(BOOL saveT = FALSE);        //CFile file);
	BOOL WritePtaData(CFile& fpta, BOOL saveT= FALSE );  
	CString strprompt;
// Implementation
public:
	virtual ~CETApplicationView();
	virtual void OnPrepareDC(CDC* pDC,CPrintInfo* pInfo = NULL );
	//virtual BOOL OnPreparePrinting(CPrintInfo* pInfo );
 
	bool				PerformLinking();
	bool				PerformLinkingToCU();
 
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions

protected:
	afx_msg void OnFilePrintPreview();
	afx_msg LRESULT OnProgressBarStep(WPARAM   wParam   ,LPARAM   lParam);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnButtonON();
	afx_msg void OnButtonBand();
	afx_msg void OnButtonMod();
	afx_msg void OnButtonStart();
	afx_msg void OnButtonxxx();
	afx_msg void OnButtonPrint();
	afx_msg void OnButtonPAUSE();
	afx_msg void OnButtonDATA();
	afx_msg void OnFirstBut();
	afx_msg void OnButtonHighRt();
	afx_msg void OnButtonHighLf();
	afx_msg void OnTimer( UINT time );
	afx_msg void OnMouseMove(UINT nFlags,CPoint point);
	afx_msg void OnVScroll(UINT nSBCode,UINT nPos,CScrollBar* pScrollBar );
	afx_msg void OnButtonMemberFn(UINT nID);
	afx_msg void OnButtonMemberFnLeft(UINT nID);
	afx_msg BOOL OnDeviceChange(UINT nEventType,DWORD dwData);
	afx_msg void OnKeyDown( UINT, UINT, UINT );
	afx_msg void OnKeyUp( UINT, UINT, UINT );
	//afx_msg HBRUSH OnCtlColor( CDC*, CWnd*, UINT );
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


UINT PatientLinkingThread( LPVOID p_pDialog );
UINT PatientLinkingToCUThread( LPVOID p_pDialog );

#ifndef _DEBUG  // debug version in ETApplicationView.cpp
inline CETApplicationDoc* CETApplicationView::GetDocument() const
   { return reinterpret_cast<CETApplicationDoc*>(m_pDocument); }
#endif

