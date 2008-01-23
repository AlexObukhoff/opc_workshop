#pragma once

#include "../include/opcda.h"
#include "ShutdownImpl.h"

#include "OPCDataCallback.h"
#include "../include/OPCDataReceiver.h"
#include "OpcDaAbstractor.h"
#include "Browser.h"

#include <AtlSync.h>
#include <string>

// �����, ������������ ��������� �������. 
//  OPC_CLIENT_IGNORE_SHUTDOWN	- ��������� ����������� � �������, � �������� 
//   ����������� ��������� IOPCShutdown
#define OPC_CLIENT_IGNORE_SHUTDOWN	0x00000001  

class OPCClient : protected AG_OpcDA::COpcDaAbstractor
{
protected:
	/// ��������� �� ������ ������������ �� ������� 
	COPCShutdownImpl m_Shutdowner;
	/// ������ ��� ������������ �� �������
	ATL::CEvent m_hShutdownEvent;

	/// ������ �� ���������� ������ 
	COPCReceiveData *m_DataReceiver; 
	/// ���� - �������� ��������� ������ �������, ��� �������� 
	bool						m_Local;
	/// ����� ��� ������
    OPCHANDLE					m_GroupHandle;

	/// ���� ����������� �������� 
	DWORD						m_Cookie;
	/// ������ ����������� IOPCDataCallback
	COPCDataCallback*			m_OPCDataCallback;
	/// ����� �������������� � �������� 
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
	
	/// ��-�� ������� ��� �������������� � ������� 
	std::string m_Host,m_ProgID,m_GroupName;

	/// ������ ���������� ���������� �� ������������ ����������
	DWORD m_UpdateRate;

	/// ���������� ����� ������������ � ��� ������ 
	/// true - ������������ ����������� ��������� 
	bool m_UseAsync;

	/*! ����� ���������� ���������� � ������ 
		 true - ����������� ������ 
		 false - ����������� �� ������ ���������
	*/
	bool m_AddItemMode; 

private:
	/// ������ ������� 
	CComPtr<IOPCServer>			m_Server;	
	/// ���� ������ �� ���� ������ 
	CComPtr<IOPCGroupStateMgt>  m_Group;	
public:

	/// if name == NULL, using m_ProgID member 
	/// if host == NULL, using m_Host member 
	virtual HRESULT Connect( LPCTSTR name, LPCTSTR host = NULL );

	/// ������������ �� ������� 
	virtual void Disconnect();

	bool isConnected();

	/// �������� ������ ������� 
	HRESULT getServerState( OPCSERVERSTATE &state );

	/*! ��������� � ������ ��������, ���� ������ �� ���������, �� ��������� �������� � ������� �� �����������
		���������� ���������� �����, ��� 0 � ������ ������� 

		@param clientHandle - ���������������� ����� ���������, �� ����� ���� ����� 0 
	*/
	OPCHANDLE AddTag( OPCHANDLE clientHandle, LPCTSTR tag_name, VARTYPE type );
	
	/// ������� �� ������ ��������� ��������
	void RemoveTag( OPCHANDLE clientHandle ); 

	bool WriteValue( DWORD clientID, FILETIME &time, VARIANT &value, WORD Quality );
	bool WriteValue( LPCTSTR name, FILETIME &time, VARIANT &value, WORD Quality );
	
	bool WriteValues(int nValues, DWORD clientIDs[], VARIANT values[]);

	/// ������ �������� ��������� 
	bool ReadValue( DWORD clientID, FILETIME &time, VARIANT &value, WORD &Quality );

private:

	CComPtr<IOPCItemMgt> m_itemMgt;

	HRESULT AddParamToGroup( AG_OpcDA::Item* item/*, OPCHANDLE clientID*/ );
	HRESULT AddParamsTogetherToGroup( IOPCItemMgt *itemMgt, DWORD &m_ItemsAdded  );
	HRESULT AddParamsToGroup( DWORD &m_ItemsAdded );
	void fillOPCITEMDEF( AG_OpcDA::Item *item, OPCITEMDEF *idef );

	void ReportError( HRESULT hr ,LPCTSTR name);

	/// �������� �������� ��������� � ������ ASYNC 
	HRESULT PutValueToOPC_Async( AG_OpcDA::Item * item );
	/// �������� �������� ��������� � ������ SYNC 
	HRESULT PutValueToOPC_Sync( AG_OpcDA::Item * item );


	HRESULT OPCClient::PutValuesToOPC_Sync( int nValues, 
		CString names[], // ��� �����������
		DWORD serverHdls[], 
		VARIANT values[] );
/// ��� OPCMonitor 
public:
	IOPCServer *GetServer()  {
		return m_Server;
	}

	IOPCGroupStateMgt *GetOpcGroup() {
		return m_Group;
	}
	
	/// �������� ��� ��������� 
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

