// TestSpeedDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ETApplication.h"
#include "TestSpeedDlg.h"
#include "afxdialogex.h"

extern int g_countDown;
extern int g_leftVolume;
extern int g_rightVolume;


// CTestSpeedDlg 对话框

IMPLEMENT_DYNAMIC(CTestSpeedDlg, CDialogEx)

CTestSpeedDlg::CTestSpeedDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestSpeedDlg::IDD, pParent)
{
	manualstat = FALSE;
	treatmentid = FALSE;
	ttbox = FALSE;
	m_TestSpeedInx = 0;
	m_check = FALSE;
}

CTestSpeedDlg::CTestSpeedDlg(ETTestSpeed  testSpeed, CWnd* pParent /* = NULL*/)
	: CDialogEx(CTestSpeedDlg::IDD, pParent)
{
	m_testSpeed = testSpeed;
	m_amconnect = FALSE;
	// left = 40;
	// right = 40;
	// playtime = 40;
}


CTestSpeedDlg::~CTestSpeedDlg()
{
}

void CTestSpeedDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Radio(pDX, IDC_RADIO_SPEED1, m_testSpeed);
	DDX_Radio(pDX, IDC_RADIO_SPEED1, m_TestSpeedInx);

	//DDX_Text(pDX,IDC_AD_PLAYTIME,playtime);
	//DDV_MinMaxInt(pDX,playtime,20,100);

	//DDX_Text(pDX,IDC_AD_DEFV_RIGHT,right);
    //DDV_MinMaxInt(pDX,right,30,84);

	//DDX_Text(pDX,IDC_AD_DEFV_LEFT,left);
    //DDV_MinMaxInt(pDX,left,40,84);

 

   
}


BEGIN_MESSAGE_MAP(CTestSpeedDlg, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO_SPEED1, &CTestSpeedDlg::OnBnClickedRadioSpeed1)
	ON_BN_CLICKED(IDC_RADIO_SPEED2, &CTestSpeedDlg::OnBnClickedRadioSpeed2)
	ON_BN_CLICKED(IDC_RADIO_SPEED3, &CTestSpeedDlg::OnBnClickedRadioSpeed3)
	ON_BN_CLICKED(IDC_RADIO_SPEED4, &CTestSpeedDlg::OnBnClickedRadioSpeed4)
	ON_BN_CLICKED(IDC_RADIO_SPEED5, &CTestSpeedDlg::OnBnClickedRadioSpeed5)
END_MESSAGE_MAP()


// CTestSpeedDlg 消息处理程序


BOOL CTestSpeedDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	switch (m_testSpeed)
	{
		case PTA_SPEED_VERY_SLOW:
			m_TestSpeedInx = 0;
			break;
		case PTA_SPEED_SLOW:
			m_TestSpeedInx = 1;
			break;
		case PTA_SPEED_NORMAL:
			m_TestSpeedInx = 2;
			break;
		case PTA_SPEED_QUICK:
			m_TestSpeedInx = 3;
			break;
		case PTA_SPEED_VERY_QUICK:
			m_TestSpeedInx = 4;
			break;
		default:
			break;
	}

	UpdateData(FALSE);

	pmanualpta = (CButton*)GetDlgItem(IDC_CHECK2);
	ptreatbox = (CButton*)GetDlgItem(IDC_TREATBOX);

   //  pm_left      =  (CEdit*)GetDlgItem(IDC_AD_DEFV_LEFT);
   //  pm_right     =  (CEdit*)GetDlgItem(IDC_AD_DEFV_RIGHT);
	 pm_playtime  =  (CEdit*)GetDlgItem(IDC_AD_PLAYTIME);
	 pm_playtime->ShowWindow(SW_HIDE);
	 ptreatbox->SetCheck(m_check);  //BST_CHECKED  BST_UNCHECKED



	 CString str;
	 str.Format(_T("%d"),g_countDown);
	 pm_playtime->SetWindowTextW(str);

//	 str.Format(_T("%d"),g_leftVolume);
//	 pm_left->SetWindowTextW(str);

//	 str.Format(_T("%d"),g_rightVolume);
	// pm_right->SetWindowTextW(str);

	 if(manualstat)
	    pmanualpta->SetCheck(BST_CHECKED );   //ptreatbox
	 else
		pmanualpta->SetCheck(BST_UNCHECKED );


	 if(m_amconnect)
		 ptreatbox->EnableWindow(FALSE);
	 else 
	 {
		 if(treatmentid)
		   ptreatbox->EnableWindow(TRUE);
	     else
		 {
           ptreatbox->EnableWindow(FALSE);
		   ptreatbox->SetCheck(FALSE);
		 }
	 }

	return TRUE;  // return TRUE unless you set the focus to a control
	// ??: OCX ?????? FALSE
}


void CTestSpeedDlg::OnBnClickedRadioSpeed1()
{
	m_testSpeed = PTA_SPEED_VERY_SLOW;
	m_TestSpeedInx = 0;
}

void CTestSpeedDlg::GetCheckStat(BOOL stat)
{
	manualstat = stat;
}
void CTestSpeedDlg::SetPretreatmentIDstat(BOOL stat, BOOL treatcheck ,BOOL amconnect)
{
	treatmentid = stat;
	m_check = treatcheck;
	m_amconnect = amconnect;
}
void CTestSpeedDlg::OnOK( )
{
	int val = 0;
	CString str;

	pm_playtime->GetWindowTextW(str);
	val = _ttoi(str);
	g_countDown =  val;
	AfxGetApp()->WriteProfileInt(_T("Audiomed"),_T("count down"),g_countDown);

	if(pmanualpta->GetCheck() == BST_UNCHECKED)
	{
             manualstat = FALSE;
	}
	else if(pmanualpta->GetCheck() == BST_CHECKED)
	{
             manualstat = TRUE;
	}

	if(ptreatbox->GetCheck() == BST_UNCHECKED)
	{
             ttbox = FALSE;
	}
	else if(ptreatbox->GetCheck() == BST_CHECKED)
	{
             ttbox = TRUE;
	}

	//pm_left->GetWindowTextW(str);
  //	val = _ttoi(str);
  //  g_leftVolume = val; 
  //	pm_right->GetWindowTextW(str);
	//val = _ttoi(str);
  //  g_rightVolume = val;

	CDialog::OnOK();
}
void CTestSpeedDlg::OnBnClickedRadioSpeed2()
{
	m_testSpeed = PTA_SPEED_SLOW;
	m_TestSpeedInx = 1;
}


void CTestSpeedDlg::OnBnClickedRadioSpeed3()
{
	m_testSpeed = PTA_SPEED_NORMAL;
	m_TestSpeedInx = 2;
}


void CTestSpeedDlg::OnBnClickedRadioSpeed4()
{
	m_testSpeed = PTA_SPEED_QUICK;
	m_TestSpeedInx = 3;
}


void CTestSpeedDlg::OnBnClickedRadioSpeed5()
{
	m_testSpeed = PTA_SPEED_VERY_QUICK;
	m_TestSpeedInx = 4;
}
