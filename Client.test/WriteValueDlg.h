#pragma once


// CWriteValueDlg dialog

class CWriteValueDlg : public CDialog
{
	DECLARE_DYNAMIC(CWriteValueDlg)

public:
	CWriteValueDlg(OPCClient &client, CWnd* pParent = NULL);   // standard constructor
	virtual ~CWriteValueDlg();

// Dialog Data
	enum { IDD = IDD_WRITE_VALUE };

protected:
	OPCClient& m_OPCClient;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_TagName;
	CString m_TagValue;
	OPCHANDLE  m_tag_client_id;
	VARIANT  m_last_value; // Нужен как образец для определения требуемого типа данных

	afx_msg void OnBnClickedWrite();
};
