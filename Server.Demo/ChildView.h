// ChildView.h : interface of the CChildView class
//

#pragma once

#include "ServerDemoDoc.h"

// CChildView window

class CChildView : public CListView
{
protected:
	DECLARE_DYNCREATE(CChildView)

// Construction
public:
	CChildView();

// Attributes
public:

// Operations
public:

// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();
	CServerDemoDoc* GetDocument() const;

	// Generated message map functions
protected:
	afx_msg void OnUpdateAddTag(CCmdUI *pCmdUI);
	afx_msg void OnAddTagList();
	afx_msg void OnTimer( UINT_PTR ptr );

	DECLARE_MESSAGE_MAP()
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);

	void InsertItem(DemoTagDescriptor &tag);

	void UpdateAddTag(int tagno);
	void UpdateDeleteTag(int tagno);
	void UpdateEditTag(int tagno);
	void UpdateValueChanged(int tagno);
public:
	virtual void OnInitialUpdate();
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
};

