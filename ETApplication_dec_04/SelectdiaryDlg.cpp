// SelectdiaryDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ETApplication.h"
#include "SelectdiaryDlg.h"
#include "afxdialogex.h"
#include "ETApplicationView.h"


// CSelectdiaryDlg 对话框

IMPLEMENT_DYNAMIC(CSelectdiaryDlg, CDialogEx)
extern HINSTANCE hResourceHandle;
CSelectdiaryDlg::CSelectdiaryDlg(CString str, CWnd* pParent /*=NULL*/)
	: CDialogEx(CSelectdiaryDlg::IDD, pParent)
{
	memset(m_bufftime,0,sizeof(TCHAR)*8);
	m_strpath = str;
	m_id = -1;
}

CSelectdiaryDlg::~CSelectdiaryDlg()
{
}

void CSelectdiaryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}



BOOL CSelectdiaryDlg::OnInitDialog()
{
	    CETApplicationView *prev = (CETApplicationView*) this->GetParent();
	    CDialogEx::OnInitDialog();
		CRect rect;

		HINSTANCE hinst = ::AfxGetInstanceHandle();
		if(hResourceHandle != NULL)
		   hinst = hResourceHandle;

		LoadString(hinst,IDS_STR_TIME,m_bufftime,8);

		ploadiary = (CListCtrl*)GetDlgItem(IDC_LIST_DY);

		ploadiary->GetClientRect(rect);
		ploadiary->ModifyStyle(0,LVS_SINGLESEL);
	    ploadiary->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES|LVS_EX_AUTOCHECKSELECT); 
		ploadiary->InsertColumn(0,_T("NO."));
		ploadiary->InsertColumn(1,m_bufftime);
		ploadiary->SetColumnWidth(0,rect.Width()*2/5);
		ploadiary->SetColumnWidth(1,rect.Width()*3/5);
	//--------------------------------------------------------------------
		BOOL fret = FALSE, result = TRUE;
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
		int timenodenum = 0;
		std::string strday ,stroldday;
		std::wstring strwday;

		std::string absoluteFilename;
		std::wstring absolutewFilename;

		CString strpath;
		int count = 0;

		//m_d_TimeList.RemoveAll();
		
		memset(buffhead,0,256*sizeof(char));
		memset(onerecordbuff,0,128*sizeof(char));

		strpath = m_strpath;

		if(result)
		{
				fret = freadiary.Open(strpath,CFile::modeRead);   //CFile::modeReadWrite);
				if(fret == 0)
				{
					MessageBox(_T("No diary founded!"));
					return -1;
				}
		}
		
		ASSERT(freadiary);
		
		int  len = freadiary.GetLength();
		int s=0;

		pbuff = new char[len];
		memset(pbuff,0,len * sizeof(char));
	
		if(len)
		{
			freadiary.SeekToBegin();
			s = freadiary.Read(pbuff,len);
		}
	
		while(psession = strstr(pbuff,"AMPSTART"))
		{
			CString strid;
			CString retstr;
			pbuff = psession;
			pbuff += 8;

			phead = strstr(pbuff,"DIARYSTART");
			headsize = phead - pbuff;
			strncpy(buffhead,pbuff,headsize);
			//retstr = _T("");
			retstr = prev->GetDiaryHead(buffhead,FALSE);
			memset(buffhead,0,256*sizeof(char));
			strid.Format(_T("%d"),count);
			ploadiary->InsertItem(count,strid);
			ploadiary->SetItemText(count,1,retstr);
			count++;
		}

		ploadiary->SetCheck(count - 1);

		freadiary.Close();		
		
	//--------------------------------------------------------------------

	return TRUE;
}
void CSelectdiaryDlg::OnOK( )
{
	int j = 0;
	int s = -1;
	BOOL set = FALSE;
	int num  = ploadiary->GetItemCount();
	for(int i =0 ; i < num; i++)
	{
		if(ploadiary->GetCheck(i))
		{ 
			if(!set)
			  s = i;
			
			set = TRUE;
			j++;
		}
	}


	if(j>=2)
	{
	    MessageBox(_T("Only one record can be check ,please reset!"));
		return ;
	}
	else
	{
	    if(set)
		   m_id = s;
	}

	CDialog::OnOK();
}

BEGIN_MESSAGE_MAP(CSelectdiaryDlg, CDialogEx)
END_MESSAGE_MAP()


// CSelectdiaryDlg 消息处理程序

