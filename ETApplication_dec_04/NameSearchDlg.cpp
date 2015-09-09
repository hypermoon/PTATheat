// NameSearchDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ETApplication.h"
#include "NameSearchDlg.h"
#include "afxdialogex.h"
#include "ETApplicationView.h"


// CNameSearchDlg 对话框

IMPLEMENT_DYNAMIC(CNameSearchDlg, CDialogEx)

extern CString GetPTAResultLocalPath();
extern HINSTANCE hResourceHandle;
CNameSearchDlg::CNameSearchDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNameSearchDlg::IDD, pParent)
{
	selindex = 0;
	memset(m_buffDelUserIDWarn,0,sizeof(TCHAR)*128);
}

CNameSearchDlg::~CNameSearchDlg()
{
}

void CNameSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BOOL CNameSearchDlg::OnInitDialog()
{

	     BOOL ret;
	     CDialogEx::OnInitDialog();

		HINSTANCE hinst = ::AfxGetInstanceHandle();
		if(hResourceHandle != NULL)
			hinst = hResourceHandle;

		LoadString(hinst,IDS_STR_DEL_ID, m_buffDelUserIDWarn,128);

		 pid     = (CComboBox*)GetDlgItem(IDC_ID);
	     pname   = (CComboBox*)GetDlgItem(IDC_NAME);
	     page    = (CComboBox*)GetDlgItem(IDC_AGE);;
	     pgender = (CComboBox*)GetDlgItem(IDC_GENDER);;

	   //  p_editname = (CEdit*)GetDlgItem(IDC_EDIT_NAME);
	     pfindbutton = (CButton*)GetDlgItem(IDC_FIND);

		 pid->ModifyStyle(CBS_SORT,0);
		 pname->ModifyStyle(CBS_SORT,0);
		 page->ModifyStyle(CBS_SORT,0);
		 pgender->ModifyStyle(CBS_SORT,0);

		 page->EnableWindow(FALSE);
		 pgender->EnableWindow(FALSE);

		 ret = ReadIdxFile();
		 if(ret)
		 {
		      return  TRUE;
		 }
		 else
		 {
			 OnOK();
			 return FALSE;
		 }

}
BOOL CNameSearchDlg::ReadIdxFile()
{
	 CString strpath;
	 CFile fidx;
	 int bufsize = 0;
	 TCHAR *pbuffMemo,*pbuff;
	 TCHAR strstorebuff[64];
	 int tcharlen = sizeof(TCHAR);
	 LPTSTR pret = NULL;
	 LPTSTR psingle = NULL;

	 CString pBeg  = _T("  --- Beg ---  \r\n");
	 CString pSep  = _T("  --- End ---  ");
	 CString p2tab = _T("	");

	 CString str[6];
	 int cnum = 0;
	 int offset = 0;

	 BOOL validrow = TRUE;

	 strpath = GetPTAResultLocalPath();
	 strpath = strpath + _T("idx.txt");

	 fidx.Open(strpath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite);
	 ASSERT(fidx);
	 ULONGLONG lens = fidx.GetLength();
	 bufsize = lens / tcharlen;

	 if(lens)
	 {
	
			pbuffMemo = new TCHAR [bufsize];
			memset(pbuffMemo,0,bufsize*sizeof(TCHAR));
		    fidx.SeekToBegin();
			fidx.Read(pbuffMemo,lens);

			while((pret = _tcsstr(pbuffMemo,pBeg)) != NULL)
			{
				cnum = 0;
				pbuffMemo = pret + pBeg.GetLength();
				memset(strstorebuff,0,64*tcharlen);

				while((psingle = _tcsstr(pbuffMemo,_T("\r\n")) ) != NULL)
				{
					CString strtmp;
					offset = psingle - pbuffMemo;
					strtmp = _tcsncpy(strstorebuff,pbuffMemo,offset);
					if(strtmp.Find(p2tab) == -1)  //not deleted row
					{
						str[cnum] = strtmp;
						pbuffMemo = psingle + 2;
						memset(strstorebuff,0,64*tcharlen);
						cnum++;
						if((cnum >5) || !(strtmp.Compare(pSep)))
						{
							break;
						}
					}
					else
					{
						validrow = FALSE;
						break;
					}
				}
				 
				  if(validrow)
				  {
					pid->AddString(str[0]);
					pname->AddString(str[1]);
					page->AddString(str[2]);
					pgender->AddString(str[3]);
				  }
				    validrow = TRUE;

			}
			
	 }
	 else
	 {
		 MessageBox(_T("NO user info finded"));
		 fidx.Close();
		 return FALSE;
	 }

	 pid->SetCurSel(0);
	 pname->SetCurSel(0);
	 page->SetCurSel(0);
	 pgender->SetCurSel(0);

	 fidx.Close();
	 return TRUE;
}

void CNameSearchDlg::IdChange()
{

	selindex =  pid->GetCurSel(); 
	pname->SetCurSel(selindex);
	page->SetCurSel(selindex);
	pgender->SetCurSel(selindex);

}
void CNameSearchDlg::NameChange()
{
	selindex =  pname->GetCurSel(); 
	pid->SetCurSel(selindex);
	page->SetCurSel(selindex);
	pgender->SetCurSel(selindex);
	
}
 void CNameSearchDlg::OnOK()
 {

	TCHAR buffid[16];
	TCHAR buffname[64];
	TCHAR buffage[16];
	TCHAR buffgender[16];

	memset(buffid,0,16 * sizeof(TCHAR));
	memset(buffname,0,64 * sizeof(TCHAR));
	memset(buffage,0,16 * sizeof(TCHAR));
	memset(buffgender,0,16 * sizeof(TCHAR));

	pid->GetWindowTextW(buffid,16);
	pname->GetWindowTextW(buffname,64);
	page->GetWindowTextW(buffage,16);
	pgender->GetWindowTextW(buffgender,16);

	str_id = buffid;
	str_name = buffname;
	str_age = buffage;
	str_gender = buffgender;
	 

	 CDialogEx::OnOK();
 }
 void CNameSearchDlg::EditChange()
 {
	 CString str;
	 int retval = 0;
	 TCHAR strbuff[64];
	 memset(strbuff,0,64*sizeof(TCHAR));
	 pname->GetWindowTextW(strbuff,64);
	 str = strbuff;
	 retval = pname->FindStringExact(-1,str);
	 if( retval != CB_ERR )
	 {
		pid->SetCurSel(retval);
		page->SetCurSel(retval);
		pgender->SetCurSel(retval);
		selindex = retval;
	 }
	 else
	 {
		pid->SetCurSel(-1);
		page->SetCurSel(-1);
		pgender->SetCurSel(-1);

	 }
 }
 
 void CNameSearchDlg::DeleteIDfolder(CString strpath)
 {
	 CString strdiary;
	 CString strpta;
	 
	 strdiary = strpath + _T("diary.txt");

	 strpta = strpath + _T("ptaresult.txt");

	 DeleteFile(strdiary);
	 DeleteFile(strpta);
	 RemoveDirectory(strpath);

 }
 void CNameSearchDlg::OnDelete()
 {
	 CString strid;
	 int ret=0;
	 CString strpath;
	 CFile fidx;
	 int bufsize = 0;
	 TCHAR *pbuffMemo,*pbuff;
	 int tcharlen = sizeof(TCHAR);
	 LPTSTR pret = NULL;
	 LPTSTR pend = NULL;

	 TCHAR* pstr = NULL;
	 //TCHAR strstorebuff[64];
	 int offlen = 0, reclen = 0;
	 TCHAR buffid[16];

	 CString pSep  = _T("  --- End ---  ");
	 CString p2tab = _T("	"); //tab
	 CString rnend = _T(" ");   //space

	 memset(buffid,0,16 * sizeof(TCHAR));

	 ret  = MessageBox(m_buffDelUserIDWarn,0, MB_YESNO); ///   _T("Are you sure to delete this patient's record?"),0,MB_YESNO);

	 if(ret == IDYES)
	 {
		      CString idpath;
			  strpath = GetPTAResultLocalPath();
			  idpath = strpath;
			  strpath = strpath + _T("idx.txt");
			  pid->GetWindowTextW(buffid,16);
			  strid = buffid;

			  idpath = idpath + strid;
			  idpath += _T("\\");
			  DeleteIDfolder(idpath);

			  fidx.Open(strpath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite);
			  ASSERT(fidx);

		  	  ULONGLONG lens = fidx.GetLength();
			  bufsize = lens / tcharlen;

				 if(lens)
				 {
						pbuffMemo = new TCHAR [bufsize];
						memset(pbuffMemo,0,bufsize*sizeof(TCHAR));
						fidx.SeekToBegin();
						fidx.Read(pbuffMemo,lens);

						pret = _tcsstr(pbuffMemo,strid);
						pend = _tcsstr(pret,pSep);
						if(pret == NULL) 
							return;

						offlen = pret - pbuffMemo;
						reclen = pend - pret;

						pstr = new TCHAR [reclen-1];
						
						//memset(pstr,0,(reclen-1) * sizeof(TCHAR));
						//memset(pstr,' ',(reclen-1) * sizeof(TCHAR));

						wmemset(pstr,_T(' '),(reclen-1));

						//memset(pstr,_T(' '),(reclen-1) * sizeof(TCHAR));
						pstr[reclen-1-1] = _T('\0');
						pstr[reclen-1-2] = _T('\n');
						pstr[reclen-1-3] = _T('\r');
						
						int slen = 0;
						slen = _tcslen(pstr);

						fidx.Seek(offlen*sizeof(TCHAR),CFile::begin);
						fidx.Write(p2tab,_tcslen(p2tab) * tcharlen);
						fidx.Write(pstr,_tcslen(pstr) * tcharlen);

						//fidx.Write(rnend,_tcslen(rnend)*tcharlen);
						//fidx.Write(rnend,_tcslen(rnend)*tcharlen);
						//fidx.Write(rnend,_tcslen(rnend)*tcharlen);

						fidx.Close();

		
						pid->DeleteString(selindex);
						pname->DeleteString(selindex);
						page->DeleteString(selindex);
						pgender->DeleteString(selindex);

						pid->SetCurSel(-1);
						pname->SetCurSel(-1);
						page->SetCurSel(-1);
						pgender->SetCurSel(-1);

				 }
	 }



 }


BEGIN_MESSAGE_MAP(CNameSearchDlg, CDialogEx)
	ON_CBN_SELENDOK(IDC_ID, IdChange)
	ON_BN_CLICKED(IDC_FIND,OnDelete)
	ON_CBN_EDITCHANGE(IDC_NAME,EditChange)
	ON_CBN_SELENDOK(IDC_NAME, NameChange)
END_MESSAGE_MAP()


// CNameSearchDlg 消息处理程序
