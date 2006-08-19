#pragma once

#include "ServerDemoDoc.h"
#include "afxwin.h"
// CTagEditDlg dialog

class CTagEditDlg : public CDialog
{
	DECLARE_DYNAMIC(CTagEditDlg)

public:
	CTagEditDlg(DemoTagDescriptor tag, CWnd* pParent = NULL);   // standard constructor
	virtual ~CTagEditDlg();

// Dialog Data
	enum { IDD = IDD_TAG_EDIT };

	DemoTagDescriptor edit_tag;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_TagName;
	int m_TagDatatype;
	CString m_TagValue;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CComboBox m_TagTypeCtrl;
};
