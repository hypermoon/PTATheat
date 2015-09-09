#pragma once


// CEarProgressDlg 对话框

class CEarProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CEarProgressDlg)

public:
	CEarProgressDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CEarProgressDlg();
	virtual BOOL PreTranslateMessage(MSG* pMsg );
// 对话框数据
	enum { IDD = IDD_ProgressDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
