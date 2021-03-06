//-----------------------------------------------------------------------------
// File: BandSelDlg.h
//
// Desc: The dialog for band selection.
//
// Copyright (c) 2011 Eartronic Ltd. All rights reserved.
//-----------------------------------------------------------------------------
#pragma once

typedef enum { BANDS_CLASSIC_6, 
				BANDS_CLASSIC_11, 
				BANDS_RANGE_NARROWED}
	BANDS_TYPE;


// CBandSelDlg 对话框

class CBandSelDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBandSelDlg)

public:
	CBandSelDlg(CWnd* pParent = NULL);   // 标准构造函数
	CBandSelDlg(BANDS_TYPE  bandType, CWnd* pParent = NULL);   // 构造函数
	virtual ~CBandSelDlg();
	void  SetHStat(BOOL show);
	virtual void OnOK();
	void GetCheckStat(BOOL stat);
	CButton *pbandmanualpta;
	BOOL  manualbandstat;

// 对话框数据
	enum { IDD = IDD_DIALOG_BANDSEL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadio6bands();
	afx_msg void OnBnClickedRadio11bands();
	afx_msg void OnBnClickedRadioHighBands();
protected:
	int  m_bandType;
	BOOL m_hshow;

public:
	BANDS_TYPE getSelectedBands();
	virtual BOOL OnInitDialog();
};
