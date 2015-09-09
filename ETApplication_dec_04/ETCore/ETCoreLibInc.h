//-----------------------------------------------------------------------------
// File: ETCoreLibInc.h
//
// Desc: global interface of Eartronic core software library.
//
// Copyright (c) 2011 Eartronic Ltd. All rights reserved.
//-----------------------------------------------------------------------------

#if !defined(_ETCORELIBINC_INCLUDED_)
#define _ETCORELIBINC_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "string"
//#include <dshow.h>

// At most 11 frequency bands
#define ET_MAX_FREQUENCY_NUM   11

#define NO_SESSION_ID  -1

typedef int ETSessionID;

typedef enum _SessionType
		{	ET_PTA, 
			ET_AMP, 
			ET_UNKNOWN 
		} ETSessionType;

typedef enum _ReftRight
		{	ET_LEFT, 
			ET_RIGHT 
		} ETLeftRight;

typedef enum _TestSpeed { PTA_SPEED_VERY_SLOW = 0,
							PTA_SPEED_SLOW, 
							PTA_SPEED_NORMAL, 
							PTA_SPEED_QUICK, 
							PTA_SPEED_VERY_QUICK,
							PTA_SPEED_NUM
						} ETTestSpeed;

typedef enum _ScreenMode { SCREEN_MODE_ON, 
							SCREEN_MODE_OFF 
						} ETScreenMode;

typedef enum _linkingStatus 
			{	NOLINKED,  
				LINKING,
				LINKED 
			} ETLinkingStatus;

typedef enum _operationMode 
			{	ET_AUTOMATIC, 
				ET_MANUAL 
			} ETOperationMode;

typedef enum _sessionState 
			{ ET_SESSION_FOCUSED, 
				ET_SESSION_IDLE, 
				ET_SESSION_END 
			} ETSessionState;

typedef struct
{
		int nLeftFrequencyNum;
		int nRightFrequencyNum;
		int aLeftFreqList[ET_MAX_FREQUENCY_NUM];
		int aRightFreqList[ET_MAX_FREQUENCY_NUM];
} ETFrequencyArray;

typedef struct 
{
	long nFrequency;
	float ndBA;
}
ETDiagValue;

typedef struct 
{
		int nLeftFrequencyNum;
		int nRightFrequencyNum;
		ETDiagValue aLeftDiagList[ET_MAX_FREQUENCY_NUM];
		ETDiagValue aRightDiagList[ET_MAX_FREQUENCY_NUM];
} ETPTAResult;


typedef struct 
{
	long nFrequency;
	ETLeftRight leftRight;
	int ndBr;
}
ETAMPResultValue;

typedef struct 
{
		int nResultFreqNum;
		ETAMPResultValue aResultValueList[ET_MAX_FREQUENCY_NUM*2];
} ETAMPResult;


typedef struct 
{
		int nFreqSelNum;
		long aFreqSelList[ET_MAX_FREQUENCY_NUM*2];
		int aFreqdBrList[ET_MAX_FREQUENCY_NUM*2];
} ETAMPLRSelection;


typedef struct 
{
	long nFreqMin;
	long nFreqMax;

} FrequencyRange;


typedef struct
{
		int nFrequencyNum;
		int aFreqList[ET_MAX_FREQUENCY_NUM];
} ETFrequencyRangeArray;


//////////////////////////////////////////////////////////////////
// Setting
//////////////////////////////////////////////////////////////////

		void	SetFrequencyBands(ETFrequencyArray &freArray);
		void	SetFrequencyBands(ETPTAResult &freArray);

		void	GetFrequencyBands(ETFrequencyArray &freArray);

		ETSessionID BeginTestSession(ETSessionType sessionType,  
										ETOperationMode operationMode);

		void EndTestSession(ETSessionID sessionID);
		void SwitchToSession(ETSessionID sessionID);   // Reserve old session in idle state, and activate another session to be focused

		ETSessionID  GetCurrentFocusedSession();
		ETSessionID  GetAlivePTASession();
		ETSessionID  GetAliveAMPSession();

		ETSessionState GetSessionState(ETSessionID sessionID);

		bool  GetPTAResult(ETSessionID session, ETPTAResult& result);
		bool  GetAMPResult(ETSessionID session, ETAMPResult& result);

///////////////////////////////////////////////////////////////////
// PTA 
///////////////////////////////////////////////////////////////////
		typedef void (*CallbackFunc)(bool, void*);
		typedef void (*CallbackPTAProgressFunc)(int, long, double, void*);

		bool InitPTAFrequencyBands(ETSessionID sessionID);

		// Automatic
		void SetPTASpeed(ETSessionID sessionID, ETTestSpeed speed);
		ETTestSpeed GetPTASpeed(ETSessionID sessionID);

		void SetPTAScreeningMode(ETSessionID sessionID, ETScreenMode screenMode);
		void StartAutomaticPTA(ETSessionID sessionID);
		void PauseAutomaticPTA(ETSessionID sessionID);
		void ResumeAutomaticPTA(ETSessionID sessionID);
		void StopAutomaticPTA(ETSessionID sessionID);
		void NotifyHeard(ETSessionID sessionID);                     
		void RegisterTestCompleteIndication(ETSessionID sessionID, 
											CallbackFunc pTestCompleteIndication, 
											void* pwin,  
											CallbackPTAProgressFunc pPTAProgressIndication = NULL);

		bool GetRangedFrequencyBands(ETSessionID sessionID,
										FrequencyRange range, 
										ETFrequencyRangeArray &freArray);

		// Manual
		void StartManualPTA(ETSessionID sessionID);
		bool SetPTAManualFreqSelection(ETSessionID sessionID, ETLeftRight leftRight,  long freq);   
		void RemovePTAManualFreqSelection(ETSessionID sessionID, ETLeftRight leftRight, long freq);
		void AdjustdBHLToValue(ETSessionID sessionID, ETLeftRight leftRight, long freq, float lValue );
		long PTAManualFocusFreqPlaying(ETSessionID sessionID);   // The last selected index will be the focused index
		void StopManualPTA(ETSessionID sessionID);
		bool FinishManualPTA(ETSessionID sessionID);   // Return false if PTA manual mode can not be accomplished

//		bool IsPTAManualAccomplished(ETSessionID sessionID);

/*
        void IncreasedBAPlay(ETSessionID sessionID, int index, ETLeftRight leftRight,  bool loop=false);
		void DecreasedBAPlay(ETSessionID sessionID, int index, ETLeftRight leftRight,  bool loop=false);
		void DataRenderForGraph();   // Give out dBA value list for display, including current focused, can be no focus.
*/
		bool GetPTAStorePathName(std::string &absoluteFilename);
		void ActiveSelfTestToneFromPause();

/////////////////////////////////////////////////////////////////////
//  AMP
/////////////////////////////////////////////////////////////////////
//		ETFrequencyArray  SetMediateMode();
		typedef void (*GenCallbackFunc)(int);
//		typedef void (*NotifyWinCallbackFunc)(void*, IMediaEventEx*);
		typedef void (*NotifyWinCallbackFunc)(void*, void*);

		bool InitSound(ETSessionID sessionID, ETLeftRight leftRight);

		bool SetFreqSelection(ETSessionID sessionID, ETLeftRight leftRight,  long freq);    
		void RemoveFreqSelection(ETSessionID sessionID, ETLeftRight leftRight, long freq);
		
		bool SwitchFreqFocus(ETSessionID sessionID, ETLeftRight leftRight, long freq);

		void AdjustDBrVolumeToValue(ETSessionID sessionID, ETLeftRight leftRight, long freq, long lValue );

		long FocusFreqPlaying(ETSessionID sessionID, ETLeftRight leftRight);			// The last selected index will be the focused index
		bool GetFreqLRSelection(ETSessionID sessionID, ETLeftRight leftRight, ETAMPLRSelection& result);

		void Playing(ETSessionID sessionID, ETLeftRight leftRight, int index);

		void DataRenderForGraph(ETSessionID sessionID);       	// Give out selection lists and dBr value for display, including current focused.

		void SetUserID(std::string id);

		bool ETFGen(ETSessionID sessionID, bool bAsyn = false, GenCallbackFunc pGenIndFunc = NULL);		// Cost time, needs progress bar.
		int GetETFGenProgress(ETSessionID sessionID);          // In case it is in Asynchonized mode

		bool ETFDownload(ETSessionID sessionID, int limitPlay, int leftVol, int RightVol, bool bAsyn = false);  // Cost time, needs progress bar.
		int GetETFDownloadProgress(ETSessionID sessionID);          // In case it is in Asynchonized mode.

		bool GetDiaryStorePathName(std::string &absoluteFilename);

		void StopTreatmentPlaying();
		void StartTreatmentPlaying();

		bool InitPreTreatmentSound(ETSessionID sessionID, ETAMPResult &preAMPValue);

		void RegisterNotifyWin(void* pwin, NotifyWinCallbackFunc pNotifyWinFun=NULL);
		void FreshLoopPosition();

//////////////////////////////////////////////////////////////////////
// SECU-Link
//////////////////////////////////////////////////////////////////////
		ETLinkingStatus LinkingToAM(std::string szID, 
							std::string szName, 
							std::string sexAge, 
							std::string szGender);

		ETLinkingStatus LinkingFromAM(std::string& szID, 
										std::string& szName, 
										std::string& sexAge, 
										std::string& szGender);
		bool IsLinked();
		bool FilterUSBHIDDevice(TCHAR * usbDevicePath);
		bool FilterUSBMSDDevice(TCHAR * usbDevicePath);
		void QuitUsingAudiomed();

		bool DetectUSBHID();

		void FlushDataToAU();

//////////////////////////////////////////////////////////////////////
// Utility
//////////////////////////////////////////////////////////////////////
		int GetDateGap(std::string& day1, std::string& day2);


#endif   // _ETCORELIBINC_INCLUDED_




