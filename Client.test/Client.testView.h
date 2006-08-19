// Client.testView.h : interface of the CClienttestView class
//


#pragma once


class CClienttestView : public CListView
{
protected: // create from serialization only
	CClienttestView();
	DECLARE_DYNCREATE(CClienttestView)

// Attributes
public:
	CClienttestDoc* GetDocument() const;

// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CClienttestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // debug version in Client.testView.cpp
inline CClienttestDoc* CClienttestView::GetDocument() const
   { return reinterpret_cast<CClienttestDoc*>(m_pDocument); }
#endif

