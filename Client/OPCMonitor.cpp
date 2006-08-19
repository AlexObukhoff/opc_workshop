// OPCMonitor.cpp: implementation of the OPCMonitor class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable : 4786 4100 4512 )

#include "StdAfx.h"
#include "Client.h"

#include "opcerr.h"

#define IID_DEFINED
#include "OPCDA_i.c"


//FILETIME FILETIME_NULL = {0,0};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

OPCMonitor::OPCMonitor() :
			m_queue( m_UseSync )
{
	m_UseSync = FALSE;
	running = false;	
	m_UseSystemTimestamp = FALSE;
	m_ConnectionRefreshRate = m_Client.m_UpdateRate = 1000; // 1 sec.
}

OPCMonitor::~OPCMonitor()
{
	DoStopThreadTask();

	if( running )
		StopThread( 30000 );

}

void OPCMonitor::FireOPCEvent( COPCDataImportEvent &ev )
{
	vector<OPCEventConsumer*>::iterator it;

	for( it = consumers.begin(); it != consumers.end(); ++it )
	{
		try {
			if( *it != NULL )
				(*it)->ProcessEvent( ev );
		} catch(...)
		{
			throw opcError( "Исключение при вызове OPCEventConsumer::ProcessEvent()" );
		}
	}
}

bool OPCMonitor::DoPreThreadTask()
{	
	m_hShutdownEvent.Create(NULL,TRUE,FALSE,NULL);

	m_waiters.push_back( m_queue.WaitObj() );
	m_waiters.push_back( (HANDLE)m_hShutdownEvent );

	//m_OPCDataCallback = new COPCDataCallbackImpl(this, mtx);
	//m_OPCDataCallback->AddRef();

	return true;	
}

bool OPCMonitor::DoStopThreadTask()
{
	// Disconnect from OPC server
	
	OpcDisconnect();

//	m_hShutdownEvent.Close();
	m_waiters.erase( m_waiters.begin(), m_waiters.end() );

	//if( m_OPCDataCallback != NULL ) {
	//	m_OPCDataCallback->Release(); // shadow delete 
	//	m_OPCDataCallback = NULL;
	//}

	return true;
}

bool OPCMonitor::DoPostThreadTask()
{
	// Do connect to OPC server
	bool ret  = false;

	//if( m_OPCDataCallback == NULL ) {
	//	m_OPCDataCallback = new COPCDataCallbackImpl(this, mtx);
	//	m_OPCDataCallback->AddRef();
	//}

	//try {
		m_Client.m_UpdateRate = m_ConnectionRefreshRate;
		ret = SUCCEEDED( m_Client.Connect( m_ServerName, m_ServerHost ) );
		running = ret;
	//} catch (opcError &e) {
	//	throw opcError(ARMLogger::LogMessage(ArmStatusError, "Ошибка подсоединения к OPC серверу: '%s'. ",
	//		e.GetMessage());
	//	ret = false;
	//} catch (...) {
	//	ARMLogger::LogMessage(ArmStatusError, "Неизвестная ошибка подсоединения к OPC серверу.");
	//	ret = false;
	//}
	
	return ret;
}

bool OPCMonitor::OpcConnect(LPCTSTR server_name, LPCTSTR host )
{
	if( !running ) { // заплатка, что бы можно было создавать клиента без потока 
		DoPreThreadTask();
	}

//	assert( opcServer == NULL );
	//m_Client.m_ProgID = server_name;
	//m_Client.m_Host = host;

	return SUCCEEDED( m_Client.Connect( server_name, host ) );

	//// Get the class ID of that server.
	//HRESULT hr = CLSIDFromProgID( CT2W(server_name) , &clsid );
	//if( FAILED(hr))
	//	throw opcError( "Ошибка поиска OPC сервера : 0x%x. Сервер %s",  hr,  (LPCSTR)server_name ); 		 

	//return OpcConnect( clsid, host );
}

void OPCMonitor::OpcDisconnect() 
{
	if (m_Client.isConnected()) {
		m_Client.Disconnect();
	}
	running = false;
}

void OPCMonitor::AddOpcItem(std::string it, 
							std::string accessPath, 
							BOOL UseOpcTagId,
							std::string m_OpcTagId,
							VARTYPE vt, int index)
{
	if (running) {
		OPCParameterDesc *pdesc = new OPCParameterDesc(it.c_str(), 
			accessPath.c_str(), 
			UseOpcTagId,
			m_OpcTagId.c_str(),
			vt, index/*, ch*/);
		
		m_queue.push_back( pdesc );

		/// ждем пока гарантированно не подпишемся на данные 

		DWORD res = WaitForSingleObject( m_queue.WaitResult(), 3000 );

		switch (res) {
			case WAIT_ABANDONED:
				throw opcError("Can't add tag %s: wait ABANDONED", m_OpcTagId.c_str());
				break;
			case WAIT_OBJECT_0:
				// OK
				break;
			case WAIT_TIMEOUT:
				throw opcError("Can't add tag %s: wait TIMEOUT", m_OpcTagId.c_str());
				break;
		}

	}
}

//
//COPCDataCallbackImpl::COPCDataCallbackImpl(OPCMonitor *_monitor, mutex_t &_mtx) //NN: callback_mtx(_mtx)
//{
//	m_cRef=0;
//	monitor = _monitor;
//    return;
//}
//
//COPCDataCallbackImpl::~COPCDataCallbackImpl(void)
//{
//    return;
//}
//
//
//STDMETHODIMP COPCDataCallbackImpl::QueryInterface(REFIID riid, void** ppv)
//{
//    *ppv=NULL;
//	
//    if (IID_IUnknown==riid || IID_IOPCDataCallback==riid)
//        *ppv=this;
//	
//    if (NULL!=*ppv)
//	{
//        ((LPUNKNOWN)*ppv)->AddRef();
//        return NOERROR;
//	}
//	
//    return ResultFromScode(E_NOINTERFACE);
//}
//
//
//STDMETHODIMP_(ULONG) COPCDataCallbackImpl::AddRef(void)
//{
//    return ++m_cRef;
//}
//
//
//STDMETHODIMP_(ULONG) COPCDataCallbackImpl::Release(void)
//{
//    if (0!=--m_cRef)
//        return m_cRef;
//	
//    delete this;
//    return 0;
//}
//
//HRESULT COPCDataCallbackImpl::OnDataReceive( /* [in] */ DWORD dwCount,
//    /* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
//    /* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
//    /* [size_is][in] */ WORD __RPC_FAR *pwQualities,
//    /* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
//    /* [size_is][in] */ HRESULT __RPC_FAR *pErrors)
//{
//	critical_section cs(callback_mtx);
//
//	for (DWORD i = 0; i < dwCount; i++)
//	{
//		int index = phClientItems[i];
//
//		if ( !monitor->IsExist(index) ) {
//			// Item deleted
//			//ARMLogger::LogMessage(ArmStatusTrace, "Параметр не ожидается: id=%d", index);
//			continue;
//		}
//		
//		COPCDataImportEvent ev;
//
//		ev.param_id = index;
//		ev.quality = pwQualities[i];
//		ev.Value(pvValues[i]);
//
//		COleDateTime dt(pftTimeStamps[i]);
//
//		if (monitor->m_UseSystemTimestamp) {
//			ev.stamp = COleDateTime::GetCurrentTime();
//		} else if (pftTimeStamps[i].dwHighDateTime == 0 && pftTimeStamps[i].dwHighDateTime == 0) 
//		{
//			// Invalid date/time - log it and catch current system
//			// ARMLogger::LogMessage(ArmStatusWarning, "Нулевое время параметра с OPC сервера - значение отбраковано");
//			continue;
//		} else {
//			ev.stamp = pftTimeStamps[i];
//		}
//
//		monitor->FireOPCEvent(ev);
//	}
//
//	return S_OK;
//}
//
//STDMETHODIMP COPCDataCallbackImpl::OnDataChange( 
//    /* [in] */ DWORD dwTransid,
//    /* [in] */ OPCHANDLE hGroup,
//    /* [in] */ HRESULT hrMasterquality,
//    /* [in] */ HRESULT hrMastererror,
//    /* [in] */ DWORD dwCount,
//    /* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
//    /* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
//    /* [size_is][in] */ WORD __RPC_FAR *pwQualities,
//    /* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
//    /* [size_is][in] */ HRESULT __RPC_FAR *pErrors)
//{
//	return OnDataReceive( dwCount, phClientItems, pvValues, pwQualities, pftTimeStamps, pErrors );
//}
//
//STDMETHODIMP COPCDataCallbackImpl::OnReadComplete( 
//    /* [in] */ DWORD dwTransid,
//    /* [in] */ OPCHANDLE hGroup,
//    /* [in] */ HRESULT hrMasterquality,
//    /* [in] */ HRESULT hrMastererror,
//    /* [in] */ DWORD dwCount,
//    /* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
//    /* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
//    /* [size_is][in] */ WORD __RPC_FAR *pwQualities,
//    /* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
//    /* [size_is][in] */ HRESULT __RPC_FAR *pErrors)
//{
//	return OnDataReceive( dwCount, phClientItems, pvValues, pwQualities, pftTimeStamps, pErrors );
//}
//
//STDMETHODIMP COPCDataCallbackImpl::OnWriteComplete( 
//    /* [in] */ DWORD dwTransid,
//    /* [in] */ OPCHANDLE hGroup,
//    /* [in] */ HRESULT hrMastererr,
//    /* [in] */ DWORD dwCount,
//    /* [size_is][in] */ OPCHANDLE __RPC_FAR *pClienthandles,
//    /* [size_is][in] */ HRESULT __RPC_FAR *pErrors)
//{
//	return S_OK;
//}
//
//STDMETHODIMP COPCDataCallbackImpl::OnCancelComplete( 
//    /* [in] */ DWORD dwTransid,
//    /* [in] */ OPCHANDLE hGroup)
//{
//	return S_OK;
//}

void OPCMonitor::OnWorkerEvent(int event_no)
{
	switch (event_no) {
	case 0:
		ProcessAddParameter();
		break;
	case 1: /// Отсоединение от сервера по сигналу IOPCShutdown 
		OpcDisconnect();		
		break;
	}

}

void OPCMonitor::ProcessAddParameter(void)
{
OPCParameterDesc *pdesc = NULL;
vector<OPCParameterDesc *> pdesc_vector;

	// OPCMonitor already locked by ArmImportConfigurator::LoadConfiguration
	//	critical_section cs(opc_mon_mtx);

	while ( m_queue.Size())
	{
		pdesc = m_queue.pop();
		pdesc_vector.push_back( pdesc );
	}

	AddOpcParameter(pdesc_vector);

	for( size_t i =0; i < pdesc_vector.size(); ++i )
		delete pdesc_vector[i];

	m_queue.ResultDone();
}

void OPCMonitor::AddOpcParameter(vector<OPCParameterDesc*> &pdesc_vector)
{
	vector<OPCParameterDesc*>::iterator it;

	for (it = pdesc_vector.begin(); it != pdesc_vector.end(); ++it) {
		try {
			OPCParameterDesc	*pdesc = *it;
			m_Client.AddTag(pdesc->index, 
				pdesc->UseOpcTagId ? pdesc->m_OpcTagId : pdesc->it,
				pdesc->vt);

			in_params.insert(make_pair( pdesc->index, *pdesc)); // Copy c-tor

		} catch (opcError &) {
		}
	}
}

string OPCMonitor::GetParameterName(int index)
{
	critical_section cs(opc_mon_mtx);
	
	map <int, OPCParameterDesc>::const_iterator it = in_params.find(index);

	if (it == in_params.end()) {
		return "";
	} else {
		return (LPCTSTR)(it->second).it;
	}
}

//int OPCMonitor::LoadParameters(void)
//{
//	int count = 0;
//	try {
//		CARMDatabase *db = CARMDatabaseManager::Instance()->GetActiveDatabase();
//
//		CdboImportData import_rs(db);
//
//		try {
//			import_rs.Open("select ParamId, ParamName, OPCTagId, ParamSource from ImportDataDescription where ParamSource = 0");
//		} catch (opcError &e) {
//			throw opcError(e.status, e.module, 
//				"Невозможно получить список OPC параметров :%s",
//				e.GetMessage());
//		}
//		
//		HRESULT hr = S_OK;
//		if (import_rs.MoveFirst() == S_OK) {
//			// Procees with table data
//			
//			while (hr == S_OK) {
//				// Get Channel for this parameter (from meaner, which should be already started at this moment)
//				vector <CDataMeanChannel *> channels = theApp.meaner->GetChannels(import_rs.m_Id);
//				vector <CDataMeanChannel *>::iterator it;
//
//				if (channels.size() == 0) {
//					ARMLogger::LogMessage(
//						ArmStatusError, "Нет каналов осреднения для параметра %d",
//						import_rs.m_Id);
//				}	
//				for (it = channels.begin(); it != channels.end(); ++it) {
//					switch (import_rs.m_ParamSource) {
//					case 0:
//						AddOpcItem(import_rs.m_Name, "", 
//							import_rs.statusTagId != DBSTATUS_S_ISNULL,
//							import_rs.m_OPCTagId,
//							VT_R4, 
//							import_rs.m_Id, 
//							*it);
//
//						count ++;
//						break;
//					default:
//						ARMLogger::LogMessage(ArmStatusWarning, 
//							"Неизвестный источник (%d) для параметра %d(%s)",
//							import_rs.m_ParamSource, import_rs.m_Id, (LPCSTR)import_rs.m_Name);
//					} 
//				}
//
//				InParameter data;
//				data.id = import_rs.m_Id;
//				data.name = import_rs.m_Name;
//				data.source = (InputItemSource) import_rs.m_ParamSource;
//
//				theApp.SetInputParameterDescr(data);
//
//				hr = import_rs.MoveNext();
//			}
//		} else {
//			ARMLogger::LogMessage(ArmStatusError, "Нет входных данных для OPC");
//		}
//		import_rs.Close();
//	} catch (_com_error &e) {
//		::MessageBox(NULL, e.ErrorMessage(), "Ошибка обращения к БД", MB_OK);
//	}
//
//	return count;
//}

void OPCMonitor::ApplyParameter(LONG ParamId, const char *ParamName, bool UseOpcTag, const char *OpcTagId)
{
	map<int, OPCParameterDesc>::iterator it = in_params.find(ParamId);

	if (it != in_params.end()) {
		if (it->second.it == ParamName) {
			// Nothing has changed - return
			return;
		} else {
			// Delete old and insert new parameter
			DeleteParameter(ParamId);
			InsertParameter(ParamId, ParamName, UseOpcTag, OpcTagId);
		}
	} else {
		// Insert new parameter

		InsertParameter(ParamId, ParamName, UseOpcTag, OpcTagId);
	}
}

void OPCMonitor::InsertParameter(LONG ParamId, const char *ParamName, bool UseOpcTag, const char *OpcTagId)
{
	AddOpcItem(ParamName, "", UseOpcTag, OpcTagId, VT_R4, ParamId/*, NULL*/);

	//// Get Channel for this parameter (from meaner, which should be already started at this moment)
	//vector <CDataMeanChannel *> channels = theApp.meaner->GetChannels(ParamId);
	//vector <CDataMeanChannel *>::iterator it;

	//for (it = channels.begin(); it != channels.end(); ++it) {
	//	AddOpcItem(ParamName, "", 
	//		UseOpcTag, OpcTagId, VT_R4, ParamId, *it);
	//}
}

void OPCMonitor::DeleteParameter(LONG ParamId)
{
	map<int, OPCParameterDesc>::iterator it = in_params.find(ParamId);

	if (it == in_params.end()) {
		throw opcError( "Internal Error %s:%d", __FILE__, __LINE__ );
	}

	// Delete this item from mapping, but not from the subscription

	in_params.erase(it);
}

bool OPCMonitor::IsExist(int itemid)
{
	map<int, OPCParameterDesc>::iterator it = in_params.find(itemid);
	if (it == in_params.end()) {
		return false;
	}

	return true;
}

string OPCMonitor::DumpStatus()
{
	stringstream ss;
//	critical_section cs(mtx);

	ss << "\tПоток OPC монитор. \n";

	ss << COPCWorkerThread::DumpStatus();

	ss << "\tВходные параметры (in_params):\n";
	map<int, OPCParameterDesc>::iterator m;
	for (m = in_params.begin(); m != in_params.end(); ++m) {
		ss << "\t\t'" << m->first << "' ==> " << (LPCTSTR)m->second.it << endl;
	}

	ss << "Queue size = " << m_queue.Size() << endl;
	return ss.str();
}

/*!
	Заносит значение параметра в очередь данных, для записи в ОРС сервер
	Возвращает true если все успешно.
	Возможно исключение opcError 
*/
bool OPCMonitor::WriteToOPCQueue( LONG ParamId, double val, FILETIME ft, DWORD opc_quality  )
{
	if( in_params.find( ParamId ) == in_params.end() )
		throw opcError( "WriteToOPCQueue - invalid paramId (%d)", ParamId );

	OPCValueForWrite p;

	p.param_id = ParamId;
	p.value = val;
	ft; // NOT USED - TODO
	opc_quality; // NOT USED - TODO

	OPCWriteQueue.push_back( p );
	return true;
}

bool OPCMonitor::WriteToOPCQueue( LONG ParamId, int val, FILETIME ft, DWORD opc_quality   )
{
	if( in_params.find( ParamId ) == in_params.end() )
		throw opcError( "WriteToOPCQueue - invalid paramId (%d)", ParamId );

	OPCValueForWrite p;

	p.param_id = ParamId;
	p.value = val;
	ft; // NOT USED - TODO
	opc_quality; // NOT USED - TODO

	OPCWriteQueue.push_back( p );
	return true;
}

/*!
	Записывает всю очередь данных, одним вызовом в ОРС сервер
*/
int OPCMonitor::FlushOPCQueue()
{
int count_writing = 0;

	//TODO 

	//if( opcGroup == NULL )
	//	throw opcError( "FlushOPCQueue() - нет подключения к ОРС серверу." );

	//CComPtr<IOPCSyncIO> sync;
	//opcGroup.QueryInterface( &sync );

	//if( sync == NULL )
	//	throw opcError( "FlushOPCQueue() - ОРС сервером Не поддерживается IOPCSyncIO." );

	//while( OPCWriteQueue.Size() )
	//{
	//	OPCValueForWrite val( OPCWriteQueue.pop() );
	//	map <int, OPCParameterDesc>::iterator it = in_params.find( val.param_id );
	//	if( it == in_params.end() )
	//		throw opcError( "FlushOPCQueue - invalid paramId (%d)", val.param_id  );

	//	OPCParameterDesc &param( it->second );

	//	HRESULT *pErrors = NULL;
	//	HRESULT hr = sync->Write( 1, &param.ServerHandle, &val.value, &pErrors );

	//	if( FAILED(hr) || ( pErrors != NULL && FAILED(pErrors[0]) ) )
	//		opcError( "Ошибка записи OPC параметра (%s).", (LPCTSTR)param.it );
	//	else
	//		++count_writing;

	//	if( pErrors != NULL )
	//		CoTaskMemFree( pErrors );

	//	OPCWriteQueue.ResultDone();
	//}

	return count_writing;
}

bool OPCMonitor::IsConnected()
{
	return m_Client.isConnected(); 
}

// получаем текст ошибки из _com_error
//  Если там есть IErrorInfo, это будет действительный текст ошибки, иначе - 
//  полученный из HR
string GetComErrorMessage(_com_error &e)
{
	_bstr_t txt = e.Description();

	string ret = (const char *)txt;

	if (ret == "") {
		ret = e.ErrorMessage();
	}

	return ret;
}

string GetComErrorMessage(HRESULT hr )
{
	_com_error e(hr);
	_bstr_t txt = e.Description();

	string ret = (const char *)txt;

	if (ret == "") {
		ret = e.ErrorMessage();
	}

	return ret;
}
