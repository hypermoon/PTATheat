// EarProgressDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ETApplication.h"
#include "EarProgressDlg.h"
#include "afxdialogex.h"


// CEarProgressDlg �Ի���

IMPLEMENT_DYNAMIC(CEarProgressDlg, CDialog)

CEarProgressDlg::CEarProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEarProgressDlg::IDD, pParent)
{

}

CEarProgressDlg::~CEarProgressDlg()
{
}

BOOL CEarProgressDlg::PreTranslateMessage(MSG* pMsg )
{
	if(pMsg->message==WM_KEYDOWN   &&  pMsg->wParam==VK_RETURN )
	{
	     return   TRUE;
	}
    return   CDialog::PreTranslateMessage(pMsg);
}
void CEarProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEarProgressDlg, CDialog)
END_MESSAGE_MAP()


// CEarProgressDlg ��Ϣ�������
