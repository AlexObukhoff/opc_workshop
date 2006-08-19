// Client.testView.cpp : implementation of the CClienttestView class
//

#include "stdafx.h"
#include "Client.test.h"

#include "Client.testDoc.h"
#include "Client.testView.h"
#include ".\client.testview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClienttestView

IMPLEMENT_DYNCREATE(CClienttestView, CListView)

BEGIN_MESSAGE_MAP(CClienttestView, CListView)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
END_MESSAGE_MAP()

// CClienttestView construction/destruction

CClienttestView::CClienttestView()
{
   m_dwDefaultStyle |= LVS_REPORT; 
}

CClienttestView::~CClienttestView()
{
}

BOOL CClienttestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CListView::PreCreateWindow(cs);
}

// CClienttestView drawing

void CClienttestView::OnDraw(CDC* /*pDC*/)
{
	CClienttestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CClienttestView diagnostics

#ifdef _DEBUG
void CClienttestView::AssertValid() const
{
	CListView::AssertValid();
}

void CClienttestView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CClienttestDoc* CClienttestView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CClienttestDoc)));
	return (CClienttestDoc*)m_pDocument;
}
#endif //_DEBUG


// CClienttestView message handlers

void CClienttestView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	// Create columns for tag list

	CListCtrl &TagList = GetListCtrl();

	TagList.ModifyStyle(0, LVS_REPORT);
	while( TagList.DeleteColumn( 0 ) )
		;

	CRect r;
	TagList.GetClientRect(&r);
	int w = r.right - r.left - 10;

	TagList.InsertColumn( 0,  _T("Tag"), LVCFMT_LEFT, w / 2, -1 );
	TagList.InsertColumn( 1,  _T("Type"), LVCFMT_LEFT, w / 6, 2 );
	TagList.InsertColumn( 2,  _T("Value"), LVCFMT_LEFT, w / 6, 3 );
	TagList.InsertColumn( 3,  _T("Status"), LVCFMT_LEFT, w / 6, 4 );

	TagList.SetExtendedStyle(TagList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
}

void CClienttestView::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	// Tag changed (or added)
	CClienttestDoc *pDoc = GetDocument();
	CListCtrl &tagList = GetListCtrl();

	if (pDoc->m_TagList.size() == 0) {
		// Это мог быть Disconnect

		tagList.DeleteAllItems();
		return;
	}

	tagDsc &tag_descriptor = pDoc->m_TagList[lHint];

	if (tag_descriptor.itemNo == -1) {
		// Add new item

		int count = tagList.GetItemCount();
		tag_descriptor.itemNo = tagList.InsertItem(count, tag_descriptor.tagName);
	} 

	tagList.SetItemText(tag_descriptor.itemNo, 1, FormatValueType(tag_descriptor.last_value).c_str());
	tagList.SetItemText(tag_descriptor.itemNo, 2, FormatValue(tag_descriptor.last_value, tag_descriptor.last_quality).c_str());
	tagList.SetItemText(tag_descriptor.itemNo, 3, FormatQuality(tag_descriptor.last_quality).c_str());
}

void CClienttestView::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	// Get selected item;
	int selected = GetListCtrl().GetSelectionMark();

	if (selected == -1) {
		return;
	}

	GetDocument()->SetSelection(selected);

	*pResult = 0;
}
