#pragma once


// CNameSearchDlg �Ի���

class CNameSearchDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNameSearchDlg)

public:
	CNameSearchDlg(CWnd* pParent = NULL);   // ��׼���캯��
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
// �Ի�������
	enum { IDD = IDD_DLG_SEARCH };

	virtual BOOL OnInitDialog();
	 void DeleteIDfolder(CString strpath);
	BOOL ReadIdxFile();
private:
	int selindex;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual void OnOK( );
	afx_msg void IdChange( );
	afx_msg void EditChange();
	afx_msg void NameChange();
	afx_msg void OnDelete();
	DECLARE_MESSAGE_MAP()
};
