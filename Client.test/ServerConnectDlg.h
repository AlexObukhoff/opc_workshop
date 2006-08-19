#pragma once
#include "afxwin.h"


// CServerConnectDlg dialog

class CServerConnectDlg : public CDialog
{
	DECLARE_DYNAMIC(CServerConnectDlg)

public:
	CServerConnectDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CServerConnectDlg();

	std::map<string,string> m_OPCServerNames;

// Dialog Data
	enum { IDD = IDD_CONNECT_SERVER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CComboBox m_ServerList;
	CString m_GroupName;
	CString m_OPCServer;
	afx_msg void OnBnClickedOk();
	CString m_HostName;
};
