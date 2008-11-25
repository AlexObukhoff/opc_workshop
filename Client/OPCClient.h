#pragma once

#include "../include/opcda.h"
#include "ShutdownImpl.h"

#include "OPCDataCallback.h"
#include "../include/OPCDataReceiver.h"
#include "OpcDaAbstractor.h"
#include "Browser.h"

#include <AtlSync.h>
#include <string>

// Falgs, defining client behaviour
//  OPC_CLIENT_IGNORE_SHUTDOWN	- allow connection to server with out IOPCShutdown interface supporting
#define OPC_CLIENT_IGNORE_SHUTDOWN	0x00000001  

class OPCClient : protected AG_OpcDA::COpcDaAbstractor
{
protected:
	/// subscriber for IOPCShutdown interface
	COPCShutdownImpl m_Shutdowner;
	/// event for disconnecting from server
	ATL::CEvent m_hShutdownEvent;

	/// data subscriber link
	COPCReceiveData *m_DataReceiver; 
	/// flag - local or remote server creation method
	bool						m_Local;
	/// ÎÐÑ handle of group
    OPCHANDLE					m_GroupHandle;

	/// coocies of async callbacks
	DWORD						m_Cookie;
	/// IOPCDataCallback implementation object 
	COPCDataCallback*			m_OPCDataCallback;
	/// connection point for events
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
	
	/// Client properties for connection to server
	std::string m_Host,m_ProgID,m_GroupName;

	/// async interface updating timeout
	DWORD m_UpdateRate;

	/// describe method for writing data to OPC server
	/// true - use async interface
	bool m_UseAsync;

	/*! method addition tags to group
		 true - adding by block 
		 false - adding each tag individually
	*/
	bool m_AddItemMode; 

private:
	/// server object
	CComPtr<IOPCServer>			m_Server;	
	/// on group for server
	CComPtr<IOPCGroupStateMgt>  m_Group;	
public:

	/// if name == NULL, using m_ProgID member 
	/// if host == NULL, using m_Host member 
	virtual HRESULT Connect( LPCTSTR name, LPCTSTR host = NULL );

	/// disconnect from server
	virtual void Disconnect();

	bool isConnected();

	/// get server status
	HRESULT getServerState( OPCSERVERSTATE &state );

	/*! append tag to group, 
		if client not connected, adding tag to list "to connect" and return client handle, 
		or NULL if error

		@param clientHandle - user handle of tag, must be not zero 
	*/
	OPCHANDLE AddTag( OPCHANDLE clientHandle, LPCTSTR tag_name, VARTYPE type );
	
	/// remove and unsubscribe tag
	void RemoveTag( OPCHANDLE clientHandle ); 

	bool WriteValue( DWORD clientID, FILETIME &time, VARIANT &value, WORD Quality );
	bool WriteValue( LPCTSTR name, FILETIME &time, VARIANT &value, WORD Quality );
	
	bool WriteValues(int nValues, DWORD clientIDs[], VARIANT values[]);

	/// read value from server
	bool ReadValue( DWORD clientID, FILETIME &time, VARIANT &value, WORD &Quality );

private:

	CComPtr<IOPCItemMgt> m_itemMgt;

	HRESULT AddParamToGroup( AG_OpcDA::Item* item/*, OPCHANDLE clientID*/ );
	HRESULT AddParamsTogetherToGroup( IOPCItemMgt *itemMgt, DWORD &m_ItemsAdded  );
	HRESULT AddParamsToGroup( DWORD &m_ItemsAdded );
	void fillOPCITEMDEF( AG_OpcDA::Item *item, OPCITEMDEF *idef );

	void ReportError( HRESULT hr ,LPCTSTR name);

	/// write value to server ASYNC 
	HRESULT PutValueToOPC_Async( AG_OpcDA::Item * item );
	/// write value to server SYNC 
	HRESULT PutValueToOPC_Sync( AG_OpcDA::Item * item );


	HRESULT OPCClient::PutValuesToOPC_Sync( int nValues, 
		CString names[], // for diagnostic
		DWORD serverHdls[], 
		VARIANT values[] );
/// for OPCMonitor 
public:
	IOPCServer *GetServer()  {
		return m_Server;
	}

	IOPCGroupStateMgt *GetOpcGroup() {
		return m_Group;
	}
	
	/// reload all tags
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

