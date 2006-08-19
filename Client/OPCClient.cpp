#include "StdAfx.h"

#include "Client.h"

#include "OPCDataCallback.h"
#include "OPCDataReceiver.h"

#include "OpcClient.h"

#include <sstream>
#include "ag_Clocker.h"

//**************************************************************************
std::string FormatQuality(WORD quality)
{
	switch (quality) {
			case QUAL_GOOD:
				return "GOOD";
			case QUAL_UNCERTAIN:
				return "UNCERTAIN";
			case QUAL_BAD:
				return "BAD";
			default:
				return "UNKNOWN";
	}
}

std::string FormatValueType(const VARIANT& v)
{
	std::string str;

	switch(v.vt) {
		case VT_BSTR: 
			str = _T("VT_BSTR");	
			break;
		case VT_I1: 
			str = _T("VT_I1"); 		
			break;
		case VT_I2: 
			str = _T("VT_I2"); 		
			break;
		case VT_I4: 
			str = _T("VT_I4"); 		
			break;
		case VT_I8: 
			str = _T("VT_I8"); 		
			break;
		case VT_R4: 
			str = _T("VT_R4"); 		
			break;
		case VT_R8: 
			str = _T("VT_R8"); 		
			break;
		case VT_UI1: 
			str = _T("VT_UI1"); 	
			break;
		case VT_UI2: 
			str = _T("VT_UI2"); 	
			break;
		case VT_UI4: 
			str = _T("VT_UI4"); 	
			break;
		case VT_UI8: 
			str = _T("VT_UI8"); 	
			break;
		default: 
			str = _T("unknown");		
			break;
	};
	return str;
}

std::string FormatValue(const VARIANT& v, WORD quality)
{
	CString number;

     switch( v.vt )
        {  // simple types
        case VT_BOOL:
				number = v.boolVal ? _T("On") : _T("Off");
           break;
		case VT_UI1:
		case VT_I1:
			{
				int i = v.bVal;
				number.Format( _T("%hd"), i);
				break;
			}
        case VT_I2:
		case VT_UI2:
           number.Format( _T("%hd"), v.iVal );
           break;
        case VT_I4:
		case VT_UI4:
           number.Format( _T("%d"), v.lVal );
           break;
		case VT_R4:
			{
				double x = v.fltVal;
				number.Format( _T("%g"), x);
				break;
			}
        case VT_R8:
           number.Format( _T("%g"), v.dblVal );
           break;
        case VT_BSTR:
           number = v.bstrVal;
           break;
        default:
           {
#ifdef REV_12
           // Arrays of simple types
           if( (v.vt & VT_ARRAY)==VT_ARRAY )
              {
              CString temp;
              SAFEARRAY* pArray = v.parray;
              LONG lBound = 0, uBound = 0;
              SafeArrayGetLBound( pArray, 1, &lBound );
              SafeArrayGetUBound( pArray, 1, &uBound );
              for( long element=lBound; element<=uBound; element++ )
                 {
                 if( !temp.IsEmpty() )
                    temp += _T(", ");
                 switch( v.vt & ~VT_ARRAY )
                    {
                    case VT_BOOL:
                       {
                       VARIANT_BOOL b;
	                    SafeArrayGetElement(pArray, &element, &b);
                       number = b ? _T("On") : _T("Off");
                       }
                       break;
					case VT_UI1:
						{
							char b;
		                    SafeArrayGetElement(pArray, &element, &b);
							int i = b;
							number.Format( _T("%hd"), i);
							break;
						}
                    case VT_I2:
                       {
                       short b;
	                    SafeArrayGetElement(pArray, &element, &b);
                       number.Format( _T("%hd"), b );
                       }
                       break;
                    case VT_I4:
                       {
                       long b;
	                    SafeArrayGetElement(pArray, &element, &b);
                       number.Format( _T("%d"), b );
                       }
                       break;
					case VT_R4:
						{
							FLOAT f;
		                    SafeArrayGetElement(pArray, &element, &f);
							double x = f;
							number.Format( _T("%g"), x);
							break;
						}
                    case VT_R8:
                       {
                       double d;
	                    SafeArrayGetElement(pArray, &element, &d);
                       number.Format( _T("%g"), d );
                       }
                       break;
                    case VT_BSTR:
                       {
                       BSTR b;
	                    SafeArrayGetElement(pArray, &element, &b);
                       number = b;
                       }
                       break;
                    }
                 temp += number;
                 }
              number = temp;
              }
           else
              number = _T("?");
#endif // REV_12
           }
        }
     if( (quality & QUAL_STATUS_MASK) != QUAL_GOOD )
        {
        if( (quality & QUAL_STATUS_MASK) == QUAL_UNCERTAIN )
           number += _T(" (uncertain)");
        else if( (quality & QUAL_STATUS_MASK) == QUAL_BAD )
           number = _T("Bad");
        }

	 return (LPCSTR)number;
}


OPCClient::OPCClient(void) : m_Shutdowner( &m_DataReceiver )
{
	m_Local = true;
	m_Cookie = 0;
	m_OPCDataCallback = NULL;
	m_GroupHandle = 0;

	m_DataReceiver = NULL;
	m_GroupName = "EA_OPC_Client";
	m_UpdateRate = 1000;
	m_UseAsync = false;

	m_flags = 0;
}

OPCClient::~OPCClient(void)
{
	if( isConnected() )
		Disconnect();
	CleanDaAbstractor();
}

void OPCClient::setDataReceiver( COPCReceiveData * rec )
{
	m_DataReceiver = rec;
}

bool OPCClient::isConnected()
{
	return ( m_Server != NULL && m_Group != NULL );
}


/// if name == NULL, using m_ProgID member 
/// if host == NULL, using m_Host member 
HRESULT OPCClient::Connect( LPCTSTR name, LPCTSTR host )
{
	HRESULT hr = S_OK;
	/// количество тегов реально добавленных в группу клиента 
	DWORD ItemsAdded = 0;

	if( name != NULL )
		m_ProgID = name;

	m_Local = ( host == NULL || string(host) == "");
	if( !m_Local )
		m_Host = host;

	if( m_DataReceiver == NULL )
		return E_FAIL;

	if( isConnected() ) {
		m_DataReceiver->log_message( LogError, "Повторная попытка соединения: клиент уже подсоединен" );
		return E_FAIL;
	}

	try {

		if( m_Server == NULL )   {
			if( m_Local ) 
			{				 // создаем локально
				hr = m_Server.CoCreateInstance( CT2W( m_ProgID.c_str() ) );

				if( m_Server == NULL || FAILED(hr) ) {
					_com_error err( hr);
					m_DataReceiver->log_message( LogError, "Ошибка создания объекта %s на машине %s. (%s)", 
						m_ProgID.c_str(), m_Host.c_str(), err.ErrorMessage() );
					return E_FAIL;
				}
			}
			else { // создаем удаленно

				if( m_Host == "DCOM" ) {
					hr = m_Server.CoCreateInstance( CT2W(m_ProgID.c_str()),NULL, CLSCTX_REMOTE_SERVER );
					if( FAILED(hr) ) {
						_com_error err( hr);
						m_DataReceiver->log_message( LogError, "Ошибка создания объекта %s на машине %s. (%s)", 
							m_ProgID.c_str(), m_Host.c_str(), err.ErrorMessage() );
						return E_FAIL;
					}
				}
				else {
					CLSID clsid = CLSID_NULL;
					hr = CLSIDFromProgID( CT2W(m_ProgID.c_str()), &clsid );
					if( FAILED(hr) || clsid == CLSID_NULL) {
						m_DataReceiver->log_message( LogError, "Не удалось получить CLSID из ProgID (%s)", m_ProgID.c_str() );
						return E_FAIL;
					}

					COSERVERINFO sinfo;
					ZeroMemory( &sinfo, sizeof(COSERVERINFO) );
					sinfo.pAuthInfo = NULL;
					sinfo.pwszName = _wcsdup( CT2W( m_Host.c_str() ) );

					MULTI_QI mqi[] = { {&IID_IOPCServer, NULL ,S_OK } };

					hr = CoCreateInstanceEx( clsid, NULL, CLSCTX_REMOTE_SERVER, &sinfo, 
						sizeof(mqi)/sizeof(mqi[0]), mqi );
					free( sinfo.pwszName );

					if( FAILED(hr) || FAILED(mqi[0].hr) ) {
						hr = FAILED(hr) ? hr : mqi[0].hr;
						m_DataReceiver->log_message( LogError, "Ошибка создания объекта %s на машине %s. (%s)", 
							m_ProgID.c_str(), m_Host.c_str(), _com_error(hr).ErrorMessage());
						return E_FAIL;
					}
					else 
						m_Server.p = (IOPCServer*)mqi[0].pItf;
				}
			}
			hr = m_Shutdowner.Advise( m_Server );
			if( FAILED(hr) ) {

				m_DataReceiver->log_message( LogError, "Ошибка подписки на IOPCShutdown. (%s)", _com_error(hr).ErrorMessage() ); 

				if (! (m_flags & OPC_CLIENT_IGNORE_SHUTDOWN)) {
					return E_FAIL;
				}
			}
		}

		/// дожидаемся когда сервер придет в рабочее состояние 
		CAG_Clocker clocker( NULL, false );
		int index = 0;
		OPCSERVERSTATE state = OPC_STATUS_TEST;
		hr = getServerState( state );
		if( state != OPC_STATUS_RUNNING && SUCCEEDED(hr) ) {
			m_DataReceiver->log_message( LogWarning, "Wait while server status not OPC_STATUS_RUNNING." );
		}
		while( state != OPC_STATUS_RUNNING && ++index < 4 ) {
			Sleep( 500 );
			hr = getServerState( state );
			if( FAILED(hr) ) return hr;
		} 
		if( state != OPC_STATUS_RUNNING ) {
			m_DataReceiver->log_message( LogWarning, "В течении %.2f секунд сервер не перешел в режим OPC_STATUS_RUNNING.", clocker.stop() );
			return E_FAIL;
		}

		// создаем группу на сервере
		if( m_Group == NULL )  
		{
			long time_bias = 0;
			float deadband = 0.;
			DWORD update_rate = 0;
createGroup:
			hr = m_Server->AddGroup( CT2W(m_GroupName.c_str()), TRUE, m_UpdateRate, 1, &time_bias, &deadband, GetSystemDefaultLCID(), 
				&m_GroupHandle, &update_rate, IID_IOPCGroupStateMgt, (LPUNKNOWN *) &m_Group);

			switch( hr ) {
			case OPC_E_DUPLICATENAME: 
				{
					stringstream ss; 
					ss << m_GroupName << "_" << rand();
					m_GroupName = ss.str(); 
				}
				m_DataReceiver->log_message( LogError, "На OPC сервере уже существует группа с именем %s. Возможна ошибка конфигурации.", m_GroupName.c_str() );
				goto createGroup;
				//				return E_FAIL;
			case E_NOINTERFACE:
				m_DataReceiver->log_message( LogError, "IOPCServer::AddGroup return E_NOINTERFACE (IID_IOPCGroupStateMgt)" );
				return E_FAIL;
			}	

			if( FAILED(hr) || m_Group == NULL) {
				m_DataReceiver->log_message( LogError, "1003: Не удалось создать группу на сервере", m_GroupName.c_str() );
				return E_FAIL;
			}
			if( update_rate != m_UpdateRate )
				m_DataReceiver->log_message( LogWarning, "1004: OPC сервер отверг предложенный период обновления данных. Новый период: %d ms.", update_rate );

			// Подсоединяемся к событиям группы 
			CComPtr<IConnectionPointContainer> cpoint;
			hr = m_Group->QueryInterface( &cpoint );
			if( FAILED(hr) || cpoint == NULL) { 
				_com_error err( hr );
				m_DataReceiver->log_message( LogError, "1008: Не удалось получить от объекта OPC группы интерфейс IConnectionPointContainer (%s).", err.ErrorMessage() ); 
				return E_FAIL; 
			}

			hr = cpoint->FindConnectionPoint( IID_IOPCDataCallback, &m_ConnectionPoint );
			if( FAILED(hr) || m_ConnectionPoint == NULL) { 
				_com_error err( hr );
				m_DataReceiver->log_message( LogError, "1009: У группы OPC отсутствует IOPCDataCallback. Возможно OPC сервер не соответствует стандарту OPC DA 2.0 (%s).", err.ErrorMessage() ); 
				return E_FAIL; 
			}

			if( m_OPCDataCallback == NULL ) {
				m_OPCDataCallback = new COPCDataCallback;
				m_OPCDataCallback->AddRef();
				m_OPCDataCallback->receiver = m_DataReceiver;
			}

			hr = m_ConnectionPoint->Advise(m_OPCDataCallback, &m_Cookie);
			if( FAILED(hr) ) 
			{ 
				delete m_OPCDataCallback;
				m_OPCDataCallback = NULL;

				_com_error err( hr );
				m_DataReceiver->log_message( LogError, "1010: Advise к группе OPC сервера не прошел. (%s)", err.ErrorMessage() );  
				return E_FAIL; 
			}
		}

		if( ItemsCount() > 1 )
			hr = AddParamsToGroup( ItemsAdded );
		else 
			hr = S_OK;
	} 
	catch (_com_error e) {
		m_DataReceiver->log_message( LogError, "EXCEPTION: CXfrProxyOPC::CreateServerObject (%s)", (LPCTSTR)e.ErrorMessage() );
		return E_FAIL;
	}
	catch (CAtlException e) {
		m_DataReceiver->log_message( LogError, "EXCEPTION: CXfrProxyOPC::CreateServerObject (HR = %d)", e.m_hr );
		return E_FAIL;
	}
	catch(...) {
		m_DataReceiver->log_message( LogError, "EXCEPTION: CXfrProxyOPC::CreateServerObject" );
		return E_FAIL;
	}

	m_DataReceiver->log_message( LogDebugInfo, "1020: Добавлено в группу сервера %d из %d параметров.", ItemsAdded, ItemsCount() );

	if( SUCCEEDED(hr) )
		m_DataReceiver->StatusChanged( 1, NULL );

	return hr;
}

void OPCClient::Disconnect()
{
	if( !isConnected() ) {

		if (m_DataReceiver != NULL ) {
			m_DataReceiver->log_message( LogError, "Попытка повторного отсоединения." );
		}
		return;
	}

	m_Shutdowner.Unadvise();

	/// подчистка всех объектов
	if( m_ConnectionPoint ) {
		try {
			m_ConnectionPoint->Unadvise(m_Cookie);
			m_ConnectionPoint.Release();
		} catch(...) {}
		m_Cookie = 0;
		m_ConnectionPoint.p = NULL;
	}

	if( m_OPCDataCallback ) {
		try {
			m_OPCDataCallback->Release();
		} catch(...) {}
		m_OPCDataCallback = NULL;
	}

	if( m_Group ) {
		try {
			m_Group.Release();
		} catch(...) {}
		m_Group.p = NULL;
	}

	if( m_itemMgt ) {
		try {
			m_itemMgt.Release();
		} catch(...) {}
		m_itemMgt.p = NULL;
	}

	if(m_Server) {
		try {
			m_Server->RemoveGroup( m_GroupHandle, TRUE );
			m_Server.Release();
		} catch(...) {}
		m_Server.p = NULL;
	}
}

/*! добавляет в группу параметр, если клиент не подключен, то добавляет параметр в сиписок на подключение
возвращает клиентский хендл 
*/
OPCHANDLE OPCClient::AddTag( OPCHANDLE clientHandle, LPCTSTR tag_name, VARTYPE type )
{
	// Add a new item
	AG_OpcDA::Item* it = new AG_OpcDA::Item;
	it->quality = OPC_QUALITY_GOOD;
	it->name = tag_name;
	it->type = type;
	it->hServerHandle = NULL;
	it->hClientHandle = clientHandle;

	OPCHANDLE clientID = (OPCHANDLE)addNewItem( it );

	/// если подсоединены к серверу - добавляем тег на ходу 
	if( isConnected() ) {
		HRESULT hr = AddParamToGroup( it );

		if (FAILED (hr) ) {
			throw opcError ("Can't add parameter %s to group", tag_name);
		}
	}

	return clientID;
}

	/// удалить из группы указанный параметр
void OPCClient::RemoveTag( OPCHANDLE clientHandle )
{
	
}


/// чтение значения параметра 
bool OPCClient::ReadValue( DWORD clientID, FILETIME &time, VARIANT &value, WORD &Quality )
{
	CComPtr<IOPCSyncIO> SyncIO;
	if( m_Group == NULL ) return false;

	m_Group.QueryInterface( &SyncIO );
	if( SyncIO == NULL ) return false;


	HRESULT      *pErrors    = NULL;
	OPCITEMSTATE* pItemState = NULL;

	OPCHANDLE servH = getItemByClientHandle(clientID)->hServerHandle;

	// проверка правильности записанного значения
	HRESULT hr = SyncIO->Read( OPC_DS_CACHE, 1, &servH, &pItemState, &pErrors );
	if( FAILED(hr) || ( pErrors != NULL && FAILED(*pErrors) ) || pItemState == NULL ) {
		if( FAILED(*pErrors) ) 
			ReportError( *pErrors, "OPCClient::ReadValue" );

		if( pErrors )
			CoTaskMemFree( pErrors );

		if( pItemState )
			CoTaskMemFree( pItemState );

		return false;
	}

	if( pErrors )
		CoTaskMemFree( pErrors );

	if( pItemState ) {
		hr = VariantCopy( &value, &pItemState->vDataValue );
		if( FAILED (hr) ) {
			AtlTrace ("OPCClient::ReadValue: VariantCopy failed !\n");
		}
		Quality = pItemState->wQuality;
		time = pItemState->ftTimeStamp;
		CoTaskMemFree( pItemState );
		return true;
	}

	return false;
}


void OPCClient::fillOPCITEMDEF( AG_OpcDA::Item* item, OPCITEMDEF *idef )
{
	ATLASSERT( item );
	CString str = "";;

	idef->szItemID = item->name.AllocSysString();
	idef->dwBlobSize = 0;
	idef->pBlob = NULL;
	idef->bActive = TRUE;
	idef->hClient = item->hClientHandle; //(OPCHANDLE)it;  // pointer to item is its "handle"
	idef->szAccessPath = str.AllocSysString(); // it->name.AllocSysString();
	idef->vtRequestedDataType = VT_EMPTY; //item->getComType();
}

void OPCClient::ReportError( HRESULT hr ,LPCTSTR name)
{
	CString msg;

	switch( hr ) 
	{
	case S_OK:
		return;
	case E_INVALIDARG:
		msg = "An argument to the function was invalid. ( e.g dwCount=0).";
		break;
	case E_FAIL:
		msg.Format( _T("%s - E_FAIL:The operation failed for this item"), name);
		break;
	case OPC_E_BADRIGHTS:
		msg.Format( _T("%s - OPC_E_BADRIGHTS:The item is not readable"), name);
		break;
	case OPC_E_INVALIDHANDLE:
		msg.Format( _T("%s - OPC_E_INVALIDHANDLE:The passed item handle was invalid."), name);
		break;
	case OPC_E_INVALIDITEMID:
		msg.Format( _T("%s - OPC_E_INVALIDITEMID:The ItemID is not syntactically valid."), name);
		break;
	case OPC_E_UNKNOWNITEMID:
		msg.Format( _T("%s - OPC_E_UNKNOWNITEMID:The ItemID is not in the server address space."), name);
		break;
	case OPC_E_BADTYPE:
		msg.Format( _T("%s - OPC_E_BADTYPE:The requested data type cannot be returned for this item."), name);
		break;
	case OPC_E_UNKNOWNPATH:
		msg.Format( _T("%s - OPC_E_UNKNOWNPATH:The item's access path is not known to the server."), name);
		break;
	default:
		msg.Format( _T("%s - (%s) The function was unsuccessful."), name, _com_error(hr).ErrorMessage() );
		break;
	}
	m_DataReceiver->log_message( LogError,  msg );
}

void clearOPCITEMDEF( OPCITEMDEF *idef, int count = 1 )
{
	if( idef )
		for(int i=0;i<count;i++) 
		{
			if( idef[i].szItemID != NULL ) 
				SysFreeString( idef[i].szItemID );
			if( idef[i].szAccessPath != NULL )
				SysFreeString( idef[i].szAccessPath );
		}
}

HRESULT OPCClient::AddParamToGroup( AG_OpcDA::Item* item/*, OPCHANDLE clientID*/ )
{
	HRESULT hr = S_OK;

	if( item == NULL ) return E_POINTER;

	// add the item
	OPCITEMRESULT * pResults = NULL;
	HRESULT *pErrors = NULL;
	OPCITEMDEF idef;

	fillOPCITEMDEF( item, &idef );
	/// заполняем клиентский хендл в структуру 
	//	idef.hClient = clientID;

	if( m_itemMgt == NULL && m_Group == NULL) return E_FAIL;
	if( m_itemMgt == NULL ) 
		m_Group.QueryInterface( &m_itemMgt );
	ATLASSERT( m_itemMgt );

	hr = m_itemMgt->AddItems(1, &idef, &pResults, &pErrors);
	if( SUCCEEDED( hr ) && SUCCEEDED(pErrors[0]) )
		item->type = pResults->vtCanonicalDataType;

	clearOPCITEMDEF( &idef );

	if( FAILED( hr )) {
		m_DataReceiver->log_message( LogError, "Ошибка подключения параметра %s.", (LPCTSTR) item->name );
		//		freeItem( item_index );
		item->hServerHandle = 0;
		return E_FAIL;
	}
	item->hServerHandle = pResults->hServer;  // save the server handle

	if( pErrors )
		if( FAILED(*pErrors) ) {
			ReportError( *pErrors, item->name );//ReportAddingError
			//freeItem( item_index );
			//delete item;
			//item = NULL;
		}

		if( pResults )
			CoTaskMemFree( pResults );
		if( pErrors )
			CoTaskMemFree( pErrors );

		return hr;
}


HRESULT OPCClient::AddParamsTogetherToGroup( IOPCItemMgt *itemMgt, DWORD &m_ItemsAdded )
{
	HRESULT hr = S_OK;
	//DWORD added = 0;
	//int i;

	try {

		OPCITEMDEF *idef = (OPCITEMDEF *)CoTaskMemAlloc( ItemsCount() * sizeof(OPCITEMDEF));

		{
			critical_section lock( m_Sect );
			OPCITEMDEF *def = idef;

			for( size_t index = 1; index < m_items.size(); ++index )
			{
				fillOPCITEMDEF( m_items[index], def );
				//def->hClient = (OPCHANDLE)index;
				++def;
			}
		}

		//{
		//	XFRImportItemMap::iterator it;
		//	OPCITEMDEF *def = idef;
		//	for( it = m_Subscribe.begin(); it != m_Subscribe.end(); it++) 
		//	{
		//		Item* item = fillOPCITEMDEF( def, (*it).second );
		//		def->hClient = addNewItem( item );
		//	}
		//}

		// add the item
		OPCITEMRESULT * pResults = NULL;
		HRESULT *pErrors = NULL;

		hr = itemMgt->AddItems( ItemsCount(), idef, &pResults, &pErrors);

		if( FAILED( hr )) {
			m_DataReceiver->log_message( LogError, "Ошибка подключения группы параметров. Возможно OPC сервер не поддерживает такой режим." );
			ReportError( hr, NULL ); //ReportAddingError
		}

		//	CItemList::iterator it;
		if( pResults != NULL && pErrors != NULL ) {
			critical_section lock( m_Sect );

			size_t count = ItemsCount();

			for( size_t i = 0; i < count; ++i )
			{
				AG_OpcDA::Item *it = getItemByClientHandle( i+1 );
				if( FAILED(pErrors[i]) )
					ReportError( pErrors[i], it->name ); //ReportAddingError
				else {
					it->hServerHandle = pResults[i].hServer;  // save the server handle
					it->type = pResults[i].vtCanonicalDataType;
					m_ItemsAdded++;
				}

			}
		}


		//for(i=0,it = m_items.begin(); it != m_items.end(); it++, i++) 
		//{
		//	if( FAILED(pErrors[i]) )
		//		ReportError( pErrors[i], (*it)->name ); //ReportAddingError
		//	else {
		//		(*it)->hServerHandle = pResults[i].hServer;  // save the server handle
		//		(*it)->type = pResults[i].vtCanonicalDataType;
		//		m_items.m_Added++;
		//	}
		//}

		if( pResults )	CoTaskMemFree( pResults );
		if( pErrors )	CoTaskMemFree( pErrors );

		clearOPCITEMDEF( idef, ItemsCount() );
		CoTaskMemFree( idef );
	} 
	catch (_com_error e) {
		m_DataReceiver->log_message( LogError, _T("EXCEPTION: OPCClient::AddParamsTogetherToGroup (%s)"), (LPCTSTR)e.ErrorMessage() );
		return E_FAIL;
	}
	catch (CAtlException e) {
		m_DataReceiver->log_message( LogError, _T("EXCEPTION: OPCClient::AddParamsTogetherToGroup (HR = %d)"), e.m_hr );
		return E_FAIL;
	}
	catch(...) {
		m_DataReceiver->log_message( LogError, _T("EXCEPTION: OPCClient::AddParamsTogetherToGroup"), 0 );
		return E_FAIL;
	}
	return hr;
}

void OPCClient::Refresh()
{
	DWORD cancelID = 0;

	if( isConnected() ) {
		// Read its initial value
		CComQIPtr<IOPCAsyncIO2> opcAsyncIO = m_Group;
		if( opcAsyncIO != NULL )
			opcAsyncIO->Refresh2( OPC_DS_CACHE, 1324, &cancelID );
	}
}


HRESULT OPCClient::AddParamsToGroup( DWORD &m_ItemsAdded )
{
	HRESULT hr = S_OK;

	m_ItemsAdded = 0;

	try {
		/// подписываемся на параметры от OPC сервера
		if( m_itemMgt == NULL ) {
			hr = m_Group->QueryInterface( & m_itemMgt );
			if( FAILED(hr) || m_itemMgt == NULL) { 
				_com_error err( hr );
				m_DataReceiver->log_message( LogError, "Не удалось получить интерфейс IOPCItemMgt. (%s)", err.ErrorMessage() );  
				return hr; 
			}
		}

		if( ItemsCount() <= 1 )
		{
			m_DataReceiver->log_message( LogError, _T("Подключение к ОРС серверу [%s] прошло, но список подключаемых параметров пуст."), m_ProgID.c_str() );
			return S_OK;
		}

		if( m_AddItemMode ) { // 1- подключение блоком
			hr = AddParamsTogetherToGroup( m_itemMgt, m_ItemsAdded );
			Refresh();
			return hr;
		}

		critical_section lock( m_Sect );

		for( size_t index = 1; index < m_items.size(); ++index )
		{
			if( SUCCEEDED( AddParamToGroup( m_items[index]/*, index*/ ) ) )
				++m_ItemsAdded;
		}

		Refresh();
	} 
	catch (_com_error e) {
		m_DataReceiver->log_message( LogError, _T("EXCEPTION: OPCClient::AddParamsToGroup (%s)"), (LPCTSTR)e.ErrorMessage() );
		return E_FAIL;
	}
	catch (CAtlException e) {
		m_DataReceiver->log_message( LogError, _T("EXCEPTION: OPCClient::AddParamsToGroup (HR = %d)"), e.m_hr );
		return E_FAIL;
	}
	catch(...) {
		m_DataReceiver->log_message( LogError, _T("EXCEPTION: OPCClient::AddParamsToGroup"), 0 );
		return E_FAIL;
	}
	return S_OK;
}

HRESULT OPCClient::getServerState( OPCSERVERSTATE &state )
{
	HRESULT hr = S_OK;
	OPCSERVERSTATUS *status = NULL;


	if( m_Server == NULL )  return E_FAIL;
	try {	
		hr = m_Server->GetStatus( &status );
	} catch( ... ) {
		return E_FAIL;
	}
	if( FAILED(hr) ) { 
		m_DataReceiver->log_message( LogError,"IOPCServer::GetStatus returned error: %s", _com_error(hr).ErrorMessage() );
		return hr;
	}
	if( status == NULL ) 
		return E_FAIL;
	else {
		state = status->dwServerState;
		//if( status->szVendorInfo ) {
		//	try { SysFreeString( status->szVendorInfo ); }
		//	catch(...) { free( status->szVendorInfo); }
		//}
		CoTaskMemFree( status->szVendorInfo );
		CoTaskMemFree( status );
		return S_OK;
	}
}

bool OPCClient::WriteValue( DWORD clientID, FILETIME &time, VARIANT &value, WORD Quality )
{
	AG_OpcDA::Item * item = getItemByClientHandle( clientID );
	if( item == NULL )
		return false;

	item->quality = Quality;
	item->value = value;
	item->time = time;

	if( m_UseAsync )
		return SUCCEEDED( PutValueToOPC_Async( item ) );
	else
		return SUCCEEDED( PutValueToOPC_Sync( item ) );
}

bool OPCClient::WriteValue( LPCTSTR name, FILETIME &time, VARIANT &value, WORD Quality )
{
	AG_OpcDA::Item * item = _findItem(name);
	if( item == NULL )
		return false;

	item->quality = Quality;
	item->value = value;
	item->time = time;

	if( m_UseAsync )
		return SUCCEEDED( PutValueToOPC_Async( item ) );
	else
		return SUCCEEDED( PutValueToOPC_Sync( item ) );
}

/// записать значение параметра в сервер ASYNC 
HRESULT OPCClient::PutValueToOPC_Async( AG_OpcDA::Item * item )
{
	CString str;
	HRESULT hr = S_OK;	
	ATLASSERT( item );
	HRESULT      *pErrors    = NULL;
	CComPtr<IOPCAsyncIO2> AsyncIO;

	if( m_DataReceiver == NULL ) return E_FAIL;
	if( !isConnected() ) return E_FAIL;

	ATLASSERT( m_Group );
	m_Group->QueryInterface( &AsyncIO );

	if( AsyncIO == NULL ) {
		m_DataReceiver->log_message( LogError, "Не удалось получить интерфейс IOPCAsyncIO2" );
		return E_FAIL;
	}
	DWORD cancelID = 0;
	hr = AsyncIO->Write( 1, &item->hServerHandle, &item->value,rand(),&cancelID, &pErrors);
	if( FAILED(hr) /*|| FAILED(pErrors[0])*/) {
		m_DataReceiver->log_message( LogError, "Параметр [%s] не передан", (LPCTSTR)item->name);
		hr = E_FAIL;	
	}
	if( pErrors )
	{
		switch( pErrors[0] ) {
			case OPC_S_CLAMP: m_DataReceiver->log_message( LogError, "AsyncIO->Write(%s) -> [OPC_S_CLAMP] The value was accepted but was clamped.", (LPCTSTR)item->name  ); break;
			case OPC_E_RANGE: m_DataReceiver->log_message( LogError, "AsyncIO->Write(%s) -> [OPC_E_RANGE] The value was out of range.", (LPCTSTR)item->name  ); break;
			case OPC_E_BADTYPE: 
				str.Format( "AsyncIO->Write(%s) -> [OPC_E_BADTYPE] The passed data type (%d) cannot be accepted for this item.", (LPCTSTR)item->name, item->value.vt  ); 
				m_DataReceiver->log_message( LogError, str );
				break;
			case OPC_E_BADRIGHTS: m_DataReceiver->log_message( LogError, "AsyncIO->Write(%s) -> [OPC_E_BADRIGHTS] The item is not writeable.", (LPCTSTR)item->name  ); break;
			case OPC_E_INVALIDHANDLE: m_DataReceiver->log_message( LogError, "AsyncIO->Write(%s) -> [OPC_E_INVALIDHANDLE] The passed item handle was invalid.", (LPCTSTR)item->name  ); break;
			case OPC_E_UNKNOWNITEMID: m_DataReceiver->log_message( LogError, "AsyncIO->Write(%s) -> [OPC_E_UNKNOWNITEMID] The item is no longer available in the server address space.", (LPCTSTR)item->name  ); break;
		}
	}
	if( pErrors )
		CoTaskMemFree( pErrors );

	return hr;
}


/// записать значение параметра в сервер Sync 
HRESULT OPCClient::PutValueToOPC_Sync( AG_OpcDA::Item * item )
{
	HRESULT hr = S_OK;	
	ATLASSERT( item );
	HRESULT      *pErrors    = NULL;
	CComPtr<IOPCSyncIO> SyncIO;

	if( m_DataReceiver == NULL ) return E_FAIL;
	if( !isConnected() ) return E_FAIL;

	ATLASSERT( m_Group );
	m_Group->QueryInterface( &SyncIO );


	hr = SyncIO->Write( 1, &item->hServerHandle, &item->value, &pErrors);
	if( FAILED(hr) /*|| FAILED(pErrors[0])*/) {
		m_DataReceiver->log_message( LogError, "Параметр [%s] не передан", (LPCTSTR)item->name);
		hr = E_FAIL;	
	}
	if( pErrors )
	{
		switch( pErrors[0] ) {
			case OPC_S_CLAMP: m_DataReceiver->log_message( LogError, "SyncIO->Write(%s) -> [OPC_S_CLAMP] The value was accepted but was clamped.", (LPCTSTR)item->name  ); break;
			case OPC_E_RANGE: m_DataReceiver->log_message( LogError, "SyncIO->Write(%s) -> [OPC_E_RANGE] The value was out of range.", (LPCTSTR)item->name  ); break;
			case OPC_E_BADTYPE: m_DataReceiver->log_message( LogError, "SyncIO->Write(%s) -> [OPC_E_BADTYPE] The passed data type cannot be accepted for this item.", (LPCTSTR)item->name  ); break;
			case OPC_E_BADRIGHTS: m_DataReceiver->log_message( LogError, "SyncIO->Write(%s) -> [OPC_E_BADRIGHTS] The item is not writeable.", (LPCTSTR)item->name  ); break;
			case OPC_E_INVALIDHANDLE: m_DataReceiver->log_message( LogError, "SyncIO->Write(%s) -> [OPC_E_INVALIDHANDLE] The passed item handle was invalid.", (LPCTSTR)item->name  ); break;
			case OPC_E_UNKNOWNITEMID: m_DataReceiver->log_message( LogError, "SyncIO->Write(%s) -> [OPC_E_UNKNOWNITEMID] The item is no longer available in the server address space.", (LPCTSTR)item->name  ); break;
		}
	}

	if( pErrors )
		CoTaskMemFree( pErrors );

	return hr;
}

BrowseItems OPCClient::Browse()
{
	BrowseItems itms(this);

	itms.Browse("");

	////TODO
	return itms;
}
