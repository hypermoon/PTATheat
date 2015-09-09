#pragma once


// CNameSearchDlg 对话框

class CNameSearchDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNameSearchDlg)

public:
	CNameSearchDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CNameSearchDlg();

	CComboBox *pid;
	CComboBox *pname;
	CComboBox *page;
	CComboBox *pgender;

	CString  str_id;
	CString  str_name;
	CString  str_age;
	CString  str_gender;
	//CEdit    *p_editname;
	CButton  *pfindbutton;
	TCHAR m_buffDelUserIDWarn[128];
// 对话框数据
	enum { IDD = IDD_DLG_SEARCH };

	virtual BOOL OnInitDialog();
	 void DeleteIDfolder(CString strpath);
	BOOL ReadIdxFile();
private:
	int selindex;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnOK( );
	afx_msg void IdChange( );
	afx_msg void EditChange();
	afx_msg void NameChange();
	afx_msg void OnDelete();
	DECLARE_MESSAGE_MAP()
};
