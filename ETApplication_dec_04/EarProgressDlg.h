#pragma once


// CEarProgressDlg �Ի���

class CEarProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CEarProgressDlg)

public:
	CEarProgressDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CEarProgressDlg();
	virtual BOOL PreTranslateMessage(MSG* pMsg );
// �Ի�������
	enum { IDD = IDD_ProgressDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
