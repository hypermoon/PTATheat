//-----------------------------------------------------------------------------
// File: ETApplicationView.cpp
//
// Desc: implementation of the CETApplicationView class
//
// Copyright (c) 2011 Eartronic Ltd. All rights reserved.
//-----------------------------------------------------------------------------


#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ETApplication.h"
#endif

#include  "MainFrm.h"

#include "SvLdPropertyPage.h"
#include "ETApplicationDoc.h"
#include "ETApplicationView.h"
#include "EarProgressDlg.h"
#include <Mmdeviceapi.h>
#include  <endpointvolume.h>
#include  <Audioclient.h>
#include  <Audiopolicy.h>
#include <dbt.h>

#include "BandSelDlg.h"
#include "TestSpeedDlg.h"
#include "DlgSelfTest.h"
#include "NameSearchDlg.h"
#include "SelectdiaryDlg.h"

#include "control.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_PTA_END_EVENT	     WM_USER+100
#define WM_PTA_PROGRESS_EVENT	 WM_USER+101
#define WM_DISPLAY_WELCOME		 WM_USER+102
#define WM_MEDIA_EVENT           WM_USER+103

#define   BKCOLOR                RGB(192,192,192)
#define   RT_RED_COLOR           RGB(210,20,20)
#define   LF_BLUE_COLOR          RGB(20,20,170)
#define   FIRS_COLUMN_WID        90
#define   AMP_MIN_THRESHOLD      25
#define   AMP_MAX_THRESHOLD      80
#define   NUM_DB_VOLUME          40
#define   PRGRESSSTEP_MSG        WM_USER+200            

BANDS_TYPE g_bands_type = BANDS_CLASSIC_11;
BOOL g_bPatientLinkingThreadRunning = FALSE;
bool g_bETGenerated = false;
bool g_bIsLocalPlaying = false;
bool g_bPTANeedSaved = false;

bool g_timerAndPause = false;

bool g_ptaUserResponse = false;
#define PTA_SYSTEM_CHECK_TIME 30
#define PLAYTIME_ONCE  5

int g_countDown ;
int g_leftVolume = 72;
int g_rightVolume = 72;

ETTestSpeed g_testSpeed = PTA_SPEED_NORMAL;

// CETApplicationView

IMPLEMENT_DYNCREATE(CETApplicationView, CView)

BEGIN_MESSAGE_MAP(CETApplicationView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CETApplicationView::OnFilePrintPreview)
	ON_BN_CLICKED(IDC_BUTTON_ON,OnButtonON)
	ON_BN_CLICKED(IDC_BUTTON_BAND,OnButtonBand)
	ON_BN_CLICKED(IDC_BUTTON_MOD,OnButtonMod)
	ON_BN_CLICKED(IDC_BUTTON_START,OnButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE,OnButtonPAUSE)
	ON_BN_CLICKED(IDC_BUTTON_DATA,OnButtonDATA)
	ON_BN_CLICKED(IDC_BUTTON_PRT,OnButtonPrint)
	ON_BN_CLICKED(IDC_BUTTON_XXX,OnButtonxxx)
	ON_BN_CLICKED(IDC_BUT_HIGH_RT,OnButtonHighRt)
	ON_BN_CLICKED(IDC_BUT_HIGH_LF,OnButtonHighLf)
	ON_MESSAGE(PRGRESSSTEP_MSG,OnProgressBarStep)
	ON_CONTROL_RANGE(BN_CLICKED , IDC_SUB_BUTT_RT, IDC_SUB_BUTT_RT+40, OnButtonMemberFn )
	ON_CONTROL_RANGE(BN_CLICKED , IDC_SUB_BUTT_LF, IDC_SUB_BUTT_LF+40, OnButtonMemberFnLeft )
	ON_WM_VSCROLL()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE( )
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_TIMER()
	ON_WM_DEVICECHANGE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
END_MESSAGE_MAP()

// CETApplicationView construction/destruction

/*
#ifdef _DEBUG
#pragma comment(lib,"ETCore//lib//ETCoreLib_debug.lib")
#elif
#pragma comment(lib,"ETCore//lib//ETCoreLib.lib")
#endif
*/

extern std::wstring s2ws(const std::string& s);
extern std::string ws2s(std::wstring& s);
extern HINSTANCE hResourceHandle;

void CETApplicationView::FillPTAResult()
{
	//CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	//CETApplicationDoc* pDoc =  (CETApplicationDoc*)pFrame->GetActiveDocument( );
	//CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    //	CETApplicationView *pView = (CETApplicationView *) pChild->GetActiveView();
    // CETApplicationDoc *pDoc = pdocstore;
	/*CETApplicationDoc *pDoc = GetDocument();
	 ASSERT_VALID(pDoc);if (!pDoc)return;*/

	//CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
	//CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->MDIGetActive();
	//CETApplicationDoc* pDocs =  (CETApplicationDoc*)pFrame->GetActiveDocument( );

	CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	FREZNODE node;
	CDocList *pdoclist;   //m_Lf_list[3], m_Rt_list[3];  m_manualmod
	BOOL retval = FALSE;


	ETPTAResult result;
	m_isPTATesting = false;

	if(m_manualmod)
	{
		retval = m_manualmod;
	}
	else
	{
	    retval = GetPTAResult(m_sessionID, result);
	}

	if(retval)  // GetPTAResult(m_sessionID, result))
	{

		std::wstring wsDisplay;
		std::string sDisplay;

		char sTMP[1024];
		WCHAR wsTMP[1024];
		memset(wsTMP, 0, sizeof(WCHAR)*1024);
		int cnt;

		//for (cnt=0; cnt <result.nLeftFrequencyNum; cnt++)
		for (cnt=0; cnt <pDoc->m_Lf_list[0].GetCount(); cnt++)
		{
			pdoclist = &pDoc->m_Lf_list[0];
			node  = pdoclist->GetAt(pdoclist->FindIndex(cnt));
            if(!m_manualmod)
			{
				memset(sTMP,0,sizeof(char)*1024);
				sprintf(sTMP, "%.1f",result.aLeftDiagList[cnt].ndBA );

				sDisplay = sTMP;
				wsDisplay = s2ws(sDisplay);

				plistctr[1]->SetItemText(0,cnt+1,wsDisplay.c_str());

				node.dbhl = result.aLeftDiagList[cnt].ndBA;
				pdoclist->SetAt(pdoclist->FindIndex(cnt),node);
			}
			else
			{
				CString strval;
				strval = plistctr[1]->GetItemText(0,cnt+1);
				node.dbhl  = _ttof(strval);
				pdoclist->SetAt(pdoclist->FindIndex(cnt),node);
				
				pRveLeft[cnt].ShowWindow(SW_HIDE);
				pRveLeft[cnt].EnableWindow(FALSE);
				pSliderLf[cnt].ShowWindow(SW_HIDE);
				pfontLf[cnt].ShowWindow(SW_HIDE);


			}

		}

		//for (cnt=0; cnt <result.nRightFrequencyNum; cnt++)
		for (cnt=0; cnt <pDoc->m_Rt_list[0].GetCount(); cnt++)
		{
			pdoclist = &pDoc->m_Rt_list[0];
			node  = pdoclist->GetAt(pdoclist->FindIndex(cnt));
			if(!m_manualmod)
			{
				memset(sTMP,0,sizeof(char)*1024);
				sprintf(sTMP, "%.1f",result.aRightDiagList[cnt].ndBA );

				sDisplay = sTMP;
				wsDisplay = s2ws(sDisplay);

				plistctr[0]->SetItemText(0,cnt+1,wsDisplay.c_str());
						
				//pdoclist = &pDoc->m_Rt_list[0];
			
				//node  = pdoclist->GetAt(pdoclist->FindIndex(cnt));
				node.dbhl = result.aRightDiagList[cnt].ndBA;
				pdoclist->SetAt(pdoclist->FindIndex(cnt),node);
			}
			else
			{
				CString strval;
				strval = plistctr[0]->GetItemText(0,cnt+1);
				node.dbhl  = _ttof(strval);
				pdoclist->SetAt(pdoclist->FindIndex(cnt),node);
				pRveRight[cnt].ShowWindow(SW_HIDE);
				pRveRight[cnt].EnableWindow(FALSE);
				pSliderRt[cnt].ShowWindow(SW_HIDE);
				pfontRt[cnt].ShowWindow(SW_HIDE);
			}

		}

		//__asm int 3
	//	MessageBox(_T("wori"));
		pbutt[2]->EnableWindow(TRUE);
	
		UpdateCtrlListData();
		g_bPTANeedSaved = true;

	}

	if(pbutt[3]->pressed)
	{
		pbutt[3]->ModifyStyle(BS_OWNERDRAW,0 );
		pbutt[3]->pressed = FALSE;
		pbutt[3]->SetWindowTextW(strButt[3]);  //_T("START"));
		pbutt[3]->Invalidate(true);
	}

	if(pbutt[4]->pressed)
	{
		pbutt[4]->ModifyStyle(BS_OWNERDRAW,0 );
		pbutt[4]->pressed = FALSE;
		pbutt[4]->Invalidate(true);
	}

	plistctr[0]->DrawSubItemText(0,-1,-1);
	plistctr[1]->DrawSubItemText(0,-1,-1);

//	pDoc->m_IsPTAtested  = TRUE;
	dwEndTime = GetTickCount();
	strendTime = strTime;

	//this->KillTimer(m_nTimer);
}


void CETApplicationView::PTAProgressIndication()
{
		std::wstring wsDisplay;
		std::string sDisplay;

		char sTMP[1024];
		WCHAR wsTMP[1024];

		memset(wsTMP, 0, sizeof(WCHAR)*1024);
		//int cnt;
		memset(sTMP,0,sizeof(char)*1024);
		sprintf(sTMP, "%.1f",m_db_intermediate );

		sDisplay = sTMP;
		wsDisplay = s2ws(sDisplay);

		for (int i=0; i<m_FreArray.nLeftFrequencyNum; i++)
		{
			if((m_frequency == 0)||(m_frequency == m_FreArray.aLeftFreqList[i]))
			{
				if(m_LR == 1)
				{
					plistctr[m_LR]->SetItemText(0,i+1,wsDisplay.c_str());
					plistctr[m_LR]->DrawSubItemText(TRUE,i+1,1);
				}
			}
		}

		for (int j=0; j<m_FreArray.nRightFrequencyNum; j++)
		{
			if((m_frequency == 0)||(m_frequency == m_FreArray.aRightFreqList[j]))
			{
				if(m_LR == 0)
				{
					plistctr[m_LR]->SetItemText(0,j+1,wsDisplay.c_str());
					plistctr[m_LR]->DrawSubItemText(TRUE,j+1,0);
				}
			}
		}

		RECT rect, rect1,rect2;
		plistctr[0]->GetWindowRect(&rect1); 
		plistctr[1]->GetWindowRect(&rect2); 
//		rect.right = rect.right*2;
		UnionRect (&rect, &rect1, &rect2);
		this->RedrawWindow(&rect);

	
}

// This is a sample for function registration to receive test result
void TestAccomplishmentIndication(bool result, PVOID pwin)
{
	CETApplicationView* pTestPTALibDlg = (CETApplicationView*)pwin;
	if (result)
	{
//		pTestPTALibDlg->FillPTAResult();
		pTestPTALibDlg->PostMessage(WM_PTA_END_EVENT);
	}
	else
	{
//		pTestPTALibDlg->MessageBox(L"PTA test is terminated");
//		pTestPTALibDlg->MessageBox(L"PTA test interupted");
	}
}

void NotifyWinRequest(void* pwin, void* pMediaEvent)
{
	CETApplicationView* pTestPTALibDlg = (CETApplicationView*)pwin;
	if(pMediaEvent != NULL)
	{
		IMediaEventEx* pIMediaEvent = (IMediaEventEx*)pMediaEvent;
		pIMediaEvent->SetNotifyWindow((OAHWND)(pTestPTALibDlg->GetSafeHwnd()), WM_MEDIA_EVENT, 0);
	}
}


void TestProgressIndication(int LR, long frequency, double db, PVOID pwin)
{
	CETApplicationView* pTestPTALibDlg = (CETApplicationView*)pwin;
	pTestPTALibDlg->SetPTAProgressInd(LR, frequency, db);
	pTestPTALibDlg->PostMessage(WM_PTA_PROGRESS_EVENT);

}


int CharToUnicode(char *pchIn, CString *pstrOut)
{
    int nLen;
    WCHAR *ptch;
    if(pchIn == NULL)
    {
        return 0;
    }
    nLen = MultiByteToWideChar(CP_ACP, 0, pchIn, -1, NULL, 0);
    ptch = new WCHAR[nLen];
    MultiByteToWideChar(CP_ACP, 0, pchIn, -1, ptch, nLen);
    pstrOut->Format(_T("%s"), ptch);
    delete [] ptch;
    return nLen;
}

CString  GetPTAResultLocalPath()
{
	WCHAR tmp_wstrPtaPath[376];
	WCHAR  syswDir[MAX_PATH];
	memset( syswDir, NULL, MAX_PATH*sizeof(WCHAR));
	GetSystemDirectory(syswDir, MAX_PATH);

	wsprintf(tmp_wstrPtaPath, L"%s\\pts\\", syswDir);
	return tmp_wstrPtaPath;

}


CETApplicationView::CETApplicationView()
{
	// TODO: add construction code here
	strprompt = _T("Pls Input ID ...");
	iscal = 1;
	yiscal = 1;
	xoffset = 0;
	yoffset = 0;
	gap[0] = 0;
	gap[1] = 0;
	pRveLeft = NULL;
	pRveRight =NULL;
	pSliderLf = NULL; 
	pSliderRt = NULL;
	m_pos  =0;
	m_exchangex = 0;
	m_exchangey = 0;
	m_ex2=0;
	m_ey2=0;
	linewidth = 0;
	ampsideID = 0;
	ptamanualID = 0;
	pfrez[0] = NULL;
	pfrez[1] = NULL;
	currmicrvol = 0;
	pmic = NULL;
	m_manualcheckboxstat = FALSE;
	m_LDPage_T_isChecked = FALSE;

	pHighRangeBtn[0] = NULL;
	pHighRangeBtn[1] = NULL;

	m_grayHighLine[0] = TRUE;
	m_grayHighLine[1] = TRUE;
	m_screenxLogPixPerInch = 96;
	m_screenyLogPixPerInch = 96;
	m_manualmod = FALSE;

	strdiarypcpath = _T("");
	m_audiomedpluged = FALSE;
	m_usbhidleved = FALSE;
	m_highRangeSet = FALSE;
	m_keypressed[0]  = FALSE;
	m_keypressed[1]  = FALSE;
	m_micstat = FALSE;
	m_set_highside = 0;
	m_nlen[0] = 0;
	m_nlen[1] = 0;
	m_keytype = 0;
	m_ShowWarning = FALSE;
	m_narrowWarningRT = FALSE;
	m_narrowWarningLF = FALSE;

	m_treatstage = FALSE;

	m_PTArecordChecked[0] = -1;
	m_PTArecordChecked[1] = -1;
	m_PTArecordChecked[2] = -1;

	m_LinkState = NOLINKED;
	m_pPatientLinkingThread =  NULL;
	strptapath = _T("");
	m_stridxpath = _T("");
	strTime = _T("");
	strpauseTime = _T("");
	strendTime = _T("");
	m_lowfreqLFWarning = _T("");
	m_lowfreqRTWarning = _T("");

	ptalistctltime[0] = _T("");
	ptalistctltime[1] = _T("");
	ptalistctltime[2] = _T("");

	
	memset(m_RtNode,0,sizeof(AMPNODE)*2);
	memset(m_LfNode,0,sizeof(AMPNODE)*2);

	memset(&m_highRange[0],0, sizeof(ETFrequencyRangeArray));
	memset(&m_highRange[1],0, sizeof(ETFrequencyRangeArray));
	memset(m_buffprtbotm,0,256*sizeof(TCHAR));
	memset(m_buffprtbotm2,0,256*sizeof(TCHAR));

	memset(m_buffstrname,0,10*sizeof(TCHAR));
	memset(m_buffstrage,0,10*sizeof(TCHAR));
	memset(m_buffstrid,0,10*sizeof(TCHAR));
	memset(m_buffstradd,0,10*sizeof(TCHAR));
	memset(m_buffstrgender,0,12*sizeof(TCHAR));

	memset(m_buffstrSysstat,0,20*sizeof(TCHAR));
	memset(m_buffstrpushbutt,0,20*sizeof(TCHAR));
	memset(m_buffstrMic,0,20*sizeof(TCHAR));
	memset(m_buffstrunlink,0,20*sizeof(TCHAR));
	memset(m_buffstrlinking,0,20*sizeof(TCHAR));
	memset(m_buffstrlinked,0,20*sizeof(TCHAR));
	memset(m_buffstroff,0,20*sizeof(TCHAR));
	memset(m_buffstron,0,20*sizeof(TCHAR));
	memset(m_buffassign,0,256*sizeof(TCHAR));
	memset(m_buffPatData,0,24*sizeof(TCHAR));
	memset(m_buffStop,0,12*sizeof(TCHAR));

	memset(m_buffManCancel,0,12*sizeof(TCHAR));
	memset(m_buffManFinish,0,12*sizeof(TCHAR));

	memset(m_buffplaystop,0,32*sizeof(TCHAR));

	memset(m_diaryarr[0],0,DIARYNUM * sizeof(int));
	memset(m_diaryarr[1],0,DIARYNUM * sizeof(int));

	memset(m_buffAMPGood1,0,sizeof(TCHAR)*256);
	memset(m_buffAMPGood2,0,sizeof(TCHAR)*256);
	memset(m_buffAMPGood3,0,sizeof(TCHAR)*256);

	memset(m_buffAMPBad1,0,sizeof(TCHAR)*256);
	memset(m_buffAMPBad2,0,sizeof(TCHAR)*256);
	memset(m_buffAMPBad3,0,sizeof(TCHAR)*256);

	//memset(m_buffAMPAsk,0,sizeof(TCHAR)*128);
	memset(m_buffTimer,0,sizeof(TCHAR)*12);

	memset(m_buffWarning,0,sizeof(TCHAR)*12);

	memset(m_buffPTANoSaveWarning,0,sizeof(TCHAR)*256);
	memset(m_buffNoFinishWarning,0,sizeof(TCHAR)*256);

	memset(m_buffgenfirst,0,sizeof(TCHAR)*128);

	memset(m_buffcantloadmul,0,sizeof(TCHAR)*128);

	memset(m_rcdvalarr[0],0,sizeof(int)*11);
	memset(m_rcdvalarr[1],0,sizeof(int)*11);
	

	g_bPTANeedSaved = false;
	m_warncount = 15 ;
	m_strwarnct = _T("15");
	
	defposval = 0;
	defsliderPt.x = 0;
	defsliderPt.y = 0;
	btnDown = FALSE;
	operaID = -2;
	IsAmp =  FALSE;
	sliderpressed = FALSE;
	m_RightCount = 0;
	m_diaryMod  = FALSE;

	 dwStartTickTime = 0;
	 dwPauseTime = 0;
	 dwResumeTime = 0;
	 dwEndTime =0;
	 strWarning = _T("");
	 m_fontlen = 1;
	 
	 m_nTimer = NULL;
	 m_nwarnTime = NULL;

	memset(m_freqsetArry[0],1,10*sizeof(BOOL));
	memset(m_freqsetArry[1],1,10*sizeof(BOOL));
	memset(m_buffarr,0,sizeof(TCHAR)*32*8);

	memset(m_buffstrside[0],0,sizeof(TCHAR)*20);
	memset(m_buffstrside[1],0,sizeof(TCHAR)*20);
	memset(m_bufstrprompt,0,sizeof(TCHAR)* 64);
	memset(m_buffave,0,sizeof(TCHAR)*18);
	memset(m_buffave1,0,sizeof(TCHAR)*18);
	memset(m_buffave2,0,sizeof(TCHAR)*18);
	memset(m_delta,0,sizeof(TCHAR)*18);

	memset(m_buffcontrol,0,sizeof(TCHAR)*12);
	memset(m_buffresume,0,sizeof(TCHAR)*12);

	memset(m_buffcancel,0,sizeof(TCHAR)*12);
	memset(m_buffapply,0,sizeof(TCHAR)*12);

	memset(m_bufstopta,0,sizeof(TCHAR)*128);
	memset(m_buffnopta,0,sizeof(TCHAR)*32);
	memset(m_buffassfail,0,sizeof(TCHAR)*32);
	memset(m_buffwarnid,0,sizeof(TCHAR)*32);
	memset(m_buffstpgen,0,sizeof(TCHAR)*24);
	memset(m_buffNewUserSaved,0,sizeof(TCHAR)*64);

	memset(m_buffTreatmentStop,0,sizeof(TCHAR)*128);
	memset(m_buffTreatmentPlay,0,sizeof(TCHAR)*128);

	memset(m_buffAMDlinked,0,sizeof(TCHAR)*32);
	memset(m_buffFreSelLimit,0,sizeof(TCHAR)*256);
	memset(m_buffProfileSuccess,0,sizeof(TCHAR)*128);
	memset(m_buffamptreat,0,sizeof(TCHAR)*32);

	m_AMP_List[0].RemoveAll();
	m_AMP_List[1].RemoveAll();

}

CETApplicationView::~CETApplicationView()
{
	if(pRveLeft != NULL)
	{
		delete [] pRveLeft;
		pRveLeft = NULL;

	}
	if(pRveRight != NULL)
	{
	    delete [] pRveRight; // plineNode
		pRveRight = NULL;
	}
	if(pfrez[0] != NULL)
	{
		delete [] pfrez[0];
		pfrez[0] = NULL;
	}
		if(pfrez[1] != NULL)
	{
		delete [] pfrez[1];
		pfrez[1] = NULL;
	}
}

void CETApplicationView::SetPTAProgressInd(int LR, long frequency, double db)
{ 
	m_LR=LR; 
	m_frequency=frequency; 
	m_db_intermediate=db; 
}

void CETApplicationView::OnButtonBand()
{
	//int frez[8] = {1000,1500,2000,3000,4000,6000,8000,12000};
	BOOL    presstat = FALSE;
	RECT fontrect;
	int frez = 0;
	int high = 0;

					    CMainFrame *pmnfm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
				    CRect rect;
                    pmnfm->GetWindowRect( rect );

					::SetWindowPos(pmnfm->GetSafeHwnd() ,       // handle to window
					HWND_TOPMOST,  // placement-order handle
					rect.left,     // horizontal position
					rect.top,      // vertical position
					rect.Width(),  // width
					rect.Height(), // height
					SWP_SHOWWINDOW); // window-positioning options

	if(IsAmp)
	{
		if(!pbutt[0]->pressed)
			return;

		AMPNODE ampnode;
		int idx, num = 0;

			if(!InitSound(m_sessionID, ET_RIGHT))
				MessageBox(_T("Right sound initialization failed"));
			else
			{
				m_leftright = ET_RIGHT;
//				FocusFreqPlaying(m_sessionID, m_leftright);
			
				int num = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;

				if(!pbutt[1]->pressed)               //to press button
				{
					
					ampsideID = 0;
					pbutt[1]->ModifyStyle(0,BS_OWNERDRAW );
					pbutt[1]->pressed = TRUE;
					if(pbutt[2]->pressed)
					{
						pbutt[2]->ModifyStyle(BS_OWNERDRAW,0 );
						pbutt[2]->pressed = FALSE;
						pbutt[2]->Invalidate();
						for(int s=0; s< num; s++)
						{
							pRveLeft[s].EnableWindow(FALSE);
							pSliderLf[s].ShowWindow(FALSE);
						}
					}

					 for(idx=0;idx < num; idx++)
					  {
						  ampnode = m_AMP_List[ampsideID].GetAt(m_AMP_List[ampsideID].FindIndex(idx));
						  if(ampnode.IsSet)
							  pSliderRt[idx].ShowWindow(TRUE);

					  }
					 //m_treatstage

					for(int i = 0 ;i < num; i++)  //pRveRight pRveLeft
						pRveRight[i].EnableWindow(TRUE);

					FocusFreqPlaying(m_sessionID, m_leftright);
				}
				else                                //reset button
				{
					  pbutt[1]->ModifyStyle(BS_OWNERDRAW,0 );
					  pbutt[1]->pressed = FALSE;
					  ///////////////////
					  ReSetAmpRightStat();
					  //////////////
					if(!JudgeETFfileGen() )
							pbutt[3]->EnableWindow(FALSE);
					else
						   pbutt[3]->EnableWindow(TRUE);

				}
			}

	}
	else   // PTA bands selection
	{
		if(!pbutt[0]->pressed)
			return;

		if(m_isPTATesting)
			return;

		m_highRangeSet = FALSE;

		//pHighRangeBtn[0]->EnableWindow(FALSE);
		pHighRangeBtn[0]->ModifyStyle(BS_OWNERDRAW,0);
		pHighRangeBtn[0]->pressed = FALSE;
		pHighRangeBtn[0]->Invalidate();

		//pHighRangeBtn[1]->EnableWindow(FALSE);
		pHighRangeBtn[1]->ModifyStyle(BS_OWNERDRAW,0);
		pHighRangeBtn[1]->pressed = FALSE;
		pHighRangeBtn[1]->Invalidate();

		CBandSelDlg bandSelDlg(g_bands_type);
		bandSelDlg.SetHStat(m_narrowWarningRT || m_narrowWarningLF);
		//bandSelDlg.GetCheckStat(m_manualcheckboxstat);
		if(IDOK == bandSelDlg.DoModal())
		{
			// Set frequency bands
			CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;
			pDoc->m_IsPTAtested = FALSE;                              //press ok in band means discard before PTA result to re-create new one.
			g_bPTANeedSaved = false;
			g_bands_type = bandSelDlg.getSelectedBands();
			OnInitalPTAMP();
			UpdateCtrlTabFromList();
			StoreDataInList();
			if(BANDS_RANGE_NARROWED == g_bands_type)   //reset high range line
			{
				memset(m_freqsetArry[0],1,10*sizeof(BOOL));
				memset(m_freqsetArry[1],1,10*sizeof(BOOL));
				
			}
			else
			{
				m_narrowWarningLF = FALSE;
			    m_narrowWarningRT = FALSE;
			}

		   CString idarr[3];
			idarr[0] = _T("1)");
			idarr[1] = _T("2)");
			idarr[2] = _T("3)");

		   ptalistctltime[0] = _T("");
		   ptalistctltime[1] = _T("");
		   ptalistctltime[2] = _T("");

		   plistctr[0]->SetItemText(0,0,idarr[0]+ptalistctltime[0]);
		    plistctr[0]->SetItemText(1,0,idarr[1]+ptalistctltime[1]);
			 plistctr[0]->SetItemText(2,0,idarr[2]+ptalistctltime[2]);

		   plistctr[1]->SetItemText(0,0,idarr[0]+ptalistctltime[0]);
		    plistctr[1]->SetItemText(1,0,idarr[1]+ptalistctltime[1]);
			 plistctr[1]->SetItemText(2,0,idarr[2]+ptalistctltime[2]);
//##################################################################
				// Initiated to allow restarting

				if(m_sessionID != NO_SESSION_ID)
					EndTestSession(m_sessionID);

				Sleep(500);
				m_sessionID = BeginTestSession(ET_PTA, ET_AUTOMATIC);
				if( m_sessionID == NO_SESSION_ID)
				{
#ifdef _DEBUG
					MessageBox(L"PTA Session can not be created");
#endif
					return;
				}
				SetPTASpeed(m_sessionID, g_testSpeed);
				RegisterTestCompleteIndication(m_sessionID, TestAccomplishmentIndication, this, TestProgressIndication);
//##################################################################

				//this->Invalidate();
				//SendMessage(WM_PAINT);


		/*	if(bandSelDlg.manualbandstat)
			{
				GoToManualMod();
				m_manualcheckboxstat = TRUE;
			}
			else
			{
				RemoveManualMod();
				m_manualcheckboxstat = FALSE;
			}*/

		
			   this->Invalidate();
		}
	}

}


void CETApplicationView::ReSetAmpRightStat()
{
	 BOOL    presstat = FALSE;
	 RECT fontrect;
	 int frez = 0;
	 int high = 0;
	 AMPNODE ampnode;
	 int idx=0;
	 int num = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;  
	 for(int i = 0 ;i < num; i++)    pRveRight[i].EnableWindow(FALSE);
  
	 num = m_AMP_List[0].GetCount();
       for(idx=0;idx < num; idx++)
       {
		  plistctr[0]->SetItemText(0,idx+1,_T(""));
          ampnode = m_AMP_List[0].GetAt(m_AMP_List[0].FindIndex(idx));
        if(ampnode.IsSet)
        {
             frez = ampnode.xband;
             ampnode.IsSet = FALSE;
             ampnode.ydbhr = 0;
             m_AMP_List[ampsideID].SetAt(m_AMP_List[0].FindIndex(idx),ampnode);
             pRveRight[idx].ModifyStyle(BS_OWNERDRAW,0 );
			 pRveRight[idx].pressed = FALSE;
			 pSliderRt[idx].SetPos(0);
             pSliderRt[idx].ShowWindow(SW_HIDE);
			 pfontRt[idx].ShowWindow(SW_HIDE);
			 BtnSelectBand(false,0, frez); //frez[idx]);   //(ctrlid+1)*1000);
			 pRveRight[idx].Invalidate();
			 presstat = FALSE;
       //  }

        }
           pfontRt[idx].GetWindowRect(&fontrect);
           pSliderRt[idx].m_dcBk = NULL;

///////////////////////////////////////////////////////////
//  Added to correct slide position problem
//////////////////////////////////////////////////////////
		   pSliderRt[idx].SetPos(10);
///////////////////////////////////////////////////////////

           ScreenToClient(&fontrect);
           high = fontrect.bottom - fontrect.top;
           fontrect.top = yoffset + 26*yiscal;
           fontrect.bottom  = fontrect.top + high;
//         pfontRt[idx].MoveWindow(fontrect.left,fontrect.top,fontrect.right - fontrect.left + 7*iscal, fontrect.bottom -fontrect.top);
		   pfontRt[idx].MoveWindow(fontrect.left,fontrect.top,m_fontlen, fontrect.bottom -fontrect.top);

           pfontRt[idx].SetWindowTextW(_T("0"));
           pfontRt[idx].ShowWindow(SW_HIDE);
       }

}


void CETApplicationView::ReSetAmpLeftStat()
{
     BOOL presstat = FALSE;
	 RECT fontrect;
	 int frez = 0;
	 int high = 0;
	 AMPNODE ampnode;
	 int idx=0;
 
	 int num = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;
	 for(int i = 0 ;i < num; i++) pRveLeft[i].EnableWindow(FALSE);
	 num = m_AMP_List[1].GetCount();
      
	 for(idx=0;idx < num; idx++)
	  {
		plistctr[1]->SetItemText(0,idx+1,_T(""));
		ampnode = m_AMP_List[1].GetAt(m_AMP_List[1].FindIndex(idx));
		if(ampnode.IsSet)
		{
		  ampnode.IsSet = FALSE;
		  ampnode.ydbhr = 0;
		  frez = ampnode.xband;
		  m_AMP_List[1].SetAt(m_AMP_List[1].FindIndex(idx),ampnode);
		 //if(1)
		 //{
      
		  pRveLeft[idx].ModifyStyle(BS_OWNERDRAW,0 );
		  pRveLeft[idx].pressed = FALSE;
		  pSliderLf[idx].SetPos(0);
		  pSliderLf[idx].ShowWindow(SW_HIDE);
		  pfontLf[idx].ShowWindow(SW_HIDE);

		  BtnSelectBand(false,1,frez);  // frez[idx]);   //(ctrlid+1)*1000);
		  pRveLeft[idx].Invalidate();
		  presstat = FALSE;
		 }

		  pfontLf[idx].GetWindowRect(&fontrect);
		  ScreenToClient(&fontrect);
		  high = fontrect.bottom - fontrect.top;
		  fontrect.top = yoffset + 26*yiscal;
		  fontrect.bottom  = fontrect.top + high;
//		  pfontLf[idx].MoveWindow(fontrect.left,fontrect.top,fontrect.right - fontrect.left + 7*iscal,fontrect.bottom -fontrect.top);
		  pfontLf[idx].MoveWindow(fontrect.left,fontrect.top,m_fontlen ,fontrect.bottom -fontrect.top);
		  pfontLf[idx].SetWindowTextW(_T("0"));
		  pfontLf[idx].ShowWindow(SW_HIDE);
		  pSliderLf[idx].m_dcBk = NULL;
		  ///////////////////////////////////////////////////////////
		  //  Added to correct slide position problem
		  //////////////////////////////////////////////////////////
				   pSliderLf[idx].SetPos(10);
		  ///////////////////////////////////////////////////////////

	 }                    //end for

}


void CETApplicationView::UpdateCtrlTabFromList(int idx)
{
	    int columnlen =0 ,i = 0,j =0;
		int nitem = 0;
		int num = 0, ctrnum =0 ;
		CRect AdjRect;
		CString str,strval;
		CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		FREZNODE nodeval;
		CDocList *pdoclist;
		
		CRect ListRect(0,384+70,670,384+158);  //683 (8)   128
	    AdjRect = AutoAdjust(ListRect);
		
		for(i=0; i < 2; i++)
		{
				if(m_LDPage_T_isChecked)
				{
//					plistctr[i]->SetItemText(0,0,_T("Average1:"));
//					plistctr[i]->SetItemText(1,0,_T("Average2:"));
//					plistctr[i]->SetItemText(2,0,_T("Total Delta:"));
					plistctr[i]->SetItemText(0,0,m_buffave1);
					plistctr[i]->SetItemText(1,0,m_buffave2);
					plistctr[i]->SetItemText(2,0,m_delta);
					plistctr[i]->SetItemText(3,0,_T(""));
				}

					pdoclist = i ? &(pDoc->m_Lf_list[idx]) : &(pDoc->m_Rt_list[idx]);
					num      = pdoclist->GetCount();
					ctrnum   = plistctr[i]->GetHeaderCtrl()->GetItemCount() -1 ;
					columnlen  = (AdjRect.Width() - FIRS_COLUMN_WID*iscal)/num;
				
					if(idx == 0)
					{
						for(j=0; j <ctrnum; j++)
						{
						   plistctr[i]->DeleteColumn(1);
						}
					}
					for(j=0; j<num ;j++)
					{
						nodeval = pdoclist->GetAt(pdoclist->FindIndex(j));
						str.Format(_T("%4.3f"),nodeval.band/1000);
						strval.Format(_T("%4.1f"),nodeval.dbhl);
						if(idx == 0)
						{
						plistctr[i]->InsertColumn(j+1,str);
						plistctr[i]->SetColumnWidth(j+1,columnlen);
						}
						
						plistctr[i]->SetItemText(idx,j+1,strval);  //_T("0.0"));

					}
			

		}


}
void CETApplicationView::OnMouseMove(UINT nFlags, CPoint point)
{
/*
	int l,r,t,b;
	

	ClientToScreen(&point);
	l = displayrect.left;
	r = displayrect.right;
	t = displayrect.top;
	b = displayrect.bottom;

	if(operaID != -2)
	{
		if(((l < point.x) && (point.x<r)) && ((t < point.y) &&( point.y< b)))
		{ 
			if(pRveLeft[operaID].pressed)
			   pSliderLf[operaID].ShowWindow(TRUE);
			
		}
		else
		{
			if((pSliderLf[operaID].currpos == pSliderLf[operaID].GetPos()) && !pSliderLf[operaID].slidermoved)
				pSliderLf[operaID].appeared = TRUE;

			//pSliderLf[operaID].ShowWindow(FALSE);
			   //slidermoved = FALSE;
				
			   //if(currpos != this->GetPos())
		       // clicked = TRUE;             //slider clicked,not movement.
		}
	}
	*/
	CWnd::OnMouseMove(nFlags,point);
	

}

void GenProgressInd(int progress)
{
	CETApplicationView *p = NULL;
	CMainFrame *pfrm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
    p =  pfrm->pm_save;

	if(progress == -1)
	{
		PostMessage(p->GetSafeHwnd(),PRGRESSSTEP_MSG,-1,0);
	}
	else
	{
		PostMessage(p->GetSafeHwnd(),PRGRESSSTEP_MSG,progress,0);
	}

}

BOOL  CETApplicationView::JudgeETFfileGen()
{
    BOOL ret = FALSE;
	int num = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;

	for(int i =0 ;i <num ;i ++)
	{
		if(plistctr[0]->GetItemText(0,i+1).Compare(_T("")))            //pfontRt[i].IsWindowVisible())
		{
			ret = TRUE;
			return ret;
		}
	}
	 num = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;

	for(int i =0 ;i <num ;i ++)
	{
		if(plistctr[1]->GetItemText(0,i+1).Compare(_T("")))   //pfontLf[i].IsWindowVisible())
		{
			ret = TRUE;
			return ret;
		}
	}
	return ret;
}


/*
BOOL CETApplicationView::JudgeLowLevelAMPset()
{
	BOOL ret = TRUE;
	int msgret = 0;
	int val = 0;
	CString str;
	
	CString standstr;
//	standstr.Format(_T("%d"),AMP_MIN_THRESHOLD);

//	standstr = _T(" Some AMP Value Setting is out of reasonable range \r\n (range indication:[") + standstr;
//	standstr = standstr + _T(" -- 90]dBr),do you want to continue?");

	CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		
	int num = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;

	for(int i =0 ;i <num ;i ++)
	{
		if(pfontRt[i].IsWindowVisible())
		{
			pfontRt[i].GetWindowTextW(str);
			val = _ttoi(str);
			if(val < AMP_MIN_THRESHOLD)
			{

				FREZNODE node = pDoc->m_Rt_list[0].GetAt(pDoc->m_Rt_list[0].FindIndex(i));
				CString LeftFreq;
				LeftFreq.Format(_T("%d"),long(node.band));

				standstr = m_buffAMPGood1 + LeftFreq ;//_T("There is no hearing loss on frequency point [") + LeftFreq;
				standstr = standstr + m_buffAMPGood2; //standstr + _T("Hz] for right ear, no need to set value for this frequency, do you want to stop generation and re-adjust value?");


				msgret = ::MessageBox(this->GetSafeHwnd(),
					             standstr, //_T("The AMP Value is out of reasonable range, do you want to continue?"),
								 _T("WARNING"),MB_YESNO);
			   if(msgret == IDYES)
			       ret = FALSE;
			   else
				   ret = TRUE;

			   return ret;
			}
		}
	}
	 num = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;

	for(int i =0 ;i <num ;i ++)
	{
		if(pfontLf[i].IsWindowVisible())
		{
			pfontLf[i].GetWindowTextW(str);
			val = _ttoi(str);
			if(val < AMP_MIN_THRESHOLD)
			{

				FREZNODE node = pDoc->m_Lf_list[0].GetAt(pDoc->m_Lf_list[0].FindIndex(i));

				CString RightFreq;
				RightFreq.Format(_T("%d"),long(node.band));

				standstr =  m_buffAMPGood1 + RightFreq;  //_T("There is no hearing loss on frequency point [") + RightFreq;
				standstr = standstr + _T("Hz] for left ear, no need to set value for this frequency, do you want to stop generation and re-adjust value?");

				msgret = ::MessageBox(this->GetSafeHwnd(),
					                  standstr, //_T("The AMP Value is out of reasonable range, do you want to continue?"),
									  _T("WARNING"),MB_YESNO);
			  if(msgret == IDYES)
			       ret = FALSE;
			   else
				   ret = TRUE;
				return ret;
			}
		}
	}
	return ret;

}
*/

BOOL CETApplicationView::JudgeLowLevelAMPset()
{
	BOOL ret = TRUE;
	int msgret = 0;
	int val = 0;
	CString str;
	CString standstr;
//	standstr.Format(_T("%d"),AMP_MIN_THRESHOLD);
//	standstr = _T(" Some AMP Value Setting is out of reasonable range \r\n (range indication:[") + standstr;
//	standstr = standstr + _T(" -- 90]dBr),do you want to continue?");
	BOOL low = FALSE,high = FALSE;
	//BOOL lflow = FALSE, lfhigh = FALSE;

	CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		
	int num = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;

	for(int i =0 ;i <num ;i ++)
	{   
		str = plistctr[0]->GetItemText(0,i+1);
		if(str.Compare(_T("")))       //pfontRt[i].IsWindowVisible())
		{
			//pfontRt[i].GetWindowTextW(str);
			val = _ttoi(str);
			if((val < AMP_MIN_THRESHOLD)|| (val > AMP_MAX_THRESHOLD))
			{

				FREZNODE node = pDoc->m_Rt_list[0].GetAt(pDoc->m_Rt_list[0].FindIndex(i));
				CString LeftFreq;
				LeftFreq.Format(_T("%d"),long(node.band));

				if(val < AMP_MIN_THRESHOLD)
				{
					standstr = m_buffAMPGood1 + LeftFreq;//_T("There is no hearing loss on frequency point [") + LeftFreq;
					standstr = standstr + m_buffAMPGood2;
					
					//standstr = standstr + m_buffAMPAsk;//_T("Hz] for right ear, no need to set value for this frequency, do you want to stop generation and re-adjust value?");
			
				   		   if(!low)
							 {
								msgret = ::MessageBox(this->GetSafeHwnd(),standstr, //_T("The AMP Value is out of reasonable range, do you want to continue?"),
													  m_buffWarning,MB_YESNO); //_T("WARNING"),MB_YESNO);
							 

								   if(msgret == IDYES)
								   {
									   ret = FALSE;
									   return ret;
								   }
								   else
								   {
									   ret = TRUE;
								   }
								   low = TRUE;
						      }

				}
				else
				{
					standstr = m_buffAMPBad1 + LeftFreq;
					standstr = standstr + m_buffAMPBad2;
						if(!high)
						{
						  msgret = ::MessageBox(this->GetSafeHwnd(),standstr, //_T("The AMP Value is out of reasonable range, do you want to continue?"),
												m_buffWarning,MB_YESNO); //_T("WARNING"),MB_YESNO);
						
						   if(msgret == IDYES)
						   {
							   ret = FALSE;
							   return ret;
						   }
						   else
						   {
							   ret = TRUE;
						   }

					      high = TRUE;
						}
					//standstr = standstr + m_buffAMPAsk;

				}



			   //return ret;
			}
		}
	}


	 num = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;
     
	for(int i =0 ;i <num ;i ++)
	{
		str = plistctr[1]->GetItemText(0,i+1);
		if(str.Compare(_T("")))
		{
			//pfontLf[i].GetWindowTextW(str);
			val = _ttoi(str);
			if((val < AMP_MIN_THRESHOLD)||(val > AMP_MAX_THRESHOLD))
			{

				FREZNODE node = pDoc->m_Lf_list[0].GetAt(pDoc->m_Lf_list[0].FindIndex(i));

				CString RightFreq;
				RightFreq.Format(_T("%d"),long(node.band));

				if(val < AMP_MIN_THRESHOLD)
				{
					standstr = m_buffAMPGood1 + RightFreq;  //_T("There is no hearing loss on frequency point [")
					standstr = standstr + m_buffAMPGood3;  //_T("Hz] for left ear, no need to set value for this frequency, do you want to stop generation and re-adjust value?");
				  //  standstr = standstr + m_buffAMPAsk;
					if(!low)
					{
						msgret = ::MessageBox(this->GetSafeHwnd(),
								   standstr, //_T("The AMP Value is out of reasonable range, do you want to continue?"),
								   m_buffWarning, MB_YESNO);  //_T("WARNING"),MB_YESNO);
						   if(msgret == IDYES)
						   {
							   ret = FALSE;
							   return ret;
						   }
						   else
						   {
							   ret = TRUE;
						   }
						   low = TRUE;
					}

				}
				else
				{
					standstr = m_buffAMPBad1 + RightFreq; 
					standstr = standstr + m_buffAMPBad3;  
			//		standstr = standstr + m_buffAMPAsk;
					if(!high)
					{
						msgret = ::MessageBox(this->GetSafeHwnd(),
								   standstr, //_T("The AMP Value is out of reasonable range, do you want to continue?"),
								   m_buffWarning, MB_YESNO);  //_T("WARNING"),MB_YESNO);
						   if(msgret == IDYES)
						   {
							   ret = FALSE;
							   return ret;
						   }
						   else
						   {
							   ret = TRUE;
						   }
						   high = TRUE;
					}

				}


			    //return ret;
			}
		}
	}
	return ret;

}

void CETApplicationView::WriteSingleRecordinPcDiary(LPCSTR strFilename)
{
		    std::string curTime;
			CTime theTime = CTime::GetCurrentTime();
			CString szCTime = theTime.Format("%Y-%m-%d ");
			std::wstring szwTime = szCTime.GetBuffer(0);
			curTime = ws2s(szwTime);

			char strside[11];
			memset(strside,0,11);

			int numR = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;
			CString str;
			int tot = 0;
			int c =0;
			for(int i=0;i<numR;i++)
			{
				str = plistctr[0]->GetItemText(0,i+1);
				if(str.Compare(_T("")))
				{
					c++;
					tot = _ttoi(str) + tot;
				}
			}
			int rvol = int(tot / c + 0.5);

			int numL = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;

		    tot = 0;
			c =0;
			for(int i=0;i<numL;i++)
			{
				str = plistctr[1]->GetItemText(0,i+1);
				if(str.Compare(_T("")))
				{
					c++;
				 	tot = _ttoi(str) + tot;
				}
			}
			int lvol = int(tot / c + 0.5);

			sprintf(strside,"L:%d R:%d ",lvol,rvol);

			char strtimes[25];
			memset(strtimes,0,25);

			sprintf(strtimes,"listen_n:00 require_n:09");

			char strreserve[84];
			memset(strreserve,0x20,84);

			strreserve[83] = '\0';
			strreserve[82] = '\n';
			strreserve[81] = '\r';


			FILE* pFile = fopen (strFilename,"ab"); //"e:\\diary.txt","ab");
			if(pFile!=NULL)
			{

				fwrite(curTime.c_str(),sizeof(char),11, pFile);
				fwrite(strside,sizeof(char),10, pFile);
				fwrite(strtimes,sizeof(char),24, pFile);
				fwrite(strreserve,sizeof(char),83,pFile);
				fclose (pFile);
			}

			//memset(strside,0,11);
}
void CETApplicationView::GeneratePCdiary(ETSessionID sessionID, int limitPlay, int leftVol, int rightVol)
{
	            std::string curDiaryTime;
				std::string absoluteFilename;         //-------------
				ETAMPResult amp;
				//bool pathResult = GetDiaryStorePathName(absoluteFilename);          //
				bool pathResult = TRUE;
			
				//absoluteFilename = "e:\\diary.txt";

				//-----------------------------------
				strdiarypcpath = GetPTAResultLocalPath();
				CString strfolder = pPatientList[0]->GetItemText(0,1);
	
				if(strfolder.CompareNoCase(strprompt) && strfolder.GetLength())  //no default str
					strdiarypcpath = strdiarypcpath + strfolder;
				else
				{
					MessageBox(strprompt);
					return;
				}
				strdiarypcpath = strdiarypcpath + _T("\\");
				strdiarypcpath = strdiarypcpath + _T("diary.txt");
				
				std::wstring szwdiary = strdiarypcpath.GetBuffer(0);
				absoluteFilename = ws2s(szwdiary);
				//-----------------------------------

				 CTime theTime = CTime::GetCurrentTime();
				 CString szCTime = theTime.Format("%Y-%m-%d_%H:%M");
				 std::wstring szwTime = szCTime.GetBuffer(0);
				 curDiaryTime = ws2s(szwTime);

				// LPSTR pstrwr ;
				// pstrwr = "e:\\diary.txt";

				 //WriteSingleRecordinPcDiary(pstrwr);

				if(pathResult)
				{
					bool ampResult = GetAMPResultbyTable(sessionID, amp);
					if(ampResult)
					{

						char DiaryDataHead[128];
						memset(DiaryDataHead, 0x20, 128);

						char* pStart = DiaryDataHead;
						sprintf(pStart, "\r\nAMPSTART");	
						pStart+=10;

						sprintf(pStart, "\r\n%s",curDiaryTime.c_str());	
						int nMove = curDiaryTime.length()+2;
						pStart+=nMove;

						sprintf(pStart, "\r\nRight");	  

						pStart+=7;

						int i;												
						for(i=0; i<amp.nResultFreqNum; i++)
						{
								 if(ET_RIGHT == amp.aResultValueList[i].leftRight)
								 {
									 sprintf (pStart,"\r\n%d, %d     ",amp.aResultValueList[i].nFrequency, amp.aResultValueList[i].ndBr);
									 pStart += 12;
								 }
						}
						sprintf (pStart, "\r\nLeft");
						pStart+=6;
																	 
						for(i=0; i<amp.nResultFreqNum; i++)
						{
								 if(ET_LEFT == amp.aResultValueList[i].leftRight)
								 {
									 sprintf (pStart,"\r\n%d, %d     ",amp.aResultValueList[i].nFrequency, amp.aResultValueList[i].ndBr);
									 pStart += 12;
								 }
						}
						sprintf (pStart, "\r\nDIARYSTART");	

						pStart += 12;
						*pStart = 0x20;

						DiaryDataHead[126] = '\r';
						DiaryDataHead[127] = '\n';

						FILE* pFile = fopen (absoluteFilename.c_str(),"ab");
						if(pFile!=NULL)
						{

							fwrite(DiaryDataHead,sizeof(char),128, pFile);
							fclose (pFile);
						}

					}                                       // ---------
				}


}

void CETApplicationView::OnButtonStart()
{
					    
	                CMainFrame *pmnfm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
				    CRect rect;
                    pmnfm->GetWindowRect( rect );

					::SetWindowPos(pmnfm->GetSafeHwnd() ,       // handle to window
					HWND_TOPMOST,  // placement-order handle
					rect.left,     // horizontal position
					rect.top,      // vertical position
					rect.Width(),  // width
					rect.Height(), // height
					SWP_SHOWWINDOW); // window-positioning options

 	if(IsAmp)
	{
		//WTL::CString strsource;
        
		CString strid;
		strid = pPatientList[0]->GetItemText(0,1);

		CString strdef =  m_bufstrprompt;//_T("Pls Input ID ...");
		if(strid.Compare(strdef) ==0 )
		{
			    AfxMessageBox(m_bufstrprompt); //L"Pls Input ID .");
			return ;
		}
		//WideCharToMultiByte(CP_OEMCP, NULL, strid ,-1, strDest.c_str(), strid.GetLength(), NULL, FALSE);
		//std::wstring strwDest = (LPCTSTR)strid;
		//strDest = ws2s(strwDest);
		//std::string strDest((LPCTSTR)strid);

		std::wstring strwWDest((LPCTSTR)strid);
		std::string  strDest;
		strDest =  ws2s(strwWDest);
		SetUserID(strDest);

		if(JudgeLowLevelAMPset() == FALSE)
			return ;

		// Calling ETF generation, will cost long time!!!!!!!!!!!!!!!!!!!!
		if(m_sessionID != NO_SESSION_ID)
		{
				RECT rect;
				int istep = 0;
				BOOL ret = FALSE;
				pdlgprogress = new CEarProgressDlg();
				pdlgprogress->Create(IDD_ProgressDlg,this);

				pdlgprogress->GetClientRect(&rect);

				pdlgprogress->MoveWindow( 683*iscal - (rect.right -rect.left)/2, 384 * yiscal - (rect.bottom - rect.top)/2,
					                      rect.right -rect.left,rect.bottom - rect.top);

				pctrlprogress = new CProgressCtrl();
				pctrlprogress->Create(WS_CHILD|WS_VISIBLE|WS_BORDER,rect,pdlgprogress,1680);
				pctrlprogress->SetRange(0,100);
				pctrlprogress->SetPos(0);
				pctrlprogress->SetStep(10);
				pdlgprogress->ShowWindow(SW_SHOW);
				//pctrlprogress->StepIt();
				pctrlprogress->Invalidate();
				pdlgprogress->Invalidate();

				pbutt[0]->EnableWindow(FALSE);
				pbutt[7]->EnableWindow(FALSE);
				pbutt[3]->EnableWindow(FALSE);
				pbutt[5]->EnableWindow(FALSE);
				ret = ETFGen(m_sessionID, true, GenProgressInd);  // start ETF generation 
				g_bETGenerated = true;
				g_bIsLocalPlaying = false;

				int nProgress =0;
				int retval = 0;

				if(!ret)
				{   
					
					g_bETGenerated = false;
					PostMessage(PRGRESSSTEP_MSG,100,0);
					AfxMessageBox(L"No dongle Error");
				}
				else
				{
					if(pPatientList[1]->GetItemText(0,3).Compare(m_buffstrlinked))
                       pbutt[5]->EnableWindow(FALSE);
					else
					   pbutt[5]->EnableWindow(TRUE);
					if(m_treatstage)
					{
						if(!CompareNoSameDayTreatBands(m_sessionID))
						{
			 				GeneratePCdiary(m_sessionID);
						}
					}
				}

				//m_treatstage = FALSE;
				//-----------------------------------------

		}
	}
	else // PTA
	{
		if(pbutt[3]->pressed)
		{
			pbutt[2]->EnableWindow(TRUE);
			if(!m_manualmod)
			{
					PauseAutomaticPTA(m_sessionID);
					m_PTAPaused = true;
					dwPauseTime = GetTickCount();
					strpauseTime = strTime;

					if(m_sessionID!= NO_SESSION_ID)
					{
						//if(IDYES != AfxMessageBox(L"Do you really want to stop PTA testing", MB_YESNO|MB_ICONSTOP))  //m_bufstopta
						if(IDYES != AfxMessageBox(m_bufstopta, MB_YESNO|MB_ICONSTOP))  //
						{

							dwResumeTime = GetTickCount();
							dwStartTickTime = dwStartTickTime + (dwResumeTime - dwPauseTime );
							ResumeAutomaticPTA(m_sessionID);
							m_PTAPaused = false;
							pbutt[4]->SetWindowTextW(strButt[4]);   //_T("PAUSE"));
							pbutt[4]->ModifyStyle(BS_OWNERDRAW,0 );
							pbutt[4]->pressed = FALSE;
							pbutt[4]->Invalidate(true);
							return;
						}

						StopAutomaticPTA(m_sessionID);
						pbutt[7]->EnableWindow(TRUE);

						/*
						dwEndTime = GetTickCount();
						strendTime = strTime;
						*/
						if(m_nTimer != NULL)
						{
							this->KillTimer(m_nTimer);		
							m_nTimer = NULL;
						}						
						strendTime = _T("");


					}
					m_PTAPaused = false;
					m_isPTATesting = false;
					pbutt[3]->ModifyStyle(BS_OWNERDRAW,0 );
					pbutt[3]->pressed = FALSE;
					pbutt[3]->SetWindowTextW(strButt[3] );//_T("START"));
					pbutt[3]->Invalidate(true);

					if(pbutt[4]->pressed)
					{
						pbutt[4]->SetWindowTextW(strButt[4]); //_T("PAUSE"));
						pbutt[4]->ModifyStyle(BS_OWNERDRAW,0 );
						pbutt[4]->pressed = FALSE;
						pbutt[4]->Invalidate(true);
					}

		//#####################################
						LNNODE nodes;
						int headnum =0;
						int ampnum = 0;
						for(int id = 0; id <2; id++)
						{
							headnum = plistctr[id]->GetHeaderCtrl()->GetItemCount() -1;
							//ampnum = m_AMP_List[id].GetCount();
							for(int row=0; row < 1;row ++)
							{
								for(int i=0 ; i < headnum; i++)
								{
									plistctr[id]->SetItemText(row,i+1,_T(" 0.0"));

									if(m_postL_list[row].GetCount()>0)
									{
										nodes = m_postL_list[row].GetAt(m_postL_list[row].FindIndex(i));
										nodes.y = yoffset + 26*yiscal;
										m_postL_list[row].SetAt(m_postL_list[row].FindIndex(i),nodes);
									}
									if(m_postR_list[row].GetCount() >0)
									{
										nodes = m_postR_list[row].GetAt(m_postR_list[row].FindIndex(i));
										nodes.y = yoffset + 26*yiscal;
										m_postR_list[row].SetAt(m_postR_list[row].FindIndex(i),nodes);
									}
							
								}
							}
						}

						plistctr[0]->DrawSubItemText(0,-1,-1);
						plistctr[1]->DrawSubItemText(0,-1,-1);

		//#####################################

					if(pbutt[0]->pressed)
					{
						// Initiated to allow restarting
						if(m_sessionID != NO_SESSION_ID)
							EndTestSession(m_sessionID);

						Sleep(500);
						m_sessionID = BeginTestSession(ET_PTA, ET_AUTOMATIC);
						if( m_sessionID == NO_SESSION_ID)
						{
		#ifdef _DEBUG
							MessageBox(L"PTA Session can not be created");
		#endif
							return;
						}
						SetPTASpeed(m_sessionID, g_testSpeed);
						RegisterTestCompleteIndication(m_sessionID, TestAccomplishmentIndication, this, TestProgressIndication);

					}
				}
				else               //manual mode, and pressed, going to cancel
				{


						if(IDYES == AfxMessageBox(m_bufstopta, MB_YESNO|MB_ICONSTOP))  //
						{
							RemoveManualMod();
							pbutt[1]->EnableWindow(TRUE);
					        m_isPTATesting = false;
							int numR = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;
							for(int i=0; i < numR; i++)
							{
								plistctr[0]->SetItemText(0,i+1,_T(""));
							}

							int numL = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;
							for(int i=0; i < numL; i++)
							{
								plistctr[1]->SetItemText(0,i+1,_T(""));
							}

						//	dwResumeTime = GetTickCount();
						//	dwStartTickTime = dwStartTickTime + (dwResumeTime - dwPauseTime );
					    //  m_PTAPaused = false;
							m_manualcheckboxstat = FALSE;
							pbutt[4]->SetWindowTextW(strButt[4]);   //_T("PAUSE"));
							pbutt[3]->SetWindowTextW(strButt[3]);
							pbutt[3]->ModifyStyle(BS_OWNERDRAW,0 );
							pbutt[3]->pressed = FALSE;
							pbutt[3]->Invalidate(true);
					        pbutt[7]->EnableWindow(TRUE);
						}

				}
		}
		else                       //not pressed start button
		{

			// Added to warn no PTA saving
			if(g_bPTANeedSaved)
			{
				if (IDYES != MessageBox( m_buffPTANoSaveWarning,//L"Do you want to leave latest PTA test result without being saved?",  //m_buffassign, 
								   L"Warning",MB_YESNO|MB_ICONWARNING))   
				{
					return;
				}
				g_bPTANeedSaved = false;
			}

			if(m_sessionID!= NO_SESSION_ID)
			{
						plistctr[0]->SetItemText(0,0,_T("1)"));
						plistctr[0]->SetItemText(1,0,_T("2)"));
						plistctr[0]->SetItemText(2,0,_T("3)"));

						plistctr[1]->SetItemText(0,0,_T("1)"));
						plistctr[1]->SetItemText(1,0,_T("2)"));
						plistctr[1]->SetItemText(2,0,_T("3)"));

						PTAQuickCalibrate();
	//					InitPTAFrequencyBands(m_sessionID); // ??????
						GetFrequencyBands(m_FreArray);
	
						strTime = _T("");
						if(m_nTimer != NULL)
							this->KillTimer(m_nTimer);
						m_nTimer = this->SetTimer(IDT_TIMER,1000,0); //IDT_TIMERWARN
						dwStartTickTime = GetTickCount();

						LNNODE nodes;
						int headnum =0;
						int ampnum = 0;
						for(int id = 0; id <2; id++)
						{
							headnum = plistctr[id]->GetHeaderCtrl()->GetItemCount() -1;
							//ampnum = m_AMP_List[id].GetCount();
							for(int row=0; row < 4;row ++)
							{
								for(int i=0 ; i < headnum; i++)
								{
									plistctr[id]->SetItemText(row,i+1,_T(""));

									if(m_postL_list[row].GetCount()>0)
									{
										nodes = m_postL_list[row].GetAt(m_postL_list[row].FindIndex(i));
										nodes.y = yoffset + 26*yiscal;
										m_postL_list[row].SetAt(m_postL_list[row].FindIndex(i),nodes);
									}
									if(m_postR_list[row].GetCount() >0)
									{
										nodes = m_postR_list[row].GetAt(m_postR_list[row].FindIndex(i));
										nodes.y = yoffset + 26*yiscal;
										m_postR_list[row].SetAt(m_postR_list[row].FindIndex(i),nodes);
									}
							
								}
							}
						}

						if(!m_manualmod)       //auto mode
						{
							pbutt[3]->ModifyStyle(0,BS_OWNERDRAW );
							pbutt[3]->pressed = TRUE;
							pbutt[3]->SetWindowTextW(m_buffStop); //_T("STOP"));
							pbutt[2]->EnableWindow(FALSE);
							pbutt[7]->EnableWindow(FALSE);
							this->Invalidate();
							//Sleep(2000);
							m_isPTATesting = true;
							m_PTAPaused = false;
							g_ptaUserResponse = false;
							g_timerAndPause = false;
							StartAutomaticPTA(m_sessionID);
						}
						else                   //manual mode
						{
							pbutt[3]->ModifyStyle(0,BS_OWNERDRAW );
							pbutt[3]->pressed = TRUE;
//							pbutt[3]->SetWindowTextW(_T("CANCEL"));
//							pbutt[4]->SetWindowTextW(_T("FINISH"));

							pbutt[3]->SetWindowTextW(m_buffManCancel);
							pbutt[4]->SetWindowTextW(m_buffManFinish);

							pbutt[2]->EnableWindow(FALSE);
							pbutt[7]->EnableWindow(FALSE);
							this->Invalidate();
							m_isPTATesting = true;
							m_PTAPaused = false;
							g_ptaUserResponse = false;
							StartManualPTA();
						}
			}
		}	
	}

}

bool CETApplicationView::GetAMPResultbyTable(ETSessionID sessionID, ETAMPResult& amp)
{
	CString str;
	int s=0;
	
	TCHAR strbuff[10];
	LVCOLUMN lvcolmn;
	lvcolmn.mask = LVCF_TEXT;
	lvcolmn.pszText = strbuff;
	lvcolmn.cchTextMax = 8;
	memset(strbuff,0,10*sizeof(TCHAR));

	if(sessionID != NO_SESSION_ID)
	{
		int numR = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;
		for(int i=0; i< numR; i++)
		{
			memset(strbuff,0,10*sizeof(TCHAR));
			str = plistctr[0]->GetItemText(0,i+1);
			if(str.Compare(_T("")) && str.GetLength() >= 1)  //have val
			{
				plistctr[0]->GetColumn(i+1,&lvcolmn);
				amp.aResultValueList[s].leftRight = ET_RIGHT;
				amp.aResultValueList[s].ndBr = _ttoi(str);
				amp.aResultValueList[s].nFrequency = _ttof(strbuff) * 1000;
				s++;
			}
		}

		int numL = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;
		for(int i=0; i< numL; i++)
		{
			memset(strbuff,0,10*sizeof(TCHAR));
			str = plistctr[1]->GetItemText(0,i+1);
			if(str.Compare(_T("")) && str.GetLength() >= 1)  //have val
			{
				plistctr[1]->GetColumn(i+1,&lvcolmn);
				amp.aResultValueList[s].leftRight = ET_LEFT;
				amp.aResultValueList[s].ndBr = _ttoi(str);
				amp.aResultValueList[s].nFrequency = _ttof(strbuff) * 1000;
				s++;
			}
		}

		amp.nResultFreqNum = s;
	}
	return TRUE;

}

BOOL CETApplicationView::CompareNoSameDayTreatBands(ETSessionID sessionID)
{
		ETAMPResult amp;
		BOOL rtEqual = FALSE, lfEqual = FALSE;
		int num = 0;
		int j =0,k =0;
		long rband[2] = {0,0};
		long lband[2] = {0,0};

		long curRtband[2] = {0,0};
		long curLfband[2] = {0,0};

        bool ampResult = GetAMPResultbyTable(sessionID, amp);
		 //m_RtNode
        //m_LfNode
		if(m_RtNode[0].IsSet)
			num++;
		if(m_RtNode[1].IsSet)
			num++;
		if(m_LfNode[0].IsSet)
		    num++;
		if(m_LfNode[1].IsSet)
		    num++;
		if(num == amp.nResultFreqNum)
		{
			rband[0] = m_RtNode[0].xband;
			rband[1] = m_RtNode[1].xband;
			lband[0] = m_LfNode[0].xband;
			lband[1] = m_LfNode[1].xband;
			for(int i=0; i<num;i++)
			{
				if(amp.aResultValueList[i].leftRight == ET_RIGHT )
				{
					curRtband[j++] = amp.aResultValueList[i].nFrequency;
				}
				if(amp.aResultValueList[i].leftRight == ET_LEFT )
				{
					curLfband[k++] = amp.aResultValueList[i].nFrequency;
				}
			}

			if((rband[0] == curRtband[0]) || (rband[0] == curRtband[1]))
			{
				if((rband[0] + rband[1]) == (curRtband[0] + curRtband[1]))
					rtEqual = TRUE;
			}

			if((lband[0] == curLfband[0]) || (lband[0] == curLfband[1]))
			{
				if((lband[0] + lband[1]) == (curLfband[0] + curLfband[1]))
					lfEqual = TRUE;
			}
			if(rtEqual && lfEqual)
				return TRUE;

		}
		else
		{
			return FALSE;
		}
		return FALSE;


}

void CETApplicationView::StartManualPTA()
{
	if(!pbutt[0]->pressed)
			return;

	m_leftright = ET_RIGHT;

	int numR = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;
	if(pfrez[0] != NULL)
	{
		delete [] pfrez[0];
		pfrez[0] = NULL;
	}
		pfrez[0] = new int[numR];
	for(int i=0;i < numR; i++)
	{
		pfrez[0][i] = m_FreArray.aRightFreqList[i];
		(pRveRight[i]).EnableWindow(TRUE);
		pSliderRt[i].EnableWindow(TRUE);
		//pSliderRt[i].ShowWindow(TRUE);
		//pSliderRt[i].SetRange(0,NUM_DB_VOLUME);
		//pfontRt[i].ShowWindow(TRUE);

	}

	int numL = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;
	if(pfrez[1] != NULL)
	{
		delete [] pfrez[1];
		pfrez[1] = NULL;
	}
	pfrez[1] = new int[numL];
	for(int i =0; i < numL; i++)
	{
		pfrez[1][i] = m_FreArray.aLeftFreqList[i];
		(pRveLeft[i]).EnableWindow(TRUE);
		pSliderLf[i].EnableWindow(TRUE);
		//pSliderLf[i].ShowWindow(TRUE);
		//pSliderLf[i].SetRange(0,NUM_DB_VOLUME);
		//pfontLf[i].ShowWindow(TRUE);
	}

	//m_sessionID = BeginTestSession(ET_PTA, ET_AUTOMATIC);
	//m_sessionID = BeginTestSession(ET_PTA, ET_AUTOMATIC); //ET_AMP, ET_MANUAL);
	//pfrez[id] = new int[num];
	
	//GetFrequencyBands(m_FreArray);

	//if(!InitSound(m_sessionID, ET_RIGHT))
	//	MessageBox(_T("Right sound initialization failed"));
	//else
	//{
	//	m_leftright = ET_RIGHT;
	//	FocusFreqPlaying(m_sessionID, m_leftright);
	// }

	/*if(!InitSound(m_sessionID, ET_LEFT))
			MessageBox(_T("Left sound initialization failed"));
		else
		{
			m_leftright = ET_LEFT;
			FocusFreqPlaying(m_sessionID, m_leftright);
		}*/

}
void CETApplicationView::OnInitalDiaryList()
{
		CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		FREZNODE node;
		//ETFrequencyArray freArray;
		int num = DIARYNUM;

		pDoc->m_Rt_list[0].RemoveAll();
		pDoc->m_Lf_list[0].RemoveAll();

		if((pDoc->m_Rt_list[0].GetCount() ==0) && 
			(pDoc->m_Lf_list[0].GetCount() ==0) )
		{
			for(int i = 0 ; i < num; i++)
			{
				if((i % 5) == 0 )
				{
				   node.band =  i * 1000;
				   if(i ==0)
					   node.band = -1000;
				}
				else 
				   node.band =  0; //freArray.aRightFreqList[i];
				
				node.dbhl = 0;
				pDoc->m_Rt_list[0].AddTail(node);
			}

			for(int i = 0 ; i < num; i++)
			{
                if((i % 5) == 0)
				{
					node.band = i*1000;
					if(i==0)
						node.band = -1000;
				}
				else
				    node.band = 0; //freArray.aLeftFreqList[i];

				node.dbhl = 0;
				pDoc->m_Lf_list[0].AddTail(node);
			}
	     }


	

}

void CETApplicationView::OnButtonPrint()
{
	if(IsAmp)
	{
		if(!pbutt[6]->pressed)         // going to press unpressed diary button!
		{
				int id =0 ,num = 0;
				int i = 0;
				BOOL ret ;
					
/////////////////////////////////////////////////////////////////////////
//    Added to suppress incorrect state of diary switching
/////////////////////////////////////////////////////////////////////////

					CString strid,strname,strage,strgender,strdate;
					CTime theTime = CTime::GetCurrentTime();
					CString szTime = theTime.Format(" %Y-%m-%d_%H:%M ");

					m_strId     = pPatientList[0]->GetItemText(0,1);
					m_strName   = pPatientList[0]->GetItemText(0,3);
					m_strAge    = pPatientList[0]->GetItemText(1,1);
					m_strGender = pPatientList[0]->GetItemText(1,3);
					m_strDate   = szTime;

					strid = _T("\r\n  "); 
					strid = strid + m_buffstrid;
					strid = strid + _T(":     ");

					strname = _T("\r\n  ");
					strname = strname + m_buffstrname;
					strname = strname + _T(":   ");

					strage = _T("\r\n  ");
					strage = strage + m_buffstrage;
					strage = strage + _T(":    ");

					strgender = _T("\r\n  ");
					strgender = strgender + m_buffstrgender;
					strgender = strgender + _T(": ");

					m_strId     = strid + m_strId;
					m_strName   = strname + m_strName;
					m_strAge    = strage + m_strAge;
					m_strGender = strgender + m_strGender;
					m_strDate   = _T("\r\n  DATE:   ") + m_strDate;

		//----------------------------------------------


			
					CString  diarystr;
					if(FindIfDiaryFile(diarystr) == FALSE)
					{
						plistctr[0]->ShowWindow(SW_SHOW);
						plistctr[1]->ShowWindow(SW_SHOW);
						return ;
					}
					else
					{
						
						pbutt[7]->EnableWindow(FALSE);
						pbutt[6]->pressed = TRUE;
						pbutt[6]->ModifyStyle(0,BS_OWNERDRAW);
						pbutt[6]->Invalidate();

					}
						
			
			//pbutt[6]->ModifyStyle(0,BS_OWNERDRAW );
			//pbutt[6]->pressed = TRUE;

			CSelectdiaryDlg  diarydlg(diarystr); //strdiarypcpath); // diarystr);  //strdiarypcpath); //strdiarypcpath

			if(IDOK == diarydlg.DoModal())
			{
				if(diarydlg.m_id != -1)
				{
						if(pbutt[0]->pressed /* pbutt[1]->pressed */)
						{
							    pbutt[0]->EnableWindow(FALSE);
								pbutt[1]->EnableWindow(FALSE);
								pbutt[2]->EnableWindow(FALSE);

								pbutt[1]->ModifyStyle(BS_OWNERDRAW,0 );
								pbutt[1]->pressed = FALSE;
								pbutt[5]->SetWindowTextW(strprt);   //_T("Print")); //strButt[6]);   //_T("Print"));
								pbutt[5]->EnableWindow(TRUE);
								ReSetAmpRightStat();

								if(!JudgeETFfileGen() )
		   						pbutt[3]->EnableWindow(FALSE);
								else
								pbutt[3]->EnableWindow(TRUE);

						}

						if(pbutt[0]->pressed  /* pbutt[2]->pressed */)
						{
							pbutt[2]->ModifyStyle(BS_OWNERDRAW,0 );
							pbutt[2]->pressed = FALSE;

								ReSetAmpLeftStat();

							if(!JudgeETFfileGen() )
								pbutt[3]->EnableWindow(FALSE);
							else
								pbutt[3]->EnableWindow(TRUE);

						}

						for(id =0 ; id<=1;id++)         //0right  1left   
						{
								num	= plistctr[id]->GetHeaderCtrl()->GetItemCount() -1;
								for(i=0; i < num; i++)
								{
									if(id==0)
									{
										pRveRight[i].ShowWindow(SW_HIDE);
										pSliderRt[i].ShowWindow(SW_HIDE);
										pfontRt[i].ShowWindow(SW_HIDE);
									}
									if(id == 1)
									{
										pRveLeft[i].ShowWindow(SW_HIDE);
										pSliderLf[i].ShowWindow(SW_HIDE);
										pfontLf[i].ShowWindow(SW_HIDE);

									}
								}
						}


							plistctr[0]->ShowWindow(SW_HIDE);
							plistctr[1]->ShowWindow(SW_HIDE);

							int s_id = diarydlg.m_id;
							OnInitalDiaryList();
							UpdateCtrlTabFromList();
							StoreDataInList();
							ReadDiaryFile(s_id);
						    m_diaryMod = TRUE;
				}
			}
			else
			{
				   // pbutt[5]->SetWindowTextW(_T("PLAYSTOP"));
					pbutt[7]->EnableWindow(TRUE);

				    pbutt[6]->pressed = FALSE;
					pbutt[6]->ModifyStyle(BS_OWNERDRAW,0);
					pbutt[6]->Invalidate();

			}

			this->Invalidate();
			
		}
		else
		{
			int id  = 0,num  =0;
			int i = 0; 
			pbutt[0]->EnableWindow(TRUE);
			pbutt[7]->EnableWindow(TRUE);

			pbutt[1]->EnableWindow(TRUE);
			pbutt[2]->EnableWindow(TRUE);
		
			pbutt[6]->ModifyStyle(BS_OWNERDRAW,0 );
			pbutt[6]->pressed = FALSE;

//			pbutt[5]->SetWindowTextW(_T("DownLoad"));
//			LoadResourceStr(hinst,IsAmp);

			// Judge and display
			
			if(m_treatstage)
				 pbutt[5]->SetWindowTextW(m_buffplaystop);
			else
			     pbutt[5]->SetWindowTextW(m_buffarr[5]);

			pdiaryctl[0]->ShowWindow(SW_HIDE);
			pdiaryctl[1]->ShowWindow(SW_HIDE);
			pdiaryctl[0]->DestroyWindow();
			pdiaryctl[1]->DestroyWindow();

//		    g_bands_type = BANDS_CLASSIC_11;
//			OnInitalPTAMP();

///////////////////////////////////////////////////////////////////////////

			CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;
			FREZNODE node;

			GetFrequencyBands(m_FreArray);

			if(!pDoc->m_IsPTAtested)
			{

				pDoc->m_Rt_list[0].RemoveAll();
				pDoc->m_Lf_list[0].RemoveAll();

				if((pDoc->m_Rt_list[0].GetCount() ==0) && (pDoc->m_Lf_list[0].GetCount() ==0) )
				{
					for(int i = 0 ; i < m_FreArray.nRightFrequencyNum; i++)
					{
						node.band = m_FreArray.aRightFreqList[i];
						node.dbhl = 0;
						pDoc->m_Rt_list[0].AddTail(node);
					}

					for(int i = 0 ; i < m_FreArray.nLeftFrequencyNum; i++)
					{
						node.band = m_FreArray.aLeftFreqList[i];
						node.dbhl = 0;
						pDoc->m_Lf_list[0].AddTail(node);
					}
				 }
			}

////////////////////////////////////////////////////////////////////////////////

			UpdateCtrlTabFromList();
			StoreDataInList();

			plistctr[0]->ShowWindow(SW_SHOW);
			plistctr[1]->ShowWindow(SW_SHOW);

			for(id =0 ; id<=1;id++)         //0right  1left   
		    {
		           num	= plistctr[id]->GetHeaderCtrl()->GetItemCount() -1;
                   for(i=0; i < num; i++)
				  {
					  if(id==0)
					  {
						  pRveRight[i].ShowWindow(SW_SHOW);
						  pSliderRt[i].ShowWindow(SW_HIDE);
						  pfontRt[i].ShowWindow(SW_HIDE);
					  }
					  if(id == 1)
					  {
						  pRveLeft[i].ShowWindow(SW_SHOW);
						  pSliderLf[i].ShowWindow(SW_HIDE);
						  pfontLf[i].ShowWindow(SW_HIDE);

					  }
					  plistctr[id]->SetItemText(0,i+1,_T(""));  //added to remove 0 as defect 12
				  }
			}

			m_diaryMod = FALSE;
			this->Invalidate();

		}
	}
	else                      //PTA
	{
		//m_strId,m_strName,m_strAge,m_strGender,m_strDate;
		/*if(!pbutt[6]->pressed)
		{
		        CRect rect;
                      GetWindowRect( rect );

					 ::SetWindowPos(this->GetSafeHwnd() ,       // handle to window
						HWND_NOTOPMOST,  // placement-order handle
						rect.left,     // horizontal position
						rect.top,      // vertical position
						rect.Width(),  // width
						rect.Height(), // height
						SWP_SHOWWINDOW); // window-positioning options
		}
		else
		{
				CRect rect;
                      GetWindowRect( rect );

					 ::SetWindowPos(this->GetSafeHwnd() ,       // handle to window
						HWND_TOPMOST,  // placement-order handle
						rect.left,     // horizontal position
						rect.top,      // vertical position
						rect.Width(),  // width
						rect.Height(), // height
						SWP_SHOWWINDOW); // window-positioning options

		}*/


		//--------------------------------------------------

		BOOL prestat = FALSE;
		prestat = m_highRangeSet;
		m_highRangeSet = FALSE;


		CString strid,strname,strage,strgender,strdate;
	    CTime theTime = CTime::GetCurrentTime();
	    CString szTime = theTime.Format(" %Y-%m-%d_%H:%M ");

		int width = ::GetSystemMetrics(SM_CXSCREEN);
		int height = ::GetSystemMetrics(SM_CYSCREEN);

		m_strId     = pPatientList[0]->GetItemText(0,1);
		m_strName   = pPatientList[0]->GetItemText(0,3);
		m_strAge    = pPatientList[0]->GetItemText(1,1);
		m_strGender = pPatientList[0]->GetItemText(1,3);
		m_strDate   = szTime;

		strid = _T("\r\n  "); 
		strid = strid + m_buffstrid;
		strid = strid + _T(":     ");

		strname = _T("\r\n  ");
		strname = strname + m_buffstrname;
		strname = strname + _T(":   ");

		strage = _T("\r\n  ");
		strage = strage + m_buffstrage;
		strage = strage + _T(":    ");

		strgender = _T("\r\n  ");
		strgender = strgender + m_buffstrgender;
		strgender = strgender + _T(": ");
	//	m_strId     = _T("\r\n  ID:     ") + m_strId;
	//	m_strName   = _T("\r\n  NAME:   ") + m_strName;
	//	m_strAge    = _T("\r\n  AGE:    ") + m_strAge;
	//	m_strGender = _T("\r\n  GENDER: ") + m_strGender;
	//	m_strDate   = _T("\r\n  DATE:   ") + m_strDate;



	    m_strId     = strid + m_strId;
		m_strName   = strname + m_strName;
		m_strAge    = strage + m_strAge;
		m_strGender = strgender + m_strGender;
		m_strDate   = _T("\r\n  DATE:   ") + m_strDate;

		CopyScreenToBitmap(0,0,width,height,0,0);
		Invalidate();

//		OnFilePrintPreview();
		            CMainFrame *pmnfm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
				    CRect rect;
                    pmnfm->GetWindowRect( rect );

					::SetWindowPos(pmnfm->GetSafeHwnd() ,       // handle to window
					HWND_NOTOPMOST,  // placement-order handle
					rect.left,     // horizontal position
					rect.top,      // vertical position
					rect.Width(),  // width
					rect.Height(), // height
					SWP_SHOWWINDOW); // window-positioning options

		OnFilePrint();
		
		m_listdc.Detach();

		m_highRangeSet = prestat;


	}
}

BOOL CETApplicationView::FindIfDiaryFile(CString& diarystr)
{
	/*BOOL fret = FALSE, result = TRUE;
	CFile freadiary;
	CString strdtime;
	CString strbeforetime = _T("");
	char *pbuff = NULL;
	char *psession = NULL;
	char *phead = NULL;
	char *psinglerow = NULL;
	char buffhead[256];
	char onerecordbuff[128];
	int headsize = 0;
	int rowlen = 0;
	int timenodenum = 0;*/
	BOOL fret = FALSE, result = TRUE;
	CFile freadiary;
	BOOL ret = TRUE;

	std::string strday ,stroldday;
	std::wstring strwday;

	std::string absoluteFilename;
	std::wstring absolutewFilename;

	CString strpath;

	if(LINKED == m_LinkState)
	{
		 result = GetDiaryStorePathName(absoluteFilename);
		 int cnt = 0;
		 while (!result&&(cnt<10)){
			result = GetDiaryStorePathName(absoluteFilename);
			Sleep(100);
			cnt++;
		}
		absolutewFilename = s2ws(absoluteFilename);
	}

	if(NOLINKED == m_LinkState)
	{
		if(m_treatstage)
		{
			absolutewFilename = strdiarypcpath.GetBuffer(0);
		}
	}

	strpath = absolutewFilename.c_str();

	if(result)
	{
			fret = freadiary.Open(strpath,CFile::modeRead); //CFile::modeReadWrite);
			if(fret == 0)
			{
				MessageBox(_T("No diary founded!"));
				ret = FALSE;
				return ret;
			}
			else
			{
				diarystr = strpath;
			}
	}

	return ret ; 

}
BOOL  CETApplicationView::ReadDiaryFile(int s_id)
{
	//m_diaryarr   //DIARYNUM  //linewidth  //gap[id] = linewidth /num;
	BOOL fret = FALSE, result = TRUE;
	CFile freadiary;
	//CString strpath = _T("C:\\diary.txt");
	CString strdtime;
	CString strbeforetime = _T("");
	int count = 0;
	char *pbuff = NULL;
	char *psession = NULL;
	char *phead = NULL;
	char *psinglerow = NULL;
	char buffhead[256];
	char onerecordbuff[128];
	int headsize = 0;
	int rowlen = 0;
	int timenodenum = 0;
	std::string strday ,stroldday;
	std::wstring strwday;

	std::string absoluteFilename;
	std::wstring absolutewFilename;

	CString strpath;

	m_d_TimeList.RemoveAll();
	//m_postR_list[0].RemoveAll();
	//m_postL_list[0].RemoveAll();

	memset(buffhead,0,256*sizeof(char));
	memset(onerecordbuff,0,128*sizeof(char));

	if(LINKED == m_LinkState)
	{
		 result = GetDiaryStorePathName(absoluteFilename);
		 int cnt = 0;
		 while (!result&&(cnt<10)){
			result = GetDiaryStorePathName(absoluteFilename);
			Sleep(100);
			cnt++;
		}
		absolutewFilename = s2ws(absoluteFilename);
	}

	if(NOLINKED == m_LinkState)
	{
		if(m_treatstage)
		{
			absolutewFilename = strdiarypcpath.GetBuffer(0);
		}
	}

	strpath = absolutewFilename.c_str();

   
	if(result)
	{
			fret = freadiary.Open(strpath,CFile::modeRead); //CFile::modeReadWrite);
			if(fret == 0)
			{
				MessageBox(_T("No diary founded!"));
				return -1;
			}
	}
		
	ASSERT(freadiary);
	//ULONGLONG len = freadiary.GetLength();
	int  len = freadiary.GetLength();

	pbuff = new char[len];
	memset(pbuff,0,len * sizeof(char));
	int s=0;
	if(len)
	{
		
		freadiary.SeekToBegin();
		s = freadiary.Read(pbuff,len);
	}
	
	while(psession = strstr(pbuff,"AMPSTART"))
	{
		pbuff = psession;
		pbuff += 8;
		if(count != s_id)
		{
			count++;
		}
		else
		{
			break;
		}
	}

	phead = strstr(pbuff,"DIARYSTART");
	headsize = phead - pbuff;
	strncpy(buffhead,pbuff,headsize);

	GetDiaryHead(buffhead);

	//phead = phead + 10 + 2;      //diarystart + \r\n
	//phead = strstr(phead,"TS");
	char *pheadend = NULL;

	pheadend = strchr(phead,'\n');
	if ((*(pheadend +1)) != '2')            //no record
	{
	    freadiary.Close();
		return 0;
	}
	else
		phead = strstr(phead,"2");   //locate front of str "20xx" 

	char *pnext = NULL;
	int rcdnum = 0;
	pnext = strstr(phead,"AMPSTART");
	if(pnext != NULL)
	{  
		rcdnum = (pnext - phead)/128;
	}
	/*else
	{
		freadiary.Close();
		return 0;

	}*/

	if(phead == NULL)  //freadiary.Close();
	{
		freadiary.Close();
		return 0;
	}
	else
	{
		while(psinglerow = strstr(phead,"listen"))//strchr(phead,'\r'))
		{
			memset(onerecordbuff,0,128*sizeof(char));
			rowlen = psinglerow - phead;
			if(rowlen == 0)
				break;
			strncpy(onerecordbuff,phead,rowlen);
			//...............
			strdtime = GetDiaryTime(onerecordbuff);
			if(strdtime.CompareNoCase(strbeforetime))  //diff day strday ,stroldday;
			{
				int nodeNumToAdd=1;
				//int offset  = strdtime - strbeforetime;   //library  GetDateGap
				strwday = strbeforetime;
				stroldday = ws2s(strwday);

				strwday = strdtime;
				strday  = ws2s(strwday);

				nodeNumToAdd = GetDateGap(stroldday,strday);

				GetSingleDiaryRecord(strdtime,onerecordbuff,nodeNumToAdd);
				strbeforetime = strdtime;
			}
			else                                      //same day
			{
				; //strbeforetime = strdtime;
			}
			//...............
			int gaplen = 0;
			gaplen = strstr(psinglerow,"\r\n") - psinglerow;

			phead = psinglerow;
			phead = phead + gaplen + 2;                  //jump \r\n
			rcdnum--;
			if(rcdnum == 0)
				break;
		}
	}

	timenodenum = m_d_TimeList.GetCount();
	for(int j=timenodenum; j< DIARYNUM ; j++)
	{
		CString strempty = _T("");
		m_d_TimeList.AddTail(strempty);
	}

	freadiary.Close();
	return 0;
}

void  CETApplicationView::GetTreatDiaryHeadStruct(char *pheadbuff , BOOL& nomatch)
{
	AMPNODE Rtnode[2],Lfnode[2], *pampnode;
	BOOL Regualwarning = TRUE;
	CString strln;
	CString strband,strdbr;
	char *pright = NULL;
	char *plnbuff = NULL;
	char linebuff[32];
	int lnsize = 0;
	int retval = 0;
	int band=0,dbr=0;
	int i =0;

	Rtnode[0].xband = 0;
	Rtnode[0].ydbhr = 0;
	Rtnode[0].IsSet = FALSE;

	Rtnode[1].xband = 0;
	Rtnode[1].ydbhr = 0;
	Rtnode[1].IsSet = FALSE;

	Lfnode[0].xband = 0;
	Lfnode[0].ydbhr = 0;
	Lfnode[0].IsSet = FALSE;

	Lfnode[1].xband = 0;
	Lfnode[1].ydbhr = 0;
	Lfnode[1].IsSet = FALSE;

	pright = strstr(pheadbuff,"Right");
	pright = pright + 7;

	pampnode = &Rtnode[0];
	while(plnbuff = strchr(pright,'\r'))
	{
		memset(linebuff,0,32*sizeof(char));
		lnsize = plnbuff - pright;
		strncpy(linebuff,pright,lnsize);
		strln = linebuff;
		strln.Trim();
		retval = strln.Find(',');
		if(retval != -1)
		{  
			LPTSTR pstr;
			strband = strln.Left(retval);
			pstr = strband.GetBuffer(strband.GetLength());
			band = _ttoi(pstr);
			pampnode->xband = band;

			strdbr = strln.Right(strln.GetLength() - retval - 1);
			strdbr.Trim();
			pstr = strdbr.GetBuffer(strdbr.GetLength());
			dbr  = _ttoi(pstr);
			pampnode->ydbhr = dbr;
			pampnode->IsSet = TRUE;
			pampnode++;
		}
		else   //left
		{
			pampnode = &Lfnode[0];
		}

		pright = plnbuff + 2;     ///jump \r\n

	}

	m_RtNode[0] = Rtnode[0];
	m_RtNode[1] = Rtnode[1];

	m_LfNode[0] = Lfnode[0];
	m_LfNode[1] = Lfnode[1];

	//pRveLeft pRveRight;
	AMPNODE  ampnode; 
	CRect fontrect;
	int high = 0;
	int offsetlen = 0;
	int sliderlen = 100;

	if(m_RtNode[0].IsSet)
	{
		int band = m_RtNode[0].xband;
		int retval = IfCurrentSelectedBand(0,band);
		if(retval != -1)             //exists in current band
		{
			CString strdbr;
			int dbr = m_RtNode[0].ydbhr;
			strdbr.Format(_T("%d"),dbr);
			plistctr[0]->SetItemText(0,retval+1,strdbr);
			//pfontRt[retval].SetWindowTextW(_T("11"));
		   pfontRt[retval].GetWindowRect(&fontrect);
		   ScreenToClient(&fontrect);
		   offsetlen = ((26 * 10) * (dbr+10) * yiscal) /sliderlen;  //100;      sliderlen  =100
           high = fontrect.bottom - fontrect.top;
		   fontrect.top = yoffset + offsetlen ; 
		   fontrect.bottom  = fontrect.top + high;
		   pfontRt[retval].MoveWindow(fontrect.left,fontrect.top,fontrect.right - fontrect.left,fontrect.bottom -fontrect.top);
		   pfontRt[retval].SetWindowTextW(strdbr);
		   pfontRt[retval].ShowWindow(TRUE);

			pSliderRt[retval].SetPos(dbr+10);
			pRveRight[retval].pressed = TRUE;
			pRveRight[retval].ModifyStyle(0,BS_OWNERDRAW );
			Regualwarning = FALSE;
			ampnode = m_AMP_List[0].GetAt(m_AMP_List[0].FindIndex(retval));
			ampnode.IsSet = TRUE;
			m_AMP_List[0].SetAt(m_AMP_List[0].FindIndex(retval),ampnode);
		}
	}
	if(m_RtNode[1].IsSet)
	{
		int band = m_RtNode[1].xband;
		int retval = IfCurrentSelectedBand(0,band);
		if(retval != -1)             //exists in current band
		{
			CString strdbr;
			int dbr = m_RtNode[1].ydbhr;
			strdbr.Format(_T("%d"),dbr);
		
			plistctr[0]->SetItemText(0,retval+1,strdbr);
			pSliderRt[retval].SetPos(dbr+10);
			pRveRight[retval].pressed = TRUE;
			pRveRight[retval].ModifyStyle(0,BS_OWNERDRAW );
			Regualwarning = FALSE;
			pfontRt[retval].GetWindowRect(&fontrect);
		   ScreenToClient(&fontrect);
		   offsetlen = ((26 * 10) * (dbr+10) * yiscal) /sliderlen;  //100;      sliderlen  =100
           high = fontrect.bottom - fontrect.top;
		   fontrect.top = yoffset + offsetlen ; 
		   fontrect.bottom  = fontrect.top + high;
		   pfontRt[retval].MoveWindow(fontrect.left,fontrect.top,fontrect.right - fontrect.left,fontrect.bottom -fontrect.top);
		   pfontRt[retval].SetWindowTextW(strdbr);
		   pfontRt[retval].ShowWindow(TRUE);

			ampnode = m_AMP_List[0].GetAt(m_AMP_List[0].FindIndex(retval));
			ampnode.IsSet = TRUE;
			m_AMP_List[0].SetAt(m_AMP_List[0].FindIndex(retval),ampnode);
		}
	}

    if(m_LfNode[0].IsSet)
	{
		int band = m_LfNode[0].xband;
		int retval = IfCurrentSelectedBand(1,band);
		if(retval != -1)             //exists in current band
		{
			CString strdbr;
			int dbr = m_LfNode[0].ydbhr;
			strdbr.Format(_T("%d"),dbr);
			//pfontLf[retval].SetWindowTextW(_T(""));
		   pfontLf[retval].GetWindowRect(&fontrect);
		   ScreenToClient(&fontrect);
		   offsetlen = ((26 * 10) * (dbr+10) * yiscal) /sliderlen;  //100;      sliderlen  =100
           high = fontrect.bottom - fontrect.top;
		   fontrect.top = yoffset + offsetlen ; 
		   fontrect.bottom  = fontrect.top + high;
		   pfontLf[retval].MoveWindow(fontrect.left,fontrect.top,fontrect.right - fontrect.left,fontrect.bottom -fontrect.top);
		   pfontLf[retval].SetWindowTextW(strdbr);
		   pfontLf[retval].ShowWindow(TRUE);

			plistctr[1]->SetItemText(0,retval+1,strdbr);
			pSliderLf[retval].SetPos(dbr+10);
			pRveLeft[retval].pressed = TRUE;
			pRveLeft[retval].ModifyStyle(0,BS_OWNERDRAW );
			Regualwarning = FALSE;
			ampnode = m_AMP_List[1].GetAt(m_AMP_List[1].FindIndex(retval));
			ampnode.IsSet = TRUE;
			m_AMP_List[1].SetAt(m_AMP_List[1].FindIndex(retval),ampnode);
		}
	}

	if(m_LfNode[1].IsSet)
	{
		int band = m_LfNode[1].xband;
		int retval = IfCurrentSelectedBand(1,band);
		if(retval != -1)             //exists in current band
		{
			CString strdbr;
			int dbr = m_LfNode[1].ydbhr;
			strdbr.Format(_T("%d"),dbr);
			plistctr[1]->SetItemText(0,retval+1,strdbr);
			//pfontLf[retval].SetWindowTextW(_T(""));
		   pfontLf[retval].GetWindowRect(&fontrect);
		   ScreenToClient(&fontrect);
		   offsetlen = ((26 * 10) * (dbr+10) * yiscal) /sliderlen;  //100;      sliderlen  =100
           high = fontrect.bottom - fontrect.top;
		   fontrect.top = yoffset + offsetlen ; 
		   fontrect.bottom  = fontrect.top + high;
		   pfontLf[retval].MoveWindow(fontrect.left,fontrect.top,fontrect.right - fontrect.left,fontrect.bottom -fontrect.top);
		   pfontLf[retval].SetWindowTextW(strdbr);
		   pfontLf[retval].ShowWindow(TRUE);

			pSliderLf[retval].SetPos(dbr+10);
			pRveLeft[retval].pressed = TRUE;
			pRveLeft[retval].ModifyStyle(0,BS_OWNERDRAW );
			Regualwarning = FALSE;
			ampnode = m_AMP_List[1].GetAt(m_AMP_List[1].FindIndex(retval));
			ampnode.IsSet = TRUE;
			m_AMP_List[1].SetAt(m_AMP_List[1].FindIndex(retval),ampnode);
		}
	}

	if(Regualwarning)
	{
		nomatch = TRUE;
		MessageBox(_T("Treatment band not fall in current bands,please reload corresponse PTA or reset!"));
	}

}

int CETApplicationView::IfCurrentSelectedBand(int side, int band)
{
	ETFrequencyArray freArray;
	freArray = m_FreArray;
	int s = 0;
	if(side ==0)
	{
		for(s=0; s< freArray.nRightFrequencyNum;s++)
		{
			if(band ==  freArray.aRightFreqList[s])
			{
				return s;
			}
		}
	}
	if(side == 1)
	{
		for(s=0; s< freArray.nLeftFrequencyNum;s++)
		{
			if(band ==  freArray.aLeftFreqList[s])
			{
				return s;
			}
		}

	}
	return -1;
}

CString CETApplicationView::GetDiaryHead(char *pheadbuff,BOOL localinvoke )
{
	AMPNODE Rtnode[2],Lfnode[2], *pampnode;
	CString strln;
	CString strband,strdbr;
	CString strtimediy;
	char *pright = NULL;
	char *plnbuff = NULL;
	char linebuff[32];
	char diarynamestr[22];
	int lnsize = 0;
	int retval = 0;
	int band=0,dbr=0;
	int i =0;

	Rtnode[0].xband = 0;
	Rtnode[0].ydbhr = 0;
	Rtnode[0].IsSet = FALSE;

	Rtnode[1].xband = 0;
	Rtnode[1].ydbhr = 0;
	Rtnode[1].IsSet = FALSE;

	Lfnode[0].xband = 0;
	Lfnode[0].ydbhr = 0;
	Lfnode[0].IsSet = FALSE;

	Lfnode[1].xband = 0;
	Lfnode[1].ydbhr = 0;
	Lfnode[1].IsSet = FALSE;

	memset(diarynamestr,0,22);
	
	pright = strstr(pheadbuff,"Right");

	strtimediy = strncpy(diarynamestr,pheadbuff, (pright - pheadbuff));
	strtimediy.Trim(_T("\r\n"));
		
    if(!localinvoke)
		 return strtimediy;

	pright = pright + 7;

	pampnode = &Rtnode[0];
	while(plnbuff = strchr(pright,'\r'))
	{
		memset(linebuff,0,32*sizeof(char));
		lnsize = plnbuff - pright;
		strncpy(linebuff,pright,lnsize);
		strln = linebuff;
		strln.Trim();
		retval = strln.Find(',');
		if(retval != -1)
		{  
			LPTSTR pstr;
			strband = strln.Left(retval);
			pstr = strband.GetBuffer(strband.GetLength());
			band = _ttoi(pstr);
			pampnode->xband = band;

			strdbr = strln.Right(strln.GetLength() - retval - 1);
			strdbr.Trim();
			pstr = strdbr.GetBuffer(strdbr.GetLength());
			dbr  = _ttoi(pstr);
			pampnode->ydbhr = dbr;
			pampnode->IsSet = TRUE;
			pampnode++;
		}
		else   //left
		{
			pampnode = &Lfnode[0];
		}

		pright = plnbuff + 2;     ///jump \r\n

	}
	

	m_RtNode[0] = Rtnode[0];
	m_RtNode[1] = Rtnode[1];

	m_LfNode[0] = Lfnode[0];
	m_LfNode[1] = Lfnode[1];


	
	//-------------------------------------------------------------
	
			CRect posRect;
			for(i=0;i<2;i++)    //m_Lf_list[3] 1,  m_Rt_list[3]; 0
			{
					pdiaryctl[i] = new CListCtrl();

					posRect.left   =  6*iscal*(i+1) + i*(m_storelistrect.Width()+ 4*iscal);
					posRect.right  =  posRect.left + m_storelistrect.Width();
					posRect.top    =  m_storelistrect.top;
					posRect.bottom =  posRect.top + m_storelistrect.Height();

					pdiaryctl[i]->Create(WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT,posRect, this, 550+i);
					pdiaryctl[i]->SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_AUTOSIZECOLUMNS );
					pdiaryctl[i]->InsertColumn(0,_T("AMP Setting"));
					pdiaryctl[i]->SetColumnWidth(0,FIRS_COLUMN_WID*iscal);

					if(i == 0)
					{ 
						CString str, strdbr[2];
						int band =0,dbr[2] = {0,0};

						for(int j=0;j <2; j++)
						{
							band = m_RtNode[j].xband;
							dbr[j] = m_RtNode[j].ydbhr;
							str.Format(_T("%d"),band);
							strdbr[j].Format(_T("%d"),dbr[j]);
							pdiaryctl[i]->InsertColumn(j+1,str);
							pdiaryctl[i]->SetColumnWidth(j+1,(posRect.Width()-FIRS_COLUMN_WID*iscal)/2);

						}
						pdiaryctl[i]->InsertItem(0,_T("dBr"));
						pdiaryctl[i]->SetItemText(0,1,strdbr[0]);
						pdiaryctl[i]->SetItemText(0,2,strdbr[1]);
					}
					if(i == 1)
					{ 
						CString str, strdbr[2];
						int band =0,dbr[2] = {0,0};
						for(int j=0;j <2; j++)
						{
							band = m_LfNode[j].xband;
							dbr[j] = m_LfNode[j].ydbhr;
							str.Format(_T("%d"),band);
							strdbr[j].Format(_T("%d"),dbr[j]);
							pdiaryctl[i]->InsertColumn(j+1,str);
							pdiaryctl[i]->SetColumnWidth(j+1,(posRect.Width()-FIRS_COLUMN_WID*iscal)/2);
						}
						pdiaryctl[i]->InsertItem(0,_T("dBr"));
						pdiaryctl[i]->SetItemText(0,1,strdbr[0]);
						pdiaryctl[i]->SetItemText(0,2,strdbr[1]);
					}
					pdiaryctl[i]->SetBkColor(BKCOLOR);
					pdiaryctl[i]->SetTextBkColor(BKCOLOR);

			}
	//-------------------------------------------------------------

	
	return strtimediy;
}

void CETApplicationView::GetSingleDiaryRecord(CString strtime,char *pbuff,int offset)
{
	//m_postL_list[3],m_postR_list[3];
	LNNODE node, nodeleft;
	int ncount = 0;
	int i = 0;
	int xright =0 ,xleft =0;
	char buffRt[8],buffLf[8];
	char *psave = pbuff;
	char *pstrRtside=NULL, *pstrLfside=NULL;
	CString  strR,strL, strExchange;
	CString  strempty = _T("");
	int right=0,left=0;
	LPTSTR  ptstr;
	float yPosVal = 0;

	gap[0] = linewidth / DIARYNUM;
	gap[1] = linewidth / DIARYNUM;

	memset(buffRt,0,8);
	memset(buffLf,0,8);

	//yPosVal = plineNode[i].y * 2.6 * yiscal;
	//yPosVal = yoffset + 26*yiscal+ yPosVal;   //0 is base line
	if(offset ==0 )
		offset = 1;

	for(i = 1; i<= offset; i++)
	{
		ncount = m_d_TimeList.GetCount();
		if(ncount >= 30 )
		{
			return ;
		}

        xright = xoffset + (ncount+1) * gap[0];
	    xleft  = xoffset + (ncount+1) * gap[1] + 683*iscal;

		node.x = xright;
		nodeleft.x = xleft;

		pbuff = NULL;

		if(i == offset)
		{
			pbuff = psave;
		}

		if(pbuff == NULL)
		{
			node.y =  yoffset + 26*yiscal+ 0;
			nodeleft.y  = yoffset + 26*yiscal+ 0;

			if(m_d_TimeList.GetCount() !=0)
			   m_d_TimeList.AddTail(strempty);

			m_postR_list[0].SetAt(m_postR_list[0].FindIndex(ncount), node);
			m_postL_list[0].SetAt(m_postL_list[0].FindIndex(ncount), nodeleft);

		}
		else
		{
			m_d_TimeList.AddTail(strtime);

			pstrRtside = strstr(pbuff,"L:");
			pstrRtside = pstrRtside + 2;

			pstrLfside = strstr(pstrRtside,"R:");
			strR = strncpy(buffRt,pstrRtside,(pstrLfside - pstrRtside));
			strL = pstrLfside + 2;

			strR.Trim();
			strL.Trim();

			strExchange = strR;
			strR = strL;
			strL = strExchange;            //because diary format changed, exchange var here

			ptstr = strR.GetBuffer(strR.GetLength());
			right = _ttoi(ptstr);
			yPosVal = right * 2.6 * yiscal;
			yPosVal = yoffset + 26*yiscal+ yPosVal;

			node.y = yPosVal;
			

			m_postR_list[0].SetAt(m_postR_list[0].FindIndex(ncount), node);

			ptstr = strL.GetBuffer(strL.GetLength());
			left = _ttoi(ptstr);
			yPosVal = left * 2.6 * yiscal;
			yPosVal = yoffset + 26*yiscal+ yPosVal;
			nodeleft.y = yPosVal;

			m_postL_list[0].SetAt(m_postL_list[0].FindIndex(ncount), nodeleft);
		}
	}


}

CString CETApplicationView::GetDiaryTime(char *p)
{
	 CString retstr;
	 char *pmin = NULL;
	 char bufftime[16];
	 int  len = 0;
	 //p = p + 3;

	 memset(bufftime,0,16);
	 pmin = strchr(p,' ');
	 len = pmin - p;
	 strncpy(bufftime,p,len);
	// bufftime[16] = '\0';
	 retstr = bufftime;
	 retstr.Trim();
	 return retstr;

}
void CETApplicationView::InitalFrezArray()
{
	//plistctr
    AMPNODE ampnode;
	int num = 0, i = 0,id =0 ;
	CString str;
	//TCHAR strbuff[20];
	_tcscpy(m_buffcontrol,_T("Hz"));

	LVCOLUMN lvcolmn;
	lvcolmn.mask = LVCF_TEXT;
	lvcolmn.pszText = m_buffcontrol; //_T("Controller"); // strbuff;
	lvcolmn.cchTextMax = 20;

	//memset(strbuff,0,20*sizeof(TCHAR));

    for(id = 0 ; id <2; id++)
	{
			num = m_AMP_List[id].GetCount();
			if(pfrez[id])
				delete [] pfrez[id];
			pfrez[id] = new int[num];
			//plistctr[id]->SetItemText(0,0,_T("Hz"));
			//plistctr[id]->SetItemText(1,0,_T("dBr"));
			plistctr[id]->SetItemText(0,0,_T("")); //dBr"));
			plistctr[id]->SetItemText(1,0,_T(""));
			plistctr[id]->SetItemText(2,0,_T(""));
			plistctr[id]->SetColumn(0,&lvcolmn);

		for(i =0 ;i < num; i++)
		{
			ampnode = m_AMP_List[id].GetAt(m_AMP_List[id].FindIndex(i));
			pfrez[id][i] = ampnode.xband + 0.5;
			str.Format(_T("%d"),ampnode.ydbhr);
			//plistctr[id]->SetItemText(1,i+1,str);
			plistctr[id]->SetItemText(0,i+1,_T(""));

		}
	}
}

void CETApplicationView::ReSetPTACtrlListLabel()
{
	//AMPNODE ampnode;
	int num = 0, i = 0,id =0 ;

	LVCOLUMN lvcolmn;
	lvcolmn.mask = LVCF_TEXT;
	lvcolmn.pszText = _T("[kHz]"); // strbuff;
	lvcolmn.cchTextMax = 20;

    for(id = 0 ; id <2; id++)
	{
		num = m_AMP_List[id].GetCount();
		pfrez[id] = new int[num];
		plistctr[id]->SetItemText(0,0,_T("1)"));  //PTA1
		plistctr[id]->SetItemText(1,0,_T("2)"));  //PTA2
		plistctr[id]->SetItemText(2,0,_T("3)"));  //PTA3
		plistctr[id]->SetColumn(0,&lvcolmn);

		for(i =0 ;i < num; i++)
		{
			;                                     //ampnode = m_AMP_List[id].GetAt(m_AMP_List[id].FindIndex(i));
	
		}
	}


}
void CETApplicationView::OnButtonxxx()
{

   HINSTANCE hinst = ::AfxGetInstanceHandle();
  if(hResourceHandle != NULL)
		 hinst = hResourceHandle;

	if(m_nTimer != NULL)
	{
		this->KillTimer(m_nTimer);		
		m_nTimer = NULL;
	}						
	strendTime = _T("");

				    CMainFrame *pmnfm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
				    CRect rect;
                    pmnfm->GetWindowRect( rect );

					::SetWindowPos(pmnfm->GetSafeHwnd() ,       // handle to window
					HWND_TOPMOST,  // placement-order handle
					rect.left,     // horizontal position
					rect.top,      // vertical position
					rect.Width(),  // width
					rect.Height(), // height
					SWP_SHOWWINDOW); // window-positioning options

	if(IsAmp)
	{
		// End  AMP before moving to PTA
		//InitalFrezArray();

		  ReSetPTACtrlListLabel();
		  m_LDPage_T_isChecked = FALSE;

		  pbutt[1]->EnableWindow(FALSE);
		  pbutt[2]->EnableWindow(FALSE);
		  pbutt[3]->EnableWindow(FALSE);
		  pbutt[4]->EnableWindow(FALSE);
		  pbutt[5]->EnableWindow(FALSE);
		  pbutt[6]->EnableWindow(FALSE);

		  pHighRangeBtn[0]->ShowWindow(SW_SHOW);
		  pHighRangeBtn[1]->ShowWindow(SW_SHOW);

		  pHighRangeBtn[0]->EnableWindow(TRUE);
		  pHighRangeBtn[1]->EnableWindow(TRUE);


		if(m_sessionID != NO_SESSION_ID)
		{
		     EndTestSession(m_sessionID);
			 m_sessionID =  NO_SESSION_ID;
		}

		   IsAmp = FALSE;
	
			int i = 0;
			for(i=1; i<8; i++)
			{
				pbutt[i]->pressed = FALSE;
				pbutt[i]->ModifyStyle(BS_OWNERDRAW,0 );
			}
			if(pbutt[0]->pressed)
			{
				m_sessionID = BeginTestSession(ET_PTA, ET_AUTOMATIC);
				if( m_sessionID == NO_SESSION_ID)
					MessageBox((LPCTSTR)"PTA Session can not be created");
			
				SetPTASpeed(m_sessionID, g_testSpeed);
				RegisterTestCompleteIndication(m_sessionID, TestAccomplishmentIndication, this, TestProgressIndication);
				pbutt[1]->EnableWindow(TRUE);
				pbutt[2]->EnableWindow(TRUE);
				pbutt[3]->EnableWindow(TRUE);
				pbutt[4]->EnableWindow(TRUE);
				pbutt[5]->EnableWindow(TRUE);
				pbutt[6]->EnableWindow(TRUE);
	//			   pbutt[7]->EnableWindow(FALSE);

			}



		ReSetAmpRightStat();
		ReSetAmpLeftStat();

		int numR = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;
		plistctr[0]->SetItemText(3,0,m_buffave);// _T("Average"));
		for(int i=0; i < numR; i++)
		{
			(pRveRight[i]).ShowWindow(FALSE);
		}

		int numL = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;
		plistctr[1]->SetItemText(3,0,m_buffave); //_T("Average"));
		for(int i=0; i < numL; i++)
		{
			(pRveLeft[i]).ShowWindow(FALSE);
		}

		Invalidate();

			    /*
					int numR = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;
					//num -= 2;
					for(int i=0;i < numR; i++)
					{
						   //*pRveLeft, *pRveRight;
						  (pRveRight[i]).ShowWindow(FALSE);
						  (pSliderRt[i]).ShowWindow(FALSE);
						  (pfontRt[i]).ShowWindow(FALSE);
					}
					int numL = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;
					for(int i =0;i < numL ; i++)
					{
							(pRveLeft[i]).ShowWindow(FALSE);
							(pSliderLf[i]).ShowWindow(FALSE);
							(pfontLf[i]).ShowWindow(FALSE);

					}
					*/


				/*for(int id =0 ;id < 2;id++)
				  {
					  int num  =  m_AMP_List[id].GetCount();
					  for(i =0 ;i < num; i++)
					  {
						  if(id == 0)
						  {
							  pRveRight[i].ModifyStyle(BS_OWNERDRAW,0);
							  pRveRight[i].Invalidate();
							  pRveRight[i].EnableWindow(FALSE);
							  pSliderRt[i].ShowWindow(SW_HIDE);
							  pfontRt[i].ShowWindow(SW_HIDE);

						  }
						  if(id == 1)
						  {
							  pRveLeft[i].ModifyStyle(BS_OWNERDRAW,0);
							  pRveLeft[i].Invalidate();
							  pRveLeft[i].EnableWindow(FALSE);
							  pSliderLf[i].ShowWindow(SW_HIDE);
							  pfontLf[i].ShowWindow(SW_HIDE);

						  }
						  AMPNODE node = m_AMP_List[id].GetAt(m_AMP_List[id].FindIndex(i));
						  node.IsSet = FALSE;
						  node.ydbhr = 0;
						  m_AMP_List[id].SetAt(m_AMP_List[id].FindIndex(i),node);
						  // m_AMP_List[id].RemoveAll();

					  }
				  }
				*/
		



				//for(int j =0 ;j <8;j++)
				/*pbutt[0]->SetWindowTextW(_T("ON"));
				pbutt[1]->SetWindowTextW(_T("BAND"));
				pbutt[2]->SetWindowTextW(_T("TEST SPEED"));
				pbutt[3]->SetWindowTextW(_T("START"));

				pbutt[4]->SetWindowTextW(_T("PAUSE"));
				pbutt[5]->SetWindowTextW(_T("DATA"));
				pbutt[6]->SetWindowTextW(_T("PRINT"));
				pbutt[7]->SetWindowTextW(_T("GO TO"));*/

			    //LoadResourceStr(hinst,IsAmp);

	}
	else
	{       // End  PTA before moving to AMP

			// Added to warn no PTA saving
			if((pbutt[0]->pressed)&&(g_bPTANeedSaved))
			{
				if (IDYES != MessageBox( m_buffPTANoSaveWarning, // L"Do you want to leave latest PTA test result without being saved?",  //m_buffassign, 
									     L"Warning",MB_YESNO|MB_ICONWARNING))   
				{
						return;
				}

			}

		// End  PTA before moving to AMP
		  InitalFrezArray();
		  
		  m_manualmod = FALSE;
		  m_manualcheckboxstat = FALSE;

		  pbutt[1]->EnableWindow(FALSE);
		  pbutt[2]->EnableWindow(FALSE);
		  pbutt[3]->EnableWindow(FALSE);
		  pbutt[4]->EnableWindow(FALSE);
		  pbutt[5]->EnableWindow(FALSE);
		  pbutt[6]->EnableWindow(FALSE);

		  pHighRangeBtn[0]->pressed = FALSE;
		  pHighRangeBtn[1]->pressed = FALSE;

		  pHighRangeBtn[0]->ModifyStyle(BS_OWNERDRAW,0 );
		  pHighRangeBtn[1]->ModifyStyle(BS_OWNERDRAW,0 );

		  pHighRangeBtn[0]->ShowWindow(SW_HIDE);
		  pHighRangeBtn[1]->ShowWindow(SW_HIDE);

		  if(m_sessionID != NO_SESSION_ID)
		  {
		     EndTestSession(m_sessionID);
			 m_sessionID =  NO_SESSION_ID;
		  }

		  m_isPTATesting = false;

		  if(pbutt[3]->pressed)
		  {
			  pbutt[3]->ModifyStyle(BS_OWNERDRAW,0 );
			  pbutt[3]->pressed = FALSE;
			  pbutt[3]->SetWindowTextW(strButt[3]);  //_T("START"));
			  pbutt[3]->Invalidate(true);
		  }

		  if(pbutt[4]->pressed)
		  {
			pbutt[4]->ModifyStyle(BS_OWNERDRAW,0 );
			pbutt[4]->pressed = FALSE;
			pbutt[4]->Invalidate(true);
		  }

		int i = 0;
		IsAmp = TRUE;
		CRect rect, sldrect,fontrect; 
		int wlen=0,hlen=0;
		m_highRangeSet = FALSE;
		for(i=1; i<7; i++)
		{
			pbutt[i]->pressed = FALSE;
			pbutt[i]->ModifyStyle(BS_OWNERDRAW,0 );
		}

		if(pbutt[0]->pressed)
		{
			m_sessionID = BeginTestSession(ET_AMP, ET_MANUAL);
			if( m_sessionID == NO_SESSION_ID)
				MessageBox((LPCTSTR)"AMP Session can not be created");
			
			   RegisterNotifyWin(this, NotifyWinRequest);

			   pbutt[1]->EnableWindow(TRUE);
			   pbutt[2]->EnableWindow(TRUE);
			   pbutt[3]->EnableWindow(FALSE);
			   pbutt[4]->EnableWindow(TRUE);
			   //pbutt[5]->EnableWindow(TRUE);
			   if(m_treatstage)
			       pbutt[6]->EnableWindow(TRUE);
			   else
				   pbutt[6]->EnableWindow(FALSE);
//			   pbutt[7]->EnableWindow(FALSE);
			   if(m_treatstage)
                  pbutt[5]->EnableWindow(TRUE);
			   else
				  pbutt[5]->EnableWindow(FALSE);

		}

		    Invalidate();
			int numR = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;
			plistctr[0]->SetItemText(3,0,_T(""));
			for(int i=0;i < numR; i++)
			{
				plistctr[0]->SetItemText(0,i+1,_T(""));
				plistctr[0]->SetItemText(1,i+1,_T(""));
				plistctr[0]->SetItemText(2,i+1,_T(""));
				plistctr[0]->SetItemText(3,i+1,_T(""));
			    
				(pRveRight[i]).GetClientRect(rect);

				(pRveRight[i]).MoveWindow(xoffset + gap[0] * (i+1) - 20 *iscal ,yoffset-46*yiscal,rect.Width(),rect.Height());
				(pRveRight[i]).ShowWindow(TRUE);
				(pSliderRt[i]).m_dcBk = NULL;

//              Added for AMP display bug

				(pSliderRt[i]).GetWindowRect(sldrect);
				(pSliderRt[i]).SetRange(0,100);
				(pSliderRt[i]).SetPos(10);
				(pSliderRt[i]).ShowWindow(SW_HIDE);
				(pSliderRt[i]).m_dcBk = NULL;
				 ScreenToClient(sldrect);

				 wlen = sldrect.Width();
				 hlen = sldrect.Height();

				sldrect.left = xoffset + gap[0] * (i+1) - 20 *iscal;
				sldrect.left += rect.Width()/5;
				sldrect.right = sldrect.left + wlen;
				sldrect.top = sldrect.top ;//+ 26*yiscal;
				sldrect.bottom = sldrect.top + hlen;

				(pSliderRt[i]).MoveWindow(sldrect);

				(pfontRt[i]).GetClientRect(fontrect);
				wlen = fontrect.Width();
				hlen = fontrect.Height();
				fontrect.left  = sldrect.left - rect.Width()/2 - 7*iscal;
				fontrect.right = fontrect.left + wlen - 7*iscal;
				fontrect.top   = yoffset + 26 * yiscal + 1;
				fontrect.bottom = fontrect.top + hlen;
				(pfontRt[i]).MoveWindow(fontrect);
				(pfontRt[i]).ShowWindow(SW_HIDE);
				(pfontRt[i]).SetWindowTextW(_T("0"));

			}
			int numL = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;
			plistctr[1]->SetItemText(3,0,_T(""));
			for(int i = 0; i <numL; i++)
			{
				plistctr[1]->SetItemText(0,i+1,_T(""));
				plistctr[1]->SetItemText(1,i+1,_T(""));
				plistctr[1]->SetItemText(2,i+1,_T(""));
				plistctr[1]->SetItemText(3,i+1,_T(""));

				(pRveLeft[i]).GetClientRect(rect);
				(pRveLeft[i]).MoveWindow(xoffset + gap[1] * (i+1) - 20 *iscal + 683*iscal ,yoffset-46*yiscal,rect.Width(),rect.Height());
				(pRveLeft[i]).ShowWindow(TRUE);
				(pSliderLf[i]).m_dcBk = NULL;

				(pSliderLf[i]).GetWindowRect(sldrect);
				(pSliderLf[i]).ShowWindow(SW_HIDE);
				(pSliderLf[i]).m_dcBk = NULL;
				(pSliderLf[i]).SetRange(0,100);
				(pSliderLf[i]).SetPos(10);
				 ScreenToClient(sldrect);

				wlen = sldrect.Width();
				hlen = sldrect.Height();

				 sldrect.left = xoffset + gap[1] * (i+1) - 20 *iscal + 683*iscal;
				 sldrect.left += rect.Width()/5;
				 sldrect.right = sldrect.left + wlen ;
				 sldrect.top = sldrect.top ;//+ 26*yiscal;
				 sldrect.bottom = sldrect.top + hlen;
				(pSliderLf[i]).MoveWindow(sldrect);

				(pfontLf[i]).GetClientRect(fontrect);
				wlen = fontrect.Width();
				hlen = fontrect.Height();
				fontrect.left  = sldrect.left - rect.Width()/2 - 7*iscal;
				fontrect.right = fontrect.left + wlen - 7*iscal;
				fontrect.top   = yoffset + 26 * yiscal + 1;
				fontrect.bottom = fontrect.top + hlen;
				(pfontLf[i]).MoveWindow(fontrect);
				(pfontLf[i]).ShowWindow(SW_HIDE);
				(pfontLf[i]).SetWindowTextW(_T("0"));
			}

			if(LINKED == m_LinkState)
			{
				pbutt[6]->EnableWindow(TRUE);
			}
			else
			{
				if((NOLINKED == m_LinkState) && (pbutt[0]->pressed))
				{
					if(m_treatstage)
					{
						if(LoadAmpTreatHeadSetting())
						{
							pbutt[6]->EnableWindow(TRUE);
						}
						else
						{
							pbutt[6]->EnableWindow(FALSE);
						}
					}

				}
				
			}

	}

	        LoadResourceStr(hinst,IsAmp);
			
			pbutt[0]->SetWindowTextW(strButt[0]);
			pbutt[1]->SetWindowTextW(strButt[1]);
			pbutt[2]->SetWindowTextW(strButt[2]);
			pbutt[3]->SetWindowTextW(strButt[3]);

			pbutt[4]->SetWindowTextW(strButt[4]);
			pbutt[5]->SetWindowTextW(strButt[5]);
			pbutt[6]->SetWindowTextW(strButt[6]);
			pbutt[7]->SetWindowTextW(strButt[7]);

			g_bETGenerated = false;
			g_bIsLocalPlaying = false;
			g_bPTANeedSaved = false;

			PTAQuickCalibrate();

}

BOOL CETApplicationView::LoadAmpTreatHeadSetting()
{
		        std::string curDiaryTime;
				std::string absoluteFilename;         //-------------
				ETAMPResult amp;

				bool pathResult = TRUE;
				strdiarypcpath = GetPTAResultLocalPath();
				CString strfolder = pPatientList[0]->GetItemText(0,1);
	
				if(strfolder.CompareNoCase(strprompt) && strfolder.GetLength())  //no default str
					strdiarypcpath = strdiarypcpath + strfolder;
				else
				{
					MessageBox(strprompt);
					return FALSE;
				}

				strdiarypcpath = strdiarypcpath + _T("\\");
				strdiarypcpath = strdiarypcpath + _T("diary.txt");
				
				std::wstring szwdiary = strdiarypcpath.GetBuffer(0);
				absoluteFilename = ws2s(szwdiary);

				FILE* pFile = NULL;
				pFile = fopen (absoluteFilename.c_str(),"rb");
				if(pFile!=NULL)   //found diary.txt!
				{
					//-------------------------------------------------------------
					 char *pbuff;
					 char *phead = NULL;
					 char *psession = NULL;
					 char buffhead[256];
					 int s=0,headsize=0;

					 memset(buffhead,0,256*sizeof(char));

						    ASSERT(pFile);
							fseek   ( pFile   ,0   ,SEEK_END); 
							long   len   =   ftell(pFile); 
							pbuff = new char[len];
							memset(pbuff,0,len * sizeof(char));
							
							if(len)
							{
		                        fseek(pFile,0,SEEK_SET );
								fread(pbuff,sizeof(char),len,pFile);
							}
	
							while(psession = strstr(pbuff,"AMPSTART"))
							{
								pbuff = psession;
								pbuff += 8;
							}

							phead = strstr(pbuff,"DIARYSTART");
							headsize = phead - pbuff;
							strncpy(buffhead,pbuff,headsize);
							
							BOOL nomatch = FALSE;
							GetTreatDiaryHeadStruct(buffhead,nomatch);
							if(!nomatch)
							    SetDefaultTreatSetting();
					//-------------------------------------------------------------
					
					fclose (pFile);
					return TRUE;
				}
				else            //not found diary.txt!
				{
					return FALSE;
				}

				return FALSE;
	  
}

void CETApplicationView::SetDefaultTreatSetting()
{
	//m_RtNode   m_LfNode
	//if(!InitSound(m_sessionID, ET_RIGHT)) ET_RIGHT
	//MessageBox(_T("Right sound initialization failed"));
	
	/*
	BOOL rtinied = FALSE, lfinied = FALSE;
	if(m_RtNode[0].IsSet || m_RtNode[1].IsSet)
	{
			if(!InitSound(m_sessionID, ET_RIGHT))
	            MessageBox(_T("Right sound initialization failed"));
			else
			{
				rtinied = TRUE;
			}

	}

	if(m_RtNode[0].IsSet)
	{
			SetFreqSelection(m_sessionID,ET_RIGHT,m_RtNode[0].xband);
			AdjustDBrVolumeToValue(m_sessionID,ET_RIGHT,m_RtNode[0].xband,m_RtNode[0].ydbhr);
	}

	if(m_RtNode[1].IsSet)
	{
			SetFreqSelection(m_sessionID,ET_RIGHT,m_RtNode[1].xband);
			AdjustDBrVolumeToValue(m_sessionID,ET_RIGHT,m_RtNode[1].xband,m_RtNode[1].ydbhr);
	}


		if(m_LfNode[0].IsSet || m_LfNode[1].IsSet)
	{
			if(!InitSound(m_sessionID, ET_LEFT))
	            MessageBox(_T("Left sound initialization failed"));
			else
			{
				lfinied = TRUE;
			}
	}

	if(m_LfNode[0].IsSet)
	{
			SetFreqSelection(m_sessionID,ET_LEFT,m_LfNode[0].xband);
			AdjustDBrVolumeToValue(m_sessionID,ET_LEFT,m_LfNode[0].xband,m_LfNode[0].ydbhr);
	}
	if(m_LfNode[1].IsSet)
	{
			SetFreqSelection(m_sessionID,ET_LEFT,m_LfNode[1].xband);
			AdjustDBrVolumeToValue(m_sessionID,ET_LEFT,m_LfNode[1].xband,m_LfNode[1].ydbhr);
	}
	if(rtinied || lfinied )
		pbutt[3]->EnableWindow(TRUE);
		

		*/
	ETAMPResult preAMPValue;
	int n = 0 ;

	if(m_RtNode[0].IsSet)
	{
		preAMPValue.aResultValueList[n].leftRight = ET_RIGHT;
		preAMPValue.aResultValueList[n].ndBr = m_RtNode[0].ydbhr;
		preAMPValue.aResultValueList[n].nFrequency = m_RtNode[0].xband;
		n++;
	}
	if(m_RtNode[1].IsSet)
	{
		preAMPValue.aResultValueList[n].leftRight = ET_RIGHT;
		preAMPValue.aResultValueList[n].ndBr = m_RtNode[1].ydbhr;
		preAMPValue.aResultValueList[n].nFrequency = m_RtNode[1].xband;
		n++;
	}

	if(m_LfNode[0].IsSet)
	{
		preAMPValue.aResultValueList[n].leftRight = ET_LEFT;
		preAMPValue.aResultValueList[n].ndBr = m_LfNode[0].ydbhr;
		preAMPValue.aResultValueList[n].nFrequency = m_LfNode[0].xband;
		n++;
	}

	if(m_LfNode[1].IsSet)
	{
		preAMPValue.aResultValueList[n].leftRight = ET_LEFT;
		preAMPValue.aResultValueList[n].ndBr = m_LfNode[1].ydbhr;
		preAMPValue.aResultValueList[n].nFrequency = m_LfNode[1].xband;
		n++;
	}

	preAMPValue.nResultFreqNum = n;

	InitPreTreatmentSound(m_sessionID,preAMPValue);
	pbutt[3]->EnableWindow(TRUE);
	
}
void CETApplicationView::OnButtonON()
{
	int id =0 ,i = 0;
	int num = 0;
	AMPNODE node;
	CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;

		///////////////////////////////////////////////////////
	// Added to warn no PTA saving
	if((pbutt[0]->pressed)&&(g_bPTANeedSaved))
	{
		if (IDYES != MessageBox( m_buffPTANoSaveWarning, //L"Do you want to leave latest PTA test result without being saved?",  //m_buffassign, 
		                   L"Warning",MB_YESNO|MB_ICONWARNING))   
		{
			return;
		}

	}
	///////////////////////////////////////////////////////

	g_bETGenerated = false;
	g_bIsLocalPlaying = false;

					    CMainFrame *pmnfm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
				    CRect rect;
                    pmnfm->GetWindowRect( rect );

					::SetWindowPos(pmnfm->GetSafeHwnd() ,       // handle to window
					HWND_TOPMOST,  // placement-order handle
					rect.left,     // horizontal position
					rect.top,      // vertical position
					rect.Width(),  // width
					rect.Height(), // height
					SWP_SHOWWINDOW); // window-positioning options

	if(!pbutt[0]->pressed)
	{
		SetMuteSysVol(true);
		// Check audiomed for linking
		if ( NOLINKED == m_LinkState && !g_bPatientLinkingThreadRunning)
		{		
			g_bPatientLinkingThreadRunning = true;
			m_pPatientLinkingThread = ::AfxBeginThread( PatientLinkingThread, this);
			if ( m_pPatientLinkingThread )
			{
				m_pPatientLinkingThread->ResumeThread();
			}
		}
		
			pPatientList[0]->SetItemText(0,1,m_bufstrprompt);
	}
	else
	{
		if(m_nTimer != NULL)
		{
			this->KillTimer(m_nTimer);		
			m_nTimer = NULL;
		}
		strendTime = _T("");

		if(m_nwarnTime != NULL)
		{
			this->KillTimer(m_nwarnTime);		
			m_nwarnTime = NULL;
		}
		m_treatstage = FALSE;

		QuitUsingAudiomed();
//		SetMuteSysVol(false);
		m_LinkState = NOLINKED;
		pPatientList[1]->SetItemText(0,1,m_buffstrunlink); //_T("  UNLINKED"));
		pPatientList[1]->SetItemText(0,2,_T(""));
		pPatientList[1]->SetItemText(0,3,_T(""));

		pPatientList[0]->SetItemText(0,1,_T(""));
		pPatientList[0]->SetItemText(0,3,_T(""));
		pPatientList[0]->SetItemText(1,1,_T(""));
		pPatientList[0]->SetItemText(1,3,_T(""));
		pPatientList[0]->SetItemText(2,1,_T(""));
		pPatientList[0]->SetItemText(2,3,_T(""));

		g_bPTANeedSaved = false;
		g_testSpeed = PTA_SPEED_NORMAL;

	}


	if(IsAmp)    // AMP  m_IsPTAtested
	{
		if(!pbutt[0]->pressed)
		{
			m_sessionID = BeginTestSession(ET_AMP, ET_MANUAL);
			if( m_sessionID == NO_SESSION_ID)
				MessageBox((LPCTSTR)"AMP Session can not be created");

			RegisterNotifyWin(this, NotifyWinRequest);

			pbutt[0]->ModifyStyle(0,BS_OWNERDRAW );
			pbutt[0]->pressed = TRUE;

		   pbutt[1]->EnableWindow(TRUE);
		   pbutt[2]->EnableWindow(TRUE);
		   pbutt[3]->EnableWindow(FALSE);
		   pbutt[4]->EnableWindow(TRUE);
		   pbutt[5]->EnableWindow(TRUE);

		  // pbutt[6]->EnableWindow(TRUE);
		   	if(LINKED == m_LinkState)
			{
				pbutt[6]->EnableWindow(TRUE);
			}

		  // pPatientList[0]->SetItemText(0,1,strprompt);
		   //pbutt[7]->EnableWindow(FALSE); 
		}
		else
		{
			  
			pDoc->m_IsPTAtested  = FALSE;

			if(m_sessionID != NO_SESSION_ID)
			EndTestSession(m_sessionID);

			#ifdef  SUPPORT_SITE_TREATMENT
						pbutt[1]->EnableWindow(TRUE);
						pbutt[2]->EnableWindow(TRUE);
						pbutt[3]->EnableWindow(TRUE);
			#endif  
				for(i =0 ;i <8 ; i++)
					{
						if(pbutt[i]->pressed)
						{
							pbutt[i]->ModifyStyle(BS_OWNERDRAW,0 );
							pbutt[i]->pressed = FALSE;
						}

					}
				
				/*for(id =0 ;id < 2;id++)
				  {
					  num  =  m_AMP_List[id].GetCount();
					  for(i =0 ;i < num; i++)
					  {
						  if(id == 0)
						  {
							  pRveRight[i].ModifyStyle(BS_OWNERDRAW,0);
							  pRveRight[i].Invalidate();
							  pRveRight[i].EnableWindow(FALSE);
							  pSliderRt[i].ShowWindow(SW_HIDE);
							  pfontRt[i].ShowWindow(SW_HIDE);

						  }
						  if(id == 1)
						  {
							  pRveLeft[i].ModifyStyle(BS_OWNERDRAW,0);
							  pRveLeft[i].Invalidate();
							  pRveLeft[i].EnableWindow(FALSE);
							  pSliderLf[i].ShowWindow(SW_HIDE);
							  pfontLf[i].ShowWindow(SW_HIDE);

						  }
						  node = m_AMP_List[id].GetAt(m_AMP_List[id].FindIndex(i));
						  node.IsSet = FALSE;
						  node.ydbhr = 0;
						  m_AMP_List[id].SetAt(m_AMP_List[id].FindIndex(i),node);
						  // m_AMP_List[id].RemoveAll();

					  }
				  }*/

				ReSetAmpRightStat();
				ReSetAmpLeftStat();

				
				m_grayHighLine[0] = TRUE;
				m_grayHighLine[1] = TRUE;

				g_bands_type = BANDS_CLASSIC_11;
				OnInitalPTAMP();
				UpdateCtrlTabFromList();
				StoreDataInList();
				InitalFrezArray();

				m_highRangeSet = FALSE;
				memset(m_freqsetArry[0],1,10*sizeof(BOOL));
				memset(m_freqsetArry[1],1,10*sizeof(BOOL));
				this->Invalidate();
				
				//-----------------------------------------------------------------------------
			   int numR = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;
			   CRect rect;
			   CRect sldrect;
			   CRect fontrect;
			   int wlen, hlen;
			   //plistctr[0]->SetItemText(3,0,_T(""));
				for(int i=0;i < numR; i++)
				{
					//plistctr[0]->SetItemText(0,i+1,_T(""));
					//plistctr[0]->SetItemText(1,i+1,_T(""));
					//plistctr[0]->SetItemText(2,i+1,_T(""));
					//plistctr[0]->SetItemText(3,i+1,_T(""));
			    
					(pRveRight[i]).GetClientRect(rect);
					(pRveRight[i]).MoveWindow(xoffset + gap[0] * (i+1) - 20 *iscal ,yoffset-46*yiscal,rect.Width(),rect.Height());
					(pRveRight[i]).ShowWindow(TRUE);
					(pRveRight[i]).EnableWindow(FALSE);

						(pSliderRt[i]).GetWindowRect(sldrect);
						(pSliderRt[i]).SetRange(0,100);
						(pSliderRt[i]).SetPos(10);
						(pSliderRt[i]).ShowWindow(SW_HIDE);
						(pSliderRt[i]).m_dcBk = NULL;
						 ScreenToClient(sldrect);

						 wlen = sldrect.Width();
						 hlen = sldrect.Height();

						sldrect.left = xoffset + gap[0] * (i+1) - 20 *iscal;
						sldrect.left += rect.Width()/5;
						sldrect.right = sldrect.left + wlen;
						sldrect.top = sldrect.top ; //+ 26*yiscal;
						sldrect.bottom = sldrect.top + hlen;

						(pSliderRt[i]).MoveWindow(sldrect);

						(pfontRt[i]).GetClientRect(fontrect);
						wlen = fontrect.Width();
						hlen = fontrect.Height();
						fontrect.left  = sldrect.left - rect.Width()/2 - 7*iscal;
						fontrect.right = fontrect.left + wlen - 7*iscal;
						fontrect.top   = yoffset + 26 * yiscal + 1;
						fontrect.bottom = fontrect.top + hlen;
						(pfontRt[i]).MoveWindow(fontrect);
						(pfontRt[i]).ShowWindow(SW_HIDE);
						(pfontRt[i]).SetWindowTextW(_T("0"));
				
				  }
			   int numL = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;
				//plistctr[1]->SetItemText(3,0,_T(""));
				for(int i = 0; i <numL; i++)
				{
					//plistctr[1]->SetItemText(0,i+1,_T(""));
					//plistctr[1]->SetItemText(1,i+1,_T(""));
					//plistctr[1]->SetItemText(2,i+1,_T(""));
					//plistctr[1]->SetItemText(3,i+1,_T(""));

					(pRveLeft[i]).GetClientRect(rect);
					(pRveLeft[i]).MoveWindow(xoffset + gap[1] * (i+1) - 20 *iscal + 683*iscal ,yoffset-46*yiscal,rect.Width(),rect.Height());
					(pRveLeft[i]).ShowWindow(TRUE);
					(pRveLeft[i]).EnableWindow(FALSE);

					(pSliderLf[i]).GetWindowRect(sldrect);
				    (pSliderLf[i]).ShowWindow(SW_HIDE);
				    (pSliderLf[i]).m_dcBk = NULL;
				    (pSliderLf[i]).SetRange(0,100);
				    (pSliderLf[i]).SetPos(10);
				     ScreenToClient(sldrect);

				     wlen = sldrect.Width();
				     hlen = sldrect.Height();

					 sldrect.left = xoffset + gap[1] * (i+1) - 20 *iscal + 683*iscal;
					 sldrect.left += rect.Width()/5;
					 sldrect.right = sldrect.left + wlen ;
					 sldrect.top = sldrect.top ;//+ 26*yiscal;
					 sldrect.bottom = sldrect.top + hlen;
					 (pSliderLf[i]).MoveWindow(sldrect);

					 (pfontLf[i]).GetClientRect(fontrect);
					 wlen = fontrect.Width();
					 hlen = fontrect.Height();
					 fontrect.left  = sldrect.left - rect.Width()/2 - 7*iscal;
					 fontrect.right = fontrect.left + wlen - 7*iscal;
					 fontrect.top   = yoffset + 26 * yiscal + 1;
					 fontrect.bottom = fontrect.top + hlen;
					 (pfontLf[i]).MoveWindow(fontrect);
					 (pfontLf[i]).ShowWindow(SW_HIDE);
					 (pfontLf[i]).SetWindowTextW(_T("0"));
				}
			//-----------------------------------------------------------------------------


				pbutt[1]->EnableWindow(FALSE);
				pbutt[2]->EnableWindow(FALSE);
				pbutt[3]->EnableWindow(FALSE);
				pbutt[4]->EnableWindow(FALSE);
				pbutt[5]->EnableWindow(FALSE);
				pbutt[6]->EnableWindow(FALSE);
				pbutt[7]->EnableWindow(TRUE);

		}
			
	}
	else    // PTA
	{
		if(!pbutt[0]->pressed)
		{
			pbutt[1]->EnableWindow(TRUE);
			pbutt[2]->EnableWindow(TRUE);
			pbutt[3]->EnableWindow(TRUE);
			pbutt[4]->EnableWindow(TRUE);
			pbutt[5]->EnableWindow(TRUE);
			pbutt[6]->EnableWindow(TRUE);
			pbutt[7]->EnableWindow(TRUE);
		   m_sessionID = BeginTestSession(ET_PTA, ET_AUTOMATIC);
		   if( m_sessionID == NO_SESSION_ID)
		   {
			  MessageBox(L"PTA Session can not be created");
			  return;
		   }
		   SetPTASpeed(m_sessionID, g_testSpeed);
		   RegisterTestCompleteIndication(m_sessionID, TestAccomplishmentIndication, this, TestProgressIndication);

		   m_isPTATesting = false;
		   pbutt[0]->ModifyStyle(0,BS_OWNERDRAW );
		   pbutt[0]->pressed = TRUE;
//		   SetPTASpeed(m_sessionID,PTA_SPEED_VERY_QUICK);  // temp test
		   pHighRangeBtn[0]->EnableWindow(TRUE);
		   pHighRangeBtn[1]->EnableWindow(TRUE);
		   pPatientList[0]->SetItemText(0,1,strprompt);
		}
		else
		{
			pDoc->m_IsPTAtested  = FALSE;

			pbutt[1]->EnableWindow(FALSE);
			pbutt[2]->EnableWindow(FALSE);
			pbutt[3]->EnableWindow(FALSE);
			pbutt[4]->EnableWindow(FALSE);
			pbutt[5]->EnableWindow(FALSE);
			pbutt[6]->EnableWindow(FALSE);
			pbutt[7]->EnableWindow(TRUE);

		  if(m_sessionID != NO_SESSION_ID)
		     EndTestSession(m_sessionID);

		  m_isPTATesting = false;

		  pbutt[0]->ModifyStyle(BS_OWNERDRAW,0 );
		  pbutt[0]->pressed = FALSE;

		   m_narrowWarningRT = FALSE;
	       m_narrowWarningLF = FALSE;

		   CString idarr[3];
			idarr[0] = _T("1)");
			idarr[1] = _T("2)");
			idarr[2] = _T("3)");

		   ptalistctltime[0] = _T("");
		   ptalistctltime[1] = _T("");
		   ptalistctltime[2] = _T("");

		   plistctr[0]->SetItemText(0,0,idarr[0]+ptalistctltime[0]);
		    plistctr[0]->SetItemText(1,0,idarr[1]+ptalistctltime[1]);
			 plistctr[0]->SetItemText(2,0,idarr[2]+ptalistctltime[2]);

		   plistctr[1]->SetItemText(0,0,idarr[0]+ptalistctltime[0]);
		    plistctr[1]->SetItemText(1,0,idarr[1]+ptalistctltime[1]);
			 plistctr[1]->SetItemText(2,0,idarr[2]+ptalistctltime[2]);


		  if(pbutt[3]->pressed)
		  {
			  pbutt[3]->ModifyStyle(BS_OWNERDRAW,0 );
			  pbutt[3]->pressed = FALSE;
			  pbutt[3]->SetWindowTextW(strButt[3]);  //_T("START"));
			  pbutt[3]->Invalidate(true);
		  }

		  if(pbutt[4]->pressed)
		  {
			pbutt[4]->SetWindowTextW(strButt[4]); //_T("PAUSE"));
			pbutt[4]->ModifyStyle(BS_OWNERDRAW,0 );
			pbutt[4]->pressed = FALSE;
			pbutt[4]->Invalidate(true);
		  }
		   pHighRangeBtn[0]->EnableWindow(FALSE);
		   pHighRangeBtn[1]->EnableWindow(FALSE);

		   pHighRangeBtn[0]->ModifyStyle(BS_OWNERDRAW,0);
		   pHighRangeBtn[0]->pressed = FALSE;
		   pHighRangeBtn[0]->Invalidate();
		   pHighRangeBtn[1]->ModifyStyle(BS_OWNERDRAW,0);
		   pHighRangeBtn[1]->Invalidate();
		   pHighRangeBtn[1]->pressed = FALSE;

		   m_grayHighLine[0] = TRUE;
	       m_grayHighLine[1] = TRUE;

		    g_bands_type = BANDS_CLASSIC_11;
			OnInitalPTAMP();
			UpdateCtrlTabFromList();
			StoreDataInList();

		   	m_highRangeSet = FALSE;
			memset(m_freqsetArry[0],1,10*sizeof(BOOL));
			memset(m_freqsetArry[1],1,10*sizeof(BOOL));

			RemoveManualMod();
			m_manualcheckboxstat = FALSE;

			this->Invalidate();
		 
		}
	    //pPatientList[1]->SetItemText(1,2,_T(" "));

	}

}

void CETApplicationView:: OnButtonMod() //OnButtonBand()
{

	//int frez[8] = {1000,1500,2000,3000,4000,6000,8000,12000};
	int frez=0;
	BOOL    presstat = FALSE;
    AMPNODE ampnode;
	int idx, num = 0;
	RECT fontrect;
	int high = 0;
					    
	                CMainFrame *pmnfm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
				    CRect rect;
                    pmnfm->GetWindowRect( rect );

					::SetWindowPos(pmnfm->GetSafeHwnd() ,       // handle to window
					HWND_TOPMOST,  // placement-order handle
					rect.left,     // horizontal position
					rect.top,      // vertical position
					rect.Width(),  // width
					rect.Height(), // height
					SWP_SHOWWINDOW); // window-positioning options


	if(IsAmp)
	{
		if(!pbutt[0]->pressed)
			return;

		if(!InitSound(m_sessionID, ET_LEFT))
			MessageBox(_T("Left sound initialization failed"));
		else
		{
//			ampsideID = 1;
			m_leftright = ET_LEFT;
//			FocusFreqPlaying(m_sessionID, m_leftright);
		    int num = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;
			if(!pbutt[2]->pressed)
			{
				ampsideID = 1;
				pbutt[2]->ModifyStyle(0,BS_OWNERDRAW );
				pbutt[2]->pressed = TRUE;

				if(pbutt[1]->pressed)
				{
					pbutt[1]->ModifyStyle(BS_OWNERDRAW,0 );
					pbutt[1]->pressed = FALSE;
					pbutt[1]->Invalidate();
					for(int s=0; s< num; s++)
						{
							pRveRight[s].EnableWindow(FALSE);
							pSliderRt[s].ShowWindow(FALSE);
						}
				}

				     
					 for(idx=0;idx < num; idx++)
					  {
						  ampnode = m_AMP_List[ampsideID].GetAt(m_AMP_List[ampsideID].FindIndex(idx));
						  if(ampnode.IsSet)
							  pSliderLf[idx].ShowWindow(TRUE);

					  }

				
//				pbutt[2]->ModifyStyle(0,BS_OWNERDRAW );
//				pbutt[2]->pressed = TRUE;
				for(int i = 0 ;i < num; i++)  
						pRveLeft[i].EnableWindow(TRUE);

				FocusFreqPlaying(m_sessionID, m_leftright);
			}
			else
			{
				pbutt[2]->ModifyStyle(BS_OWNERDRAW,0 );
				pbutt[2]->pressed = FALSE;
				////////////
	            ReSetAmpLeftStat();
			   ///////////////////
				if(!JudgeETFfileGen() )
						pbutt[3]->EnableWindow(FALSE);
				else
					   pbutt[3]->EnableWindow(TRUE);
			}
		}
	}
	else
	{
		if(m_sessionID == NO_SESSION_ID)
			return;

		          
					CString strsyspath,strfile,str;
					DWORD errorret = 0;
					BOOL  ret, idexists = FALSE;
					strsyspath = GetPTAResultLocalPath();
					str = pPatientList[0]->GetItemText(0,1);
					if(str.CompareNoCase(strprompt) && str.GetLength())
					{
							 strsyspath = strsyspath + str;
							 ret = CreateDirectory(strsyspath,NULL);
							 if(!ret)
							 {
								 errorret = GetLastError();
								 if(ERROR_ALREADY_EXISTS == errorret)
								 {
									 idexists = TRUE;
								 }
							 }
							 else        //temporey folder create! need delete!
							 {
								 RemoveDirectory(strsyspath);
								 idexists = FALSE;
							 }
					}
					else
					{
						         idexists = FALSE;
					}


		CTestSpeedDlg testSpeedDlg(g_testSpeed);

		testSpeedDlg.GetCheckStat(m_manualcheckboxstat);
		//pPatientList[1]->SetItemText(0,3,m_buffstrlinked); //_T("  LINKED"));
//		if(pPatientList[1]->GetItemText(0,3).Compare(m_buffstrlinked)   )
		   testSpeedDlg.SetPretreatmentIDstat(idexists,m_treatstage);
//		else
//		   testSpeedDlg.SetPretreatmentIDstat(idexists,m_treatstage,TRUE);

		if(testSpeedDlg.DoModal() == IDOK)
		{
			g_testSpeed = testSpeedDlg.GetTestSpeed();
			m_treatstage = testSpeedDlg.ttbox;
			SetPTASpeed(m_sessionID, g_testSpeed);
			if(testSpeedDlg.manualstat)
			{
				g_bPTANeedSaved = false;
				GoToManualMod();
				pbutt[1]->EnableWindow(FALSE);
				m_manualcheckboxstat = TRUE;
			}
			else
			{
				RemoveManualMod();
				pbutt[1]->EnableWindow(TRUE);
				m_manualcheckboxstat = FALSE;
			}
		}
	}


}
void CETApplicationView::GoToManualMod()
{
	CRect rect;
	CRect sldrect;
	CRect fontrect;
	int wlen =0,hlen =0 ;

	 pHighRangeBtn[0]->ShowWindow(FALSE);
	 pHighRangeBtn[1]->ShowWindow(FALSE);

	m_manualmod = TRUE;

	plistctr[0]->SetItemText(0,0,_T("1)"));
	plistctr[0]->SetItemText(1,0,_T("2)"));
	plistctr[0]->SetItemText(2,0,_T("3)"));

	plistctr[1]->SetItemText(0,0,_T("1)"));
	plistctr[1]->SetItemText(1,0,_T("2)"));
	plistctr[1]->SetItemText(2,0,_T("3)"));

    int numR = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;

	for(int i=0;i < numR; i++)
	{
		(pRveRight[i]).GetClientRect(rect);
		(pRveRight[i]).MoveWindow(xoffset + gap[0] * (i+1) - 20 *iscal ,yoffset-46*yiscal,rect.Width(),rect.Height());
		(pRveRight[i]).ShowWindow(TRUE);
		 
		 //------------------------------
		 (pSliderRt[i]).GetWindowRect(sldrect);
          ScreenToClient(sldrect);
		//  (pSliderRt[i]).GetClientRect(sldrect);

			wlen = sldrect.Width();
			hlen = sldrect.Height();

		   sldrect.left = xoffset + gap[0] * (i+1) - 20 *iscal;
		   sldrect.left += rect.Width()/5;
		   sldrect.right = sldrect.left + wlen;
		  (pSliderRt[i]).MoveWindow(sldrect);
		  // pSliderRt[i].ShowWindow(TRUE);


		  	(pfontRt[i]).GetClientRect(fontrect);
			wlen = fontrect.Width();
			hlen = fontrect.Height();
			fontrect.left  = sldrect.left - rect.Width()/2 - 7*iscal;
			fontrect.right = fontrect.left + wlen;

			fontrect.top   = yoffset + 26 * yiscal + 1;
			fontrect.bottom = fontrect.top + hlen;
			(pfontRt[i]).MoveWindow(fontrect);

		 //-----------------------------
		 pSliderRt[i].SetRange(0,NUM_DB_VOLUME);
		 pSliderRt[i].SetPos(0);
		 plistctr[0]->SetItemText(0,i+1,_T("0.0"));
		 plistctr[0]->SetItemText(1,i+1,_T(""));
		 plistctr[0]->SetItemText(2,i+1,_T(""));
		 plistctr[0]->SetItemText(3,i+1,_T(""));

		 pSliderRt[i].EnableWindow(FALSE);
		
	}

	int numL = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;

	for(int i = 0; i <numL; i++)
	{
		(pRveLeft[i]).GetClientRect(rect);
		(pRveLeft[i]).MoveWindow(xoffset + gap[1] * (i+1) - 20 *iscal + 683*iscal ,yoffset-46*yiscal,rect.Width(),rect.Height());
		(pRveLeft[i]).ShowWindow(TRUE);
		

		 //---------------------------------
		 (pSliderLf[i]).GetWindowRect(sldrect);
          ScreenToClient(sldrect);

			wlen = sldrect.Width();
			hlen = sldrect.Height();

		   sldrect.left = xoffset + gap[1] * (i+1) - 20 *iscal +  683*iscal;
		   sldrect.left += rect.Width()/5;
		   sldrect.right = sldrect.left + wlen;
		  (pSliderLf[i]).MoveWindow(sldrect);
		  // pSliderLf[i].ShowWindow(TRUE);


		  	(pfontLf[i]).GetClientRect(fontrect);
			wlen = fontrect.Width();
			hlen = fontrect.Height();
			fontrect.left  = sldrect.left - rect.Width()/2 - 7*iscal ;
			fontrect.right = fontrect.left + wlen;

			fontrect.top   = yoffset + 26 * yiscal + 1;
			fontrect.bottom = fontrect.top + hlen;
			(pfontLf[i]).MoveWindow(fontrect);

		 //----------------------------------
		 pSliderLf[i].SetRange(0,NUM_DB_VOLUME);
		 pSliderLf[i].SetPos(0);
		 plistctr[1]->SetItemText(0,i+1,_T("0.0"));
		 plistctr[1]->SetItemText(1,i+1,_T(""));
		 plistctr[1]->SetItemText(2,i+1,_T(""));
		 plistctr[1]->SetItemText(3,i+1,_T(""));
		 pSliderLf[i].EnableWindow(FALSE);
		
	}
	
		StoreDataInList();           //added by anson at Sep 21 
		UpdateCtrlListData();

	    for(int i = 0 ;i < numR; i++)
		{
			pSliderRt[i].ShowWindow(TRUE);
		}

		for(int i = 0 ;i < numL; i++)
		{
		   pSliderLf[i].ShowWindow(TRUE);
		}
	
}

void CETApplicationView::RemoveManualMod()
{
	CRect rect;
	m_manualmod = FALSE;
    int numR = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;

	for(int i=0;i < numR; i++)
	{
		(pRveRight[i]).GetClientRect(rect);
		(pRveRight[i]).MoveWindow(xoffset + gap[0] * (i+1) - 20 *iscal ,yoffset-46*yiscal,rect.Width(),rect.Height());
		(pRveRight[i]).ShowWindow(SW_HIDE);
		 pSliderRt[i].ShowWindow(SW_HIDE);
		 pSliderRt[i].SetPos(0);
		 pSliderRt[i].EnableWindow(TRUE);
		 pfontRt[i].ShowWindow(SW_HIDE);
		 pHighRangeBtn[0]->ShowWindow(TRUE);
	}

	int numL = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;

	for(int i = 0; i <numL; i++)
	{
		(pRveLeft[i]).GetClientRect(rect);
		(pRveLeft[i]).MoveWindow(xoffset + gap[1] * (i+1) - 20 *iscal + 683*iscal ,yoffset-46*yiscal,rect.Width(),rect.Height());
		(pRveLeft[i]).ShowWindow(SW_HIDE);
		 pSliderLf[i].ShowWindow(SW_HIDE);
		 pSliderLf[i].SetPos(0);
		 pSliderLf[i].EnableWindow(TRUE);
		 pfontLf[i].ShowWindow(SW_HIDE);
		 pHighRangeBtn[1]->ShowWindow(TRUE);
	}
}

void CETApplicationView::OnButtonMemberFnLeft(UINT nID)   //  AMP LEFT
{
		//int frez[8] = {1000,1500,2000,3000,4000,6000,8000,12000};
	   // int *frez = NULL;
	    int  ctrlid = nID - IDC_SUB_BUTT_LF;
		int  offlens;
	    RECT rect;
		LNNODE node;
		AMPNODE ampnode;
		BOOL    presstat = FALSE;
		//*********

		//int count = m_AMP_List[1].GetCount();
		//	for(idx=0;idx < num; idx++)
			
			//	ampnode = m_AMP_List[ampsideID].GetAt(m_AMP_List[ampsideID].FindIndex(idx));
		//*******

		if(IsAmp)
		{
				  if(!pRveLeft[ctrlid].pressed )
				  {
			  			   if(BtnSelectBand(true,1,pfrez[1][ctrlid]))
						   {
							   pRveLeft[ctrlid].ModifyStyle(0,BS_OWNERDRAW );
							   pRveLeft[ctrlid].pressed = TRUE;
							  // pSliderLf[ctrlid].SetPos(defposval);
							   pSliderLf[ctrlid].currpos = defposval;
							   pSliderLf[ctrlid].ShowWindow(TRUE);
							   pSliderLf[ctrlid].GetThumbRect(&rect);
							   pSliderLf[ctrlid].EnableWindow(TRUE);
							   pSliderLf[ctrlid].ClientToScreen(&rect);
							   pSliderLf[ctrlid].SetFocus();
							   this->displayrect = rect;
							   pbutt[3]->EnableWindow(TRUE);
							   pfontLf[ctrlid].ShowWindow(FALSE); //TRUE);

							   CString strval;
					           pfontLf[ctrlid].GetWindowTextW(strval);
							   plistctr[1]->SetItemText(0,1+ctrlid,strval);

							   presstat = TRUE;

						   }

				  }
				  else
				  {
						  pRveLeft[ctrlid].ModifyStyle(BS_OWNERDRAW,0 );
						  pRveLeft[ctrlid].pressed = FALSE;
						  pSliderLf[ctrlid].ShowWindow(SW_HIDE);
						  pfontLf[ctrlid].ShowWindow(SW_HIDE);
						  plistctr[1]->SetItemText(0,ctrlid+1,_T(""));
						  BtnSelectBand(false,1, pfrez[1][ctrlid]);   //(ctrlid+1)*1000);
						  presstat = FALSE;

						  if(!JudgeETFfileGen() )
							pbutt[3]->EnableWindow(FALSE);
					     else
						   pbutt[3]->EnableWindow(TRUE);

				  }
						
				          ampnode = m_AMP_List[1].GetAt(m_AMP_List[1].FindIndex(ctrlid));
						  ampnode.IsSet = presstat;
						  m_AMP_List[1].SetAt(m_AMP_List[1].FindIndex(ctrlid),ampnode);

		}

		 if(m_manualmod)
		{

			pSliderLf[ctrlid].SetPos(0);
			pfontLf[ctrlid].ShowWindow(SW_HIDE);
			plistctr[1]->SetItemText(0,ctrlid+1,_T(""));
			m_rcdvalarr[1][ctrlid] = 0;

		}

}

void CETApplicationView::OnButtonMemberFn(UINT nID)   //  AMP RIGHT
{
	   // int  posval=0;

	//int frez[8] = {1000,1500,2000,3000,4000,6000,8000,12000};
	    
	/*	CETApplicationDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
		return;*/

	    int  ctrlid = nID - IDC_SUB_BUTT_RT;
		int  offlens;
	    RECT rect;
		//LNNODE node;
		AMPNODE ampnode;
		BOOL    presstat = FALSE;;
	
		/*node = this->m_AMP_List[0].GetAt(this->m_AMP_List[0].FindIndex(ctrlid+2));     //iori need to get index of plist
		defsliderPt.x = node.x;
		defsliderPt.y = node.y;
		ClientToScreen(&defsliderPt);
		defposval = ((node.y - yoffset)  /(26 * SLIDERGAP * yiscal))*100;
		*/

		if(IsAmp )
		{
		  if(!pRveRight[ctrlid].pressed )   //not pressed
		  {
			//   if(m_RightCount < 2)
			//   {
			  	   if(BtnSelectBand(true,0,pfrez[0][ctrlid]))
				   {
					  
					   pRveRight[ctrlid].ModifyStyle(0,BS_OWNERDRAW );
					   pRveRight[ctrlid].pressed = TRUE;
					   //pSliderLf[ctrlid].SetPos(defposval);
					   pSliderRt[ctrlid].currpos = defposval;
					   pSliderRt[ctrlid].ShowWindow(TRUE);
					   pSliderRt[ctrlid].EnableWindow(TRUE);
					   pSliderRt[ctrlid].GetThumbRect(&rect);
					   pSliderRt[ctrlid].SetFocus();
					   pSliderRt[ctrlid].ClientToScreen(&rect);
					   this->displayrect = rect;
					   presstat = TRUE;
					   pbutt[3]->EnableWindow(TRUE);
					   pfontRt[ctrlid].ShowWindow(FALSE); //TRUE);
					    CString strval;
					   pfontRt[ctrlid].GetWindowTextW(strval);
					   plistctr[0]->SetItemText(0,1+ctrlid,strval);
				   }

				  // SetCursorPos(defsliderPt.x,defsliderPt.y);
				  //  BtnSelectBand(true,0,(ctrlid+1)*1000);
				  // m_RightCount++; frez
			      //   }
			  
		  }
		  else                                //pressed
		  {
				  pRveRight[ctrlid].ModifyStyle(BS_OWNERDRAW,0 );
				  pRveRight[ctrlid].pressed = FALSE;
				  pSliderRt[ctrlid].ShowWindow(SW_HIDE);
				  pfontRt[ctrlid].ShowWindow(SW_HIDE);
				  plistctr[0]->SetItemText(0,ctrlid+1,_T(""));
				//  BtnSelectBand(false,0,(ctrlid+1)*1000);
				  BtnSelectBand(false,0, pfrez[0][ctrlid]);   //(ctrlid+1)*1000);
				  presstat = FALSE;
				  	if(!JudgeETFfileGen() )
							pbutt[3]->EnableWindow(FALSE);
					else
						   pbutt[3]->EnableWindow(TRUE);
				  //m_RightCount--;

		  }
				ampnode = m_AMP_List[0].GetAt(m_AMP_List[0].FindIndex(ctrlid));
				ampnode.IsSet = presstat;
				m_AMP_List[0].SetAt(m_AMP_List[0].FindIndex(ctrlid),ampnode);
					 
		}

		if(m_manualmod)
		{

			pSliderRt[ctrlid].SetPos(0);
			pfontRt[ctrlid].ShowWindow(SW_HIDE);
			plistctr[0]->SetItemText(0,ctrlid+1,_T(""));
			m_rcdvalarr[0][ctrlid] = 0;

		}


			//this->GetThumbRect(&rect);
	       // ClientToScreen(&rect);
	      //  p->displayrect = rect;

		
		//pRveLeft[0].SetState(1);
	


	//	buttupdate = TRUE;
	//	Invalidate();
		//RedrawWindow();

		//btnDown = FALSE;
		//operaID = ctrlid;
		//btnDown = FALSE;
}

BOOL CETApplicationView::BtnSelectBand(bool on,int side,int band)
{

	ETLeftRight leftRight;
	if(side == 0)
	{
		 leftRight = ET_RIGHT;
	}
	else
	{
		leftRight = ET_LEFT;
	}

	if(on)
	{
		if(false == SetFreqSelection(m_sessionID, leftRight, band))
		{
//			AfxMessageBox(L"Frequency selection limitation is reached, or it is too close to already selected frequency point");
			AfxMessageBox(m_buffFreSelLimit);
			return FALSE;
		}
		g_bETGenerated = false;
	}
	else
	{
		RemoveFreqSelection(m_sessionID, leftRight, band);  
		g_bETGenerated = false;
	}

	FocusFreqPlaying(m_sessionID, m_leftright);
	return TRUE;


}


/*
void CETApplicationView::OnFirstBut()
{
	//if( pRveLeft[0].GetState() & 0x0004)
	// pRveLeft[0].SetState(0);//!(pRveLeft[0].GetState() & 0x0004));
	//UINT s = pRveLeft[0].GetState();
	// pRveLeft[0].SetState(!(s * 0x0004));

			//      if((pRveLeft[0].GetState() & 0x0008) )  //highlight
					{
						if(pSliderLf[0].IsWindowVisible())  //visible
							pSliderLf[0].ShowWindow(FALSE);
					}
					else
					{
						pRveLeft[0].SetState(1);
    		//			pSliderLf[0].ShowWindow(TRUE);
			//		}

	    int  offlens;
	    RECT rect;
		
		pRveLeft[0].SetState(1);
		pRveLeft[0].GetClientRect(&rect);

		//lens = pSliderLf[0].GetThumbLength();
		pSliderLf[0].SetThumbLength(rect.right -rect.left -2);

		RECT brect,thumbrect;
		pSliderLf[0].GetThumbRect(&thumbrect);	
		offlens = thumbrect.top;
	
		pSliderLf[0].GetWindowRect(&brect);
		ScreenToClient(&brect);
		pSliderLf[0].MoveWindow(brect.left,yoffset-offlens,brect.right -brect.left,brect.bottom -brect.top);
		//pSliderLf[0].SetThumbRect(rect);
		//lens = pSliderLf[0].GetPos();
    	pSliderLf[0].ShowWindow(TRUE);
}
*/

BOOL CETApplicationView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CETApplicationView drawing

void CETApplicationView::OnInitalHighRangeArray()
{
	FREZNODE node;
	ETFrequencyArray freArray;

	freArray = m_initalexchangearry;

	    m_highRange[0].nFrequencyNum = 	freArray.nRightFrequencyNum ;
		for(int i = 0 ; i < freArray.nRightFrequencyNum; i++)
		{
			m_highRange[0].aFreqList[i] =  freArray.aRightFreqList[i] ;
		}

		m_highRange[1].nFrequencyNum = freArray.nLeftFrequencyNum;
		for(int i = 0 ; i < freArray.nLeftFrequencyNum; i++)
		{
			 m_highRange[1].aFreqList[i] = freArray.aLeftFreqList[i];
		}

}
void CETApplicationView::OnInitalPTAMP()
{
//#########################################################################################
	/*CDocList *pdoclist;
	CETApplicationDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;*/

	CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;

	FREZNODE node;
	ETFrequencyArray freArray;

	if( BANDS_CLASSIC_6 == g_bands_type)
	{
		freArray.nLeftFrequencyNum = 6;
		freArray.aLeftFreqList[0]= 250; 
		freArray.aLeftFreqList[1]= 500; 
		freArray.aLeftFreqList[2]= 1000; 
		freArray.aLeftFreqList[3]= 2000; 
		freArray.aLeftFreqList[4] = 4000;
		freArray.aLeftFreqList[5] = 8000;
			
		freArray.nRightFrequencyNum = 6;
		freArray.aRightFreqList[0]= 250; 
		freArray.aRightFreqList[1]= 500; 
		freArray.aRightFreqList[2]= 1000; 
		freArray.aRightFreqList[3]= 2000;
		freArray.aRightFreqList[4]= 4000;
		freArray.aRightFreqList[5]= 8000;

		SetFrequencyBands(freArray);

	}
	else if(BANDS_CLASSIC_11 == g_bands_type)
	{

		freArray.nLeftFrequencyNum = 11;
		freArray.aLeftFreqList[0]= 250; 
		freArray.aLeftFreqList[1]= 500; 
		freArray.aLeftFreqList[2]= 750; 
		freArray.aLeftFreqList[3]= 1000; 
		freArray.aLeftFreqList[4] = 1500;
		freArray.aLeftFreqList[5] = 2000;
		freArray.aLeftFreqList[6] = 3000;
		freArray.aLeftFreqList[7] = 4000;
		freArray.aLeftFreqList[8] = 6000;
		freArray.aLeftFreqList[9] = 8000;
		freArray.aLeftFreqList[10] = 12000;


		freArray.nRightFrequencyNum = 11;
		freArray.aRightFreqList[0]= 250; 
		freArray.aRightFreqList[1]= 500; 
		freArray.aRightFreqList[2]= 750; 
		freArray.aRightFreqList[3]= 1000;
		freArray.aRightFreqList[4]= 1500;
		freArray.aRightFreqList[5]= 2000;
		freArray.aRightFreqList[6]= 3000;
		freArray.aRightFreqList[7]= 4000;
		freArray.aRightFreqList[8]= 6000;
		freArray.aRightFreqList[9]= 8000;
		freArray.aRightFreqList[10]= 12000;

		SetFrequencyBands(freArray);
		m_initalexchangearry = freArray;

	}
	else if(BANDS_RANGE_NARROWED == g_bands_type)
	{	// m_set_highside
		freArray.nRightFrequencyNum  =  m_highRange[0].nFrequencyNum;
		for(int i = 0 ; i < m_highRange[0].nFrequencyNum; i++)
		{
			freArray.aRightFreqList[i] = m_highRange[0].aFreqList[i];
		}

		freArray.nLeftFrequencyNum  =  m_highRange[1].nFrequencyNum;
		for(int i = 0 ; i < m_highRange[1].nFrequencyNum; i++)
		{
			freArray.aLeftFreqList[i] = m_highRange[1].aFreqList[i];
		}

		SetFrequencyBands(freArray);
	}

	GetFrequencyBands(m_FreArray);

	if(!pDoc->m_IsPTAtested)
	{

		pDoc->m_Rt_list[0].RemoveAll();
		pDoc->m_Lf_list[0].RemoveAll();

		if((pDoc->m_Rt_list[0].GetCount() ==0) && (pDoc->m_Lf_list[0].GetCount() ==0) )
		{
			for(int i = 0 ; i < freArray.nRightFrequencyNum; i++)
			{
				node.band = freArray.aRightFreqList[i];
				node.dbhl = 0;
				pDoc->m_Rt_list[0].AddTail(node);
			}

			for(int i = 0 ; i < freArray.nLeftFrequencyNum; i++)
			{
				node.band = freArray.aLeftFreqList[i];
				node.dbhl = 0;
				pDoc->m_Lf_list[0].AddTail(node);
			}
	     }


	}
//##########################################################################################
}

void CETApplicationView::OnPrepareDC(CDC* pDC,CPrintInfo* pInfo  )
{
	CView::OnPrepareDC(pDC, pInfo);
    if(pDC->IsPrinting())		
	{
		  int cx = GetSystemMetrics(SM_CXSCREEN);
          int cy = GetSystemMetrics(SM_CYSCREEN);

		  int hinch = 0;
		  pDC->SetMapMode(MM_ANISOTROPIC); //转换坐标映射方式 
		  CSize size = CSize(cx, cy);
		  pDC->SetWindowExt(size);
		  //确定窗口大小 

		   hinch = pDC->GetDeviceCaps(HORZSIZE);
		  
		  //得到实际设备每逻辑英寸的象素数量
		  int xLogPixPerInch = pDC->GetDeviceCaps(LOGPIXELSX); 
		  int yLogPixPerInch = pDC->GetDeviceCaps(LOGPIXELSY);

		  //得到设备坐标和逻辑坐标的比例 
//		  long xExt = (long)size.cx * xLogPixPerInch/120 ; 
//		  long yExt = (long)size.cy * yLogPixPerInch/120 ;

		//  int sizex = m_screenxLogPixPerInch * 128 /96;
		//  int sizey = m_screenyLogPixPerInch * 128 /96;

		  long xExt = 1366 * xLogPixPerInch/132;
		  long yExt = 768 * yLogPixPerInch/132;
          pDC->SetViewportExt((int)xExt, (int)yExt);
		
	}

}

void CETApplicationView::OnInitialUpdate( )
{
	//*pOn,*pBand_L,*pMod_R,*pStar_ETF,*pAuse_Lk,*pData_Copy,*pRt_Test,*pXxx_Back;
	//*plistctrL,*plistctrR;
	// default screen mode is 1366 *768

	m_sessionID =  NO_SESSION_ID;

	 //m_Lf_list[3], m_Rt_list[3];
	CDocList *pdoclist;
	//CETApplicationDoc* pDoc = GetDocument();
	//CMainFrame *pDoc = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	//ASSERT_VALID(pDoc);
	//if (!pDoc) return;

	CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;

//	CreateDirectory(_T("C:\\Windows\\System32\\pts\\"),NULL);
	CreateDirectory(GetPTAResultLocalPath(),NULL);
	pDoc->pm_save = this;
	//*************************
	HINSTANCE hinst = ::AfxGetInstanceHandle();
	
	if(hResourceHandle != NULL)
		hinst = hResourceHandle;
	
	OnInitalPTAMP();

	OnInitalHighRangeArray();

	

	TCHAR buff[8][20];
	memset(buff,0,sizeof(TCHAR)*20*8);

	CView::OnInitialUpdate();
	int i = 0;
	int number =0;
	
	CRect rect(0,10,150,45); 

	CRect AdjRect,posRect;


    LoadResourceStr(hinst,IsAmp);


	AdjRect = AutoAdjust(rect);
	//double iscal = 0;
	iscal  = (double)AdjRect.right/(double)rect.right;
	yiscal = (double)AdjRect.Height()/(double)rect.Height();; 

	for(i =0; i<8;i++)
	{
	    pbutt[i] = new CEarButton();

		posRect.left   = 18* iscal *(i+1) + i*AdjRect.Width();

		posRect.right  = posRect.left + AdjRect.Width();
		posRect.top    = AdjRect.top;
		posRect.bottom = posRect.top + AdjRect.Height();

	    pbutt[i]->Create(strButt[i],WS_VISIBLE|WS_CHILD,posRect,this,IDC_BUTTON_ON+i);
	}

	pbutt[1]->EnableWindow(FALSE);
	pbutt[2]->EnableWindow(FALSE);
	pbutt[3]->EnableWindow(FALSE);
	pbutt[4]->EnableWindow(FALSE);
	pbutt[5]->EnableWindow(FALSE);
	pbutt[6]->EnableWindow(FALSE);



	//CRect ListRect(0,384+70,670,384+158);  //683 (8)   128 each item high is 22+ -
	CRect ListRect(0,384+70,670,384+180);
	AdjRect = AutoAdjust(ListRect);
	
	m_storelistrect = AdjRect;

	for(i=0;i<2;i++)    //m_Lf_list[3] 1,  m_Rt_list[3]; 0
	{
			plistctr[i] = new CStableListCtrl();

			posRect.left   =  6*iscal*(i+1) + i*(AdjRect.Width()+4*iscal);
			posRect.right  =  posRect.left + AdjRect.Width();
			posRect.top    =  AdjRect.top;
			posRect.bottom =  posRect.top + AdjRect.Height();

			//plistctrR->Create(WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT,ListRect, this, 520); pPatientList
			plistctr[i]->Create(WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT,posRect, this, 500+i);
			plistctr[i]->SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_AUTOSIZECOLUMNS );

			plistctr[i]->InsertColumn(0,_T("[kHz]"));
			plistctr[i]->SetColumnWidth(0,FIRS_COLUMN_WID*iscal);
			/*plistctr[i]->InsertColumn(1,_T("0.5"));
			plistctr[i]->SetColumnWidth(1,50*iscal);
			plistctr[i]->InsertColumn(2,_T("0.75"));
			plistctr[i]->SetColumnWidth(2,50*iscal);
			plistctr[i]->InsertColumn(3,_T("1"));
			plistctr[i]->SetColumnWidth(3,50*iscal);
			plistctr[i]->InsertColumn(4,_T("1.5"));
			plistctr[i]->SetColumnWidth(4,50*iscal);
			plistctr[i]->InsertColumn(5,_T("2"));
			plistctr[i]->SetColumnWidth(5,50*iscal);
			plistctr[i]->InsertColumn(6,_T("3"));
			plistctr[i]->SetColumnWidth(6,50*iscal);
			plistctr[i]->InsertColumn(7,_T("4"));
			plistctr[i]->SetColumnWidth(7,50*iscal);
			plistctr[i]->InsertColumn(8,_T("6"));
			plistctr[i]->SetColumnWidth(8,50*iscal);
			plistctr[i]->InsertColumn(9,_T("8"));
			plistctr[i]->SetColumnWidth(9,50*iscal);
			plistctr[i]->InsertColumn(10,_T("12"));
			plistctr[i]->SetColumnWidth(10,50*iscal);*/

			 pdoclist = i ? &(pDoc->m_Lf_list[0]) : &(pDoc->m_Rt_list[0]);
			 number = pdoclist->GetCount();
			 int columnlen  = (AdjRect.Width() - FIRS_COLUMN_WID*iscal)/number;
			if(number > 0 )
			{
				for(int j=0;j< number;j++ )
				{   CString str;
				    FREZNODE node;
					
					//columnlen = columnlen -1*iscal ;
					node = pdoclist->GetAt(pdoclist->FindIndex(j));
					str.Format(_T("%4.3f"),node.band/1000);
					//strval.Format(_T("%f"),node.dbhl);
					plistctr[i]->InsertColumn(j+1,str);
					plistctr[i]->SetColumnWidth(j+1,columnlen); //50*iscal);
					
					//plistctr[i]->SetItemText();

				}
			}

		
			plistctr[i]->InsertItem(0,_T("1)"));   //PTA1
			plistctr[i]->InsertItem(1,_T("2)"));   //PTA2
			plistctr[i]->InsertItem(2,_T("3)"));   //PTA3
			plistctr[i]->InsertItem(3,m_buffave);  //_T("Average"));

			plistctr[i]->SetBkColor(BKCOLOR);
			plistctr[i]->SetTextBkColor(BKCOLOR);

			for(int k =0 ;k <3; k++)   //set value
			{ 
				CString strval;
				FREZNODE nodeval;
				pdoclist = i ? &(pDoc->m_Lf_list[k]) : &(pDoc->m_Rt_list[k]);
		
				for(int m=0;m< number; m++)
				{
					if(pdoclist->FindIndex(m))
					{
						nodeval = pdoclist->GetAt(pdoclist->FindIndex(m));
						strval.Format(_T("%3.1f"),nodeval.dbhl);
						plistctr[i]->SetItemText(k,m+1,strval);
					}
				}
			}
						
	}

	        /*plistctr[0]->SetItemText(0,1,_T("3"));
			plistctr[0]->SetItemText(0,2,_T("1"));
			plistctr[0]->SetItemText(0,3,_T("8"));
			plistctr[0]->SetItemText(0,4,_T("3"));
			plistctr[0]->SetItemText(0,5,_T("5"));
			plistctr[0]->SetItemText(0,6,_T("18"));
			plistctr[0]->SetItemText(0,7,_T("18"));
			plistctr[0]->SetItemText(0,8,_T("85"));
			plistctr[0]->SetItemText(0,9,_T("10"));
			plistctr[0]->SetItemText(0,10,_T("15"));

		    plistctr[0]->SetItemText(1,1,_T("1"));
			plistctr[0]->SetItemText(1,2,_T("-4.5"));
			plistctr[0]->SetItemText(1,3,_T("4"));
			plistctr[0]->SetItemText(1,4,_T("12"));
			plistctr[0]->SetItemText(1,5,_T("12"));
			plistctr[0]->SetItemText(1,6,_T("20"));
			plistctr[0]->SetItemText(1,7,_T("15"));
			plistctr[0]->SetItemText(1,8,_T("70"));
			plistctr[0]->SetItemText(1,9,_T("20"));
			plistctr[0]->SetItemText(1,10,_T("12"));

		    plistctr[1]->SetItemText(0,1,_T("15"));
			plistctr[1]->SetItemText(0,2,_T("8"));
			plistctr[1]->SetItemText(0,3,_T("16"));
			plistctr[1]->SetItemText(0,4,_T("15"));
			plistctr[1]->SetItemText(0,5,_T("18"));
			plistctr[1]->SetItemText(0,6,_T("28"));
			plistctr[1]->SetItemText(0,7,_T("35"));
			plistctr[1]->SetItemText(0,8,_T("30"));
			plistctr[1]->SetItemText(0,9,_T("65"));
			plistctr[1]->SetItemText(0,10,_T("42"));*/

			psysfont = plistctr[0]->GetFont();

	//CRect ListPatiRect(0,548,671,548+86);
	//CRect ListPatiRect(0,588,671,588+90);  latest
			//CRect ListPatiRect(0,610,671,610+90);
			CRect ListPatiRect(0,590,671,590+90);
	        AdjRect = AutoAdjust(ListPatiRect);

	for(i=0;i<2;i++)
	{
		    pPatientList[i] = new CEditListCtrl();

		 	posRect.left   =  6*iscal*(i+1) + i*(AdjRect.Width()+2*iscal);
			posRect.right  =  posRect.left + AdjRect.Width();
			posRect.top    =  AdjRect.top;
			posRect.bottom =  posRect.top + AdjRect.Height();
			pPatientList[i]->Create(WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT|LVS_SINGLESEL,posRect, this, !i ? IDC_LIST_PATIENTDATA :IDC_LIST_PATIENTHST );
			pPatientList[i]->SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_AUTOSIZECOLUMNS );
			pPatientList[i]->SetBkColor(BKCOLOR);
			pPatientList[i]->SetTextBkColor(BKCOLOR);
	 
	}
	        strprompt = m_bufstrprompt; //_T("Pls Input ID ...");
	        
			pPatientList[0]->InsertColumn(0,m_buffPatData); //_T("PATIENT DATA"));
			pPatientList[0]->InsertColumn(1,_T(""));
			pPatientList[0]->InsertColumn(2,_T(""));
			pPatientList[0]->InsertColumn(3,_T(""));
		//	pPatientList[0]->InsertColumn(4,_T(""));
			
			int itemlen = (664/4)*iscal;       //default len is 670  def high of subitem is 21   //2011_Nov_09 update for adjust len of column
			int itemlenpat = (664/6) * iscal;
			pPatientList[0]->SetColumnWidth(0,itemlenpat);
			pPatientList[0]->SetColumnWidth(1,itemlenpat*2); //itemlen);
            pPatientList[0]->SetColumnWidth(2,itemlenpat);
			pPatientList[0]->SetColumnWidth(3,itemlenpat *2); //itemlen);

		//	pPatientList[0]->ShowScrollBar(SB_HORZ,0);
			
	//		TCHAR        m_buffstrname[10];
	//   TCHAR        m_buffstrage[10];
	  // TCHAR        m_buffstrid[10];
	 //  TCHAR        m_buffstradd[10];
	//  TCHAR        m_buffstrgender[10];
			
			pPatientList[0]->InsertItem(0,m_buffstrid);//_T("ID"));
			pPatientList[0]->InsertItem(1,m_buffstrage);//_T("Age"));
			pPatientList[0]->InsertItem(2,m_buffstradd );//_T("Name"));
		
			

			//pPatientList[0]->SetItemText(0,1,strprompt); //_T("...Pls input ..."));
			pPatientList[0]->SetItemText(0,1,_T(""));

			pPatientList[0]->SetItemText(0,2,m_buffstrname );//_T("Gender"));
			pPatientList[0]->SetItemText(1,2,m_buffstrgender);//_T("Address"));
			//pPatientList[0]->SetItemText(2,2,_T("Insurance No."));
			

			pPatientList[1]->InsertColumn(0,m_buffstrSysstat);        //_T("SYSTEM STATUS"));
			pPatientList[1]->InsertColumn(1,_T(""));
			pPatientList[1]->InsertColumn(2,_T(""));
			pPatientList[1]->InsertColumn(3,_T(""));

			pPatientList[1]->SetColumnWidth(0,itemlen);
			pPatientList[1]->SetColumnWidth(1,itemlen);
            pPatientList[1]->SetColumnWidth(2,itemlen);
			pPatientList[1]->SetColumnWidth(3,itemlen);

			pPatientList[1]->InsertItem(0,_T("AudioMed"));
			pPatientList[1]->InsertItem(1,m_buffstrpushbutt);   //_T("Push Button"));
			pPatientList[1]->InsertItem(2,m_buffstrMic); ///_T("Microphone"));

			//pPatientList[1]->SetItemText(0,2,_T("4th visit"));
			//pPatientList[1]->SetItemText(1,2,_T("5th visit"));
			//pPatientList[1]->SetItemText(2,2,_T("6th visit"));

			pPatientList[1]->SetItemText(0,1,m_buffstrunlink);  //_T("  UnLinked"));
			pPatientList[1]->SetItemText(1,1,_T("  --------  "));
			GetMicPhoneStat();
			bmplogo.LoadBitmapW(IDB_BMP_LOGO);

			//pPatientList[1]->SetItemText(2,1,_T("  OFF "));
			//pPatientList[1]->SetItemText(2,3,_T("  [ Alt + C ] to \"ON\" "));

		    CRect mainrect(0,40,683,384+70-8);
		    AdjRect = AutoAdjust(mainrect);

		    SetMainRect(AdjRect.left,AdjRect.top,AdjRect.right,AdjRect.bottom); 

			//storefont = font;

			int xoff = 28*iscal;
			int yoff = 140*yiscal;
			int linelen = AdjRect.Width() - 48*iscal;

			CRect highRect,hposRect;
			CRect hbtnrect(0,0,30,20);
			highRect = AutoAdjust(hbtnrect);
			yoffset = m_MainDispRect.top + 100*yiscal;
			for(int idh =0 ;idh <2; idh++)   //pHighRangeBtn
			{
				
				pHighRangeBtn[idh] = new CEarButton();
				hposRect.left   = 28*iscal + linelen - 26*iscal + idh*683*iscal;
				hposRect.right  = hposRect.left + highRect.Width();
				hposRect.top    = yoffset - 46 * yiscal;
				hposRect.bottom = hposRect.top + highRect.Height();

				pHighRangeBtn[idh]->Create(_T("H"),WS_VISIBLE|WS_CHILD,hposRect,this,IDC_BUT_HIGH_RT + idh * 10);
				pHighRangeBtn[idh]->EnableWindow(FALSE);
				;

			}


#ifndef _DEBUG
			SetMicPhoneStat(TRUE); //firs start
#endif

			StoreDataInList();

			DisplayAMPElement(xoff,yoff,linelen );

			HDEVNOTIFY hDeviceNotify;
			if(true == DoRegisterDeviceInterfaceToHwnd(GUID_CLASS_USB_DEVICE,this->m_hWnd,&hDeviceNotify))
			{
				m_hDeviceNotify = hDeviceNotify;
			}

#ifndef _DEBUG
			SetMuteSysVol(true);
			SetMicPhoneStat(); 
#endif

			CDlgSelfTest  dlgTest(this);
			dlgTest.DoModal();

#ifndef _DEBUG
			SetMicPhoneStat(); 
#endif

}

bool CETApplicationView::DoRegisterDeviceInterfaceToHwnd( 
     GUID InterfaceClassGuid, 
     HWND hWnd,
     HDEVNOTIFY *hDeviceNotify 
)
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = InterfaceClassGuid;

    *hDeviceNotify = RegisterDeviceNotification( 
        hWnd,                       // events recipient
        &NotificationFilter,         // type of device
        DEVICE_NOTIFY_WINDOW_HANDLE   // type of recipient handle
        );

    if ( NULL == *hDeviceNotify ) 
    {
        // ErrorHandler(TEXT("RegisterDeviceNotification"));
        return FALSE;
    }

    return TRUE;
}
void CETApplicationView::StoreDataInList()
{
	int linewidth;
	xoffset = 28*iscal;
	yoffset = m_MainDispRect.top + 100*yiscal;
	linewidth = m_MainDispRect.Width()-48*iscal; 
	//*********************************
	CString str;
	float   xPosVal=0,yPosVal=0;
	int i=0, j=0,id=0;
	TCHAR strbuff[10];
		
	LVCOLUMN lvcolmn;
	lvcolmn.mask = LVCF_TEXT;
	lvcolmn.pszText = strbuff;
	lvcolmn.cchTextMax = 8;
	memset(strbuff,0,10*sizeof(TCHAR));

	int num = 0; 

	LNNODE *plineNode , node;
	AMPNODE ampnode;
		
	plineNode = NULL;

	for(id =0 ; id<=1;id++)         //0right  1left   
	{
		      num	= plistctr[id]->GetHeaderCtrl()->GetItemCount() -1;
			  if (num !=0 )
					gap[id] = linewidth /num;
			  else
					gap[id] = linewidth;

	
			   this->m_AMP_List[id].RemoveAll();
			 
			   plineNode = new LNNODE [num];

                for(j=0;j<=2;j++)              //nItem
				{    
					    if(id == 1)
						    this->m_postL_list[j].RemoveAll();          
					    if(id ==0)
						    this->m_postR_list[j].RemoveAll();

						for(i=0;i <num;i++)
						{
							str = plistctr[id]->GetItemText(j,i+1);
							if(!str.GetLength())
								break;
							plistctr[id]->GetColumn(i+1,&lvcolmn);    //26
							plineNode[i].y = _ttof(str);              //dbhl value
							plineNode[i].x = _ttof(strbuff);          //band

							if(j==0)
							{
								ampnode.xband = plineNode[i].x * 1000;
								ampnode.ydbhr = 0;                    //inital all dbhr val in amp to 0
								ampnode.IsSet = FALSE;
								this->m_AMP_List[id].AddTail(ampnode);
							}

							xPosVal = xoffset + gap[id]*(i+1) +  683*iscal*id;

							yPosVal = plineNode[i].y * 2.6 * yiscal;
							yPosVal = yoffset + 26*yiscal+ yPosVal;   //0 is base line

							node.x = xPosVal;
							node.y = yPosVal;

							if(id == 0)
								this->m_postR_list[j].AddTail(node);

							if(id == 1)
								this->m_postL_list[j].AddTail(node);

						   }
				}
	}
	if(plineNode != NULL)
	{
		delete [] plineNode;
		plineNode = NULL;
	}

}



void  CETApplicationView::LoadResourceStr(HINSTANCE hinst,BOOL mod)
{
	if(hinst == NULL)
	   return;

	if(mod)
	{
		LoadString(hinst,IDS_BUT_AMP_ON,m_buffarr[0],32);  //
		LoadString(hinst,IDS_BUT_AMP_BAND,m_buffarr[1],32);
		LoadString(hinst,IDS_BUT_AMP_MOD,m_buffarr[2],32);
		LoadString(hinst,IDS_BUT_AMP_START,m_buffarr[3],32); //

		LoadString(hinst,IDS_BUT_AMP_PAUSE,m_buffarr[4],32);
		LoadString(hinst,IDS_BUT_AMP_DATA ,m_buffarr[5],32);
		LoadString(hinst,IDS_BUT_AMP_PRINT ,m_buffarr[6],32);
		LoadString(hinst,IDS_BUT_AMP_GOTO ,m_buffarr[7],32);
	}
	else
	{
		LoadString(hinst,IDS_BUT_ON,m_buffarr[0],32);  //
		LoadString(hinst,IDS_BUT_BAND,m_buffarr[1],32);
		LoadString(hinst,IDS_BUT_MOD,m_buffarr[2],32);
		LoadString(hinst,IDS_BUT_STARTS,m_buffarr[3],32); //

		LoadString(hinst,IDS_BUT_PAUSE,m_buffarr[4],32);
		LoadString(hinst,IDS_BUT_DATA,m_buffarr[5],32);
		LoadString(hinst,IDS_BUT_PRINT,m_buffarr[6],32);
		LoadString(hinst,IDS_BUT_GOTO,m_buffarr[7],32);
		strprt = m_buffarr[6];

	}

		LoadString(hinst,IDS_BUT_AMP_PLAYSTOP,m_buffplaystop,32); 

		strButt[0] =  m_buffarr[0];         //on 
		strButt[1] =  m_buffarr[1];         //band
		strButt[2] =  m_buffarr[2];         //mode
		strButt[3] =  m_buffarr[3];         //start
		strButt[4] =  m_buffarr[4];         //pause
		strButt[5] =  m_buffarr[5];         //data
		strButt[6] =  m_buffarr[6];         //_T("PRINT");
		strButt[7] =  m_buffarr[7];         //go to

#ifdef SUPPORT_SITE_TREATMENT
//		if(mod &&(LINKED != m_LinkState))
//			strButt[5] = m_buffplaystop;
#endif

		LoadString(hinst,IDS_PRT_FILEEND,m_buffprtbotm,256); 
		LoadString(hinst,IDS_STR_FILEEND2,m_buffprtbotm2,256); 

		LoadString(hinst,IDS_STRING_SIDE_RT,m_buffstrside[0],20); 
		LoadString(hinst,IDS_STRING_SIDE_LF,m_buffstrside[1],20); 

		LoadString(hinst,IDS_LISTCTL_PAT_NAME,m_buffstrname,10);
		LoadString(hinst,IDS_LISTCTL_PAT_ID,m_buffstrid,10);
		LoadString(hinst,IDS_LISTCTL_PAT_AGE,m_buffstrage,10);
		LoadString(hinst,IDS_LISTCTL_PAT_ADD,m_buffstradd,10);
		LoadString(hinst,IDS_LISTCTL_PAT_GENDER,m_buffstrgender,12);  //
		LoadString(hinst,IDS_STRING_PROMPT,m_bufstrprompt,64);

		LoadString(hinst,IDS_STR_SYS_STAT,m_buffstrSysstat,20);  //sys stat
		LoadString(hinst,IDS_STR_PUSHBUTT,m_buffstrpushbutt,20);  //pushbutton
        LoadString(hinst,IDS_STR_MIC,m_buffstrMic,20);          //mic
		LoadString(hinst,IDS_STR_UNLINK,m_buffstrunlink,20);     //unlink
		LoadString(hinst,IDS_STR_LINKING,m_buffstrlinking,20);   //linking
		LoadString(hinst,IDS_STR_LINKED,m_buffstrlinked,20);     //linked
		LoadString(hinst,IDS_STR_MICSTAT_OFF,m_buffstroff,20);  ///mic off
		LoadString(hinst,IDS_STR_MICSTAT_ON,m_buffstron,20);    //mic on
		LoadString(hinst,IDS_STR_ASSGN_PROMP,m_buffassign,256);
		LoadString(hinst,IDS_STR_AVERAGE,m_buffave,18);
		LoadString(hinst,IDS_STR_AVERAGE1,m_buffave1,18);
		LoadString(hinst,IDS_STR_AVERAGE2,m_buffave2,18);
		LoadString(hinst,IDS_STR_DELTA,m_delta,18);
		LoadString(hinst,IDS_STR_CONTROL,m_buffcontrol,12);
		LoadString(hinst,IDS_STR_RESUME,m_buffresume,12);
		LoadString(hinst,IDS_STR_CANCEL,m_buffcancel,12);
		LoadString(hinst,IDS_STR_APPLY,m_buffapply,12);
		LoadString(hinst,IDS_STR_STOPPTA,m_bufstopta,128);
		LoadString(hinst,IDS_STR_NO_PTA , m_buffnopta,32);
		LoadString(hinst,IDS_STR_ASS_FAIL,m_buffassfail,32);
		LoadString(hinst,IDS_STR_WARNING_ID ,m_buffwarnid,32);
		LoadString(hinst,IDS_STR_FILE_GENED , m_buffstpgen,24);

		LoadString(hinst,IDS_STR_NEW_USER_SAVE, m_buffNewUserSaved,64);

		LoadString(hinst,IDS_STR_TREATMENT_STOP, m_buffTreatmentStop,128);
		LoadString(hinst,IDS_STR_TREATMENT_PLAY, m_buffTreatmentPlay,128);

		LoadString(hinst,IDS_STR_AMD_LINKED ,m_buffAMDlinked,32);
		LoadString(hinst,IDS_STR_FRE_SELECT_LIMITATION ,m_buffFreSelLimit,256);
		LoadString(hinst,IDS_STR_PROFILE_SUCCESS ,m_buffProfileSuccess,128);
		LoadString(hinst,IDS_LISTCTRL_STR_PATDAT,m_buffPatData,24);
		LoadString(hinst,IDS_STR_TREAT_SUPPORT,m_buffamptreat,32);
		LoadString(hinst,IDS_BUT_STOP,m_buffStop,12);

		LoadString(hinst,IDS_BUT_MAN_CANCEL,m_buffManCancel,12);
		LoadString(hinst,IDS_BUT_MAN_FINISH,m_buffManFinish,12);

		LoadString(hinst,IDS_STRING_AMPGOOD1 ,m_buffAMPGood1,256);
		LoadString(hinst,IDS_STRING_AMPGOOD2 ,m_buffAMPGood2,256);
		LoadString(hinst,IDS_STRING_AMPGOOD3 ,m_buffAMPGood3,256);
		LoadString(hinst,IDS_STRING_AMPBAD1 ,m_buffAMPBad1,256);
		LoadString(hinst,IDS_STRING_AMPBAD2 ,m_buffAMPBad2,256);
		LoadString(hinst,IDS_STRING_AMPBAD3 ,m_buffAMPBad3,256);

		//LoadString(hinst,IDS_STRING_AMPASK,m_buffAMPAsk,128);

		LoadString(hinst,IDS_STR_WARN,m_buffWarning,12);

		LoadString(hinst,IDS_STR_PRMP_TIMER,m_buffTimer,12);

		LoadString(hinst,IDS_STRING_PTANOSAVE_WARNING,m_buffPTANoSaveWarning,256);
		LoadString(hinst,IDS_STRING_WARNING_NOT_FINISH,m_buffNoFinishWarning,256);

		LoadString(hinst,IDS_STR_GENBEFORE_DOWN,m_buffgenfirst,128);
		LoadString(hinst,IDS_STR_CANT_LOADMULTPTA,m_buffcantloadmul,128);
   

}
void CETApplicationView::SetMainRect(int l,int t,int r,int b)
{
	m_MainDispRect.left   = l;
	m_MainDispRect.top    = t;
	m_MainDispRect.right  = r;
	m_MainDispRect.bottom = b;

}
void CETApplicationView::DrawWaveTable(CDC* pDC)
{      //each hight of row is 26, the title hight is 100,end hight is 20 ,total is 406
	   
		CDC *pdc;
		CFont numfont, charfont, buttonfont;
		CRect bondcircle,bondcross;
		CDC memdc;
		int num = 0;
		CString str;
		TCHAR strcol[10];

		int i =0, j=0,offset =0;

		LVCOLUMN lvcolmn;
		lvcolmn.mask = LVCF_TEXT;
		lvcolmn.pszText = strcol;
		lvcolmn.cchTextMax = 8;
		memset(strcol,0,10*sizeof(TCHAR));
	
	    CFont *pbuttonfont;
		pbuttonfont = plistctr[0]->GetFont();
	    pbuttonfont->GetLogFont(&font);
		font.lfHeight = 20;                   //button font size
		font.lfWeight = FW_MEDIUM;
	    storefont = font;

		buttonfont.CreateFontIndirectW(&storefont);
		//pdc = this->GetDC();       //
		pdc= pDC;

		CPen pen(PS_SOLID,2,RGB(148,139,128));
		pdc->SelectObject(&pen);

		xoffset = 28*iscal;
		yoffset = m_MainDispRect.top + 100*yiscal;
		linewidth = m_MainDispRect.Width()-48*iscal;       //(ahead distance is 28,after distance is 20)
   
	  // font = storefont;
	   psysfont = plistctr[0]->GetFont();
	   psysfont->GetLogFont(&font);
	   numfont.CreateFontIndirectW(&font);
	   pdc->SelectObject(&numfont);
	  
    for(j=0; j< 2; j++)
	{
		offset = j*683*iscal;
		xoffset += offset;
		for(i=0;i<12;i++)
		{
			pdc->MoveTo(CPoint(xoffset,yoffset + i*26*yiscal));
			pdc->LineTo(CPoint(xoffset + linewidth ,yoffset + i*26*yiscal));

			str.Format(_T("%d"),(i*10 - 10));
			if(i != 11)
			   pdc->TextOutW(xoffset-22*iscal,yoffset+ (i*26 - 8)*yiscal ,str);
		}

		   pdc->MoveTo(CPoint(xoffset,yoffset));
		   pdc->LineTo(CPoint(xoffset,yoffset+286*yiscal));

		   num = plistctr[j]->GetHeaderCtrl()->GetItemCount() -1;
		  if (num !=0 )
			    gap[j] = linewidth /num;
		  else
			    gap[j] = linewidth;

		  for(int s=0; s<num ;s++)  //plistctr
		  {
			 BOOL longnode = FALSE;
			  plistctr[j]->GetColumn(s+1,&lvcolmn);

			  if(!m_diaryMod)
			  {
				  if(s == num-1)
					 pdc->TextOutW(xoffset+gap[j]*(s+1) -14*iscal,yoffset-26*yiscal,strcol ); 
				  else
					 pdc->TextOutW(xoffset+gap[j]*(s+1) -8*iscal,yoffset-26*yiscal,strcol ); 
			  }
			  else
			  {
				   
				  if(StrCmpI(strcol,_T("0.000")))  //not equal str
				  {
					  //m_d_TimeList.AddTail(strdtime);


					  CString strtime;
					  if( m_d_TimeList.GetCount() == 0)
					  {
						  break;
					  }

					  strtime = m_d_TimeList.GetAt(m_d_TimeList.FindIndex(s));

					  CString strtmpcol = strcol;
					  strtmpcol.Delete(strtmpcol.GetLength() - 4,4);

					  if(!strtmpcol.Compare(_T("-1")))
						  strtmpcol = _T("0");

					  if(strtime.GetLength() !=0 )
					  {
						  strtime = _T(" (") + strtime;
						  strtime = strtime + _T(")");
						  strtmpcol = strtmpcol + strtime;
						  
					  }
					  longnode = TRUE;
					  pdc->TextOutW(xoffset+gap[j]*(s+1) -8*iscal,yoffset-26*yiscal,strtmpcol );
				  }
			  }

			  pdc->MoveTo(CPoint(xoffset+ gap[j] * (s+1),yoffset));
			  pdc->LineTo(CPoint(xoffset+ gap[j] * (s+1),yoffset - 4* yiscal));
			  if(longnode)
				  pdc->LineTo(CPoint(xoffset+ gap[j] * (s+1),yoffset - 9* yiscal));
		  }

	}
	    xoffset -= offset;                 //recover offset

		//pdc->TextOutW(xoffset-22*iscal,yoffset+ (i*26 - 8)*yiscal ,str);
		if(!m_diaryMod)
		{
			pdc->TextOutW(xoffset - 20 *iscal, yoffset - 26 * yiscal, IsAmp ? _T("")  : _T("dBHL") );  //dBr
			pdc->TextOutW(xoffset - 20 *iscal + 22*iscal , yoffset - 26 * yiscal - 16*yiscal,_T("kHz"));
			pdc->MoveTo(xoffset - 20*iscal,yoffset - 36 * yiscal);
			pdc->LineTo(xoffset - 20*iscal + 50*iscal,yoffset - 14 *yiscal);

			
		}
		if(m_isPTATesting && pbutt[3]->pressed) 
		{
			if(m_PTAPaused)
			{
			   pdc->TextOutW(1270*iscal,704*yiscal,strpauseTime); 
			}
			else
			   pdc->TextOutW(1270*iscal,704*yiscal,strTime);  
		}
		if(!pDC->IsPrinting())
		{
			if(m_ShowWarning)
			{
				pdc->TextOutW(600*iscal,704 *yiscal ,strWarning + m_strwarnct);
			}
		}


#ifdef SUPPORT_SITE_TREATMENT
		if(IsAmp&&g_bIsLocalPlaying)
		{
			pdc->TextOutW(1270*iscal,704*yiscal,strTime);  
		}
		if(!pDC->IsPrinting())
		{
			if(m_treatstage && IsAmp)
			{
				pdc->TextOutW(600*iscal,704 *yiscal ,m_buffamptreat);  //_T("Pre-treatment Supported State                 "));
			}
		}
#endif

		if( strendTime.Compare(_T("")) )  //endpta      //dwEndTime)
		{
			     pdc->TextOutW(1270*iscal,704*yiscal,strTime);
		}

		BITMAP bmpstr ;
		CRect  scnrect;
		int cx,cy;
		memset(&bmpstr,0,sizeof(BITMAP));
		bmplogo.GetBitmap(&bmpstr);
		this->GetClientRect(scnrect);
		memdc.CreateCompatibleDC(GetDC());
		memdc.SelectObject(&bmplogo);
		cy = scnrect.bottom - 680*yiscal;
		pdc->BitBlt(10*iscal,682*yiscal,bmpstr.bmWidth*iscal,bmpstr.bmHeight*yiscal,&memdc,0,0,SRCCOPY);
		//pdc->StretchBlt(10*iscal,690*yiscal,bmpstr.bmWidth*iscal,cy - 20*yiscal ,&memdc,0,0,bmpstr.bmWidth,bmpstr.bmHeight,SRCCOPY);

		font.lfHeight = 36;
		font.lfWeight = FW_BOLD;
	    charfont.CreateFontIndirectW(&font);

	    pdc->SelectObject(&charfont);
		pdc->SetTextColor(RT_RED_COLOR );

		pdc->DrawText(m_buffstrside[0],CRect(0,m_MainDispRect.top+10*yiscal,m_MainDispRect.right,100*yiscal),DT_CENTER|DT_VCENTER|DT_SINGLELINE);

		bondcircle.left = m_MainDispRect.Width()*3/5;
		bondcircle.right = bondcircle.left+ 18*iscal;
		bondcircle.top   = m_MainDispRect.top +20*yiscal+6*yiscal;
		bondcircle.bottom = bondcircle.top + 18*yiscal;

		CPen rpen(PS_SOLID,2,RT_RED_COLOR );
		pdc->SelectObject(&rpen);
		pdc->Ellipse(bondcircle);

		pdc->SetTextColor(LF_BLUE_COLOR);
		pdc->DrawText(m_buffstrside[1],CRect(683*iscal,m_MainDispRect.top+10*yiscal,m_MainDispRect.right+683*iscal,100*yiscal),DT_CENTER|DT_VCENTER|DT_SINGLELINE);

		bondcross.left = bondcircle.left + 683*iscal;
		bondcross.right = bondcross.left + 18*iscal;
		bondcross.top   = bondcircle.top;
        bondcross.bottom = bondcircle.bottom;

		pdc->SelectObject(&buttonfont);
		CPen lpen(PS_SOLID,2,LF_BLUE_COLOR);
		pdc->SelectObject(&lpen);
		pdc->MoveTo(bondcross.left,bondcross.top);
		pdc->LineTo(bondcross.right,bondcross.bottom);

		pdc->MoveTo(bondcross.left,bondcross.bottom);
		pdc->LineTo(bondcross.right,bondcross.top);

	//	pdc->TextOutW(50,120, _itot(m_MainDispRect.Width(),buff,10));
	//	pdc->TextOutW(70,160, _itot(m_MainDispRect.Height(),buff,10));
	//	charfont.DeleteObject();

		DrawBkLine(pdc);
		//this->ReleaseDC(pdc);
}

void CETApplicationView::DisplayAMPElement(int xoffset,int yoffset,int linelen)
{
	//*pRveLeft, *pRveRight;
	//*pSliderLf, *pSliderRt;

	CRect rect,sldrect,fontrect;
	RECT brect,thumbrect; 

	CString str;
	int offset = 0,i=0,id = 0 ;
	long offlens;
	int num = 0; 

   for(id =0;id <2; id++)
   {
	   	   offset = id*683*iscal;
		   xoffset += offset;

		   num  = plistctr[id]->GetHeaderCtrl()->GetItemCount() -1;
		   if(num > 0 )
			   gap[id] = linelen / num;
		   else
			   gap[id] = linelen;

		   if(id ==  1)
		   {
			  pRveLeft = new CEarButton[num];
			  pSliderLf = new CEarSliderCtrl[num];
			  pfontLf =  new CEarStatic[num];
		   }
		   if(id == 0)
		   {
			  pRveRight = new CEarButton[num];
			  pSliderRt = new CEarSliderCtrl[num];
			  pfontRt   = new CEarStatic[num];
		   }

			for(i=0;i<num;i++)
			{
				rect.left = xoffset+ gap[id]*(1+i) -20*iscal;
				rect.right = rect.left + 40*iscal;
				rect.top = yoffset-46*yiscal;
				rect.bottom = rect.top + 20*yiscal;
				str.Format(_T("%d"),i+1);
				if(id == 1)
				{
					(pRveLeft[i]).Create(str,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON ,rect,this,IDC_SUB_BUTT_LF + i );   //2050 +i
					(pRveLeft[i]).EnableWindow(FALSE);
					(pRveLeft[i]).ShowWindow(FALSE);
				}
				if(id == 0)
				{
					(pRveRight[i]).Create(str,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON ,rect,this,IDC_SUB_BUTT_RT + i );
					(pRveRight[i]).EnableWindow(FALSE);
					(pRveRight[i]).ShowWindow(FALSE);

				}
				//pslid->Create(TBS_VERT|TBS_NOTICKS|TBS_BOTH|WS_CHILD|WS_VISIBLE,CRect(430,30,460,250),this,230);

				sldrect = rect;
				sldrect.top = yoffset;
				sldrect.bottom = sldrect.top + (26*SLIDERGAP)*yiscal;
				//sldrect.right = sldrect.left + rect.Width()/3;
				sldrect.left += rect.Width()/5;
				sldrect.right -= rect.Width()/5; 
		

				if(id ==1 )
				{
					(pSliderLf[i]).Create(TBS_VERT|TBS_NOTICKS|TBS_BOTH|TBS_FIXEDLENGTH|WS_CHILD,sldrect,this,IDC_SLIDER_LF + i);
					(pSliderLf[i]).SetThumbLength(rect.Width()*3/5 - 2);
					(pSliderLf[i]).GetThumbRect(&thumbrect);
					 offlens = thumbrect.top;
					 (pSliderLf[i]).GetWindowRect(&brect);
					 ScreenToClient(&brect);
					 (pSliderLf[i]).MoveWindow(brect.left,yoffset /*+26*yiscal*/ - offlens,brect.right -brect.left,brect.bottom -brect.top);
				}
				if(id ==0 )
				{
					(pSliderRt[i]).Create(TBS_VERT|TBS_NOTICKS|TBS_BOTH|TBS_FIXEDLENGTH|WS_CHILD,sldrect,this,IDC_SLIDER_RT +i );
					(pSliderRt[i]).SetThumbLength(rect.Width()*3/5 - 2);
					(pSliderRt[i]).GetThumbRect(&thumbrect);
					 offlens = thumbrect.top;
					 (pSliderRt[i]).GetWindowRect(&brect);
					 ScreenToClient(&brect);
					 (pSliderRt[i]).MoveWindow(brect.left,yoffset /*+26*yiscal*/ - offlens,brect.right -brect.left,brect.bottom -brect.top);
				}


				 fontrect = sldrect;
				 fontrect.left  = sldrect.left - rect.Width()/2 - 7*iscal;
				 fontrect.right = fontrect.left + rect.Width()/2 + 7 *iscal;
				 fontrect.top   = sldrect.top+ 1; // +26*yiscal;
				 fontrect.bottom = fontrect.top + rect.Height()-1;
				 if(id == 1)
				    pfontLf[i].Create(_T("0"),WS_CHILD,fontrect,this,IDC_STATIC_FONT_L + i);  //SS_SIMPLE
				 if(id == 0)
				    pfontRt[i].Create(_T("0"),WS_CHILD,fontrect,this,IDC_STATIC_FONT_R  + i);  
			}
			
   }
   m_fontlen = fontrect.right - fontrect.left;
              // pRveLeft[i-1].EnableWindow(FALSE);
}

//int g_times =0;
void CETApplicationView::DrawBkLine(CDC* pdc)
{

	CString str;
	CNewList *plist;
	float   xlen[2]={0,0},ylen[2]={0,0};
	float   amp_x[2]={0,0}, amp_y[2]={0,0};
	float   neederased_x[3]={0,0,0},neederased_y[3]={0,0,0};
	int i=0, j=0,id=0;
	TCHAR strbuff[10];
	LNNODE updateNode;
	
	int prenum = 0;
	int premode = pdc->GetROP2();
	int nums[3] = {0,0,0};

	LVCOLUMN lvcolmn;
	lvcolmn.mask = LVCF_TEXT;
	lvcolmn.pszText = strbuff;
	lvcolmn.cchTextMax = 8;
	memset(strbuff,0,10*sizeof(TCHAR));

	LOGBRUSH logBrush1 ,logBrush2;   
	logBrush1.lbStyle=BS_SOLID;   
	logBrush1.lbColor=RGB(160,40,30);   
	//CPen pen(PS_DASH,nWidth,&logBrush); 
	logBrush2.lbStyle=BS_SOLID;   
	logBrush2.lbColor=RGB(40,30,160); 

	CPen pen[2]={CPen(PS_SOLID,2,RGB(208,39,28)),CPen(PS_SOLID,2,RGB(28,28,208))};
    //CPen dashpen[2]={CPen(PS_DASH,1,RGB(160,40,30)),CPen(PS_DASH,1,RGB(40,30,160))};
	CPen dashpen[2]={CPen(PS_DASH,1,&logBrush1),CPen(PS_DASH,1,&logBrush2)};

	//CPen dotpen[2]= {CPen(PS_DOT,1,RGB(180,40,40)),CPen(PS_DOT,1,RGB(40,40,180))};
	CPen dotpen[2]= {CPen(PS_DOT,1,&logBrush1),CPen(PS_DOT,1,&logBrush2)};

	 int num  =0 ;
	  //= plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;
	 LNNODE *plineNode, retNode, ampNode;
	 // plineNode = new LNNODE [num];

		for(id =0 ; id<=1;id++)         //0Right  1Left
		{
			num = plistctr[id]->GetHeaderCtrl()->GetItemCount() -1;
			//plineNode = new LNNODE [num];
		    if(m_diaryMod)
			{
				CString strdate;
				plist = id ? &(this->m_postL_list[j]) : &(this->m_postR_list[j]);
				if(plist->GetCount() == 0 )
				   continue;
				for(i =0; i<num -1; i++)
				{
					int offset = 1;
					pdc->SelectObject(&pen[id]); 
					if(m_d_TimeList.GetCount() == 0)
					{
						break;
					}

					strdate = m_d_TimeList.GetAt(m_d_TimeList.FindIndex(i));
					if(strdate.Compare(_T("")))
					{
					    retNode = plist->GetAt(plist->FindIndex(i));  //yoffset + 26*yiscal+ 0 m_d_TimeList
						xlen[0] = retNode.x;
						ylen[0] = retNode.y;
					}
					
					while(m_d_TimeList.GetAt(m_d_TimeList.FindIndex(i + offset)).Compare(_T("")) == 0)  //  no treatment day
					{
						 pdc->SelectObject(&dotpen[id]);
						 offset++;
						 if((i + offset) > (DIARYNUM-1))
							 break;
					}
					if((i + offset) <= (DIARYNUM-1))
					    strdate = m_d_TimeList.GetAt(m_d_TimeList.FindIndex(i+offset));
					else
						break;

					if(strdate.Compare(_T("")))
					{
					    retNode = plist->GetAt(plist->FindIndex(i+offset));  //yoffset + 26*yiscal+ 0 m_d_TimeList
						xlen[1] = retNode.x;
						ylen[1] = retNode.y;
					}
						

					//pdc->MoveTo(xlen[0],ylen[0]);
					//pdc->LineTo(xlen[0]+5,ylen[0]-5);         //for enhancement to clear diary point //iori
					CPen *pnodepen = NULL;
					pnodepen = pdc->SelectObject(&pen[id]); 
					pdc->Ellipse(xlen[0] -3 * iscal,ylen[0]-3 * yiscal,xlen[0] + 3*iscal,ylen[0] + 3*yiscal);
					pdc->SelectObject(pnodepen);

					
					pdc->MoveTo(xlen[0],ylen[0]);
					pdc->LineTo(xlen[1],ylen[1]);
					i = i+offset-1;

				}
			}
			else
			{
				for(j=0;j<=2;j++)              //nItem
				  {                                    
	  				   plist = id ? &(this->m_postL_list[j]) : &(this->m_postR_list[j]);

					   nums[j] = plist->GetCount();
					   if(plist->GetCount() == 0 )
							continue;
					   else if( j >0 && (nums[j] != nums[j-1]) )
					   {
						  // MessageBox(_T("Can't load multiple PTA record with different BANDS! "));
				
						   break;
					   }

					   for(i=0;i<num-1;i++)      //m_diaryMod
					   {//26
						   
							retNode = plist->GetAt(plist->FindIndex(i));
							xlen[0] = retNode.x;
							ylen[0] = retNode.y;

							retNode = plist->GetAt(plist->FindIndex(i+1));
							xlen[1] = retNode.x;
							ylen[1] = retNode.y;

							if(j==0)	
								   pdc->SelectObject(&pen[id]);    //&dotpen[id]); 
							if(j==1 )
							{
								  pdc->SelectObject(&dashpen[id]);
							}
							if(j == 2)
								  pdc->SelectObject(&dotpen[id]);

							pdc->MoveTo(xlen[0],ylen[0]);
							pdc->LineTo(xlen[1],ylen[1]);


						 }
					  }


			}


		}

}

//////********************************** code for previous AMP operation ,no more need ************** 
		/*if(operaID != -2 )
		{
				int pos,offsetlen;
					pos = pSliderLf[operaID].GetPos();
		
				 offsetlen = ((26*11) * pos * yiscal) /100;

			pdc->SetBkMode(TRANSPARENT);
			pdc->TextOutW(30,yoffset+offsetlen,_T("zzz"));
		}*/

		/*
		if(operaID != -2 )
		{
			if(pSliderLf[operaID].IsWindowVisible())
			{

				int pos,offsetlen;
				RECT  fontrect;
					pos = pSliderLf[operaID].GetPos();
		
				 offsetlen = ((26*11) * pos * yiscal) /100;

				 pfontLf[operaID].GetWindowRect(&fontrect);
				 ScreenToClient(&fontrect);
		 
				 fontrect.top += offsetlen;
				 fontrect.bottom += offsetlen;

				 if((fontrect.top - (yoffset + offsetlen))<= (fontrect.bottom- fontrect.top))
				 {
					 pfontLf[operaID].MoveWindow(fontrect.left,fontrect.top,fontrect.right - fontrect.left,fontrect.bottom -fontrect.top);
					 pfontLf[operaID].UpdateWindow();
			    }
			}
		}
		*/

    //		sldrect.top = yoffset;
    //		sldrect.bottom = sldrect.top + (26*11)*yiscal;
       //  if(!slidermoved)
		// {
	
		//if(operaID != -2)
		//{
		// if(!pSliderLf[operaID].IsWindowVisible())
		// {
	
       		//	if(btnDown || ((operaID != -2) && pSliderLf[operaID].slidermoved) )// && pSliderLf[operaID].IsWindowVisible() && slidermoved)  //&&  (operaID != -2))                     // erase origin line
			
	/*	     if(btnDown)
					{

						pdc->SelectObject(&pen[0]);
						premode = pdc->SetROP2(R2_NOTXORPEN);
		
						pdc->MoveTo(m_exchangex,m_exchangey);
						pdc->LineTo(m_ex2,m_ey2);
								//	pdc->TextOutW(m_ex2,m_ey2,_T("sss"));   //neederased_x
								//	g_times++;

					//	pdc->MoveTo(neederased_x[0],neederased_y[0]);
					//	pdc->LineTo(neederased_x[1],neederased_y[1]);
					//	pdc->MoveTo(neederased_x[1],neederased_y[1]);
					//	pdc->LineTo(neederased_x[2],neederased_y[2]);
				
						pdc->MoveTo(m_ex2,m_ey2);
						pdc->LineTo(m_ex3,m_ey3);
						//btnDown = FALSE;

					}
					
			
		  // }
	//	}
			
			
	//	 }
		if(operaID != -2)
		{
			if(pSliderLf[operaID].IsWindowVisible() ||pSliderLf[operaID].slidermoved || pSliderLf[operaID].appeared)
			{  
				 // if(slidermoved)
				//  {
					 // buttupdate = FALSE;
					  int pos=0;   //slidermoved   //defposval  //defsliderPt
					  pdc->SetROP2(premode);
					  
					 // if(pSliderLf[operaID].IsWindowVisible())
					    pos = pSliderLf[operaID].GetPos();
				

					 // if((pos == defposval+1)|| (pos == defposval-1))
					//  if(pos != defposval)
					 // {
					 // }
					//  else
					//  {
					  //***************
				  				LOGBRUSH mv_logbush;
								DWORD mv_dashlen[2]={5,1+(9*pos)/100};
								mv_logbush.lbColor = RGB(208,39,28);
								mv_logbush.lbHatch = HS_BDIAGONAL;
								mv_logbush.lbStyle = BS_SOLID;

								CPen mv_pendash(PS_USERSTYLE,1,&mv_logbush,2,mv_dashlen);
					  //***************

					   //   CPen dashpen(PS_DASH,1,RGB(208,39,28));
						int len = ((26*11) * pos * yiscal) /100;
				
					//	if(pSliderLf[operaID].IsWindowVisible() && slidermoved)
					
					//	if(!pSliderLf[operaID].appeared)
					//	{
							if(sliderpressed)
							{
								  //if(slidermoved)
								  pdc->SelectObject(&mv_pendash);  //(&pen[0]);
							}
							else
							{
								  pdc->SelectObject(&pen[0]);
							}
					//	}
					//	else
					//		       pdc->SelectObject(&pen[0]);
						
						  pdc->MoveTo(m_exchangex,m_exchangey);
						 // if(slidermoved)
						  updateNode.x = m_ex2;
						  updateNode.y = yoffset+len;
						 // pDoc->m_postR_list[0].SetAt(pDoc->m_postR_list[0].FindIndex(2),updateNode);  operaID
						 if(pSliderLf[operaID].IsWindowVisible())
						    pDoc->m_AMP_List[0].SetAt(pDoc->m_AMP_List[0].FindIndex(2 + operaID),updateNode);

						  pdc->LineTo(m_ex2,yoffset+len);          //+25*yiscal);

		
							  pdc->MoveTo(m_ex2,yoffset+len);
							  pdc->LineTo(m_ex3,m_ey3);
						  

	
					//  }
				  }
			}*/

///}


BOOL	CETApplicationView::OnDeviceChange(UINT nEventType,DWORD dwData)
{
	bool linkResult;
//	_PatientDesc patientDesc;

	PDEV_BROADCAST_DEVICEINTERFACE pdbi;

	pdbi=(PDEV_BROADCAST_DEVICEINTERFACE)dwData;

    switch(nEventType)
    {    
    case DBT_DEVICEARRIVAL:
       
		if(pdbi->dbcc_devicetype==DBT_DEVTYP_DEVICEINTERFACE)
		{
			if(!pbutt[0]->pressed)
				break;

			if ( LINKING == m_LinkState)
			{
				pbutt[1]->SetFocus();
				pbutt[1]->Invalidate();
			}

			if ( LINKED == m_LinkState)
				break;

			if(!g_bPatientLinkingThreadRunning)
			{	
				g_bPatientLinkingThreadRunning = true;

	/*
				m_LinkState = LINKING;
				pPatientList[1]->SetItemText(0,1,_T(""));
				pPatientList[1]->SetItemText(0,2,_T("  LINKING"));
				pPatientList[1]->SetItemText(0,3,_T(""));
	*/

				m_pPatientLinkingThread = ::AfxBeginThread(PatientLinkingThread, this);
				if ( m_pPatientLinkingThread )
				{
					m_pPatientLinkingThread->ResumeThread();
				}
			}
			
		}
        break;
    case DBT_DEVICEREMOVECOMPLETE:

		if(pdbi->dbcc_devicetype==DBT_DEVTYP_DEVICEINTERFACE)
		{
/*
			if (!FilterUSBHIDDevice(pdbi->dbcc_name))
			{
					// Do nothing
			}
*/
			if (!FilterUSBMSDDevice(pdbi->dbcc_name))
			{
				m_LinkState = NOLINKED;
				pPatientList[1]->SetItemText(0,1,m_buffstrunlink); //_T("  UNLINKED"));
				pPatientList[1]->SetItemText(0,2,_T(""));
				pPatientList[1]->SetItemText(0,3,_T(""));

				if(IsAmp&&m_diaryMod)
				{
					OnButtonPrint();
				}

				if(IsAmp)
					pbutt[6]->EnableWindow(FALSE);

#ifdef SUPPORT_SITE_TREATMENT
				if(IsAmp)
				{
					if(m_treatstage)
						pbutt[5]->SetWindowTextW(m_buffplaystop);
					else
						pbutt[5]->SetWindowTextW(m_buffarr[5]);
				}
#endif

			}
		}
        break;
    }
    return TRUE;
}


CRect CETApplicationView::AutoAdjust(CRect currRect)
{
    #define  STD_SCREEN_CX   1366
    #define  STD_SCREEN_CY   768

	CRect rcTemp;
	int cx = GetSystemMetrics(SM_CXSCREEN);
    int cy = GetSystemMetrics(SM_CYSCREEN);

	double iMaxCx = (double)cx/(double)STD_SCREEN_CX; 
    double iMaxCy = (double)cy/(double)STD_SCREEN_CY; 

    double dScale = iMaxCy - iMaxCx;

	int iSrcWidth  = currRect.Width();
    int iSrcHeight = currRect.Height();
	double iTemp = 0;

	if( cx > cy )
    {
       iTemp = (double)currRect.left * iMaxCx;
       currRect.left =(int)iTemp;
       iTemp = (double)currRect.right * iMaxCx;
       currRect.right =(int)iTemp;
       iTemp = (double)currRect.top * iMaxCy;
       currRect.top =(int)iTemp;
       iTemp = (double)currRect.bottom * iMaxCy;
       currRect.bottom =(int)iTemp;
 
	   rcTemp = currRect;
       return rcTemp;
    }
	else
	   return 0;
}
/*
 HBRUSH CETApplicationView::OnCtlColor( CDC *pDC, CWnd *pWnd , UINT nCtlColor  )
 {
	 HBRUSH   hbr   =   CView::OnCtlColor(pDC,   pWnd,   nCtlColor); 
	 if(nCtlColor==CTLCOLOR_STATIC) 
	 {
		pDC->SetBkMode(TRANSPARENT); 
		  return   HBRUSH(GetStockObject(HOLLOW_BRUSH)); 
	 }
	 return hbr;

 }
 */
 
void CETApplicationView::OnButtonPAUSE()
{
					    
	                CMainFrame *pmnfm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
				    CRect rect;
                    pmnfm->GetWindowRect( rect );

					::SetWindowPos(pmnfm->GetSafeHwnd() ,       // handle to window
					HWND_TOPMOST,  // placement-order handle
					rect.left,     // horizontal position
					rect.top,      // vertical position
					rect.Width(),  // width
					rect.Height(), // height
					SWP_SHOWWINDOW); // window-positioning options

	if(IsAmp)
	{
		if ( m_LinkState  == LINKED 
			|| m_LinkState  == LINKING ) // Already linked or under linking, no needs to assign identification.
		{
			return;
		}


		if(!DetectUSBHID())
		{
			return;   // Not plugged in
		}

		/*
		std::string sID =  m_pIDEdit->GetText();
		if (sID.empty())  // At least ID needs to be input for identification assignment
		{
			return;
		}
		*/

		CString strid;
		strid = pPatientList[0]->GetItemText(0,1);

		CString strdef = m_bufstrprompt; // _T("Pls Input ID ...");
		if(strid.Compare(strdef) ==0 )
		{
			return;
		}

		if ( !g_bPatientLinkingThreadRunning)
		{		
			g_bPatientLinkingThreadRunning = true;
			m_pPatientLinkingThread = ::AfxBeginThread(PatientLinkingToCUThread, this);
			if ( m_pPatientLinkingThread )
			{
				m_pPatientLinkingThread->ResumeThread();
			}
		}


	}
	else   // PTA
	{
		if((m_sessionID!= NO_SESSION_ID) &&(m_isPTATesting == true))
		{
			if((m_PTAPaused)&&(pbutt[4]->pressed))
			{
				ResumeAutomaticPTA(m_sessionID);
				m_PTAPaused = false;

				pbutt[4]->SetWindowTextW(strButt[4]); //_T("PAUSE"));
				pbutt[4]->ModifyStyle(BS_OWNERDRAW,0 );
				pbutt[4]->pressed = FALSE;

				dwResumeTime = GetTickCount();
				dwStartTickTime = dwStartTickTime + (dwResumeTime - dwPauseTime );

			}
			else // going to press
			{
				if(m_manualmod)  //anson
				{
					 BOOL acp = FALSE;;
					 int totR =0 , totL = 0;
					
					 

					 int numR = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;
					 for(int i = 0; i < numR; i++)
					 {
						totR  = totR +   m_rcdvalarr[0][i];

					 }

					 int numL = plistctr[1]->GetHeaderCtrl()->GetItemCount() -1;
					 for(int i = 0; i< numL; i++)
					 {
						 totL = totL + m_rcdvalarr[1][i];
					 }

					 if((numR + numL) == (totR + totL))
					 {
						 acp = TRUE;
						 pbutt[1]->EnableWindow(TRUE);
						 pbutt[2]->EnableWindow(TRUE);
						 memset(m_rcdvalarr[0],0,sizeof(int)*11);
	                     memset(m_rcdvalarr[1],0,sizeof(int)*11);
						 FinishManualPTA(m_sessionID);   //here use new lib'ret to judge is totally finished?
					 }
					 else
					 {
//						 MessageBox(_T("Some bands are not tested, please continue!"),_T(""),MB_OK);
						 MessageBox(m_buffNoFinishWarning,_T(""),MB_OK);

					 }

					 

					 if(acp)
					 {
						 FillPTAResult();
						 m_manualmod = FALSE;
						 m_manualcheckboxstat = FALSE;

						 pHighRangeBtn[0]->ShowWindow(TRUE);
						 pHighRangeBtn[1]->ShowWindow(TRUE);
						 pbutt[7]->EnableWindow(TRUE);
						 pbutt[4]->SetWindowTextW(strButt[4]);  //);
					 }
					// else
					// {
					//	 MessageBox("not finished");
					// }

				}
				else
				{

					PauseAutomaticPTA(m_sessionID);
					m_PTAPaused = true;

					pbutt[4]->SetWindowTextW(m_buffresume); //_T("RESUME"));
					pbutt[4]->ModifyStyle(0,BS_OWNERDRAW );
					pbutt[4]->pressed = TRUE;

					dwPauseTime = GetTickCount();
					strpauseTime = strTime;
				}

					/*	strendTime = strTime;
						dwEndTime = GetTickCount();

							CRect rect;
						this->GetClientRect(rect);
						rect.left = 1120*iscal;
						rect.top = 702*yiscal;

						this->InvalidateRect(rect);*/


			}

		}
	}

}


UINT  DownloadProgressThread(LPVOID pObject)
{

	CETApplicationView* pAppView = (CETApplicationView*)pObject;
	return pAppView->DownloadProgress();


}

int CETApplicationView::DownloadProgress()
{

	int nProgress =0;
	int retval = 0;
	Sleep(3000);
	pctrlprogress->StepIt();
	while(true)
	{
					//Sleep(13000);

					for (int i=0; i<13; i++)
					{
						Sleep(1000);
						if(LINKED != m_LinkState)
						{	
							pdlgprogress->ShowWindow(SW_HIDE);
							MessageBox(L"Check USB conncetion to AudioMed, and try again");
							return -1;
						}
					}

					pctrlprogress->StepIt();
					if(pctrlprogress->GetPos() == 90) //nProgress == 25)
					{

						retval = GetETFDownloadProgress(m_sessionID);
						while((retval !=100)&&(retval !=-1))
						{
							Sleep(4000);
							retval = GetETFDownloadProgress(m_sessionID);
						} 

						if(retval == 100)
						{
							pctrlprogress->StepIt();
							pctrlprogress->SetPos(100);
							Sleep(1000);
							pdlgprogress->ShowWindow(SW_HIDE);
							g_bETGenerated = false;
							g_bIsLocalPlaying = false;
							MessageBox(m_buffProfileSuccess);
							return 0;
						}
						else
						{
							pdlgprogress->ShowWindow(SW_HIDE);
							MessageBox(L"AudioMed profiling was failed");
							return -1;

						}
						
					}
	}
	return 0;

}


void CETApplicationView::OnButtonDATA()
{
					    CMainFrame *pmnfm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
				    CRect rect;
                    pmnfm->GetWindowRect( rect );

					::SetWindowPos(pmnfm->GetSafeHwnd() ,       // handle to window
					HWND_TOPMOST,  // placement-order handle
					rect.left,     // horizontal position
					rect.top,      // vertical position
					rect.Width(),  // width
					rect.Height(), // height
					SWP_SHOWWINDOW); // window-positioning options

	if(IsAmp)  // Download
	{
		 
		if(m_diaryMod)
		{
			int width = ::GetSystemMetrics(SM_CXSCREEN);
			int height = ::GetSystemMetrics(SM_CYSCREEN);
			Invalidate();
			CopyScreenToBitmap(0,0,width,height,0,0);
//          OnFilePrintPreview();
			OnFilePrint();
			m_listdc.Detach();
		}
		else
		{
				if(!g_bETGenerated)
				{
					MessageBox(m_buffgenfirst); //MessageBox(L"Please generate treatment file before operation.");
					return;
				}
				if (m_LinkState  != LINKED)
				{
		#ifndef  SUPPORT_SITE_TREATMENT
					   MessageBox(_T("AudioMed is not connected."));   
		#else
					if(g_bIsLocalPlaying)
					{
						StopTreatmentPlaying();
						pbutt[5]->pressed = FALSE;
						pbutt[5]->ModifyStyle(BS_OWNERDRAW,0);
						pbutt[5]->Invalidate();
						pbutt[7]->EnableWindow(TRUE);

						g_bIsLocalPlaying = false;

						if(m_nTimer != NULL)
						{
							this->KillTimer(m_nTimer);		
							m_nTimer = NULL;
						}						
						strendTime = _T("");

						pbutt[1]->EnableWindow(TRUE);
						pbutt[2]->EnableWindow(TRUE);
						pbutt[3]->EnableWindow(TRUE);

//						MessageBox(_T("Treatment file playing stopped!")); 	 
						std::wstring szwdiary = strdiarypcpath.GetBuffer(0);
				        std::string  strdiary = ws2s(szwdiary);
						LPCSTR str = strdiary.c_str();
						MessageBox(m_buffTreatmentStop); 

						CString strtreattime,substr;
						int idx=0;
						strtreattime = strTime;
						strtreattime.TrimLeft(_T("Time: "));
						idx = strtreattime.Find(_T("m"));
						substr = strtreattime.Left(idx);
						if(_ttoi(substr) >= 5 )             //more than 1 min ,just for test
						{
						   WriteSingleRecordinPcDiary(str);
						   
						}
						
							 
					}
					else
					{
						StartTreatmentPlaying();
						pbutt[5]->pressed = TRUE;
						pbutt[5]->ModifyStyle(0,BS_OWNERDRAW);
						pbutt[5]->Invalidate();
						pbutt[7]->EnableWindow(FALSE);
						g_bIsLocalPlaying = true;

						//---------------------------- generate diary.txt 
                            strdiarypcpath = GetPTAResultLocalPath();
							CString strfolder = pPatientList[0]->GetItemText(0,1);
	
							if(strfolder.CompareNoCase(strprompt) && strfolder.GetLength())  //no default str
								strdiarypcpath = strdiarypcpath + strfolder;
							else
							{
								MessageBox(strprompt);
								return;
							}
							strdiarypcpath = strdiarypcpath + _T("\\");
							strdiarypcpath = strdiarypcpath + _T("diary.txt");


						//----------------------------
						strTime = _T("");
						if(m_nTimer != NULL)
							this->KillTimer(m_nTimer);
						m_nTimer = this->SetTimer(IDT_TIMER,1000,0); //IDT_TIMERWARN
						dwStartTickTime = GetTickCount();

						pbutt[1]->EnableWindow(FALSE);
						pbutt[2]->EnableWindow(FALSE);
						pbutt[3]->EnableWindow(FALSE);

//						MessageBox(_T("Treatment file playing started!")); 
						MessageBox(m_buffTreatmentPlay); 
					}
          #endif
					return ;
				}
			// Calling ETF Downloading, will cost long time!!!!!!!!!!!!!!!!!!!!
			if(m_sessionID != NO_SESSION_ID)
			{
					BOOL ret = FALSE;
//					ret = ETFDownload(m_sessionID, 90, 72,72,true);  // start ETF Downloading 
					ret = ETFDownload(m_sessionID, g_countDown/PLAYTIME_ONCE, g_leftVolume, g_rightVolume, true);  // start ETF Downloading 

					if(!ret)
					{
	//					MessageBox(L"Treatment file downloading was failed");
						MessageBox(L"AudioMed profiling was  failed");
						return;
					}

					RECT rect;
					int istep = 0;

					pdlgprogress = new CEarProgressDlg();
					pdlgprogress->Create(IDD_ProgressDlg, this);
					pctrlprogress = new CProgressCtrl();

					pdlgprogress->GetClientRect(&rect);

					pdlgprogress->MoveWindow( 683*iscal - (rect.right -rect.left)/2, 384 * yiscal - (rect.bottom - rect.top)/2,
											  rect.right -rect.left,rect.bottom - rect.top);

					pctrlprogress->Create(WS_CHILD|WS_VISIBLE|WS_BORDER,rect,pdlgprogress,1680);
					pctrlprogress->SetRange(0,100);
					pctrlprogress->SetPos(0);
					pctrlprogress->SetStep(10);
					pdlgprogress->ShowWindow(SW_SHOW);

					pctrlprogress->Invalidate();
					pdlgprogress->Invalidate();

					AfxBeginThread(DownloadProgressThread, this);
			   }
	    }
		
	}
	else            //pta save/load
	{

		// Only activated when it is linked.
	   /*	if(LINKED != m_LinkState)
		{
			MessageBox(L"The operation is not allowed when it is not linked" );
			return;
		}
		*/
		if(NOLINKED == m_LinkState)
		{
			/*
			WCHAR tmp_wstrPtaPath[376];
			WCHAR  syswDir[MAX_PATH];
			memset( syswDir, NULL, MAX_PATH*sizeof(WCHAR));
			GetSystemDirectory(syswDir, MAX_PATH);

			wsprintf(tmp_wstrPtaPath, L"%s\\pts\\", syswDir);
			*/

			strptapath = GetPTAResultLocalPath();

//			strptapath = _T("C:\\Windows\\System32\\pts\\");
			m_stridxpath = strptapath + _T("idx.txt");

			CString strfolder = pPatientList[0]->GetItemText(0,1);
	
			if(strfolder.CompareNoCase(strprompt) && strfolder.GetLength())  //no default str
			    strptapath = strptapath + strfolder;
			else
			{
				MessageBox(strprompt);
				return ;
			}
			 
			//CreateDirectory(strptapath,NULL);
			//strptapath = strptapath + _T("\\");
			//strptapath = strptapath + _T("ptaresult.txt");

		}


		if(!pbutt[5]->pressed)
		{
			pbutt[5]->ModifyStyle(0,BS_OWNERDRAW );
			pbutt[5]->pressed = TRUE;
			pbutt[5]->Invalidate();
		}
		//else
	//{
		//	pbutt[5]->ModifyStyle(BS_OWNERDRAW,0 );
		//	pbutt[5]->pressed = FALSE;
		//}

		//CPropertySheet  sheet(_T("Save & Load"),NULL);  // IDD_PROPPAGE_LOAD  IDD_PROPPAGE_SAVE //IDC_SPAG_SAVEBUTT
		LDPTARECNODE node;
		int size = 0;
		CString strPta[3];
		CString  strT;
		float   valPta[3];
		TCHAR *pstrbuff = NULL;
		int ret =0;
		int recnum = 0;
		int headnum = 0;
		int retnum[3] = {0,0,0};
		CSLPropertySheet dlgSheet(_T(" "),NULL);
		psheet = &dlgSheet;
//		psheet = new CSLPropertySheet(_T("Save & Load"),NULL);
		CSvLdPropertyPage   savpage ;    //(IDD_PROPPAGE_SAVE);
		CSvLdPropertyPage   loadpage;     //(IDD_PROPPAGE_LOAD);
		loadpage.Construct(IDD_PROPPAGE_LOAD);
		psheet->AddPage(&savpage);
		psheet->AddPage(&loadpage);
		savpage.SaveViewPt(this);
		loadpage.SaveViewPt(this);
		ret = psheet->DoModal();
		if(ret == IDOK)
		{
			pbutt[5]->ModifyStyle(BS_OWNERDRAW,0 );
		    pbutt[5]->pressed = FALSE;
			pbutt[5]->Invalidate();

			if(g_bPTANeedSaved)
			{
				if (IDYES != MessageBox(m_buffPTANoSaveWarning, // L"Do you want to leave latest PTA test result without being saved?",  //m_buffassign, 
								   L"Warning",MB_YESNO|MB_ICONWARNING))   
				{
					return;
				}
				g_bPTANeedSaved = false;

			}

			for(int i = 0 ;i <3; i++)
			{
				plistctr[0]->SetItemText(i,0,_T(""));
				plistctr[1]->SetItemText(i,0,_T(""));
				if(m_PTArecordChecked[i] != -1)
				{
					recnum++;
					if(m_LDPage_T_isChecked)
					{
						node = cldT_list.GetAt(cldT_list.FindIndex(m_PTArecordChecked[i]));
					}
					else
					{
						CString idarr[3];
						idarr[0] = _T("1)");
						idarr[1] = _T("2)");
						idarr[2] = _T("3)");

					    node = cldlist.GetAt(cldlist.FindIndex(m_PTArecordChecked[i]));

						ptalistctltime[i] = node.time;
						ptalistctltime[i] = ptalistctltime[i].Left(10);
						ptalistctltime[i] = ptalistctltime[i].Right(5);

						ptalistctltime[i] = GetCorresponseMonth(ptalistctltime[i].Left(2)) + ptalistctltime[i].Right(2) ;

						

						plistctr[0]->SetItemText(0,0,idarr[0]+ ptalistctltime[0]);    //_T("1."));    //PTA1
						plistctr[0]->SetItemText(1,0,idarr[1]+ ptalistctltime[1]);     //_T("2."));    //PTA2
						plistctr[0]->SetItemText(2,0,idarr[2]+ ptalistctltime[2]);    //_T("3."));    //PTA3
						plistctr[0]->SetItemText(3,0,m_buffave);   //_T("Average"));

						plistctr[1]->SetItemText(0,0,idarr[0]+ ptalistctltime[0]);   //_T("1."));    //PTA1
						plistctr[1]->SetItemText(1,0,idarr[1]+ ptalistctltime[1]);  //_T("2."));    //PTA2
						plistctr[1]->SetItemText(2,0,idarr[2]+ ptalistctltime[2]);  //_T("3."));    //PTA3
						plistctr[1]->SetItemText(3,0,m_buffave);   //_T("Average"));
					}
					size = node.sizeofbuff;
					pstrbuff = new TCHAR [size];
					memset(pstrbuff,0,size * sizeof(TCHAR));
					_tcsncpy(pstrbuff,node.pstrpos,size);
					
					retnum[i] = AnalysisPTARecord(i , pstrbuff);

					//CString stri;
					//stri.Format(_T("%d"),retnum[i]);
					//MessageBox(stri);

					delete [] pstrbuff;
					pstrbuff = NULL;

					if(i >0 && (retnum[i] != retnum[i-1] ))
					{
						 MessageBox(m_buffcantloadmul);  //MessageBox(_T("Can't load multiple PTA record with different BANDS! "));
						 return ;
					}
				}
			}

			 if(fReadPta.m_hFile != CFile::hFileNull) fReadPta.Close();
					
		     if ((recnum != 2) && (recnum != 3)) 
				   return ;



			for(int id =0 ;id < 2; id++)
			{
				headnum = plistctr[id]->GetHeaderCtrl()->GetItemCount() -1;
				
				for(int j = 0 ;j < headnum; j++)              //calculate T value
				{
					if(recnum == 2)
					{
						  float tmpval = 0;
						  strPta[0] = plistctr[id]->GetItemText(0,j+1);
						  strPta[1] = plistctr[id]->GetItemText(1,j+1);

						  valPta[0] = _ttof(strPta[0]);
						  valPta[1] = _ttof(strPta[1]);

					   if(m_LDPage_T_isChecked)
					   {
						   plistctr[id]->SetItemText(2,0,m_delta);
						   plistctr[id]->SetItemText(3,0,_T(""));
						   plistctr[id]->SetItemText(3,j+1,_T(""));
						   tmpval = valPta[0] - valPta[1];
						   strT.Format(_T("%3.1f"),tmpval);
						   plistctr[id]->SetItemText(2,j+1,strT);
					   }
					   else
					   {
						 
						  tmpval = (valPta[0] + valPta[1])/2;
						  strT.Format(_T("%3.1f"),tmpval);
						  plistctr[id]->SetItemText(3,j+1,strT);
					   }
					}
					if(recnum == 3)
					{
					  float tmpval = 0;
				      strPta[0] = plistctr[id]->GetItemText(0,j+1);
					  strPta[1] = plistctr[id]->GetItemText(1,j+1);
					  strPta[2] = plistctr[id]->GetItemText(2,j+1);

					  valPta[0] = _ttof(strPta[0]);
					  valPta[1] = _ttof(strPta[1]);
					  valPta[2] = _ttof(strPta[2]);
					  tmpval = (valPta[0] + valPta[1]+ valPta[2])/3;
					  strT.Format(_T("%3.1f"),tmpval);
					  plistctr[id]->SetItemText(3,j+1,strT);

					}
				}
				
			}
			    
		}
		if(ret == IDCANCEL)
		{
			pbutt[5]->ModifyStyle(BS_OWNERDRAW,0 );
		    pbutt[5]->pressed = FALSE;
			pbutt[5]->Invalidate();

		}

	}

}
void CETApplicationView::RegisterNewPTAandIdx()
{
		CreateDirectory(strptapath,NULL);
		strptapath = strptapath + _T("\\");
		strptapath = strptapath + _T("ptaresult.txt");
}


CString CETApplicationView::GetCorresponseMonth(CString mth)
{
	CString retstr = _T("");
	int mval =0 ;
	mval = _ttoi(mth);
	switch(mval)
	{
	case 1:
		   retstr = _T("Jan");
		   break;
	case 2:
		   retstr = _T("Feb");
		   break;
	case 3:
		   retstr = _T("Mar");
		   break;
	case 4:
		   retstr = _T("Apr");
		    break;
	case 5:
		   retstr = _T("May");
		   break;
	case 6:
		   retstr = _T("Jun");
		   break;
	case 7:
		   retstr = _T("Jul");
		   break;
	case 8:
		   retstr = _T("Aug");
		   break;
	case 9:
		  retstr = _T("Sep");
		  break;
	case 10:
		  retstr = _T("Oct");
		  break;
	case 11:
		 retstr = _T("Nov");
		 break;
	case 12:
		 retstr = _T("Dec");
		 break;
		       
	}

	return retstr;
}

BOOL  CETApplicationView::SetMuteSysVol(BOOL mute)
{
	IMMDevice*   pEndpoint   =   NULL;
	IMMDeviceCollection*   pCollection=NULL;
	IMMDeviceEnumerator *pEnumerator=NULL;

	IAudioEndpointVolume*   pAudioEndVol   =   NULL; 
	IAudioSessionManager    *pAudioSession = NULL;
	IAudioStreamVolume    *pstreamvol = NULL;

	IAudioSessionManager2  *psession2 = NULL;
	IAudioSessionControl2  *pcontrols2 = NULL;
	IAudioSessionEnumerator *psessinenum = NULL;
	IAudioSessionControl  *pcontrol = NULL;
    IAudioSessionControl *pcontolslater = NULL;
	ISimpleAudioVolume  *p=NULL;
	int sessnum = 0; 
	HRESULT  hr;

	hr = CoCreateInstance(
           __uuidof(MMDeviceEnumerator), NULL,
           CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
           (void**)&pEnumerator);

	hr   =   pEnumerator->GetDefaultAudioEndpoint( eRender,     //eCapture
		                                           eConsole,  &pEndpoint); 
	
	hr   =   pEndpoint->Activate(__uuidof(IAudioSessionManager2),
		                            CLSCTX_ALL, NULL, (void**)&psession2);

	hr = psession2->GetSessionEnumerator((IAudioSessionEnumerator**)&psessinenum);

	psessinenum->GetCount(&sessnum);

	for(int i =0 ;i < sessnum; i++)
	{
		psessinenum->GetSession(i,(IAudioSessionControl**)&pcontrols2);
		hr = pcontrols2->IsSystemSoundsSession();
		if(hr == S_OK)
		{
			pcontrols2->QueryInterface(__uuidof(ISimpleAudioVolume), (void **)&p); 
			p->SetMute(mute,NULL);   
		}
	}
	return  0;
}


 int CETApplicationView::AnalysisPTARecord(int idx, LPTSTR pstrbuff) 
 {
	 CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;
     FREZNODE node;

	 CString strSide[2] = {_T("\r\n      ------ R -----   \r\n"),_T("\r\n      ------ L -----   \r\n")};
	 CString endstr = _T("\r\n    ---");
	 LPTSTR strRight,strLeft,strEnd ,strStore;
	 
	 TCHAR *pRt, *pLf;
	 TCHAR *pRtMemo, *pLfMemo;
	 TCHAR *pstr0,*pstr1;
	 TCHAR strRECDbuff[32];
	 CString strband,strvalue;
	 int sizeRt =0, sizeLf = 0;
	 float band = 0, val =0;

	 int retbandtype = 11;

	 strRight = _tcsstr(pstrbuff,strSide[0]);
	 strLeft  = _tcsstr(pstrbuff,strSide[1]);
	 strEnd   = _tcsstr(pstrbuff,endstr);

	 strStore = pstrbuff;

	 sizeRt = strLeft - strRight - strSide[0].GetLength();
	 sizeLf = strEnd -  strLeft - strSide[1].GetLength();

	 pRtMemo = new TCHAR [sizeRt+1];
	 pLfMemo = new TCHAR [sizeLf+1];

	 pRt = pRtMemo;
	 pLf = pLfMemo;
//	 pRt = new TCHAR [sizeRt];
//	 pLf = new TCHAR [sizeLf];

	 memset(pRt,0,(sizeRt+1)*sizeof(TCHAR));
	 memset(pLf,0,(sizeLf+1)*sizeof(TCHAR));
	
	 _tcsncpy(pRt,strRight +  strSide[0].GetLength(), sizeRt+1);
	 _tcsncpy(pLf,strLeft + strSide[1].GetLength(), sizeLf+1);

	 pRt[sizeRt] = _T('\0');
	 pLf[sizeLf] = _T('\0');

	  pDoc->m_Rt_list[idx].RemoveAll();
	  pDoc->m_Lf_list[idx].RemoveAll();

	  ETFrequencyArray freArray;
	  int iRight = 0;

	 while((pstr0 =  _tcsstr(pRt,_T("\r\n"))) != NULL ) //pRt right
	 {
		 if((pstr0 - pRt) > 2)
		    pstr0++;           //move \r\n byte
		 else
			break;

		 memset(strRECDbuff,0,32 * sizeof(TCHAR));
		 _tcsncpy(strRECDbuff,pRt, pstr0 - pRt);

		 pRt = pstr0;

		 strband = strRECDbuff;
		 strband = strband.Left(6);
		 strband.Trim(_T(" "));
		 strvalue = strRECDbuff;
		 strvalue = strvalue.Mid(7);
		 strvalue.Trim(_T(" "));

		 band = _ttof(strband);
		 val  = _ttof(strvalue);

		 node.band = band * 1000;
		 node.dbhl = val;
		 pDoc->m_Rt_list[idx].AddTail(node);
		 freArray.aRightFreqList[iRight++] = int(node.band + 0.5);
	 }

	 freArray.nRightFrequencyNum = iRight;
	 int iLeft = 0;

	  while((pstr1 =  _tcsstr(pLf,_T("\r\n"))) != NULL ) //pLf left
	 {
		// MessageBox(_T("time"));
		 if((pstr1 - pLf) > 2)
		    pstr1++;
		 else
			break;

		 

		 memset(strRECDbuff,0,32 * sizeof(TCHAR));
		 _tcsncpy(strRECDbuff,pLf, pstr1 - pLf);

		 pLf = pstr1;

		 strband = strRECDbuff;
		 strband = strband.Left(6);
		 strband.Trim(_T(" "));
		 strvalue = strRECDbuff;
		 strvalue = strvalue.Mid(7);
		 strvalue.Trim(_T(" "));

		 band = _ttof(strband);
		 val  = _ttof(strvalue);

		 node.band = band * 1000;
		 node.dbhl = val;
		 pDoc->m_Lf_list[idx].AddTail(node);

		 freArray.aLeftFreqList[iLeft++] = int(node.band + 0.5);

	 }

	  
		freArray.nLeftFrequencyNum = iLeft;

		if(idx == 0)
			SetFrequencyBands(freArray);

		m_FreArray = freArray;

		if(m_FreArray.nLeftFrequencyNum == 6)
		{
			g_bands_type = BANDS_CLASSIC_6;
		}
		
		if(m_FreArray.nLeftFrequencyNum == 11)
		{  
			int s = 0;
			if((m_FreArray.aRightFreqList[1] - m_FreArray.aRightFreqList[0]) != 250) //high band
			{
				g_bands_type = BANDS_RANGE_NARROWED;
				s++;
			}
			if((m_FreArray.aLeftFreqList[1] - m_FreArray.aLeftFreqList[0]) != 250) //high band
			{
				g_bands_type = BANDS_RANGE_NARROWED;
				s++;
			}

			if(s != 0)
			{
			   m_initalexchangearry  = m_FreArray;
			   OnInitalHighRangeArray();
			}
			else
			{
				g_bands_type = BANDS_CLASSIC_11;
			}
		}
		
		
	    UpdateCtrlTabFromList(idx);
	    StoreDataInList();
		    
		OnInitalPTAMP();

///////////////////////////////////////////////////////////////////////////////////////
		if(idx == 0)
		{
				// Initiated to allow restarting

				if(m_sessionID != NO_SESSION_ID)
					EndTestSession(m_sessionID);

				Sleep(500);
				m_sessionID = BeginTestSession(ET_PTA, ET_AUTOMATIC);
				if( m_sessionID == NO_SESSION_ID)
				{
#ifdef _DEBUG
					MessageBox(L"PTA Session can not be created");
#endif
				}
				SetPTASpeed(m_sessionID, g_testSpeed);
				RegisterTestCompleteIndication(m_sessionID, TestAccomplishmentIndication, this, TestProgressIndication);
		}
/////////////////////////////////////////////////////////////////////////////////////////
		this->Invalidate();

		delete [] pRtMemo;
		delete [] pLfMemo;

		if(g_bands_type == BANDS_RANGE_NARROWED)
		   retbandtype = -1;                             // -1 means high band.
		else
		   retbandtype = pDoc->m_Lf_list[idx].GetCount();

		return   retbandtype;                //pDoc->m_Lf_list[idx].GetCount();
 }

void CETApplicationView::OnDraw(CDC* pDC)
{
	
	CETApplicationDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	CFont *pstoreft = pDC->GetCurrentFont();
	COLORREF txtcolor = pDC->GetTextColor();
	//int cy = GetSystemMetrics(SM_CYSCREEN);

	if(pDC->IsPrinting())	
	{
		pDC->OffsetWindowOrg(-10,-140);
	}


	DrawWaveTable(pDC);

	if(pDC->IsPrinting())	
	{  
		int hi =0;
		pDC->OffsetWindowOrg(10,140); //m_listdc

		//m_strId,m_strName,m_strAge,m_strGender,m_strDate;
		//CPen penblack(
		//hi = pDC->GetDeviceCaps(PHYSICALHEIGHT);
		pDC->SetTextColor(RGB(0,0,0));
		pDC->TextOutW(30*iscal,10*yiscal,_T("\r\n"));
		pDC->TextOutW(30*iscal,30*yiscal,m_strId);
		pDC->TextOutW(30*iscal,50*yiscal,m_strName);
		pDC->TextOutW(30*iscal,70*yiscal,m_strAge);
		pDC->TextOutW(30*iscal,90*yiscal,m_strGender);
		pDC->TextOutW(30*iscal,110*yiscal,m_strDate);


		CFont lfont ,*poldFont;
		LOGFONT   logfont; 
        memset(&logfont,   0,   sizeof(LOGFONT)); 
		logfont.lfHeight = 14;
		logfont.lfWeight = FW_MEDIUM;
		lfont.CreateFontIndirectW(&logfont);

		pDC->BitBlt(20*iscal,650*yiscal,m_storelistrect.Width()*2 + 16*iscal,m_storelistrect.Height()+6*yiscal ,&m_listdc,0,0,SRCCOPY); //
		
		poldFont = pDC->SelectObject(&lfont);

		if(!m_diaryMod) 
		   pDC->TextOutW(460*iscal,820 * yiscal,m_buffprtbotm);
		
		pDC->TextOutW(450*iscal,840 * yiscal,m_buffprtbotm2);
		
		//pDC->StretchBlt(20,600,m_storelistrect.Width()*2 +16,m_storelistrect.Height()+6 ,&m_listdc,0,0,m_storelistrect.Width()*2 +16,m_storelistrect.Height()+6,SRCCOPY);
	}

	if(!pDC->IsPrinting())
	{
		  //  m_screenxLogPixPerInch = pDC->GetDeviceCaps(LOGPIXELSX); 
	      //  m_screenyLogPixPerInch = pDC->GetDeviceCaps(LOGPIXELSY);
			if(m_highRangeSet)
			{
				DrawHighRangeLine(pDC,m_set_highside);
				DrawHighRangeLine(pDC,1- m_set_highside );
				if(m_keypressed[m_set_highside] && pHighRangeBtn[m_set_highside]->pressed)  //pressed arrow key to adjust high line
				{
				   AdjustHighRangeLine(pDC,m_startidx,m_nlen[m_set_highside],m_keytype);
				   GetUpdateOfHighRange();
			   	
				}

			}
			
		     if(m_narrowWarningRT)  //
				{
					pDC->SelectObject(pstoreft);
					pDC->SetTextColor(txtcolor);
					pDC->TextOutW(600*iscal,684 *yiscal ,m_lowfreqRTWarning);
				}
			 if(m_narrowWarningLF)  //
				{
					pDC->SelectObject(pstoreft);
					pDC->SetTextColor(txtcolor);
					pDC->TextOutW(600*iscal,704 *yiscal ,m_lowfreqLFWarning);
				}
	}

	//DrawWaveTable(pDC);


}



void CETApplicationView::DrawHighRangeLine(CDC* pDC, int nsideId)
{
			int i, id, num = 0,offset = 0;
			int counter = 0;
			CPen *poldpen;
			CPen penR(PS_SOLID,5,RT_RED_COLOR ); //210 20 20  , 20 20 170
			CPen penL(PS_SOLID,5,LF_BLUE_COLOR);

			CPen penGray(PS_SOLID,5,RGB(192,192,192));

		    id = nsideId; //m_set_highside;

			offset = id*683*iscal;
			xoffset += offset;

			poldpen = id ? pDC->SelectObject(&penL) :pDC->SelectObject(&penR);

			if(m_grayHighLine[id])
				pDC->SelectObject(&penGray);
		

			num = plistctr[id]->GetHeaderCtrl()->GetItemCount() -1;
			if (num !=0 )
				gap[id] = linewidth /num;
			else
				gap[id] = linewidth;

			 hightmpgap[id] = gap[id];



			for(i = 0; i<num -1;i++)
			{
				/*if(!m_keypressed[id])     //initial whole line
				{
					m_freqsetArry[id][i] = TRUE;
					pDC->MoveTo(xoffset+gap[id]+4 + i*gap[id],yoffset-5);
					pDC->LineTo(xoffset+gap[id]+gap[id]-4 + i *gap[id],yoffset-5);
				}
				else
				{*/
					if(m_freqsetArry[id][i])
					{
						counter++;
						pDC->MoveTo(xoffset+gap[id]+4 + i*gap[id],yoffset-5);
						pDC->LineTo(xoffset+gap[id]+gap[id]-4 + i *gap[id],yoffset-5);
					}
				//}
			}
	
			if(m_keypressed[id])
			   m_nlen[id] = counter;
	
	
		xoffset =  28*iscal;

}
void CETApplicationView::GetUpdateOfHighRange()
{
		int id, i = 0, startidx = -1 ;
		int num=0;
		bool ret =0;
		FREZNODE retNode,retNodeMax; 
		ETFrequencyRangeArray freArray;
		FrequencyRange   range;
		range.nFreqMin = -1;
		range.nFreqMax = -1;
		CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		id  =  m_set_highside;

		CDocList *plist, *plistL, *plistR;
        plist  = id ? plistL = &pDoc->m_Lf_list[0] : plistR = &pDoc->m_Rt_list[0];

		

		for(i=0; i< plist->GetCount()-1;i++)   //HIGHFREQNUM; i++)
		{
			if(m_freqsetArry[id][i] )
			{
				num++;
				if(startidx == -1)
					startidx = i;
			}
		}

		if(startidx == -1)  return ;

		CRect rect;
	    this->GetClientRect(rect);
		rect.left = 600*iscal;
		rect.top = 702*yiscal;


	    retNode    = plist->GetAt(plist->FindIndex(startidx));
		retNodeMax = plist->GetAt(plist->FindIndex(startidx+num ));

		range.nFreqMin = retNode.band;
		range.nFreqMax = retNodeMax.band;

		ret = GetRangedFrequencyBands(m_sessionID,range,freArray);
		if(ret)
		{
			if(m_set_highside ==0)
		       m_narrowWarningRT = FALSE;
			else
			   m_narrowWarningLF = FALSE;

		   m_highRange[m_set_highside]  = freArray;
		}
		else
		{
		
			if(m_set_highside ==0 )   //right
			{
				m_narrowWarningRT = TRUE;
				m_lowfreqRTWarning = _T("Warning: Too narrow frequence in current Right select!");
			}
			else
			{
				m_narrowWarningLF = TRUE;
			    m_lowfreqLFWarning = _T("Warning: Too narrow frequence in current Left select!");
			}

		}

		//this->InvalidateRect(rect);
		//return range;
		
}
void CETApplicationView::AdjustHighRangeLine(CDC *pDC,int startidx, int nlen,UINT optType)
{
	CPen penR(PS_SOLID,5,RT_RED_COLOR );
	CPen penL(PS_SOLID,5,LF_BLUE_COLOR);
	CPen *poldpen;
	int id = 0;
	    id = m_set_highside;
	//int originmode;
	//num = plistctr[0]->GetHeaderCtrl()->GetItemCount() -1;
		//pDC->MoveTo(xoffset+gap+4 + i*gap,yoffset-5);
		// pDC->LineTo(xoffset+gap+gap-4 + i *gap,yoffset-5);
	//premode = pdc->SetROP2(R2_NOTXORPEN);

	 if(optType == 0)
		  return ;

    int offset = 0;
	int gap  = hightmpgap[id];
        poldpen = id ? pDC->SelectObject(&penL) :pDC->SelectObject(&penR);

		offset = id*683*iscal;
		xoffset += offset;



	   switch(optType)
	   {
			case VK_UP:
				if(xoffset+gap+gap-4 + gap * (startidx + nlen) < (683*iscal+offset))
				{
					pDC->MoveTo(xoffset+gap+4 + gap * (startidx + nlen ),yoffset -5);
					pDC->LineTo(xoffset+gap+gap-4 + gap * (startidx + nlen),yoffset -5);
					m_freqsetArry[id][startidx + nlen ]=TRUE;
					
				}
				break;
			case VK_DOWN:
				if(startidx+nlen-1 >0 && nlen !=1 )
				{
					pDC->SetROP2(R2_NOTXORPEN);
					pDC->MoveTo(xoffset+gap+4 + gap * (startidx + nlen -1),yoffset -5);
					pDC->LineTo(xoffset+gap+gap-4 + gap * (startidx + nlen-1),yoffset -5);
					m_freqsetArry[id][startidx + nlen -1]=FALSE;
					//m_nlen[m_set_highside] = m_nlen[m_set_highside] -1;
				}
			
				break;
			case VK_LEFT:
				if(startidx >0)
				{
					pDC->MoveTo(xoffset+gap+4 + gap * (startidx -1 ),yoffset -5);
					pDC->LineTo(xoffset+gap+gap-4 + gap * (startidx -1),yoffset -5);
					m_freqsetArry[id][startidx -1 ]=TRUE;

					pDC->SetROP2(R2_NOTXORPEN);
					pDC->MoveTo(xoffset+gap+4 + gap * (startidx + nlen -1),yoffset -5);
					pDC->LineTo(xoffset+gap+gap-4 + gap * (startidx + nlen -1),yoffset -5);
					m_freqsetArry[id][startidx + nlen-1 ]=FALSE;
				}

			
				break;
			case VK_RIGHT:
				if(xoffset+gap+gap-4 + gap * (startidx + nlen) < (683*iscal + offset) )
				{
					pDC->MoveTo(xoffset+gap+4 + gap * (startidx + nlen ),yoffset -5);
					pDC->LineTo(xoffset+gap+gap-4 + gap * (startidx + nlen),yoffset -5);
					m_freqsetArry[id][startidx + nlen ]=TRUE;

					pDC->SetROP2(R2_NOTXORPEN);
					pDC->MoveTo(xoffset+gap+4 + gap * (startidx  ),yoffset -5);
					pDC->LineTo(xoffset+gap+gap-4 + gap * (startidx ),yoffset -5);
					m_freqsetArry[id][startidx ]=FALSE;
				}

				//pDC->SetROP2(originmode);
				break;
	   }
	  	xoffset =  28*iscal;
}
BOOL CETApplicationView::WritePtaData(CFile &fpta, BOOL saveT )                //CFile fpta)
{ 
	 //LPTSTR pdata  = _T("\r\n32 45 64 34 13 45 65 23");   //LPTSTR 
	int n_number =0;
	int i = 0;
	int id =0;
	int numheader = 0;


	CTime theTime = CTime::GetCurrentTime();
	CString szTime = theTime.Format(" %Y-%m-%d_%H:%M ");
	CString newline = _T("\r\n");
	TCHAR str[10], strTR[32], strVis[2];
	CString strhead, strRight,strLeft;
	CString sepstr      = _T("\r\n    <------ Time: ");
	CString sepTstrhead = _T("\r\n  TT<------ Time: ");
	CString strTpost = _T("   ------>TT\r\n");
	CString endstr = _T("\r\n    ------ End ---");
	CString strSide[2] = {_T("\r\n      ------ R -----   \r\n"),_T("\r\n      ------ L -----   \r\n")};

	if(saveT)
	   sepstr =  sepTstrhead  + szTime;
	else
       sepstr =  sepstr  + szTime;

	if(saveT)
		sepstr +=  strTpost;
	else
	    sepstr +=  _T("   ------>  \r\n");

	LVCOLUMN lvcolmn;
	lvcolmn.mask = LVCF_TEXT;
	lvcolmn.pszText = str;
	lvcolmn.cchTextMax = 8;
	memset(str,0,10*sizeof(TCHAR));
	memset(strTR,0,32*sizeof(TCHAR));

	int tcharlen = sizeof(TCHAR);

	//int numheader = plistctr[0]->GetHeaderCtrl()->GetItemCount();

	fpta.Write(sepstr,_tcslen(sepstr)* tcharlen);

	for(id = 0; id <2; id++)
	{
		numheader = plistctr[id]->GetHeaderCtrl()->GetItemCount();
		fpta.Write(strSide[id],_tcslen(strSide[id])* tcharlen);
		for(i=0;i<numheader -1 ;i++)
		{
			plistctr[id]->GetColumn(i+1,&lvcolmn);
			strhead = lvcolmn.pszText;
			
			_tcsncat(str,_T("        "),8 - _tcslen(strhead));                  //right -22
			strhead = str;

			if(id == 0)
			{
               if(saveT)
				   strRight = plistctr[0]->GetItemText(3,i+1);
			   else
			       strRight = plistctr[0]->GetItemText(0,i+1);
			   _tcscpy(strTR,strRight);
			}
			if(id == 1)
			{
				if(saveT)
			        strLeft  = plistctr[1]->GetItemText(3,i+1);
				else
					strLeft  = plistctr[1]->GetItemText(0,i+1);
			   _tcscpy(strTR,strLeft);
			}
			strhead = strhead + strTR + _T("\r\n");
			//_tcscpy(strTR,strRight);
			//_tcsncat(strTR,_T("                      "),22 - _tcslen(strRight) );
			//strRight = strTR;
			//strhead  = strhead +strRight+strLeft+_T("\r\n");
			
			fpta.Write(strhead,_tcslen(strhead)* tcharlen);
			
		}
	}
	fpta.Write(endstr,_tcslen(endstr)* tcharlen);
	fpta.Write(newline,_tcslen(newline)*tcharlen);
	//fpta.Close();
//	Sleep(3000);
	return 0;
	//fpta.Write(pdata,_tcslen(pdata)*tcharlen);
}
BOOL CETApplicationView::WritePtaFile(BOOL saveT )
{
    CString pName  = _T("\r\n")+pPatientList[0]->GetItemText(0,3)+_T("\r\n");                 //_T("Danie\r\n");
	CString pAddr  = pPatientList[0]->GetItemText(2,1)+_T("\r\n");                                    //_T("Shanghai\r\n");
	CString pbirth = pPatientList[0]->GetItemText(1,1)+_T("\r\n");                                 //_T("04/05/80\r\n"); 
	CString pInsNO = pPatientList[0]->GetItemText(2,3)+_T("\r\n");                             // _T("12345\r\n");
	CString pID    = pPatientList[0]->GetItemText(0,1);    //+_T("\r\n");                                              //_T("54321");
	CString pGend  = pPatientList[0]->GetItemText(1,3)+_T("\r\n");                                                        // _T("Male\r\n");
	CString pEarR   = _T("       RIGHT       \r\n");
	CString pEarL   = _T("       LEFT        \r\n");

	CString pBeg  = _T("  --- Beg ---  \r\n");
	CString pSep  = _T("  --- End ---  \r\n");
	
	int tcharlen = sizeof(TCHAR);


	std::string absoluteFilename;
	std::wstring absolutewFilename;

	CString strPathName;
	bool result = FALSE;
	CFile fpta;
	CFile fidx;

		/*
			if(LINKED == m_LinkState)
			{
					result = GetPTAStorePathName(absoluteFilename);
				int cnt = 0;
				while (!result&&(cnt<10)){
					result = GetPTAStorePathName(absoluteFilename);
					Sleep(100);
					cnt++;
				}
				absolutewFilename = s2ws(absoluteFilename);
			 }
			if(NOLINKED == m_LinkState)
			{
		*/
	  //  strptapath = strptapath + _T("ptaresult.txt");

	    RegisterNewPTAandIdx();



		if(!pID.CompareNoCase(strprompt) || !pID.GetLength())
		{
			MessageBox(_T("No ID Associated, please retry"),0,MB_TOPMOST);
			return 1;
		}
		strptapath = GetPTAResultLocalPath();
		strptapath = strptapath + pID;
		strptapath = strptapath + _T("\\ptaresult.txt");
		absolutewFilename = strptapath;
		result = TRUE;

			if(result)
			{
		
				strPathName = absolutewFilename.c_str();

				//Pls Input ID ...

				if(fpta.m_hFile != CFile::hFileNull) 
				{
					MessageBox(_T("file not closed"));
				}

				if(pID.CompareNoCase(strprompt) && pID.GetLength())
				   fpta.Open(strPathName,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite);
				else
				{
				   MessageBox(_T("Please input ID first!"));
				   return 1;
				}
			}
			else
			{

				MessageBox(_T("Access Audiomed failed, please retry."));
				return -1;
				/*
						//Pls Input ID ...
						if(pID.CompareNoCase(strprompt) && pID.GetLength())
						   fpta.Open(pID,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite);
						else
						{
						   MessageBox(_T("Please input ID first!"));
						   return 1;
						}
				*/
			}
	
	ASSERT(fpta);
	ULONGLONG len = fpta.GetLength();
	if(len)
	{
	    //fpta.Seek(len,CFile::begin);
		if(len <= (500 * 1024))
		    fpta.SeekToEnd();
		else
		{
			MessageBox(_T("File records maxiam reached!"));
			return -1;
		}
	}
	else
	{
		fpta.SeekToBegin();
		fpta.Write(("\xFF\xFE"),2);
	    fpta.Write(pID,_tcslen(pID)* tcharlen);       //id is mandory
		fpta.Write(pName,_tcslen(pName)* tcharlen);
		fpta.Write(pAddr,_tcslen(pAddr)* tcharlen);
		fpta.Write(pbirth,_tcslen(pbirth)* tcharlen);
		fpta.Write(pInsNO,_tcslen(pInsNO)* tcharlen);
		fpta.Write(pGend,_tcslen(pGend)* tcharlen);
		fpta.Write(pEarR,_tcslen(pEarR)* tcharlen);
		fpta.Write(pEarL,_tcslen(pEarL)* tcharlen);

		
		fidx.Open(m_stridxpath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite);
		ASSERT(fidx);
		ULONGLONG lens = fidx.GetLength();
		if(lens)
		{
			 fidx.SeekToEnd();
		}
		else
		{
			 fidx.SeekToBegin();
			 fidx.Write(("\xFF\xFE"),2);

		}
		fidx.Write(pBeg,_tcslen(pBeg) * tcharlen);
		fidx.Write(pID,_tcslen(pID)* tcharlen);       
		fidx.Write(pName,_tcslen(pName)* tcharlen);
		fidx.Write(pbirth,_tcslen(pbirth)* tcharlen);
		fidx.Write(pGend,_tcslen(pGend)* tcharlen);
		fidx.Write(pAddr,_tcslen(pAddr)* tcharlen);
		fidx.Write(pInsNO,_tcslen(pInsNO)* tcharlen);
		fidx.Write(pSep,_tcslen(pSep) * tcharlen);
		fidx.Close();
//		MessageBox(_T("New user saved!"));
		MessageBox(m_buffNewUserSaved);
		 
	}

	   // fpta.Write(pdata,_tcslen(pdata)*tcharlen);

	    if(saveT)
		{
			// if(fReadPta.m_hFile != CFile::hFileNull) fReadPta.Close();
			WritePtaData(fpta, TRUE); 
		}
		else
		{
			if(!g_bPTANeedSaved)   //    verify normal PTA result saved only once!
			{
				 fpta.Close();
				return 1;
			}
			WritePtaData(fpta, FALSE); 
		}

		if(fpta.m_hFile != CFile::hFileNull) 
		{
			 
			  fpta.Close();
			  // MessageBox(_T("end"));
		}

	Sleep(500);
	if(LINKED == m_LinkState)
	{
		CutPcFileToAudm();
		Sleep(1000);
	}
		
	return 0;
}
LRESULT CETApplicationView::OnProgressBarStep(WPARAM   wParam   ,LPARAM   lParam)
{
	if(wParam == -1)
	{
		if(pctrlprogress)
		{
			pctrlprogress->DestroyWindow();
			pdlgprogress = NULL;
		}
		if(pdlgprogress)
		{
			pdlgprogress->DestroyWindow();
			pdlgprogress = NULL;
		}
		return -1;
	}
	if(wParam != 100)
	{
		if(pctrlprogress)
		{
			pctrlprogress->StepIt();
		}

	}
	if(wParam == 100)
	{
/*
		    while(pctrlprogress->GetPos() != 100)
		    {
				pctrlprogress->StepIt();
		    }
*/

			if(pctrlprogress)
			{
				if(pctrlprogress->GetPos() != 100)
					pctrlprogress->SetPos(100);
			}

			if(pdlgprogress)
			{
				pdlgprogress->ShowWindow(SW_HIDE);
			}

			pbutt[0]->EnableWindow(TRUE);
			pbutt[3]->EnableWindow(TRUE);
	//#ifdef SUPPORT_SITE_TREATMENT				
			if(m_treatstage)
			    pbutt[5]->EnableWindow(TRUE);
    //#endif
			pbutt[7]->EnableWindow(TRUE);
			MessageBox(m_buffstpgen);  //_T("Profile created!"));
			if(pctrlprogress)
			{
				pctrlprogress->DestroyWindow();
				pdlgprogress = NULL;
			}
			if(pdlgprogress)
			{
				pdlgprogress->DestroyWindow();
				pdlgprogress = NULL;
			}
	}
	return 0;

}

BOOL CETApplicationView::DelPTARecord(BOOL arrRec[],BOOL isloadT )
{


	CString pID    = pPatientList[0]->GetItemText(0,1);
	CString pEarR   = _T("       RIGHT       \r\n");
	CString pEarL   = _T("       LEFT        \r\n");

	CString sepTstrhead = _T("\r\n  TT<------ Time: ");
	CString sepstr      = _T("\r\n    <------ Time: ");
	CString sepstrpost = _T("  ------>  \r\n");
	CString strTpost   = _T("   ------>TT\r\n");           
	CString endstr = _T("--- End ---");

	BOOL *prec = NULL;
	prec = arrRec;
	TCHAR  *pbuff, *pbuffMemo;
	LPTSTR  pret, pstore; 
	BOOL   fret = FALSE;

	TCHAR  *pOnerecbuff = NULL;
	int strnum = 0;

	int idxnum = 0;
	int tcharlen = sizeof(TCHAR);
	int bufsize = 0, onerecsize = 0;
	int timeoffset = 0;
	bool result = FALSE;

	std::string absoluteFilename;
	std::wstring absolutewFilename;

	CString strPathName;
/*
	if(LINKED == m_LinkState)
	{
		result = GetPTAStorePathName(absoluteFilename);
		int cnt = 0;
		while (!result&&(cnt<10)){
			result = GetPTAStorePathName(absoluteFilename);
			Sleep(100);
			cnt++;
		}
		absolutewFilename = s2ws(absoluteFilename);
	}
	if(NOLINKED == m_LinkState)
	{
*/


		if(!pID.CompareNoCase(strprompt) || !pID.GetLength())
		{
			MessageBox(_T("No ID Associated, please retry"),0,MB_TOPMOST);
			return 1;
		}
		strptapath = GetPTAResultLocalPath();
		strptapath = strptapath + pID;
		strptapath = strptapath + _T("\\ptaresult.txt");
		absolutewFilename = strptapath;
		result = TRUE;
/*
	}
*/
	if(result)
	{
		strPathName = absolutewFilename.c_str();
		if(pID.CompareNoCase(strprompt) && pID.GetLength())
		{
		     fret = fReadPta.Open(strPathName,CFile::modeReadWrite);
			  if(fret == 0)
			  {
				 MessageBox(m_buffnopta,0,MB_TOPMOST); //_T("No PTA record founded!"));
			     return 1;
			  }

		}
		else
		{
				 MessageBox(_T("PTA File open fail!"));
				 return 1;
		}

	}
	else
	{
				MessageBox(_T("Access AudioMed failed, please retry."));
				return 1;
	}

	ASSERT(fReadPta);
	ULONGLONG len = fReadPta.GetLength();
		
//	len = len;			
	bufsize = len / tcharlen;

//	pbuff = new TCHAR [bufsize];
	
	pbuffMemo = new TCHAR [bufsize];
	pbuff = pbuffMemo;

	memset(pbuff,0,bufsize*sizeof(TCHAR));

	if(len)
	{
		fReadPta.SeekToBegin();
		fReadPta.Read(pbuff,len);
	}

		 pstore = pbuff;
		CString strtime;
		if(isloadT)
		{
			sepstr = sepTstrhead;
			sepstrpost = strTpost;
		}
		else
		{
			;
		}

		while((pret = _tcsstr(pbuff,sepstr)) != NULL)
		{
			 pbuff = pret;
		
			 if( _tcsstr(pbuff,endstr) != NULL)
			 {
				 onerecsize =  _tcsstr(pbuff,endstr) - pbuff + endstr.GetLength();   //here anson need to update  //prec
				 pOnerecbuff = new TCHAR[onerecsize];
				 tmemset(pOnerecbuff,_T(' '),onerecsize);
			 }
			 else
				 break;


			 if(prec[idxnum])                  //checked to del
			 {
				 _tcsncpy(pbuff,pOnerecbuff,onerecsize);
			 }

			 pbuff = pbuff + onerecsize;
			 idxnum++;

		}

		

	if(fReadPta.m_hFile != CFile::hFileNull) 
		   fReadPta.Close();    

	fret = fReadPta.Open(strPathName,CFile::modeCreate|CFile::modeReadWrite);
	fReadPta.Write(pstore,len);		
	if(fReadPta.m_hFile != CFile::hFileNull) 
		   fReadPta.Close(); 

	Sleep(500);
	if(LINKED == m_LinkState)
	{
		CutPcFileToAudm();
		Sleep(1000);
	}
		
	delete [] pbuffMemo;

	return 0;
}

BOOL CETApplicationView::ReadPtaFile(BOOL  isloadT)
{
		
	//CFile fReadPta;
    CString pName ; // = _T("\r\n")+pPatientList[0]->GetItemText(0,1)+_T("\r\n");                 //_T("Danie\r\n");
	CString pAddr ;// = pPatientList[0]->GetItemText(0,3)+_T("\r\n");                                    //_T("Shanghai\r\n");
	CString pbirth ;//= pPatientList[0]->GetItemText(1,1)+_T("\r\n");                                 //_T("04/05/80\r\n"); 
	CString pInsNO ;//= pPatientList[0]->GetItemText(1,3)+_T("\r\n");                             // _T("12345\r\n");
	CString pID    = pPatientList[0]->GetItemText(0,1);    //+_T("\r\n");                                              //_T("54321");
	CString pGend  ;//= pPatientList[0]->GetItemText(2,3)+_T("\r\n");                                                        // _T("Male\r\n");
	CString pEarR   = _T("       RIGHT       \r\n");
	CString pEarL   = _T("       LEFT        \r\n");

	CString sepTstrhead = _T("\r\n  TT<------ Time: ");
	CString sepstr      = _T("\r\n    <------ Time: ");
	CString sepstrpost = _T("  ------>  \r\n");
	CString strTpost   = _T("   ------>TT\r\n");           
	//CString endstr = _T("\r\n    ------ End ---");
	CString endstr = _T("--- End ---");

	TCHAR  *pbuff, *pbuffMemo;
	CLDPTARDList  *pLoadlist;
	LPTSTR  pret, pstore; // ,pstrtime;
	BOOL   fret = FALSE;
	TCHAR  strtimebuff[20];
	TCHAR  *pOnerecbuff = NULL;
	int strnum = 0;

	LDPTARECNODE ptaldnode;

	int tcharlen = sizeof(TCHAR);
	int bufsize = 0, onerecsize = 0;
	int timeoffset = 0;
	bool result = FALSE;

	std::string absoluteFilename;
	std::wstring absolutewFilename;

	CString strPathName;

/*
	if(LINKED == m_LinkState)
	{
		 result = GetPTAStorePathName(absoluteFilename);
		int cnt = 0;
		while (!result&&(cnt<10)){
			result = GetPTAStorePathName(absoluteFilename);
			Sleep(100);
			cnt++;
		}

		absolutewFilename = s2ws(absoluteFilename);
		
	}
	if(NOLINKED == m_LinkState)
	{
*/
	   // strptapath = strptapath + _T("ptaresult.txt");

		if(!pID.CompareNoCase(strprompt) || !pID.GetLength())
		{
			MessageBox(_T("No ID Associated, please retry"),0,MB_TOPMOST);
			return 1;
		}
		strptapath = GetPTAResultLocalPath();
		strptapath = strptapath + pID;
		strptapath = strptapath + _T("\\ptaresult.txt");
		absolutewFilename = strptapath;
		result = TRUE;
/*
	}
*/

	if(result)
	{
		strPathName = absolutewFilename.c_str();
		if(pID.CompareNoCase(strprompt) && pID.GetLength())
		{
		     fret = fReadPta.Open(strPathName,CFile::modeReadWrite);
			  if(fret == 0)
			  {
				 MessageBox(m_buffnopta,0,MB_TOPMOST) ;//"No PTA record founded!"));
			     return 1;
			  }

		}
		else
		{
				 MessageBox(_T("PTA File open fail!"),0,MB_TOPMOST);
				 return 1;
		}

	}
	else
	{
				MessageBox(_T("Access AudioMed failed, please retry."),0,MB_TOPMOST);
				return 1;
/*
		if(pID.CompareNoCase(strprompt) && pID.GetLength())
		   fReadPta.Open(pID,CFile::modeReadWrite);
		else
		{
		   MessageBox(_T("PTA File open fail!"));
		   return 1;
		}
*/
	
	}

	
	ASSERT(fReadPta);
	ULONGLONG len = fReadPta.GetLength();
	//if(len)
	//    fReadPta.Seek(len,CFile::begin);
	//else

	len = len + 8;

	bufsize = len / tcharlen;

//	pbuff = new TCHAR [bufsize];
	pbuffMemo = new TCHAR [bufsize];
	pbuff = pbuffMemo;

	memset(pbuff,0,bufsize*sizeof(TCHAR));
	memset(strtimebuff,0,20*tcharlen);

	if(len)
	{
		fReadPta.SeekToBegin();
		//fReadPta.Seek(CFile::begin,2);
		//fReadPta.Read(("\xFF\xFE"),2);
	
		//fReadPta.Read(pID,_tcslen(pID)* tcharlen);       //id is mandory
		//fReadPta.Read(pID,40);
		fReadPta.Read(pbuff,len);
		//this->PTAcharToUnic(pID);
		//fReadPta.Read(pName,40);
		/*fReadPta.ReadString(pAddr);
		fReadPta.ReadString(pbirth);
		fReadPta.ReadString(pInsNO);
		fReadPta.ReadString(pGend);
		fReadPta.ReadString(pEarR);
		fReadPta.ReadString(pEarL);*/
	}

	    pstore = pbuff;
		CString strtime;
		if(isloadT)
		{
			sepstr = sepTstrhead;
			pLoadlist  = &cldT_list;
			sepstrpost = strTpost;
		}
		else
		{
			pLoadlist = &cldlist;
		}
		pLoadlist->RemoveAll();  //cldT_list

		while((pret = _tcsstr(pbuff,sepstr)) != NULL)
		{
			 pbuff = pret + sepstr.GetLength();  ///sepstrpost
			 //pstrtime = pbuff;
			 if( _tcsstr(pbuff,endstr) != NULL)
			 {
			     onerecsize =  _tcsstr(pbuff,endstr) - pbuff;   //here anson need to update
			 }
			 else
				 break;
			 if(_tcsstr(pbuff,sepstrpost) != NULL)
			 {
				
			    timeoffset =  _tcsstr(pbuff,sepstrpost) - pbuff;   //here anson need to update
			 }
			 else
			    break;

			 strtime = _tcsncpy(strtimebuff,pbuff,timeoffset);
			 strtime.Trim(_T(" "));
			 strnum++;
			// ptaldnode.time = strtime;
			 _tcscpy(ptaldnode.time,strtime);
			 ptaldnode.pstrpos = pbuff;
			 ptaldnode.idx = strnum;
			 ptaldnode.sizeofbuff = onerecsize;
			 pLoadlist->AddTail(ptaldnode);

		}
	 
		
	    if(fReadPta.m_hFile != CFile::hFileNull) 
		   fReadPta.Close();           

//	delete [] pbuffMemo;
	return 0;

}

void CETApplicationView::OnTimer( UINT time )
{
	DWORD lasttime;
	CString str, strmin;

	CRect rect;
	this->GetClientRect(rect);

	if(time == IDT_TIMER)
	{
					lasttime = GetTickCount() - dwStartTickTime;

					if(g_bIsLocalPlaying && lasttime >= 3600000)
					{
						g_bIsLocalPlaying= FALSE;
						StopTreatmentPlaying();
						pbutt[5]->pressed = FALSE;
						pbutt[5]->ModifyStyle(BS_OWNERDRAW,0);
						pbutt[5]->Invalidate();
					}

					lasttime %= 3600000;

					str = m_buffTimer; //_T("Time:");
					strmin.Format(_T("%2dm:"),lasttime/60000);
					str += strmin;
	
					lasttime %= 60000;
					strmin.Format(_T(" %ds"),lasttime/1000);
					str += strmin;


					strTime = str;

					if(strendTime.Compare(_T("")) )  
					{
						DWORD off = GetTickCount() - dwEndTime;
						if(off <= 10000)
							strTime = strendTime;
						else
						{
							if(m_nTimer != NULL)
							{
								this->KillTimer(m_nTimer);		
								m_nTimer = NULL;
							}
							strendTime = _T("");
						}
					}
					rect.left = 1270*iscal;
					rect.top = 702*yiscal;

					this->InvalidateRect(rect);

		}
	if(time == IDT_TIMERWARN)
	{
			rect.left = 600*iscal;
		    rect.top = 702*yiscal;
			m_strwarnct = _T("");

		if(m_warncount != 0)
		{
			m_warncount--;
			m_strwarnct.Format(_T("%d"),m_warncount);
		}
		else
		{
			m_warncount = 15;
			m_ShowWarning = FALSE;
			
			if(m_nwarnTime != NULL)
			{
				this->KillTimer(m_nwarnTime);		
				m_nwarnTime = NULL;
			}
		}
		  this->InvalidateRect(rect);
	}

			CView::OnTimer(time);
	
}


void CETApplicationView::CopyScreenToBitmap(int  xStartPt,int  yStartPt, int width, int height,int  xToCopy, int yToCopy)
{
        CBitmap* pBackBitmap = NULL; 
		CBitmap* pclistctlbmp = NULL;
        CDC ScrDC,MemDC;
	    CRect viewrect;

        pBackBitmap = new CBitmap();
		if(pBackBitmap == NULL)
			return;
      
        ScrDC.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
        MemDC.CreateCompatibleDC(&ScrDC);

        pBackBitmap->CreateCompatibleBitmap(&ScrDC,width,height);
        CBitmap*  pMemDCOldBit = MemDC.SelectObject(pBackBitmap);

        //开始拷贝
        MemDC.BitBlt(xStartPt, yStartPt, width, height,&ScrDC,xToCopy,yToCopy,SRCCOPY);

		int ctlwidth,ctlhigh;
		pclistctlbmp = new CBitmap();
		if(pclistctlbmp == NULL)
		{
			if (pBackBitmap != NULL)
			{
                delete pBackBitmap;
                pBackBitmap = NULL;
			}
			return;
		}
				
		ctlwidth = m_storelistrect.Width()*2 + 16 * iscal;
		ctlhigh = m_storelistrect.Height()+ 6*yiscal ;
		pclistctlbmp->CreateCompatibleBitmap(&MemDC,ctlwidth,ctlhigh);
		//pclistctlbmp->CreateCompatibleBitmap(&ScrDC,ctlwidth,ctlhigh);


		this->GetClientRect(viewrect);
		this->ClientToScreen(viewrect);

//		if(m_listdc.m_hDC != NULL)
//		   m_listdc.DeleteDC();

//		m_listdc.Detach();
//		m_listdc.m_hDC = NULL;
//		m_listdc.m_hAttribDC  = NULL;

		m_listdc.CreateCompatibleDC(&MemDC);

		CBitmap*  pOldBitmap = m_listdc.SelectObject(pclistctlbmp);
		m_listdc.BitBlt(0,0,ctlwidth,ctlhigh,&MemDC, 8*iscal ,m_storelistrect.top + viewrect.top ,SRCCOPY);  //here we need to adjust full screen
		//m_listdc.BitBlt(0,0,ctlwidth,ctlhigh,&ScrDC, 8 ,m_storelistrect.top + 64 ,SRCCOPY); 
//		m_listdc.SelectObject(pOldBitmap);
 //       ScrDC.DeleteDC();
 //       MemDC.DeleteDC();
		MemDC.SelectObject(pMemDCOldBit);

		if (pBackBitmap != NULL)
        {
                delete pBackBitmap;
                pBackBitmap = NULL;
        }
//		delete []pclistctlbmp;
/*
		if( pclistctlbmp != NULL)
		{
			delete pclistctlbmp;
			pclistctlbmp = NULL;
		}
*/
}

void CETApplicationView::PTAcharToUnic(CString &str)
{
	int num = str.GetLength();
    char *szBuf = new char[num];
    for (int i = 0 ; i < num; i++)
    {
        szBuf[i] = str.GetAt(i);
    }
    CharToUnicode(szBuf , &str);
    delete []szBuf;
}


BOOL  CETApplicationView::ReadExistsIDinfo(CString str)
{
		CFile   fptainfo;
		CString pID    = pPatientList[0]->GetItemText(0,1); 
		CString pName ;
		CString pAddr ;
		CString pbirth ;
		CString pInsNO ;
		CString pGend  ;
		CString strstor[6];
		CString pEarR   = _T("       RIGHT       \r\n");
		CString endtoken     = _T("\r\n");

		TCHAR  *pbuff = NULL ;
		TCHAR  *pheadbuff = NULL;
		CLDPTARDList  *pLoadlist;
		LPTSTR  pret, pstore;
		BOOL   fret = FALSE;
		TCHAR  strarr[6][32];

		TCHAR  strtimebuff[20];
		TCHAR  *pOnerecbuff = NULL;
		int strnum = 0;

		LDPTARECNODE ptaldnode;

		int tcharlen = sizeof(TCHAR);
		int bufsize = 0, onerecsize = 0;
		int timeoffset = 0;
		bool result = FALSE;

		memset(strarr,0,6 * 32 *sizeof(TCHAR));

		if(pID.CompareNoCase(strprompt) && pID.GetLength())
		{
				fret = fptainfo.Open(str,CFile::modeReadWrite);
				if(fret == 0)
				{
					MessageBox(m_buffnopta,0,MB_TOPMOST) ;//"No PTA record founded!"));
					return 1;
				}

		}
		else
		{
				MessageBox(_T("PTA File open fail!"),0,MB_TOPMOST);
				return 1;
		}

		ASSERT(fptainfo);
		ULONGLONG len = fptainfo.GetLength();

		len = len + 8;

		bufsize = len / tcharlen;

		pbuff = new TCHAR [bufsize];
		memset(pbuff,0,bufsize);
		memset(strtimebuff,0,20*tcharlen);

		if(len)
		{
			fptainfo.SeekToBegin();
			fptainfo.Read(pbuff,len);

		}

	    pstore = pbuff;
		CString strtime;

		if((pret = _tcsstr(pbuff,pEarR)) != NULL )
		{
			int len = pret - pbuff;
			pheadbuff = new TCHAR [len];
			memset(pheadbuff,0,len);
			_tcsncpy(pheadbuff,pbuff,len);
		}

		int i = 0;
		while((pret = _tcsstr(pheadbuff,endtoken)) != NULL)   //strstor
		{
			int lenln = pret - pheadbuff;
			_tcsncpy(strarr[i],pheadbuff,lenln);
			strstor[i] = strarr[i];
			i++;
			pheadbuff = pret + endtoken.GetLength();

		}

		pPatientList[0]->SetItemText(0,3,strstor[1]);        //name
        pPatientList[0]->SetItemText(1,1,strstor[3]);        //age
		pPatientList[0]->SetItemText(1,3,strstor[5]);        //gender
		pPatientList[0]->SetItemText(2,1,strstor[2]);        //address
		
	    if(fptainfo.m_hFile != CFile::hFileNull) 
		   fptainfo.Close();           

	return 0;

}


BOOL CETApplicationView::TestIfUserExists(CString str)
{
	CString strsyspath;
	DWORD errorret = 0;
	BOOL  ret;
	//CRect rect;
	CString strfile;
	//this->GetClientRect(rect);
	strsyspath = GetPTAResultLocalPath();

	if(str.CompareNoCase(strprompt) && str.GetLength())
	{
		     strsyspath = strsyspath + str;

			 ret = CreateDirectory(strsyspath,NULL);
			 if(!ret)
			 {
				 errorret = GetLastError();
				 if(ERROR_ALREADY_EXISTS == errorret)
				 {
					 strfile = strsyspath + _T("\\");
					 strfile = strfile + _T("ptaresult.txt");

					 strWarning = m_buffwarnid;  //_T("Warning: ID already in use! ");
					 m_ShowWarning = TRUE;

					 ReadExistsIDinfo(strfile);

					 if(m_nwarnTime != NULL)
						this->KillTimer(m_nwarnTime);

					m_nwarnTime = this->SetTimer(IDT_TIMERWARN,1000,0); //

					 return TRUE;
				 }
				 else
					 return FALSE;
			 }
			 else
			 {
					 RemoveDirectory(strsyspath);
					 return FALSE;
			 }
	}
	else
		return FALSE;

	//-------------------------------------------------
}
CString CETApplicationView::ReadSinglePtaDataRecord(UINT idx)
{
	
	CString strtime;
	LDPTARECNODE node;

	node = cldlist.GetAt(cldlist.FindIndex(idx));
	
	strtime = node.time;

	return strtime;
}
CString CETApplicationView::ReadSingleTT_PtaDataRecord(UINT idx)
{
	
	CString strtime;
	LDPTARECNODE node;
	node = cldT_list.GetAt(cldT_list.FindIndex(idx));
	strtime = node.time;
	return strtime;
}
void    CETApplicationView::SavePTARecordIdx(int one,int two,int three)
{
	m_PTArecordChecked[0] = one;
	m_PTArecordChecked[1] = two;
	m_PTArecordChecked[2] = three;
}
int CETApplicationView::GetPtaRecordNum()
{
	return cldlist.GetCount();
}
int CETApplicationView::GetPtaTTRecordNum()
{
	return cldT_list.GetCount();
}
// CETApplicationView printing

void CETApplicationView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS

	AFXPrintPreview(this);
#endif
}

BOOL CETApplicationView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	//*******************
		PRINTDLG   pd;   
		HGLOBAL  m_hDevMode;
        pd.lStructSize=(DWORD)sizeof(PRINTDLG);   
		BOOL   bRet = ::AfxGetApp()->GetPrinterDeviceDefaults(&pd); 
		 
		m_hDevMode = pd.hDevMode;

		 if(bRet)   
        {    
            DEVMODE   FAR   *pDevMode=(DEVMODE   FAR   *)::GlobalLock(m_hDevMode);   
            pDevMode->dmOrientation=DMORIENT_LANDSCAPE;       
            ::GlobalUnlock(m_hDevMode);   
        } 


   //****************************

	return DoPreparePrinting(pInfo);
}

void CETApplicationView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CETApplicationView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing

					
}

void CETApplicationView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	//OnContextMenu(this, point);
}

BOOL CETApplicationView::PreTranslateMessage(MSG* lpmsg )
{
	BOOL bHandleNow = FALSE;
	BOOL bPushedButton = FALSE ;
	BOOL bPTAMoveUP = FALSE;
	short altkey;
			switch (lpmsg->message)
			{   
			   case WM_SYSKEYDOWN:
		       			altkey = GetKeyState(VK_MENU) & 0xff00;
					 if((altkey == (short)0xff00) && lpmsg->wParam == 0x4D)  //4D is M key  VK_TAB
						  pmic = SetMicPhoneStat();
					 if((altkey == (short)0xff00) && (lpmsg->wParam == 0x48 || lpmsg->wParam == 0x4C ))
					 {
							 if(m_micstat)
							    bHandleNow = TRUE;
							 else
								bHandleNow = FALSE;
					 }

					 if((altkey == (short)0xff00) && lpmsg->wParam == 0x53)  //0x53 is S key NameSearch
					 {
						 bPushedButton = TRUE;
						 if(pbutt[0]->pressed)
						    StartNameSearch();
					 }
					 break;

	         
				case WM_KEYDOWN:											
					switch (lpmsg->wParam)
					{
						
						case VK_UP:
						case VK_DOWN:
						case VK_LEFT:
						case VK_RIGHT:
							 if(IsAmp)    // normal action in AMP mode
							 {
								 bHandleNow = FALSE;
								 break;
							 }
							 else           //PTA high band select operation
							 {
								 if(m_manualmod && m_isPTATesting)
								 {
									  bHandleNow = FALSE;
								 }
								 else
								 {
								     bHandleNow = TRUE;
								 }
								 break;
							 }
						
						/*case 0x41:          //A key
						case 0x5A:          //Z key
							 if(m_micstat)
							    bHandleNow = TRUE;
							 else
								bHandleNow = FALSE;*/

							 break;
						case VK_RETURN:
							    bHandleNow = TRUE;
								break;

					}
          			break;

				case WM_KEYUP:
					 switch(lpmsg->wParam)
					 {
					    case VK_SPACE:
							 if(IsAmp)
							 {
								 bHandleNow = TRUE; //FALSE;
								 return bHandleNow; //break;
							 }
							 else
							 {
								// bHandleNow = FALSE;
								 if(m_manualmod && m_isPTATesting)
								 {
									OnKeyUp(lpmsg->wParam, LOWORD(lpmsg ->lParam), HIWORD(lpmsg->lParam));								 
								  }
								  //break;
								 return TRUE;
							 }

							 
						case VK_RETURN:
							  bPushedButton = TRUE;
							  OnKeyUp(lpmsg->wParam, LOWORD(lpmsg ->lParam), HIWORD(lpmsg->lParam));
							   break;
						case VK_UP:
						case VK_DOWN:
						case VK_LEFT:
						case VK_RIGHT:
							   if(IsAmp)
							   {
								 bHandleNow = FALSE;
								 break;
							   }
							   else
							   {
									 if(m_manualmod && m_isPTATesting)
									 {
										  bHandleNow = FALSE;
										  break;
									 }
									  else
									  {
										 bPTAMoveUP = TRUE;
										 OnKeyUp(lpmsg->wParam, LOWORD(lpmsg ->lParam), HIWORD(lpmsg->lParam));
										 return bPTAMoveUP;
									  }
							   }

					 }
					 break;
  

			} 
			if (bHandleNow) 
			{

					   OnKeyDown(lpmsg->wParam, LOWORD(lpmsg ->lParam), HIWORD(lpmsg->lParam));
			}
			else 
				   return bPushedButton;

	return bHandleNow; 
           //return CView::PreTranslateMessage(lpmsg);
}

void CETApplicationView::StartNameSearch()
{
	CString id,name,age, gender;
	CNameSearchDlg nmsdlg;
	if(nmsdlg.DoModal() == IDOK)
	{

			id = nmsdlg.str_id;
			name = nmsdlg.str_name;
			age = nmsdlg.str_age;
			gender = nmsdlg.str_gender;
		
			if(id.GetLength()> 0)
		{
			pPatientList[0]->SetItemText(0,1,id);
			pPatientList[0]->SetItemText(0,3,name);
			pPatientList[0]->SetItemText(1,1,age);
			pPatientList[0]->SetItemText(1,3,gender);
		}

		/*if(!TestIfUserExists(id))
		{

			m_ShowWarning = TRUE;
			m_warncount = 15;
			m_strwarnct = _T("15");
			KillTimer(m_nwarnTime);
		}*/
	}


}
void CETApplicationView::UpdateCtrlListData()
{
	//********************************
	CString str;
	float   xPosVal=0,yPosVal=0;
	int i=0, j=0,id=0;
	TCHAR strbuff[10];
		
	LVCOLUMN lvcolmn;
	lvcolmn.mask = LVCF_TEXT;
	lvcolmn.pszText = strbuff;
	lvcolmn.cchTextMax = 8;
	memset(strbuff,0,10*sizeof(TCHAR));

	int num = 0; 

	LNNODE *plineNode , node;
		
		for(id =0 ; id<=1;id++)         //0right  1left    
		{
		    num	= plistctr[id]->GetHeaderCtrl()->GetItemCount() -1;
			  if (num !=0 )
					gap[id] = linewidth /num;
			  else
					gap[id] = linewidth;
			  plineNode = new LNNODE [num];

		  for(j=0;j<=2;j++)              //nItem
		  {                                    
				for(i=0;i <num;i++)
				{
					str = plistctr[id]->GetItemText(j,i+1);
					if(!str.GetLength())
						break;
					plistctr[id]->GetColumn(i+1,&lvcolmn);
					plineNode[i].y = _ttof(str);
					plineNode[i].x = _ttof(strbuff);

				//}

				//for(i=0;i<num;i++)
				//{//26
				//	str = plistctr[id]->GetItemText(j,i+1);
				//	if(!str.GetLength())
				//		break;
					xPosVal = xoffset+gap[id]*(i+1)+ 683*iscal*id;

					yPosVal = plineNode[i].y * 2.6*yiscal;
					yPosVal = yoffset + 26*yiscal+ yPosVal;   //0 is base line

					node.x = xPosVal;
					node.y = yPosVal;
					//node.bandval = plineNode[i].x;

					if(id == 0)
						this->m_postR_list[j].SetAt(this->m_postR_list[j].FindIndex(i),node);

					if(id == 1) 
					    this->m_postL_list[j].SetAt(this->m_postL_list[j].FindIndex(i),node);


				}
		  }
		}
		
	//**********************************
	this->RedrawWindow();

}
void CETApplicationView::AdjustMicVol(UINT nChar, UINT rep)
{
	    int i = 0;
		CString strvol;
		//	strvol = _T("Vol: ") + strvol;
		//->GetMasterVolumeLevelScalar(&currvolumn);
		//	strvol.Format(_T("%3.1f"),currmicrvol*100);
	   //strvol = _T("Vol: ") + strvol;

		if(nChar == 0x48)  //0x41  :A key   0x5A   :Z key
		{
			for(i=0; i<rep; i++)
				pmic->VolumeStepUp(NULL);


		}
		if(nChar == 0x4C)
		{
			for(i=0; i<rep; i++)
				pmic->VolumeStepDown(NULL);

		}
		pmic->GetMasterVolumeLevelScalar(&currmicrvol);
		strvol.Format(_T("%3.1f"),currmicrvol*100);
		strvol = _T("Vol: ") + strvol;
		pPatientList[1]->SetItemText(2,2,strvol);

}
IAudioEndpointVolume* CETApplicationView::SetMicPhoneStat(BOOL firststart)
{
	IMMDevice*   pEndpoint   =   NULL;
	IMMDeviceCollection*   pCollection=NULL;
	IMMDeviceEnumerator *pEnumerator=NULL;
	
	IAudioEndpointVolume*   pAudioEndVol   =   NULL;
	IPropertyStore *ppropstore = NULL;
	//IAudioSessionManager    *pAudioSession = NULL;
	BOOL     mutestat = FALSE;
	HRESULT  hr;
	DWORD  pronum = 0;
	CString  strvol;
	CString  strpromp = _T(" ON (\"Alt+H\": inc \"Alt+L\":dec)");//_T(" ON (\"A\": inc \"Z\":dec)");
	float vol = 0;
 
	PROPVARIANT varName;
	PropVariantInit(&varName);
	PROPERTYKEY *pkey = NULL;

    hr = CoCreateInstance(
           __uuidof(MMDeviceEnumerator), NULL,
           CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
           (void**)&pEnumerator);


	hr   =   pEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE,   &pCollection); 
	hr   =   pEnumerator->GetDefaultAudioEndpoint(eCapture,// eRender,   
		                                          eMultimedia,   &pEndpoint); 
	if(hr == E_NOTFOUND)  //no mic connected
	{
			pPatientList[1]->SetItemText(2,1,m_buffstrunlink);   //_T("No connect"));
			pPatientList[1]->SetItemText(2,2,_T(""));
		    pPatientList[1]->SetItemText(2,3,_T(""));
			return FALSE;
	}
	hr   =   pEndpoint-> Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pAudioEndVol);

	hr   =   pEndpoint->OpenPropertyStore(STGM_READWRITE,(IPropertyStore**)&ppropstore);
	         ppropstore->GetCount(&pronum);  

   if(firststart == TRUE)  //close mic when first start
   {
	   mutestat = FALSE;
   }
   else
   {
	   	hr   =   pAudioEndVol->GetMute(&mutestat);
	           
   }

            m_micstat = mutestat;

	hr   =   pAudioEndVol->GetMasterVolumeLevelScalar(&currmicrvol);
	         
	         if(pronum >0 )
			 {
			     pkey = new PROPERTYKEY [pronum];
				 for(int i =0 ;i < pronum; i++ )
				 {
					 hr = ppropstore->GetAt(i,&pkey[i]);
					 ppropstore->GetValue(pkey[i],&varName);
					 		if(varName.vt == 11)
							{    
								//varName.vt = VT_EMPTY;
								//varName.boolVal = 0;     // 0 is remove
								//varName.boolVal = -1;   // -1 is have
								if(m_micstat)   //mic is muted ,need to open
								   varName.boolVal = -1;
								else           //mic is opened, need to remove
								   varName.boolVal = 0;

								ppropstore->SetValue(pkey[i],varName);
							}
				 }
				 delete [] pkey;
				 pkey = NULL;

			 }
	
	         strvol.Format(_T("%3.1f"),currmicrvol*100);
	         strvol = _T("Vol: ") + strvol;

	hr   =   pAudioEndVol->SetMute(!mutestat, NULL);

	COLORREF precolor = pPatientList[1]->GetTextColor();

	if(mutestat)
	{
		   
			pPatientList[1]->SetItemText(2,1,strpromp);
			 pPatientList[1]->SetTextColor(RT_RED_COLOR);
			pPatientList[1]->SetItemText(2,2,strvol);
		    pPatientList[1]->SetItemText(2,3,_T("  [ Alt + M ] to \"OFF\" "));
			
	}
	else
	{
		   pPatientList[1]->SetTextColor(RGB(0,0,0));
	       pPatientList[1]->SetItemText(2,1,_T("  OFF  "));
		   pPatientList[1]->SetItemText(2,2,_T(""));
		   pPatientList[1]->SetItemText(2,3,_T("  [ Alt + M ] to \"ON\" "));
		   pAudioEndVol = NULL;
	}
	
	//hr   =   pAudioEndVol->VolumeStepUp(NULL); IDS_PRT_FILEEND

	return pAudioEndVol;
}

BOOL CETApplicationView::GetMicPhoneStat()
{
	IMMDevice*   pEndpoint   =   NULL;
	IMMDeviceCollection*   pCollection=NULL;
	IMMDeviceEnumerator *pEnumerator=NULL;
	
	IAudioEndpointVolume*   pAudioEndVol   =   NULL;

	BOOL     mutestat = FALSE;
	HRESULT  hr;
	CString  strvol;
	CString  strpromp = _T(" ON (\"H\": inc \"L\":dec)");//_T(" ON (\"A\": inc \"Z\":dec)");
	float vol = 0;
 
    hr = CoCreateInstance(
           __uuidof(MMDeviceEnumerator), NULL,
           CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
           (void**)&pEnumerator);

	hr   =   pEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE,   &pCollection); 
	hr   =   pEnumerator->GetDefaultAudioEndpoint(eCapture,// eRender,   
		                                          eMultimedia,   &pEndpoint); 

	if(hr == E_NOTFOUND)  //no mic connected
	{
			pPatientList[1]->SetItemText(2,1,_T(""));
			pPatientList[1]->SetItemText(2,2,_T(""));
		    pPatientList[1]->SetItemText(2,3,_T(""));
			return FALSE;
	}
	hr   =   pEndpoint-> Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pAudioEndVol);

	hr   =   pAudioEndVol->GetMute(&mutestat);

	if(!mutestat)
	{
		   
			pPatientList[1]->SetItemText(2,1,strpromp);
			pPatientList[1]->SetItemText(2,2,strvol);
		    pPatientList[1]->SetItemText(2,3,_T("  [ Alt + M ] to \"OFF\" "));

	}
	else
	{
		  
	       pPatientList[1]->SetItemText(2,1,m_buffstroff); //_T("  OFF  "));
		   pPatientList[1]->SetItemText(2,2,_T(""));
		   pPatientList[1]->SetItemText(2,3,_T("  [ Alt + M ] to \"ON\" "));
		   pAudioEndVol = NULL;
	}
	return mutestat;
}


void CETApplicationView::SetHighRangPara(int startidx, int nlen,UINT optType)
{
		 m_startidx = startidx;
		 m_nlen[m_set_highside]     = nlen;
		 m_keytype  = optType;
}
void CETApplicationView::OnKeyUp(UINT nChar,UINT nRepCnt,UINT nFlags )
{
		switch (nChar)
		{
			case VK_RETURN:
				   this->pPatientList[1]->SetItemText(1,2,_T(""));
				  break;

			case VK_UP:  
			case VK_LEFT:
			case VK_RIGHT:
			case VK_DOWN:
				 if(!IsAmp)
				 {
				     m_keypressed[m_set_highside] = FALSE;
					 m_keytype = 0;
				 }
				 break;
			case VK_SPACE:
				  //m_current
				   AdjustdBHLToValue(m_current.id, m_current.side,m_current.freq,m_current.value );
				   PTAManualFocusFreqPlaying(m_current.id);
				   break;

		}

}
void CETApplicationView::OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags )
{
	
	CMainFrame *pDoc = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	BOOL   IsdecreaseBand = FALSE;
	int id = 0;

	if(nChar == 0x48 || nChar == 0x4C)    //0x41  :A key   0x5A   :Z key
	{
		if(pmic != NULL)
		   AdjustMicVol(nChar,nRepCnt);
		return;
	}


	if(!IsAmp)
	{
		if(m_isPTATesting)
		{
			if(nChar == VK_RETURN  )
			{
				NotifyHeard(m_sessionID);
				this->pPatientList[1]->SetItemText(1,2,_T("pressed"));
				g_ptaUserResponse = true;
				return;
			}

		}
	}

	if(nChar == VK_RETURN  )
	{
		  this->pPatientList[1]->SetItemText(1,2,_T("pressed"));
		  return ;
	}


	CDocList *plist,*plistR,*plistL;
 
	FREZNODE retNode,retNodeMax; 
	id  =  m_set_highside;

	if(id == 0)
		plistR = &pDoc->m_Rt_list[0];   //m_Rt_list  CDocList
	if(id == 1)
		plistL = &pDoc->m_Lf_list[0];  //plist->GetAt(plist->FindIndex(i));

	 plist  = id ? plistL = &pDoc->m_Lf_list[0] : plistR = &pDoc->m_Rt_list[0];

	int i = 0, startidx = -1 ;
	int num=0;
	bool ret =0;
	FrequencyRange   range;
	ETFrequencyRangeArray freArray;
	//bool GetRangedFrequencyBands(ETSessionID sessionID,FrequencyRange range, ETFrequencyRangeArray &freArray); m_sessionID
	if(m_highRangeSet)
	{
		for(i=0; i< plist->GetCount()-1;i++)  //HIGHFREQNUM; i++)
		{
			if(m_freqsetArry[id][i] )
			{
				num++;
				if(startidx == -1)
					startidx = i;
			}
		}

		if(startidx == -1)
			return ;


	    retNode    = plist->GetAt(plist->FindIndex(startidx));
		retNodeMax = plist->GetAt(plist->FindIndex(startidx+num -1));

		range.nFreqMin = retNode.band;
		range.nFreqMax = retNodeMax.band;
		
		

		switch (nChar)
		{
			case VK_DOWN:
				   IsdecreaseBand = TRUE;
				   break;
			case VK_UP:  //VK_RETURN
			case VK_LEFT:
			case VK_RIGHT:
                   IsdecreaseBand = FALSE;
				   break;

		} 
			if(startidx != -1)
			{
			  if(!IsdecreaseBand)
			  {
				    m_keypressed[m_set_highside] = TRUE;
					SetHighRangPara(startidx,num,nChar);
					Invalidate();
			  }
			  else 
			  {
					
				  	ret = GetRangedFrequencyBands(m_sessionID,range,freArray);
					if(ret)                   //decrease one line
					{

						m_highRange[m_set_highside]  = freArray;
						m_keypressed[m_set_highside] = TRUE;
						SetHighRangPara(startidx,num,nChar);
						Invalidate();
						// m_nlen[m_set_highside]     =  m_nlen[m_set_highside] - 1;
					}
			  }
			}
			//break;

	}

	   //if (bHandleNow) 
			 ;//OnKeyDown(lpmsg->wParam, LOWORD(lpmsg ->lParam), HIWORD(lpmsg->lParam)); 
	   // break; 
	   // CView::OnKeyDown(nChar,nRepCnt,nFlags);;

}

void CETApplicationView::OnButtonHighRt()
{
					    CMainFrame *pmnfm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
				    CRect rect;
                    pmnfm->GetWindowRect( rect );

					::SetWindowPos(pmnfm->GetSafeHwnd() ,       // handle to window
					HWND_TOPMOST,  // placement-order handle
					rect.left,     // horizontal position
					rect.top,      // vertical position
					rect.Width(),  // width
					rect.Height(), // height
					SWP_SHOWWINDOW); // window-positioning options

	if(!pHighRangeBtn[0]->pressed)
	{
		pHighRangeBtn[0]->ModifyStyle(0,BS_OWNERDRAW);
		pHighRangeBtn[0]->pressed = TRUE;
		pbutt[1]->EnableWindow(FALSE);
		pHighRangeBtn[1]->EnableWindow(FALSE);
		m_highRangeSet = TRUE;
		m_set_highside = 0;
		m_grayHighLine[0] = FALSE;
	    Invalidate();
	}
	else
	{
		pbutt[1]->EnableWindow(TRUE);
		pHighRangeBtn[1]->EnableWindow(TRUE);
		pHighRangeBtn[0]->ModifyStyle(BS_OWNERDRAW,0 );
		pHighRangeBtn[0]->pressed = FALSE;
		//m_highRangeSet = FALSE;
		m_grayHighLine[0] = TRUE;

		Invalidate();
	}
}

void CETApplicationView::OnButtonHighLf()
{
					    CMainFrame *pmnfm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
				    CRect rect;
                    pmnfm->GetWindowRect( rect );

					::SetWindowPos(pmnfm->GetSafeHwnd() ,       // handle to window
					HWND_TOPMOST,  // placement-order handle
					rect.left,     // horizontal position
					rect.top,      // vertical position
					rect.Width(),  // width
					rect.Height(), // height
					SWP_SHOWWINDOW); // window-positioning options

	if(!pHighRangeBtn[1]->pressed)
	{
		pbutt[1]->EnableWindow(FALSE);
		pHighRangeBtn[0]->EnableWindow(FALSE);
		pHighRangeBtn[1]->ModifyStyle(0,BS_OWNERDRAW);
		pHighRangeBtn[1]->pressed = TRUE;
		m_highRangeSet = TRUE;
		m_grayHighLine[1] = FALSE;
		m_set_highside = 1;
	    Invalidate();
	}
	else
	{
		pbutt[1]->EnableWindow(TRUE);
		pHighRangeBtn[0]->EnableWindow(TRUE);
		pHighRangeBtn[1]->ModifyStyle(BS_OWNERDRAW,0);
		pHighRangeBtn[1]->pressed = FALSE;
		//m_highRangeSet = FALSE;
		m_grayHighLine[1] = TRUE;
		Invalidate();
	}
}

void CETApplicationView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	;//theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

void CETApplicationView::UpdateAMPListDbhr(int index, int val)
{
	AMPNODE  ampnode;  //
	int id = ampsideID;

	ampnode = m_AMP_List[id].GetAt(m_AMP_List[id].FindIndex(index));
	ampnode.ydbhr = val;
	m_AMP_List[id].SetAt(m_AMP_List[id].FindIndex(index),ampnode);

}
void CETApplicationView::OnVScroll(UINT nSBCode,UINT nPos,CScrollBar* pScrollBar )
{
	   //int frez[8] = {1000,1500,2000,3000,4000,6000,8000,12000};

      /* float dDBVolume[NUM_DB_VOLUME+1] = {-10.0, -5.0, 0.0, 5.0, 
		                                    10.0, 15.0, 20.0, 25.0, 
											30.0, 35.0, 40.0, 45.0, 
											50.0, 55.0, 60.0, 65.0, 
											70.0, 75.0, 80.0, 85.0, 90.0};*/
	float dDBVolume[NUM_DB_VOLUME+1] = { -10.0, -7.5, -5.0, -2.5, 0.0, 2.5, 5.0, 7.5, 10.0, 12.5, 15.0, 17.5, 20.0,
                                          22.5, 25.0, 27.5, 30.0, 32.5, 35.0, 37.5, 40.0, 42.5, 45.0, 47.5, 50.0,
                                          52.5, 55.0, 57.5, 60.0, 62.5, 65.0, 67.5, 70.0, 72.5, 75.0, 77.5, 80.0, 82.5, 85.0, 87.5, 90.0 };

		RECT rect={0,0,0,0};
		RECT fontrect;
	    int  offsetlen =0;
		int high=0;
		int textval =0;
		CString strtext;
		CEarStatic *pstatic = NULL;
		float fDBA;
		int whichside =0;
		int sliderlen = 100;

		static int pre_operaID = -5;
		static ETLeftRight pre_leftright = ET_LEFT;

		if(IsAmp) 
		{
			whichside = ampsideID;
			sliderlen = 100;
		}
		else if(m_manualmod)
		{
		   whichside = ptamanualID;
		   sliderlen = NUM_DB_VOLUME;
		}

		if(pScrollBar->IsKindOf(RUNTIME_CLASS(CSliderCtrl)))
		{
			switch(nSBCode)
			{
			case TB_LINEDOWN:
			case TB_LINEUP: 
			case TB_PAGEDOWN:
			case TB_PAGEUP :
			case TB_THUMBTRACK:
				 CEarSliderCtrl *pSlider = (CEarSliderCtrl*)GetDlgItem( IDC_SLIDER_RT + operaID + whichside *50);
				 m_pos = pSlider->GetPos();
				// if(m_pos == 0 )
				//	 MessageBox(_T("gg"));
				 pSlider->GetWindowRect(&rect);
				 ScreenToClient(&rect);
				 pSlider->slidermoved = TRUE;
				          //btnDown = FALSE;
				// if(m_pos >= 90)
				//	 pSlider->SetPos(90);   //slider postion update
				 if(IsAmp)  
				 {
					 if(m_pos<=10)
					 {
						 pSlider->SetPos(10);
						 m_pos = 10;
					  }
			     }

				 break;
			//case SB_PAGEDOWN:
			//	 slidermoved = TRUE;
				// break;
			}
		}
			//pSlider->GetChannelRect(&rect);

		rect.left  = m_exchangex;
		rect.top   = yoffset;
		rect.right = m_ex3;


		long ldBr = m_pos-10;


		if(ldBr > 84)
			ldBr = 84;
/*
		if(ldBr > 90)
			ldBr = 90;
*/
		if(IsAmp)                 //AMP mod
		{
			if ((operaID!=pre_operaID)||(pre_leftright!= m_leftright))
			{
				SwitchFreqFocus(m_sessionID, m_leftright, pfrez[ampsideID][operaID]);
				FocusFreqPlaying(m_sessionID, m_leftright);
				pre_operaID = operaID;
				pre_leftright = m_leftright;
			}

		   g_bETGenerated = false;
		   AdjustDBrVolumeToValue(m_sessionID, m_leftright, pfrez[ampsideID][operaID], //frez[operaID], //(operaID+1)*1000, 
			                     ldBr );
		    pstatic =  ampsideID ?  &pfontLf[operaID] : &pfontRt[operaID];
		}
		else if(m_manualmod)      //PTA manual mod
		{
			    if(nSBCode == SB_ENDSCROLL)
               {
				   m_leftright = ptamanualID ? ET_LEFT :ET_RIGHT;
				   fDBA = dDBVolume[m_pos];
				   AdjustdBHLToValue(m_sessionID, m_leftright,pfrez[ptamanualID][operaID],fDBA );
				   PTAManualFocusFreqPlaying(m_sessionID);

				   m_current.id   = m_sessionID;
				   m_current.side = m_leftright;
				   m_current.freq = pfrez[ptamanualID][operaID];
				   m_current.value = fDBA;
				}
				 pstatic =  ptamanualID ?  &pfontLf[operaID] : &pfontRt[operaID];
		}

		
		//pfontLf[ctrlid].ShowWindow(TRUE);
		
	     /*pfontLf[operaID].GetWindowRect(&fontrect);
		 ScreenToClient(&fontrect);
		 offsetlen = ((26 * 10) * m_pos * yiscal) /100;
		 high = fontrect.bottom - fontrect.top;
		 fontrect.top = yoffset + 26*yiscal + offsetlen;
		 fontrect.bottom  = fontrect.top + high;
		 pfontLf[operaID].MoveWindow(fontrect.left,fontrect.top,fontrect.right - fontrect.left,fontrect.bottom -fontrect.top);
		 //textval = m_pos;
		 strtext.Format(_T("%d"),m_pos); //textval);
		 pfontLf[operaID].SetWindowTextW(strtext);
		 pfontLf[operaID].RedrawWindow();*/
		
		
		 pstatic->GetWindowRect(&fontrect);
		 ScreenToClient(&fontrect);

		// if(m_pos >=90 ) 
			// m_pos = 90;
/*
		  if(IsAmp)
		  {
			 if(m_pos <=10)
				 m_pos = 10;
		  }
		  */

		 offsetlen = ((26 * 10) * m_pos * yiscal) /sliderlen;  //100;
		 high = fontrect.bottom - fontrect.top;

		 
		 //textval = m_pos;
		 if(IsAmp)
		 {
			 fontrect.top = yoffset + offsetlen ; //+ 26*yiscal  ;
		     fontrect.bottom  = fontrect.top + high;
			 strtext.Format(_T("%d"),m_pos-10); 
			 pstatic->MoveWindow(fontrect.left,fontrect.top,fontrect.right - fontrect.left,fontrect.bottom -fontrect.top);
			 plistctr[ampsideID]->SetItemText(0,operaID+1,strtext);
		 }
		 else
		 {
			 fontrect.top = yoffset + offsetlen ; //+ 26*yiscal  ;
		     fontrect.bottom  = fontrect.top + high;

		     strtext.Format(_T("%.1f"),dDBVolume[m_pos]); //m_pos); //textval);
			 pstatic->MoveWindow(fontrect.left, fontrect.top,fontrect.right - fontrect.left,fontrect.bottom -fontrect.top);

			 plistctr[ptamanualID]->SetItemText(0,operaID+1,strtext);
			 m_rcdvalarr[ptamanualID][operaID] = 1;

			// plistctr[m_sessionID]->UpdateWindow();
		 }

		 pstatic->SetWindowTextW(strtext);
		 pstatic->ShowWindow(FALSE); //TRUE);
		// pstatic->RedrawWindow();



}


LRESULT CETApplicationView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch ( message )
	{
		case WM_PTA_END_EVENT:
			this->FillPTAResult();
			pbutt[7]->EnableWindow(TRUE);

			if(m_sessionID != NO_SESSION_ID)
				EndTestSession(m_sessionID);
			Sleep(1000);
			m_sessionID = BeginTestSession(ET_PTA, ET_AUTOMATIC);
			if( m_sessionID == NO_SESSION_ID)
			{
#ifdef _DEBUG
				MessageBox(L"PTA Session can not be created");
#endif
			}
			else
			{
				SetPTASpeed(m_sessionID, g_testSpeed);
				RegisterTestCompleteIndication(m_sessionID, TestAccomplishmentIndication, this, TestProgressIndication);
			}
			break;

		case WM_PTA_PROGRESS_EVENT:
			this->PTAProgressIndication();

			break;
		case WM_MEDIA_EVENT:
			FreshLoopPosition();
			break;
		default:
			break;

	}
	
	return CView::WindowProc(message, wParam, lParam);
}



// CETApplicationView diagnostics

#ifdef _DEBUG
void CETApplicationView::AssertValid() const
{
	CView::AssertValid();
}

void CETApplicationView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CETApplicationDoc* CETApplicationView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CETApplicationDoc)));
	return (CETApplicationDoc*)m_pDocument;
}
#endif //_DEBUG




void CETApplicationView::PTAQuickCalibrate()
{	 
	HRESULT hr = S_OK;  
	CoInitialize(NULL); 

	CComPtr<IMMDeviceEnumerator> spEnumerator; 

	hr = CoCreateInstance( __uuidof(MMDeviceEnumerator),  
	NULL,  
	CLSCTX_ALL,  
	__uuidof(IMMDeviceEnumerator),  
	(void**)&spEnumerator);  

	CComPtr<IMMDevice> spDevice;  

	hr = spEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &spDevice );  

	CComPtr<IAudioEndpointVolume> spAudioEndpointVolume;  

	hr = spDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL,  (void**)&spAudioEndpointVolume);  

	float pLevelDB;
	hr = spAudioEndpointVolume->GetMasterVolumeLevel(&pLevelDB) ;


	BOOL isMute;
	hr = spAudioEndpointVolume->GetMute(&isMute);
	if(isMute)
	{
		spAudioEndpointVolume->SetMute(false, NULL);  
	}

	int Vol100 = AfxGetApp()->GetProfileIntW(_T("Calibration"),_T("Volume"),100);

	float fLevelMinDB;
	float fLevelMaxDB;
	float fVolumeIncrementDB;
	float fCalibrationVolumeDB = float(Vol100)/100;

	hr = spAudioEndpointVolume->GetVolumeRange(&fLevelMinDB,&fLevelMaxDB,&fVolumeIncrementDB);

	if(fCalibrationVolumeDB >fLevelMaxDB)
		fCalibrationVolumeDB = fLevelMaxDB;

	hr = spAudioEndpointVolume->SetMasterVolumeLevel(fCalibrationVolumeDB, NULL); // ->SetMasterVolume(1, NULL);  

	CoUninitialize();

}


UINT PatientLinkingThread( LPVOID p_pDialog)
{
	CETApplicationView * pAppView = (CETApplicationView *)p_pDialog;

	if ( pAppView != NULL )
	{
		if ( pAppView->GetSafeHwnd() != NULL )
		{
			pAppView->PerformLinking();
		}
	}
	g_bPatientLinkingThreadRunning = false;
	return 0;
}


UINT PatientLinkingToCUThread( LPVOID p_pDialog)
{
	CETApplicationView * pAppView = (CETApplicationView *)p_pDialog;

	if ( pAppView != NULL )
	{
		if ( pAppView->GetSafeHwnd() != NULL )
		{
			pAppView->PerformLinkingToCU();
		}
	}
	g_bPatientLinkingThreadRunning = false;
	return 0;
}


bool CETApplicationView::PerformLinking()
{
//	_PatientDesc patientDesc;

	int i=0;
	while((m_LinkState == LINKING)&&(i<10))
	{
		Sleep(1000);
		i++;
	}
	if (m_LinkState  == LINKED)
		return true;

	m_LinkState = LINKING;

	pPatientList[1]->SetItemText(0,1,_T(""));
	pPatientList[1]->SetItemText(0,2,m_buffstrlinking);//_T("  LINKING"));
	pPatientList[1]->SetItemText(0,3,_T(""));

	//pPatientList[0]->ki
	
	
	//::SendMessage(pPatientList[0]->m_hWnd,NM_KILLFOCUS,IDC_LIST_PATIENTDATA,0);
		

	std::string szID;
	std::string szName;
	std::string sexAge;
	std::string szGender;

	ETLinkingStatus ret;
	ret = LinkingFromAM(szID,szName,sexAge,szGender);

	if(ret == LINKED)
	{
		
		std::wstring szwID; 
		std::wstring szwName; 
		std::wstring wsexAge;
		std::wstring szwGender;

		if(0 == szID.length())
				szwID = L"";
		else
				szwID = s2ws(szID);

		if(0 == szName.length())
				szwName = L"";
		else
				szwName = s2ws(szName);

		if(0 == sexAge.length())
				wsexAge = L"";
		else
				wsexAge = s2ws(sexAge);

		if(0 == szGender.length())
				szwGender = L"";
		else
				szwGender = s2ws(szGender);


		m_LinkState = LINKED;
		MessageBox(m_buffAMDlinked);  //_T("AudioMed linked!"));

		pPatientList[1]->SetItemText(0,1,_T(""));
		pPatientList[1]->SetItemText(0,2,_T(""));
		pPatientList[1]->SetItemText(0,3,m_buffstrlinked); //_T("  LINKED"));

		pPatientList[0]->SetItemText(0,1,szwID.c_str());
		pPatientList[0]->SetItemText(0,3,szwName.c_str());
		pPatientList[0]->SetItemText(1,1,wsexAge.c_str());
		pPatientList[0]->SetItemText(1,3,szwGender.c_str());

		CutPcFileToAudm();

		pbutt[6]->EnableWindow(TRUE);

		// Stop site treatment if it is
		#ifdef SUPPORT_SITE_TREATMENT		
		         // m_treatstage = FALSE;
				  m_treatstage = FALSE;
				if(IsAmp && g_bIsLocalPlaying)
				{
					StopTreatmentPlaying();
					g_bIsLocalPlaying = false;

					if(m_nTimer != NULL)
					{
						this->KillTimer(m_nTimer);		
						m_nTimer = NULL;
					}						
					strendTime = _T("");
		            //MessageBox(_T("Treatment file playing stopped")); 	 
				}

				if(IsAmp)
				{
					pbutt[5]->SetWindowTextW(m_buffarr[5]);       //download
					pbutt[5]->ModifyStyle(BS_OWNERDRAW,0);
					pbutt[5]->EnableWindow(TRUE);
					pbutt[5]->Invalidate();

					pbutt[1]->EnableWindow(TRUE);
					pbutt[2]->EnableWindow(TRUE);
					pbutt[3]->EnableWindow(TRUE);
				}
		#endif

				
				return true;
	}
	else
	{
		m_LinkState  = NOLINKED;
/*
		if (LINKING_RESULT_DONGLE_UNMATCH == ret)
			AfxMessageBox(IDS_DONGLEUNMATCHED);
*/
		pPatientList[1]->SetItemText(0,1,m_buffstrunlink); //_T("  UNLINKED"));
		pPatientList[1]->SetItemText(0,2,_T(""));
		pPatientList[1]->SetItemText(0,3,_T(""));

		return false;
	}
}


bool CETApplicationView::PerformLinkingToCU()
{
//	_PatientDesc patientDesc;

	CString strid;
	strid = pPatientList[0]->GetItemText(0,1);


	CString strdef =  m_bufstrprompt; //_T("Pls Input ID ...");
	if(strid.Compare(strdef) ==0 )
	{
		return false;
	}

	if(m_LinkState == LINKING)
		return false;     // Do nothing

	if (IDOK != MessageBox(m_buffassign, // L"Do you want to confirm assignment", 
		                   L"Confirmation",MB_OKCANCEL))   // Late the message resource ID shall be defined.
	{
		m_LinkState = NOLINKED;
		return false;
	}

	if((m_LinkState  == LINKING)||
		(m_LinkState  == LINKED))
		return true;     // Do nothing

	m_LinkState = LINKING;

	pPatientList[1]->SetItemText(0,1,_T(""));
	pPatientList[1]->SetItemText(0,2,m_buffstrlinking);//_T("  LINKING"));
	pPatientList[1]->SetItemText(0,3,_T(""));

	std::wstring szwID((LPCTSTR)strid);
	std::string szID;
	szID =  ws2s(szwID);

	CString strname;
	strname = pPatientList[0]->GetItemText(0,3);
	std::wstring szwName((LPCTSTR)strname);
	std::string szName;
	szName =  ws2s(szwName);

	CString strAge;
	strAge = pPatientList[0]->GetItemText(1,1);
	std::wstring sexwAge((LPCTSTR)strAge);
	std::string sexAge;
	sexAge =  ws2s(sexwAge);

	CString strGender;
	strGender = pPatientList[0]->GetItemText(1,3);
	std::wstring strwSex((LPCTSTR)strGender);
	std::string strSex;
	strSex =  ws2s(strwSex);

	ETLinkingStatus	ret = LinkingToAM(szID,szName,sexAge,strSex); 

	if ( LINKED == ret)
	{

		m_LinkState  = LINKED;
		pPatientList[1]->SetItemText(0,1,_T(""));
		pPatientList[1]->SetItemText(0,2,_T(""));
		pPatientList[1]->SetItemText(0,3,m_buffstrlinked); //_T("  LINKED"));

		CutPcFileToAudm();

#ifdef SUPPORT_SITE_TREATMENT
		if(IsAmp)
		{
			pbutt[5]->SetWindowTextW(m_buffarr[5]);
			pbutt[1]->EnableWindow(TRUE);
			pbutt[2]->EnableWindow(TRUE);
			pbutt[3]->EnableWindow(TRUE);
		}
#endif

		MessageBox(m_buffAMDlinked);  //_T("AudioMed linked!"));
		pbutt[5]->EnableWindow(TRUE);
		return true;
	}
	else
	{
		m_LinkState  = NOLINKED;
		pPatientList[1]->SetItemText(0,1,m_buffstrunlink); //_T("  UNLINKED"));
		pPatientList[1]->SetItemText(0,2,_T(""));
		pPatientList[1]->SetItemText(0,3,_T(""));

		MessageBox(m_buffassfail);  // L"Assignment failed");

/*
		if(LINKING_RESULT_NO_PLUGGED == linkResult)
			AfxMessageBox(IDS_NOPLUGGEDAUDIOMED);
		else if(LINKING_RESULT_USB_ACCESS_ERROR == linkResult)
			AfxMessageBox(IDS_ALREADYASSIGNEDBYOTHER);
		else
			AfxMessageBox(IDS_LINKINGFAILED);
*/
		return false;
	}
	
}


BOOL CETApplicationView::CutPcFileToAudm()
{
	//_T("ptaresult.txt");
	//_T("C:\\Windows\\System32\\pts\\"
	bool result = FALSE;
	BOOL ret = FALSE;
	std::string absoluteFilename;
	std::wstring absolutewFilename;

	CString strPathName;
	CString strdestname;
	CString strfoldername;

	CString strid;
	CString strpath;
	strid = pPatientList[0]->GetItemText(0,1);
	strpath = GetPTAResultLocalPath();
	strpath = strpath + strid;
	strfoldername = strpath;
	strpath += _T("\\");
	strpath = strpath + _T("ptaresult.txt");

    result = GetPTAStorePathName(absoluteFilename);
	int cnt = 0;
	while (!result&&(cnt<10)){
		result = GetPTAStorePathName(absoluteFilename);
		Sleep(100);
		cnt++;
	}
	if(absoluteFilename.length() == 0)
	{
		pPatientList[1]->SetItemText(0,1,m_buffstrunlink); 
		pPatientList[1]->SetItemText(0,3,_T(""));  
		return FALSE;
	}

	absolutewFilename = s2ws(absoluteFilename);

	strdestname = absolutewFilename.c_str();

	DWORD errorret = 0;

	ret = CreateDirectory(strfoldername,NULL);
	if(ret)
	{
		ret = CopyFile(strdestname,strpath, FALSE);
	}
	else
	{
		errorret = GetLastError();
		if(ERROR_ALREADY_EXISTS == errorret)
		{
			ret = CopyFile(strpath,strdestname,FALSE);
			if(!ret)
			{
				errorret = GetLastError();
				if(ERROR_FILE_NOT_FOUND == errorret)
				{
					ret = CopyFile(strdestname,strpath, FALSE);
				}
			}


		}
	}
	if(ret)
		FlushDataToAU();
	
//	DeleteFile(strpath);
//	_trmdir(strfoldername);

	return ret;
	
}


// CETApplicationView message handlers


void CETApplicationView::OnDestroy()
{
	CView::OnDestroy();

	if(m_diaryMod)
	{
		if(fReadPta.m_hFile != CFile::hFileNull) 
		    fReadPta.Close();   
	}

	QuitUsingAudiomed();
	if(m_sessionID != NO_SESSION_ID)
		EndTestSession(m_sessionID);

	SetMuteSysVol(false);
}


int CETApplicationView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}
