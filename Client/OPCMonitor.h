// OPCMonitor.h: interface for the OPCMonitor class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "comdef.h"

#include "WorkerThread.h"
#include "LockedQueue.h"

#include "OPCClient.h"

#include <assert.h>
#include <vector>
#include <map>
#include <string>

class CDataMeanChannel;

class OPCMonitor;


class OPCParameterDesc
{
public:
	OPCParameterDesc()
	{
		assert(0);
	}

public:

	CString it;
	CString accessPath;
	CString m_OpcTagId;
	BOOL UseOpcTagId;
	VARTYPE vt;
	int index;
//	CDataMeanChannel *ch;
	OPCHANDLE ServerHandle;

	OPCParameterDesc(CString _it, CString _accessPath, 							
		BOOL _UseOpcTagId,
		CString _m_OpcTagId,
		VARTYPE _vt, int _index/*, CDataMeanChannel *_ch*/) 
	{
		it = _it;
		accessPath = _accessPath;
		UseOpcTagId = _UseOpcTagId;
		m_OpcTagId = _m_OpcTagId;
		vt = _vt;
		index = _index;
//		ch = _ch;
		ServerHandle = NULL;
	}
	OPCParameterDesc( const OPCParameterDesc& pd )
	{
		*this = pd;
	}

	const OPCParameterDesc& operator=(const OPCParameterDesc& pd )
	{
		it = pd.it;
		accessPath =pd.accessPath;
		UseOpcTagId = pd.UseOpcTagId;
		m_OpcTagId = pd.m_OpcTagId;
		vt = pd.vt;
		index = pd.index;
		ServerHandle = pd.ServerHandle;
		return *this;
	}
};

class OPCEventConsumer
{
public:
	virtual void ProcessEvent(COPCDataImportEvent &ev) = 0;
};

class OPCValueForWrite
{
public:
	long param_id;
	CComVariant  value;
};

#ifndef D_FILETIME_NULL
	#define D_FILETIME_NULL
	extern FILETIME FILETIME_NULL;
#endif

class OPCMonitor : 
	public COPCWorkerThread ,
	public COPCReceiveData
{
protected:
	OPCClient m_Client; // OPCWorkshop client

	int ConnectAndCreateGroup(CString &ServerName, CString &GroupName);

	/// signal for disconnecting from server
	ATL::CEvent m_hShutdownEvent;

	bool DoPostThreadTask();
	bool DoPreThreadTask();
	bool DoStopThreadTask();

public:
	bool OpcConnect(LPCTSTR server_name, LPCTSTR host = NULL );

private:
	bool OpcConnect(CLSID &server_id, LPCTSTR host = NULL );
	void OpcDisconnect();

	void OnWorkerEvent(int event_no);


	CLockedQueue<OPCParameterDesc *> m_queue;

	void ProcessAddParameter(void);

	void DeleteParameter(LONG ParamId);
	void InsertParameter(LONG ParamId, const char *ParamName, bool UseOpcTag, const char *OpcTagId);
	void AddOpcParameter(std::vector<OPCParameterDesc*> &pdesc_vector);	

	std::map <int, OPCParameterDesc> in_params;

protected:
	mutex_t opc_mon_mtx;
public:
	OPCMonitor();
	virtual ~OPCMonitor();

	void AddOpcItem(std::string it,
		std::string accessPath, 
		BOOL UseOpcTagId,
		std::string m_OpcTagId,
		VARTYPE vt, 
		int index/*, CDataMeanChannel *ch*/);

	std::string GetParameterName(int index);
//	int LoadParameters(void);

	bool IsExist(int itemid);
	bool IsConnected();

	// "Apply" paramatr
	// If not changed - do nothing
	// If not exits (not find m_ParamId) - adding
	// if exist, but name not equal - remove old and adding new parametr
	void ApplyParameter(LONG ParamId, const char *ParamName, bool UseOpcTag, const char *OpcTagId);

	void LockOPC(void) {
		opc_mon_mtx.lock();
	}

	void UnlockOPC(void) {
		opc_mon_mtx.unlock();
	}

	OPCClient *GetClient() {
		return &m_Client;
	}

	//IOPCServer *GetServer() {
	//	return m_Client.GetServer();
	//}

	IOPCGroupStateMgt *GetOpcGroup() {
		return m_Client.GetOpcGroup();
	}

	virtual std::string DumpStatus();

	/*!
	   Add tag value to data queue, for writing to ÎÐÑ server
	   Returning true if all OK
	   Can throw opcError exception
	*/
	bool WriteToOPCQueue( LONG ParamId, double val, FILETIME ft = FILETIME_NULL, DWORD opc_quality = OPC_QUALITY_GOOD);
	bool WriteToOPCQueue( LONG ParamId, int val, FILETIME ft = FILETIME_NULL, DWORD opc_quality = OPC_QUALITY_GOOD );
	/*!
		Write all data queue to server by one server call
		return count tags, writed with out error
	*/
	int FlushOPCQueue();
private:
	CLockedQueue<OPCValueForWrite> OPCWriteQueue;

public:
	CString m_ServerName;
	CString m_ServerHost;
	DWORD m_ConnectionRefreshRate;
	BOOL m_UseSystemTimestamp;
	BOOL m_UseSync;  // instead of ARMImportConfig::instance.SyncMode()

private:
	std::vector<OPCEventConsumer*> consumers;
	void FireOPCEvent( COPCDataImportEvent &ev );
	friend class COPCDataCallbackImpl;
public:
	void addOPCEventConsumer( OPCEventConsumer *m ) 
	{
		assert( m );
		consumers.push_back( m );
	}

	const OPCMonitor& operator=(const OPCMonitor&) {  throw ("operator= NOT USING FOR OPCMonitor"); }
};

std::string GetComErrorMessage( _com_error &e );
std::string GetComErrorMessage( HRESULT hr );
