#pragma once
#include "afxcmn.h"


class treeItem
{
public:
	treeItem(const treeItem& i)
	{
		*this = i;
	}
	treeItem() { 
		brItems = NULL;
		index = -1;
	}
	const treeItem& operator=(const treeItem& i)
	{
		brItems = i.brItems;
		index = i.index;
		return *this;
	}

	BrowseItems* brItems;
	int index;
	string root_name;
};

// CTagBrowserDlg dialog

class CTagBrowserDlg : public CDialog
{
	DECLARE_DYNAMIC(CTagBrowserDlg)

public:
	CTagBrowserDlg(OPCClient &client, CWnd* pParent = NULL);   // standard constructor
	virtual ~CTagBrowserDlg();

// Dialog Data
	enum { IDD = IDD_ADD_TAG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	OPCClient &m_opc_client;

	set<BrowseItems*> brItems;
	std::set<treeItem*> treeItems;

	DECLARE_MESSAGE_MAP()

	CImageList m_ImageList;
public:
	void fillTree(HTREEITEM root, string name);
	virtual BOOL OnInitDialog();
	CTreeCtrl m_TagTree;
	afx_msg void OnTvnItemexpandingTagTree(NMHDR *pNMHDR, LRESULT *pResult);
	CString m_TagName;
	afx_msg void OnTvnSelchangedTagTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeTagValue();
	CString m_TagValue;
};
