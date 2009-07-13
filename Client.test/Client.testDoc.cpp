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

mutex_t mtx;
OPCClient m_OPCClient;

DWORD WINAPI LoadTestFn(void *arg)
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED );

	CClienttestDoc *pDoc = (CClienttestDoc *)arg;
	pDoc->DoLoadTest();

	return 1;
}

void WriteThread(void *arg)
{
//	CoInitializeEx(NULL, COINIT_MULTITHREADED );

	vector<tagDsc> *m_TagList = (vector<tagDsc> *)arg;

	vector<tagDsc>::iterator it; 

	for (it = m_TagList->begin(); it != m_TagList->end(); ++it) {
		it->active = true;
	}

	for(;;) {
		{
			// 1 step - compose array(s) 

			critical_section cs(mtx);
			for (it = m_TagList->begin(); it != m_TagList->end(); ++it)
			{
				tagDsc &tag = *it;

				if (! tag.active) {
					continue;
				}

				FILETIME now_time;
				CoFileTimeNow(&now_time);

				CComVariant ret; ret.Clear();
				ret.vt = tag.type;

				switch(ret.vt) {
		case VT_I1:  
			ret.bVal = (BYTE)(tag.counter %2);	
			break;
		case VT_I2:  
			ret.iVal = (short)(tag.counter %2);	
			break;
		case VT_I4:  
			ret.lVal = (long)(tag.counter %2);	
			break;
		case VT_I8:  
			ret.llVal = (LONGLONG)(tag.counter %2);	
			break;
		case VT_R4:  
			ret.fltVal = (float)((tag.counter %16) + 4);	
			break;
		case VT_R8:  
			ret.dblVal = (double)((tag.counter %16) + 4);	
			break;
		case VT_UI1: 
			ret.bVal = (BYTE)(tag.counter %2);	
			break;
		case VT_UI2: 
			ret.uiVal = (USHORT)(tag.counter %2);	
			break;
		case VT_UI4: 
			ret.ulVal = (tag.counter %2);	
			break;
		case VT_UI8: 
			ret.ullVal = (tag.counter %2);	
			break;
		default:
			ret.vt = VT_EMPTY;
				}
				if (! m_OPCClient.WriteValue(tag.client_handle, now_time, ret, QUAL_GOOD)) {
					AfxMessageBox("Coudn't write value to server");
					return;
				}
				tag.counter++;

				FILETIME ret_time;
				CComVariant ret_value; ret_value.Clear();
				WORD ret_quality;

				if (! m_OPCClient.ReadValue(tag.client_handle, ret_time, ret_value, ret_quality)) {
					// reading error ;(

					AfxMessageBox("Coudn't read value from server", MB_OK | MB_ICONSTOP);
					return;
				}

				// compare return value 
				HRESULT hcmp = VarCmp(&ret, &ret_value, 0);
				if (hcmp != VARCMP_EQ) {
					CString msg; msg.Format("Tag %s Values are not equal !!! continue ?",
						(LPCSTR)it->tagName);

					int res = AfxMessageBox(msg, 
						MB_YESNO | MB_ICONSTOP);

					if (res == IDYES) {
						// remove tag from list 

						it->active = false;
					} else {
						return;
					}
				}

			}
		}
		Sleep(500);
	}
}

// IMPLEMENT_SERIAL(CLoad_test_config, CObject, 1)

CLoad_test_config::CLoad_test_config()
{
	use_read_op = true;
	use_write_op = true;
	timeout = 500;
}

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
	ON_UPDATE_COMMAND_UI(ID_LOAD_TEST, OnUpdateLoadTest)
	ON_COMMAND(ID_LOAD_TEST, OnLoadTest)
END_MESSAGE_MAP()


// CClienttestDoc construction/destruction

CClienttestDoc::CClienttestDoc()
{
	selection = NULL;
	m_LoadTestRunning = FALSE;

	hStopThread = CreateEvent(NULL, FALSE, FALSE, NULL);
	hThreadStarted  = CreateEvent(NULL, FALSE, FALSE, NULL);
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

		// Save load test config
//		ar << &m_LoadTestConfig;
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

		// Load load test config
//		ar >> &m_LoadTestConfig;
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

		vector<CString> ::iterator it;

		for (it = dlg.m_TagNames.begin(); it != dlg.m_TagNames.end(); ++it)
		{
			AddTag(*it);
		}
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
#if 0
	// KDB: start massive writing to all tag's - load test 

	try
	{
		WriteThread((void *) &m_TagList);
	}
	catch (opcError &ee)
	{
		AfxMessageBox(ee.GetMessage(), MB_OK | MB_ICONSTOP);
	}
#endif
//	_beginthread(WriteThread, 0, );

	// Get selected item and it's properties
	if (selection == NULL) {
		AfxMessageBox("Not selected tag for write value!");
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

void CClienttestDoc::OnLoadTest()
{
	if (m_LoadTestRunning) {
		// stop load test - set event and wait for the thread finished
		SetEvent(hStopThread);

		DWORD res = WaitForSingleObject(hThread, 1000);
		switch(res)
		{
		case WAIT_OBJECT_0:
			// ok
			m_LoadTestRunning = false;
			break;
		case WAIT_TIMEOUT:
			TerminateThread(hThread, 666);
			AfxMessageBox("Load testing thread was terminated");
			m_LoadTestRunning = false;
		    break;
		default:
		    break;
		}
	} else {
		// start load test - start the thread and wait for event it was started

		DWORD threadid;
		hThread = CreateThread(NULL, 0, LoadTestFn, this, 0, &threadid);
	//		(HANDLE) _beginthread(LoadTestFn, 0, this);

		DWORD res = WaitForSingleObject(hThreadStarted, 1000);

		switch(res)
		{
		case WAIT_OBJECT_0:
			// Ok
			m_LoadTestRunning = true;
			break;
		case WAIT_TIMEOUT:
			AfxMessageBox("Can't start load test - thread timeout");
		    break;
		default:
			AfxMessageBox("Can't start load test - unknown error");
		    break;
		}
	}
}

void CClienttestDoc::OnUpdateLoadTest(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_LoadTestRunning);
}

void CClienttestDoc::DoLoadTest()
{
	vector<tagDsc>::iterator it; 

	for (it = m_TagList.begin(); it != m_TagList.end(); ++it) {
		it->active = true;
	}

	int step_no = 0;
	POSITION pos = GetFirstViewPosition();
	CView* pView = GetNextView(pos);

	SetEvent(hThreadStarted);


	DWORD res = 0;
	bool bExit = false;
	for (;!bExit;)
	{
		int tag_count = 0;
		DWORD start_ticks = GetTickCount();
		{

			std::vector<OPCHANDLE> client_handles;
			std::vector<VARIANT>   values;

			critical_section cs(mtx);
			// Body
			for (it = m_TagList.begin(); it != m_TagList.end(); ++it)
			{
				tagDsc &tag = *it;

				if (! tag.active) {
					continue;
				}

				FILETIME now_time;
				CoFileTimeNow(&now_time);

				CComVariant ret; ret.Clear();
				ret.vt = tag.type;

				switch(ret.vt) {
		case VT_I1:  
			ret.bVal = (BYTE)(tag.counter %2);	
			break;
		case VT_I2:  
			ret.iVal = (short)(tag.counter %2);	
			break;
		case VT_I4:  
			ret.lVal = (long)(tag.counter %2);	
			break;
		case VT_I8:  
			ret.llVal = (LONGLONG)(tag.counter %2);	
			break;
		case VT_R4:  
			ret.fltVal = (float)((tag.counter %16) + 4);	
			break;
		case VT_R8:  
			ret.dblVal = (double)((tag.counter %16) + 4);	
			break;
		case VT_UI1: 
			ret.bVal = (BYTE)(tag.counter %2);	
			break;
		case VT_UI2: 
			ret.uiVal = (USHORT)(tag.counter %2);	
			break;
		case VT_UI4: 
			ret.ulVal = (tag.counter %2);	
			break;
		case VT_UI8: 
			ret.ullVal = (tag.counter %2);	
			break;
		default:
			ret.vt = VT_EMPTY;
				}
				client_handles.push_back(tag.client_handle);
				values.push_back(ret);

				//if (! m_OPCClient.WriteValue(tag.client_handle, now_time, ret, QUAL_GOOD)) {
				//	AfxMessageBox("Coudn't write value to server");
				//	return;
				//}
				tag.counter++;
#if 0
				FILETIME ret_time;
				CComVariant ret_value; ret_value.Clear();
				WORD ret_quality;

				if (! m_OPCClient.ReadValue(tag.client_handle, ret_time, ret_value, ret_quality)) {
					// error readed

					AfxMessageBox("Coudn't read value from server", MB_OK | MB_ICONSTOP);
					return;
				}

				// compare return value 
				HRESULT hcmp = VarCmp(&ret, &ret_value, 0);
				if (hcmp != VARCMP_EQ) {
					CString msg; msg.Format("Tag %s Values are not equal !!! continue ?",
						(LPCSTR)it->tagName);

					int res = AfxMessageBox(msg, 
						MB_YESNO | MB_ICONSTOP);

					if (res == IDYES) {
						// remove tag from list

						it->active = false;
					} else {
						return;
					}
				}
#endif

				tag_count ++;
			}
			if (! m_OPCClient.WriteValues(client_handles.size(), &client_handles[0], &values[0])) {
				AfxMessageBox("Coudn't write value to server");
				return;
			}

		}


		DWORD finish_ticks = GetTickCount();



		struct load_test_step_info *step_info = new load_test_step_info;
		step_info->step_no = ++step_no;
		step_info->tags_processed = tag_count;
		step_info->msec = finish_ticks - start_ticks;
		int timeout = m_LoadTestConfig.timeout - step_info->msec;
		if (timeout < 0)
			timeout = 0;

		theApp.m_pMainWnd->PostMessage(WM_LOAD_STEP_DONE, (WPARAM)step_info, 0);

		DWORD res = WaitForSingleObject(hStopThread, timeout);

		switch(res)
		{
		case WAIT_OBJECT_0:
			bExit = true;
			break;
		case WAIT_TIMEOUT:
			// Ok
		    break;
		default:
			// Error - TODO: report it
			AfxTrace("Thread error");
		    break;
		}
	} 
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
	_vsnprintf(str, 4096, fmt, args);
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
		// not my client
		return;
	}

	tagDsc &tag_descriptor = m_TagList[f->second];
	
	tag_descriptor.last_quality = Quality;
	tag_descriptor.last_time = time;
	tag_descriptor.last_value = value;
	tag_descriptor.type = value.vt;

	TRACE("new param: %s [%d]\n", name, clientID );

	try {
		// Send a message to our window

		POSITION pos = GetFirstViewPosition();
		CView* pFirstView = GetNextView( pos );
		
		pFirstView->SendMessage(WM_UPDATE_VIEW, f->second, NULL);
//		UpdateAllViews(NULL, f->second);
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

	// Subscribe for tag
	// Read tag value, to do it add item to client object at first
	static OPCHANDLE client_hdl = 200000;

	OPCHANDLE client_id = m_OPCClient.AddTag(client_hdl, tagName, VT_UNKNOWN);
	client_hdl++;

	tag_list_items[client_id] = itemno;
	tag_descriptor.client_handle = client_id;

	tag_descriptor.type = VT_UNKNOWN;

	m_TagList.push_back(tag_descriptor);

	// Read initial value
	FILETIME time;
	VARIANT value; VariantInit(&value);
	WORD	quality;
	if (!m_OPCClient.ReadValue(client_id, time, value, quality)) {
		AfxTrace("Can't read tag value for %s\n", (LPCSTR)tagName);

	} else {
		newData(NULL, client_id, time, value, quality);
		tag_descriptor.type = value.vt;
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
		msg.Format("Error connecting to ÎÐÑ server: %s", m_LastMessage.c_str());
		AfxMessageBox(msg, MB_ICONSTOP | MB_OK);

		// Try not factor existing IOPCShutdown on server

		m_OPCClient.SetFlags (m_OPCClient.GetFlags () | OPC_CLIENT_IGNORE_SHUTDOWN);

		hr = m_OPCClient.Connect( ServerName, NULL );
		msg.Format("Error connecting to ÎÐÑ server: %s (IOPCShutdown checking disabled)", 
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