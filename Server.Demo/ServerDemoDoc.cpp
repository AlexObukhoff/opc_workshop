// ServerDemo document class
//

#include "stdafx.h"
#include "server.Demo.h"
#include "ServerDemoDoc.h"
#include "TagEditDlg.h"

#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

#include "DelayImpHlp.h"

DL_USE_MODULE_BEGIN(OpcServer, "Opcserver.dll")
	DL_DECLARE_FUN(CreateNewOPCServerObject, COPCServerObject*, (void) )
	DL_DECLARE_FUN(DestroyOPCServerObject, int, (COPCServerObject*) )
DL_USE_MODULE_END()


// {4EA2713D-CA07-11d4-BEF5-00002120DB5E}   exe-server by default 
GUID CLSID_OPCServerEXE = { 0x4ea2713d, 0xca06, 0x11d4, {0xbe, 0xf5, 0x0, 0x0, 0x21, 0x20, 0xdb, 0x5E} };
CString OPCServerProgID = _T("OPC.DemoServer");

void DemoDataReceiver::newData(LPCTSTR name, DWORD /*clientId*/, FILETIME &/*time*/, VARIANT &value, WORD Quality )
{
	if (pDoc != NULL) {
		pDoc->DataChanged(name, value, Quality);
	}
}


// CServerDemoDoc

IMPLEMENT_DYNCREATE(CServerDemoDoc, CDocument)

BEGIN_MESSAGE_MAP(CServerDemoDoc, CDocument)
	ON_COMMAND(ID_ADD_TAG, OnAddTag)
	ON_UPDATE_COMMAND_UI(ID_ADD_TAG, OnUpdateAddTag)
	ON_COMMAND(ID_DELETE_TAG, OnDeleteTag)
	ON_UPDATE_COMMAND_UI(ID_DELETE_TAG, OnUpdateDeleteTag)
	ON_COMMAND(ID_EDIT_TAG, OnEditTag)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TAG, OnUpdateEditTag)
END_MESSAGE_MAP()


// CServerDemoDoc construction/destruction

CServerDemoDoc::CServerDemoDoc()
{
	selection = NULL;
	server = NULL; 

	try {
		server = OpcServer::CreateNewOPCServerObject();
	} catch(...)
	{
		AfxMessageBox("Failed load OPCserver.dll", MB_ICONERROR );
		exit(-1);
	}

	if( server == NULL ) {
		AfxMessageBox("Failed create COPCServerObject\nMaibe failed load OPCServer.dll", MB_ICONERROR );
		exit(-1);
	}

	// set server name and clsid
	server->setServerProgID( _T("OPC.DemoServer") );
	server->setServerCLSID( CLSID_OPCServerEXE );

	// set delimeter for params name 
	server->SetDelimeter( "." );

	// register server as COM/DCOM object 
	server->RegisterServer();

	receiver.pDoc = this;
	server->setDataReceiver(&receiver);

	// create COM class factory and register it 
	server->StartServer( OPC_STATUS_RUNNING );

}

CServerDemoDoc::~CServerDemoDoc()
{
	receiver.pDoc = NULL;
	server->StopServer();
	server->UnregisterServer();
	OpcServer::DestroyOPCServerObject( server );
	server = NULL;
}

BOOL CServerDemoDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	if( server != NULL ) {
		DemoTagDescriptor tag_dsc;
		tag_dsc.tag_name = _T("Demo.Int2");
		tag_dsc.tag_type = VT_I2;
		AddTag( tag_dsc );

		tag_dsc.tag_name = _T("Demo.Int4");
		tag_dsc.tag_type = VT_I4;
		AddTag( tag_dsc );

		tag_dsc.tag_name = _T("Demo.Real4");
		tag_dsc.tag_type = VT_R4;
		AddTag( tag_dsc );
	}
	return TRUE;
}

void CServerDemoDoc::OnCloseDocument()
{
	__super::OnCloseDocument();
}

// CServerDemoDoc serialization

void CServerDemoDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// сохраняем вектор
		ar << m_TagList.size();
		vector<DemoTagDescriptor>::iterator it;
		for (it = m_TagList.begin(); it != m_TagList.end(); ++it) {
			ar << it->itemNo;
			ar << CString(it->tag_name.c_str());
			ar << int(it->tag_type);
			ar << CString (it->tag_value.c_str());
		}
	}
	else
	{
		int size;
		ar >> size;

		if (size > 0) {
			for (int i = 0; i < size; i++) {
				DemoTagDescriptor tag;
				ar >> tag.itemNo;
				CString name; ar >> name; tag.tag_name = (LPCSTR)name;
				int tp; ar >> tp; tag.tag_type = (VARENUM)tp;
				CString value; ar >> value; tag.tag_value = (LPCSTR)value;

				AddTag(tag);
			} 
		}
	}
}

// ента функция вызывается из соседнего процесса.
void CServerDemoDoc::DataChanged(LPCTSTR name, VARIANT &value, WORD quality)
{
	vector <DemoTagDescriptor>::iterator it;

	for (it = m_TagList.begin(); it != m_TagList.end(); ++it) {
		if (it->tag_name == name) {
			// Got it !

			stringstream s;

			CComVariant val = value;
			switch( val.vt ) {
				case VT_I2:
				case VT_I4:
				case VT_INT:		
					s << val.intVal; 
					break;
				case VT_FILETIME:	
					break;
				case VT_R4:			
					s << val.fltVal; 
					break;
				case VT_R8:			
					s << val.dblVal; 
					break;
				case VT_EMPTY:
					s << -1.999999; 
					break;
				default:
					s << 1.999999; 
					break;

			}
			it->tag_value = s.str();

			POSITION pos = GetFirstViewPosition();
			if(pos != NULL)
			{
				CView* pView = GetNextView(pos);
				pView->PostMessage( WM_TIMER, it->itemNo | DEMO_CMD_VALUE_CHANGED );
			}
			//UpdateAllViews(NULL, it->itemNo | DEMO_CMD_VALUE_CHANGED  );
			return;
		}
	}
}

// CServerDemoDoc diagnostics

#ifdef _DEBUG
void CServerDemoDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CServerDemoDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CServerDemoDoc commands

void CServerDemoDoc::SetSelection(int item_index)
{
	vector<DemoTagDescriptor>::iterator it;
	for (it = m_TagList.begin(); it != m_TagList.end(); ++it) {
		if (it->itemNo == item_index) {
			selection = &(*it);
			return;
		}
	}
	selection = NULL;
	AfxTrace("CServerDemoDoc WARNING ! Selection reset\n");
}

void CServerDemoDoc::LoadFile(const char *fname)
{
	// Check for file existance
	FILE *fp = fopen(fname, "r");
	if (fp == NULL)
		return;

	fclose(fp);

	OnOpenDocument(fname);
}

void CServerDemoDoc::SaveFile(const char *fname)
{
	OnSaveDocument(fname);
}

void CServerDemoDoc::AddTag(DemoTagDescriptor &tag_dsc)
{
	if( server == NULL )
		return;

	if (server->AddTag(tag_dsc.tag_name.c_str(), tag_dsc.tag_type, false) >= 0) {
		int count = m_TagList.size();
		tag_dsc.itemNo = -1;

		m_TagList.push_back(tag_dsc);
		UpdateAllViews(NULL, DEMO_CMD_ADD_TAG | count);
	}
}

void CServerDemoDoc::OnAddTag()
{
	// Позовем диалог редактирования тэга

	DemoTagDescriptor tag_dsc;
	CTagEditDlg dlg(tag_dsc);

	if (dlg.DoModal() == IDOK) {
		// Apply changes
	
		AddTag(dlg.edit_tag);
	}
}

void CServerDemoDoc::OnUpdateAddTag(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( TRUE); 
}

void CServerDemoDoc::OnDeleteTag()
{
	int sel = GetSelection();

	int ans = AfxMessageBox("удаление тэга в сервере не предусмотрено. Удалить тэг в списке?", MB_YESNO | MB_ICONQUESTION);

	if (ans == IDYES) {
		// Find the item and delete it

		vector <DemoTagDescriptor>::iterator it;
		for (it = m_TagList.begin(); it != m_TagList.end(); ++it) {
			if (it->itemNo == sel) {
				m_TagList.erase(it);		
				UpdateAllViews(NULL, sel | DEMO_CMD_DELETE_TAG);
				return;
			}
		}
	}
}

void CServerDemoDoc::OnUpdateDeleteTag(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_TagList.size() != 0); 
}

void CServerDemoDoc::OnEditTag()
{
	int ans = AfxMessageBox("Изменение тэга в сервере не предусмотрено. Изменить тэг в списке?", 
		MB_YESNO | MB_ICONQUESTION);

	if (ans == IDYES) {
		// Edit selected tag - get selection
		int sel = GetSelection();

		// Find the item and delete it

		vector <DemoTagDescriptor>::iterator it;
		for (it = m_TagList.begin(); it != m_TagList.end(); ++it) {
			if (it->itemNo == sel) {
				CTagEditDlg dlg(*it);

				if (dlg.DoModal() == IDOK) {
					UpdateAllViews(NULL, sel | DEMO_CMD_EDIT_TAG);
				}
				return;
			}
		}
	}
}

void CServerDemoDoc::OnUpdateEditTag(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_TagList.size() != 0 ); 
}

void CServerDemoDoc::OnOpenCsv()
{
	AfxMessageBox("Load CSV");
}

void CServerDemoDoc::OnSaveCsv()
{
	AfxMessageBox ("Save CSV");
}

void CServerDemoDoc::OnUpdateOpenCsv(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( TRUE); 
}

void CServerDemoDoc::OnUpdateSaveCsv(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( TRUE); 
}
