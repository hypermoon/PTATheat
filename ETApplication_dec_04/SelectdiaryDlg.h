#pragma once


// CSelectdiaryDlg 对话框

class CSelectdiaryDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSelectdiaryDlg)

public:
	CSelectdiaryDlg(CString str,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSelectdiaryDlg();

// 对话框数据
	enum { IDD = IDD_LOADDIARY };
	TCHAR   m_bufftime[8];
	int m_id;
protected:
	CString m_strpath;
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	CListCtrl *ploadiary;
	virtual void OnOK( );
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
