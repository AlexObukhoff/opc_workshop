// Client.testDoc.cpp : implementation of the CClienttestDoc class
//

#include "stdafx.h"
#include "Client.test.h"

#include "Client.testDoc.h"
#include "TagBrowserDlg.h"
#include "ServerConnectDlg.h"
#include "WriteValueDlg.h"
#include ".\client.testdoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define OPCCLIENT_FILE "OPCClient.sav"

// CClienttestDoc

IMPLEMENT_DYNCREATE(CClienttestDoc, CDocument)

BEGIN_MESSAGE_MAP(CClienttestDoc, CDocument)
	ON_COMMAND(ID_CONNECT, OnConnect)
	ON_UPDATE_COMMAND_UI(ID_CONNECT, OnUpdateConnect)
	ON_COMMAND(ID_DISCONNECT, OnDisconnect)
	ON_UPDATE_COMMAND_UI(ID_DISCONNECT, OnUpdateDisconnect)
	ON_COMMAND(ID_ADD_TAG, OnAddTag)
	ON_UPDATE_COMMAND_UI(ID_ADD_TAG, OnUpdateAddTag)
	ON_COMMAND(ID_WRITE_VALUE, OnWriteValue)
	ON_UPDATE_COMMAND_UI(ID_WRITE_VALUE, OnUpdateWriteValue)
	ON_COMMAND(ID_REFRESH, OnRefreshValue)
	ON_UPDATE_COMMAND_UI(ID_REFRESH, OnUpdateWriteValue)

	ON_COMMAND(ID_ADD_TAG_LIST, OnAddTagList)
END_MESSAGE_MAP()


// CClienttestDoc construction/destruction

CClienttestDoc::CClienttestDoc()
{
	selection = NULL;
}

CClienttestDoc::~CClienttestDoc()
{
}

BOOL CClienttestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;


	// Load from text file our connection settings and tag list
	LoadFile(OPCCLIENT_FILE);

	return TRUE;
}

void CClienttestDoc::OnCloseDocument()
{
	// Load from text file our connection settings and tag list
	SaveFile(OPCCLIENT_FILE);

	__super::OnCloseDocument();
}

// CClienttestDoc serialization

void CClienttestDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// Save Server Info

		ar << CString(m_OPCClient.m_ProgID.c_str());
		ar << CString(m_OPCClient.m_Host.c_str());
		ar << CString(m_OPCClient.m_GroupName.c_str());

		// Save tag list

		ar << m_TagList.size();
		vector <tagDsc>::iterator it;
		for (it = m_TagList.begin(); it != m_TagList.end(); ++it) {
			ar << it->tagName;
		}
	}
	else
	{
		CString ServerName, HostName, GroupName;
		ar >> ServerName;
		ar >> HostName;
		ar >> GroupName;

		ConnectAndCreateGroup(ServerName, HostName, GroupName);

		int size = 0;
		ar >> size;

		for (int i = 0; i < size; i++) {
			CString tagName;

			ar >> tagName;
			AddTag(tagName);
		}
		m_OPCClient.Refresh();
	}
}

// CClienttestDoc diagnostics

#ifdef _DEBUG
void CClienttestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CClienttestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CClienttestDoc commands

void CClienttestDoc::OnConnect()
{
	CServerConnectDlg dlg;

	if (IDOK == dlg.DoModal()) {
		ConnectAndCreateGroup(dlg.m_OPCServer, dlg.m_HostName, dlg.m_GroupName);
	}
	UpdateAllViews(NULL);
}

void CClienttestDoc::OnUpdateConnect(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(! m_OPCClient.isConnected()); // - Thomas Haase 
}

void CClienttestDoc::OnDisconnect()
{
	m_OPCClient.Disconnect();

	m_TagList.clear();
	tag_list_items.clear();

	UpdateAllViews(NULL);
}

void CClienttestDoc::OnUpdateDisconnect(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_OPCClient.isConnected()); // - Thomas Haase 
}

void CClienttestDoc::OnAddTag()
{
	CTagBrowserDlg dlg(m_OPCClient);

	if (IDOK == dlg.DoModal()) {

		AddTag(dlg.m_TagName);
	}
}

void CClienttestDoc::OnUpdateAddTag(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_OPCClient.isConnected()); // - Thomas Haase 
}

void CClienttestDoc::OnRefreshValue()
{
	m_OPCClient.Refresh();
}

void CClienttestDoc::OnWriteValue()
{
	// Get selected item and it's properties

	if (selection == NULL) {
		AfxMessageBox("Не выбран тэг для записи значения");
		return;
	}

	CWriteValueDlg dlg(m_OPCClient);

	dlg.m_TagName = selection->tagName;
	dlg.m_TagValue = FormatValue (selection->last_value, selection->last_quality).c_str();
	dlg.m_tag_client_id = selection->client_handle;
	dlg.m_last_value = selection->last_value;

	dlg.DoModal();
}

void CClienttestDoc::OnUpdateWriteValue(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_OPCClient.isConnected()); // - Thomas Haase 
}

void CClienttestDoc::log_message( LogLevel llevel, const char* fmt, ... )
{ 
	char str[4096] = {0};

	switch( llevel )
	{
	case LogDebugInfo:	
		m_LastMessage = "DBG: "; 
		break;
	case LogMessage:	
		m_LastMessage = "LOG: "; 
		break;
	case LogWarning:	
		m_LastMessage = "WAR: "; 
		break;
	case LogError:		
		m_LastMessage = "ERR: "; 
		break;
	};

	va_list args;
	va_start(args, fmt);
		_vsnprintf(str, 4000, fmt, args);
	va_end(args);

	m_LastMessage += str;
}

void CClienttestDoc::StatusChanged( int _status, const char *message )
{
	status = _status;
}

void CClienttestDoc::newData(LPCTSTR name, DWORD clientID, FILETIME &time, VARIANT &value, WORD Quality )
{
	map<OPCHANDLE, int>::iterator f = tag_list_items.find(clientID);
	if (f == tag_list_items.end()) {
		// Не наш клиент - видимо из дерева пришел
		return;
	}

	tagDsc &tag_descriptor = m_TagList[f->second];
	
	tag_descriptor.last_quality = Quality;
	tag_descriptor.last_time = time;
	tag_descriptor.last_value = value;

	TRACE("new param: %s [%d]\n", name, clientID );

	try {
		UpdateAllViews(NULL, f->second);
	} catch(...) {
		// may be exception if view not full construct 
//		OutputDebugString("UpdateAllViews() Exception!\n");
	}
}

int CClienttestDoc::AddTag(CString & tagName)
{
	int count = m_TagList.size();

	tagDsc tag_descriptor;
	tag_descriptor.tagName = tagName;
	tag_descriptor.itemNo = -1;

	int itemno = count;

	// Добавим подписку на этот тэг
	// Read tag value, to do it add item to client object at first
	static OPCHANDLE client_hdl = 200000;

	OPCHANDLE client_id = m_OPCClient.AddTag(client_hdl, tagName, VT_UNKNOWN);
	client_hdl++;

	tag_list_items[client_id] = itemno;
	tag_descriptor.client_handle = client_id;

	m_TagList.push_back(tag_descriptor);

	// Читаем начальное значение
	FILETIME time;
	VARIANT value; VariantInit(&value);
	WORD	quality;
	if (!m_OPCClient.ReadValue(client_id, time, value, quality)) {
		AfxTrace("Can't read tag value for %s\n", (LPCSTR)tagName);
	} else {
		newData(NULL, client_id, time, value, quality);
	}

	UpdateAllViews(NULL, itemno);
	return 0;
}

int CClienttestDoc::ConnectAndCreateGroup(CString &ServerName, CString &host, CString &GroupName)
{
	CString msg;

	m_OPCClient.setDataReceiver( this );
	m_OPCClient.m_ProgID = ServerName;
	m_OPCClient.m_Host = host;
	m_OPCClient.m_UseAsync = false;
	m_OPCClient.m_AddItemMode = true;
	m_OPCClient.m_GroupName = GroupName;

	HRESULT hr = m_OPCClient.Connect( ServerName, host );

	if( FAILED( hr ) ) {
		CString msg;
		msg.Format("Ошибка соединения с ОРС сервером: %s", m_LastMessage.c_str());
		AfxMessageBox(msg, MB_ICONSTOP | MB_OK);

		// Попробуем не учитывать наличие IOPCShutdown на сервере

		m_OPCClient.SetFlags (m_OPCClient.GetFlags () | OPC_CLIENT_IGNORE_SHUTDOWN);

		hr = m_OPCClient.Connect( ServerName, NULL );
		msg.Format("Ошибка соединения с ОРС сервером: %s (проверка IOPCShutdown отключена)", 
			m_LastMessage.c_str());

		AfxMessageBox(msg, MB_ICONSTOP | MB_OK);		
		return 0;
	}
	return 0;
}

void CClienttestDoc::SetSelection(int item_index)
{
	vector<tagDsc>::iterator it;
	for (it = m_TagList.begin(); it != m_TagList.end(); ++it) {
		if (it->itemNo == item_index) {
			selection = &(*it);
			return;
		}
	}
	selection = NULL;
	AfxTrace("CClienttestDoc WARNING ! Selection reset\n");
}

void CClienttestDoc::LoadFile(const char *fname)
{
	// Check for file existance
	FILE *fp = fopen(fname, "r");
	if (fp == NULL)
		return;

	fclose(fp);

	OnOpenDocument(fname);
}

void CClienttestDoc::SaveFile(const char *fname)
{
	OnSaveDocument(fname);
}


void CClienttestDoc::OnAddTagList()
{
	// Load file 

	CFileDialog dlg (TRUE, 
		".lst",   NULL,  OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST, NULL );

	if (dlg.DoModal() == IDOK) {

		CString path = dlg.GetPathName();
		FILE *fp = fopen( (LPCSTR) path, "r");

		if (fp == NULL)
			return;

		char buffer [512];

		while (fgets(buffer, 512, fp)) {
		
			CString str (buffer);
			str.Trim();
			AddTag(str);
		}

		fclose(fp);
	}
}