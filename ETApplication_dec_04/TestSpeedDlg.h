#pragma once

#include "ETCore//ETCoreLibInc.h"

// CTestSpeedDlg 对话框

class CTestSpeedDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTestSpeedDlg)

public:
	CTestSpeedDlg(CWnd* pParent = NULL);   // 标准构造函数
	CTestSpeedDlg(ETTestSpeed  testSpeed, CWnd* pParent = NULL);  
	virtual ~CTestSpeedDlg();
	virtual void OnOK( );
	void GetCheckStat(BOOL stat);
	void SetPretreatmentIDstat(BOOL stat ,BOOL treatcheck = FALSE,BOOL amconnect = FALSE);
	CEdit *pm_left,*pm_right,*pm_playtime;
	//int  left,right,playtime;
	CButton *pmanualpta;
	CButton *ptreatbox;
	BOOL  manualstat;
	BOOL  ttbox;
	BOOL  treatmentid;
	BOOL  m_check;
// 对话框数据
	enum { IDD = IDD_DIALOG_TESTSPEED };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
//	int m_testSpeed;
public:
	virtual BOOL OnInitDialog();
	
private:
	ETTestSpeed m_testSpeed;
	BOOL m_amconnect;
	int m_TestSpeedInx;

public:
	ETTestSpeed GetTestSpeed() { return m_testSpeed; }
	afx_msg void OnBnClickedRadioSpeed1();
	afx_msg void OnBnClickedRadioSpeed2();
	afx_msg void OnBnClickedRadioSpeed3();
	afx_msg void OnBnClickedRadioSpeed4();
	afx_msg void OnBnClickedRadioSpeed5();
};
