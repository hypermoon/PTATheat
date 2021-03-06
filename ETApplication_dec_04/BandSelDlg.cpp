//-----------------------------------------------------------------------------
// File: BandSelDlg.cpp
//
// Desc: The dialog for band selection.
//
// Copyright (c) 2011 Eartronic Ltd. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ETApplication.h"
#include "BandSelDlg.h"
#include "ETApplicationView.h"
#include "afxdialogex.h"


// CBandSelDlg

IMPLEMENT_DYNAMIC(CBandSelDlg, CDialogEx)

CBandSelDlg::CBandSelDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBandSelDlg::IDD, pParent)
{

}

CBandSelDlg::CBandSelDlg(BANDS_TYPE  bandType, CWnd* pParent /* = NULL*/)
	: CDialogEx(CBandSelDlg::IDD, pParent)
{
	m_bandType = bandType;
	manualbandstat = FALSE;
}


CBandSelDlg::~CBandSelDlg()
{
}

void CBandSelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Radio(pDX, IDC_RADIO_6BANDS, m_bandType);
	DDX_Radio(pDX, IDC_RADIO_6BANDS, m_bandType);
}


BEGIN_MESSAGE_MAP(CBandSelDlg, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO_6BANDS, &CBandSelDlg::OnBnClickedRadio6bands)
	ON_BN_CLICKED(IDC_RADIO_11BANDS, &CBandSelDlg::OnBnClickedRadio11bands)
	ON_BN_CLICKED(IDC_RADIO_HIGHAND, &CBandSelDlg::OnBnClickedRadioHighBands)
END_MESSAGE_MAP()


// CBandSelDlg 消息处理程序


void CBandSelDlg::OnBnClickedRadio6bands()
{
	m_bandType = 0;
}


void CBandSelDlg::OnBnClickedRadio11bands()
{
	m_bandType = 1;
}
void CBandSelDlg::OnBnClickedRadioHighBands()
{
	m_bandType = 2;
}
void  CBandSelDlg::SetHStat(BOOL show)
{
	m_hshow = show;
}

BOOL CBandSelDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    if(m_hshow)	
	   GetDlgItem(IDC_RADIO_HIGHAND)->EnableWindow(FALSE);   //warning false
	else
	   GetDlgItem(IDC_RADIO_HIGHAND)->EnableWindow(TRUE);

	//pbandmanualpta = (CButton*)GetDlgItem(IDC_CHECK1);

	//if(manualbandstat)
	//    pbandmanualpta->SetCheck(BST_CHECKED );
	//else
	//	pbandmanualpta->SetCheck(BST_UNCHECKED );

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// ??: OCX ?????? FALSE
}


void CBandSelDlg::GetCheckStat(BOOL stat)
{
	manualbandstat = stat;

}

BANDS_TYPE CBandSelDlg::getSelectedBands()
{
	if (0==m_bandType)
		 return BANDS_CLASSIC_6;           //IDC_RADIO_HIGHAND
	else if(1==m_bandType)
		 return BANDS_CLASSIC_11;
	else if(2 == m_bandType)
		 return BANDS_RANGE_NARROWED;
	
	return BANDS_CLASSIC_11;

}
void CBandSelDlg::OnOK()
{
	 CETApplicationView *prev = (CETApplicationView*) this->GetParent();
	 //m_highRangeSet = FALSE;
	// prev->m_highRangeSet = FALSE;
	// prev->Invalidate();
	// prev->UpdateWindow();

	/*if(pbandmanualpta->GetCheck() == BST_UNCHECKED)
	{
             manualbandstat = FALSE;
	}
	else if(pbandmanualpta->GetCheck() == BST_CHECKED)
	{
             manualbandstat = TRUE;
	}*/

	CDialog::OnOK();
}
