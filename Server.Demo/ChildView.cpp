// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "Server.Demo.h"
#include "ServerDemoDoc.h"
#include ".\childview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

// CChildView
IMPLEMENT_DYNCREATE(CChildView, CListView)

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CListView)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_WM_TIMER()
END_MESSAGE_MAP()

CServerDemoDoc *CChildView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CServerDemoDoc)));
	return (CServerDemoDoc*)m_pDocument;
}

// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CListView::PreCreateWindow(cs);
}

void CChildView::OnInitialUpdate()
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

	TagList.SetExtendedStyle(TagList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	UpdateDeleteTag(0);
}

void CChildView::InsertItem(DemoTagDescriptor &tag)
{
	CListCtrl &tagList = GetListCtrl();
	// Add new item
	int count = tagList.GetItemCount();
	tag.itemNo = tagList.InsertItem(count, tag.tag_name.c_str());

	CString strtype = "><";
	switch (tag.tag_type) {
		case VT_R4:
			strtype = "VT_R4";
			break;
		case VT_R8:
			strtype = "VT_R8";
			break;
		case VT_I2:
			strtype = "VT_I2";
			break;
		case VT_I4:
			strtype = "VT_I4";
			break;
	}
	tagList.SetItemText(tag.itemNo, 1, strtype);
}

void CChildView::UpdateAddTag(int tagno)
{
	CServerDemoDoc *pDoc = GetDocument();
	CListCtrl &tagList = GetListCtrl();

	if (pDoc->m_TagList.size() == 0) {
		return;
	}

	DemoTagDescriptor &tag_descriptor = pDoc->m_TagList[tagno];

	if (tag_descriptor.itemNo == -1) {
		InsertItem(tag_descriptor);
	} 
}

void CChildView::UpdateDeleteTag(int tagno)
{
// Удаляем все пункты и создаем их заново

	CServerDemoDoc *pDoc = GetDocument();
	CListCtrl &tagList = GetListCtrl();

	tagList.DeleteAllItems();

	if (pDoc->m_TagList.size() == 0) {
		return;
	}
	vector <DemoTagDescriptor>::iterator it;

	for (it = pDoc->m_TagList.begin(); it != pDoc->m_TagList.end(); ++it) {
		InsertItem(*it);
	}
}

void CChildView::UpdateEditTag(int tagno)
{
	UpdateDeleteTag(tagno);
}

void CChildView::UpdateValueChanged(int tagno)
{
	CServerDemoDoc *pDoc = GetDocument();
	// find the tag

	vector<DemoTagDescriptor>::iterator it;

	for (it = pDoc->m_TagList.begin(); it != pDoc->m_TagList.end(); ++it) {
		if (it->itemNo == tagno) {
			 // Got it !!!
			
			GetListCtrl().SetItemText(tagno, 2, it->tag_value.c_str());
			return;
		}
	}
}

void CChildView::OnTimer( UINT_PTR ptr )
{
	OnUpdate(NULL, (LPARAM)ptr, NULL);
}

void CChildView::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	int cmd = lHint & DEMO_CMD_MASK;
	int tag = lHint & DEMO_TAGNO_MASK;

	switch (cmd) {
		case DEMO_CMD_ADD_TAG:
			UpdateAddTag(tag);
			break;
		case DEMO_CMD_DELETE_TAG:
			UpdateDeleteTag(tag);
			break;
		case DEMO_CMD_EDIT_TAG:
			UpdateEditTag(tag);
			break;
		case DEMO_CMD_VALUE_CHANGED:
			UpdateValueChanged(tag);
			break;
	}
}

void CChildView::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	// Edit selected item
	CServerDemoDoc *pDoc = GetDocument();

	pDoc->OnEditTag();

	*pResult = 0;
}

void CChildView::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	// Get selected item;
	int selected = GetListCtrl().GetSelectionMark();

	if (selected == -1) {
		return;
	}

	GetDocument()->SetSelection(selected);
	*pResult = 0;
}
