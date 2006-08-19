// TagBrowserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Client.test.h"
#include "TagBrowserDlg.h"
#include ".\tagbrowserdlg.h"


// CTagBrowserDlg dialog

IMPLEMENT_DYNAMIC(CTagBrowserDlg, CDialog)
CTagBrowserDlg::CTagBrowserDlg(OPCClient &client, CWnd* pParent /*=NULL*/)
	: CDialog(CTagBrowserDlg::IDD, pParent)
	, m_opc_client(client)
	, m_TagName(_T(""))
	, m_TagValue(_T(""))
{
}

CTagBrowserDlg::~CTagBrowserDlg()
{
}

void CTagBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAG_TREE, m_TagTree);
	DDX_Text(pDX, IDC_TAG_NAME, m_TagName);
	DDX_Text(pDX, IDC_TAG_VALUE, m_TagValue);
}


void CTagBrowserDlg::fillTree(HTREEITEM root, string name)
{
	BrowseItems *br = new BrowseItems( &m_opc_client, name );

	for( size_t i=0; i < br->items.size(); ++i ) {
		BrowseItems::Item &it = br->items[i];

		HTREEITEM pItem = NULL;

		int IconId = 1;

		switch (it.flag) {
			case OPC_BROWSE_HASCHILDREN:
				IconId = 1;
				break;
			case OPC_BROWSE_ISITEM:
				IconId = 0;
				break;
			default:
				IconId = 0;
				break;
		}

		if( root ==  NULL ) {
			root = TVI_ROOT;
		}

		pItem = m_TagTree.InsertItem( it.name.c_str(), IconId, IconId, root);

		if( it.flag == OPC_BROWSE_HASCHILDREN ) {
			m_TagTree.SetItemImage( pItem, 1, 1 );
			m_TagTree.InsertItem("TEMP", pItem);
		}

		m_TagTree.SetItemText( pItem, it.itemId.c_str() );

		treeItem *item = new treeItem();
		item->brItems = br;
		item->index = i;
		item->root_name = name;

		m_TagTree.SetItemData( pItem, (DWORD)item );
		treeItems.insert( item );
	}

	brItems.insert( br );
}


BEGIN_MESSAGE_MAP(CTagBrowserDlg, CDialog)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TAG_TREE, OnTvnItemexpandingTagTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TAG_TREE, OnTvnSelchangedTagTree)
END_MESSAGE_MAP()


// CTagBrowserDlg message handlers


BOOL CTagBrowserDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ImageList.Create( IDB_IMAGE_TREE, 16, 0, RGB(255,255,255) );
	m_TagTree.SetImageList(&m_ImageList, TVSIL_NORMAL);

	fillTree(NULL, "");
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CTagBrowserDlg::OnTvnItemexpandingTagTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	HTREEITEM item = pNMTreeView->itemNew.hItem;

	HTREEITEM child = m_TagTree.GetChildItem(item);

	CString child_name = m_TagTree.GetItemText(child);

	treeItem *ti = (treeItem *)m_TagTree.GetItemData(item);
	if (ti == NULL) {
		AfxTrace("Invalid object tree item");
		return;
	}

	int FolderIconId = 2;
	if (pNMTreeView->action == TVE_EXPAND )  {
		FolderIconId = 2;
	} else if (pNMTreeView->action == TVE_COLLAPSE) {
		FolderIconId = 1;
	}

	if (child_name == "TEMP") {
		m_TagTree.DeleteItem(child);

		treeItem *titem = (treeItem *)m_TagTree.GetItemData(item);

		BrowseItems::Item &it = titem->brItems->items[ titem->index ];
		if( it.flag == OPC_BROWSE_HASCHILDREN ) {
			fillTree( item, it.itemId );
		}

	}

	m_TagTree.SetItemImage(item, FolderIconId, FolderIconId);
	*pResult = 0;
}

void CTagBrowserDlg::OnTvnSelchangedTagTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	HTREEITEM item = pNMTreeView->itemNew.hItem;

	treeItem *titem = (treeItem *)m_TagTree.GetItemData(item);

	BrowseItems::Item &it = titem->brItems->items[ titem->index ];
	m_TagName = it.itemId.c_str();

	// Read tag value, to do it add item to client object at first
	static OPCHANDLE client_hdl = 2000;

	OPCHANDLE client_id = m_opc_client.AddTag(client_hdl, m_TagName, VT_UNKNOWN);

	FILETIME time;
	VARIANT value; VariantInit(&value);
	WORD	quality;

	if (!m_opc_client.ReadValue(client_id, time, value, quality)) {
		AfxTrace("Can't read tag value for %s\n", it.itemId.c_str());
		m_TagValue = "-------";
	} else {
		m_TagValue = FormatValue(value, quality).c_str();
	}

	m_opc_client.RemoveTag(client_id);

	client_hdl++;

	UpdateData(FALSE);

	*pResult = 0;
}
