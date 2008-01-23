#pragma once

#include "../include/opcda.h"
#include "ShutdownImpl.h"

#include "OPCDataCallback.h"
#include "../include/OPCDataReceiver.h"
#include "OpcDaAbstractor.h"
#include "Browser.h"

#include <AtlSync.h>
#include <string>

// Флаги, определяющие поведение клиента. 
//  OPC_CLIENT_IGNORE_SHUTDOWN	- разрешить подключение к серверу, у которого 
//   отсутствует интерфейс IOPCShutdown
#define OPC_CLIENT_IGNORE_SHUTDOWN	0x00000001  

class OPCClient : protected AG_OpcDA::COpcDaAbstractor
{
protected:
	/// подписчик на сигнал отсоединения от сервера 
	COPCShutdownImpl m_Shutdowner;
	/// сигнал для отсоединения от сервера
	ATL::CEvent m_hShutdownEvent;

	/// ссылка на подписчика данных 
	COPCReceiveData *m_DataReceiver; 
	/// флаг - локально создавать объект сервера, или удаленно 
	bool						m_Local;
	/// хендл ОРС группы
    OPCHANDLE					m_GroupHandle;

	/// куки асинхронных колбаков 
	DWORD						m_Cookie;
	/// объект реализующий IOPCDataCallback
	COPCDataCallback*			m_OPCDataCallback;
	/// точка подъсоединения к событиям 
	CComPtr<IConnectionPoint>	m_ConnectionPoint;

	DWORD m_flags;
public:

	OPCClient(void);
	virtual ~OPCClient(void);

	void setDataReceiver( COPCReceiveData * rec );

	BrowseItems Browse();

	DWORD GetFlags(void) {
		return m_flags;
	}

	DWORD SetFlags(DWORD flags) {
		DWORD ret = m_flags;
		m_flags = flags;
		return ret;
	}
public:
	
	/// Св-ва клиента для подъсоединения к серверу 
	std::string m_Host,m_ProgID,m_GroupName;

	/// период обновления информации по асинхронному интерфейсу
	DWORD m_UpdateRate;

	/// определяет метод записиданных в ОРС сервер 
	/// true - использовать асинхронный интерфейс 
	bool m_UseAsync;

	/*! метод добавления параметров в группу 
		 true - подключение блоком 
		 false - подключение по одному параметру
	*/
	bool m_AddItemMode; 

private:
	/// объект сервера 
	CComPtr<IOPCServer>			m_Server;	
	/// одна группа на весь сервер 
	CComPtr<IOPCGroupStateMgt>  m_Group;	
public:

	/// if name == NULL, using m_ProgID member 
	/// if host == NULL, using m_Host member 
	virtual HRESULT Connect( LPCTSTR name, LPCTSTR host = NULL );

	/// отсоединение от сервера 
	virtual void Disconnect();

	bool isConnected();

	/// получить статус сервера 
	HRESULT getServerState( OPCSERVERSTATE &state );

	/*! добавляет в группу параметр, если клиент не подключен, то добавляет параметр в сиписок на подключение
		возвращает клиентский хендл, или 0 в случае неудачи 

		@param clientHandle - пользовательский хендл параметра, не может быть равен 0 
	*/
	OPCHANDLE AddTag( OPCHANDLE clientHandle, LPCTSTR tag_name, VARTYPE type );
	
	/// удалить из группы указанный параметр
	void RemoveTag( OPCHANDLE clientHandle ); 

	bool WriteValue( DWORD clientID, FILETIME &time, VARIANT &value, WORD Quality );
	bool WriteValue( LPCTSTR name, FILETIME &time, VARIANT &value, WORD Quality );
	
	bool WriteValues(int nValues, DWORD clientIDs[], VARIANT values[]);

	/// чтение значения параметра 
	bool ReadValue( DWORD clientID, FILETIME &time, VARIANT &value, WORD &Quality );

private:

	CComPtr<IOPCItemMgt> m_itemMgt;

	HRESULT AddParamToGroup( AG_OpcDA::Item* item/*, OPCHANDLE clientID*/ );
	HRESULT AddParamsTogetherToGroup( IOPCItemMgt *itemMgt, DWORD &m_ItemsAdded  );
	HRESULT AddParamsToGroup( DWORD &m_ItemsAdded );
	void fillOPCITEMDEF( AG_OpcDA::Item *item, OPCITEMDEF *idef );

	void ReportError( HRESULT hr ,LPCTSTR name);

	/// записать значение параметра в сервер ASYNC 
	HRESULT PutValueToOPC_Async( AG_OpcDA::Item * item );
	/// записать значение параметра в сервер SYNC 
	HRESULT PutValueToOPC_Sync( AG_OpcDA::Item * item );


	HRESULT OPCClient::PutValuesToOPC_Sync( int nValues, 
		CString names[], // для диагностики
		DWORD serverHdls[], 
		VARIANT values[] );
/// для OPCMonitor 
public:
	IOPCServer *GetServer()  {
		return m_Server;
	}

	IOPCGroupStateMgt *GetOpcGroup() {
		return m_Group;
	}
	
	/// обновить все параметры 
	void Refresh();

	const char *GetLastMessage();
};

// Quality codes
#define		QUAL_BAD						0x0000
#define		QUAL_UNCERTAIN					0x0040
#define		QUAL_GOOD						0x00C0
#define     QUAL_STATUS_MASK				0x00C0

std::string FormatValue(const VARIANT& v, WORD quality);
std::string FormatQuality(WORD quality);
std::string FormatValueType(const VARIANT& v);

