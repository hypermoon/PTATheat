#pragma once


// CDlgSelfTest 对话框

class CDlgSelfTest : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSelfTest)

public:
	CDlgSelfTest(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSelfTest();

// 对话框数据
	enum { IDD = IDD_DIALOG_SELFTEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
