#pragma once


// CSelectdiaryDlg �Ի���

class CSelectdiaryDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSelectdiaryDlg)

public:
	CSelectdiaryDlg(CString str,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSelectdiaryDlg();

// �Ի�������
	enum { IDD = IDD_LOADDIARY };
	TCHAR   m_bufftime[8];
	int m_id;
protected:
	CString m_strpath;
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	CListCtrl *ploadiary;
	virtual void OnOK( );
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
