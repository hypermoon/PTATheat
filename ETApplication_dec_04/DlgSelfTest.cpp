// DlgSelfTest.cpp : 实现文件
//

#include "stdafx.h"
#include "ETApplication.h"
#include "DlgSelfTest.h"
#include "afxdialogex.h"


// CDlgSelfTest 对话框

IMPLEMENT_DYNAMIC(CDlgSelfTest, CDialogEx)

CDlgSelfTest::CDlgSelfTest(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgSelfTest::IDD, pParent)
{

}

CDlgSelfTest::~CDlgSelfTest()
{
}

void CDlgSelfTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSelfTest, CDialogEx)
END_MESSAGE_MAP()


// CDlgSelfTest 消息处理程序
