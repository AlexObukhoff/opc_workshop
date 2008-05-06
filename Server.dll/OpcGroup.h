/***************************************************************************
 *   Copyright (C) 2005 by Alexey Obukhov                                  *
 *   obukhoff@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#pragma once

#define MINIMUM_UPDATE_RATE 100


#include "ErrorInfoImpl.h"

#include "ag_timer.h"
//#include "sf_timer.h"
//extern CTimerHost g_TimerHost;

//#include "xl_events.h"
//#include "_IGF_Events_CP.H"

/// Describe item in opc server group
class ItemInGroup
{
public:
	OPCHANDLE hClient;
	VARTYPE type;
//	volatile LONG modified;
	BOOL bActive;

	ItemInGroup(OPCHANDLE h = 0,BOOL act = FALSE) : 
		  hClient(h), 
//		  modified(FALSE), 
		  bActive(act), 
		  type(VT_EMPTY) 
	{ 
	}
};

class ItemsInGroupMap : public CRWMonitor
{ 
protected:
	std::map<OPCHANDLE,ItemInGroup*> m_items_map;
//	CRWMonitor locker;
public:

	typedef std::map<OPCHANDLE,ItemInGroup*>::iterator iterator;

	iterator find (OPCHANDLE h) {
		return m_items_map.find(h);
	}

	iterator begin() {
		return m_items_map.begin();
	}
	iterator end() {
		return m_items_map.end();
	}

	void insert(OPCHANDLE hdl, ItemInGroup *item) {
		m_items_map.insert(make_pair(hdl, item));
	}

	void erase(iterator where) {
		m_items_map.erase(where);
	}
};

//#include "NameIndex.H"
#include "BrowseItem.H"

#define OPC_GROUP_CHECK_DELETED()	T* pT = static_cast<T*>(this); \
									if(pT->m_Deleted) return E_FAIL;

class COPCServer;

#include "IOPCItemMgtImpl.h"
#include "IOPCGroupStateMgtImpl.h"
#include "IOPCSyncIOImpl.h"
#include "IOPCAsyncIO2Impl.h"
#include "IOPCDataCallback_CP.h"

class ATL_NO_VTABLE COPCGroup : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<COPCGroup>,
	public IConnectionPointContainerImpl<COPCGroup>,
	public AG_IErrorInfoImpl<COPCGroup>,

	public CProxy_IOPCDataCallback<COPCGroup>,
	public IOPCItemMgtImpl<COPCGroup>,
	public IOPCGroupStateMgtImpl<COPCGroup>,
	public IOPCSyncIOImpl<COPCGroup>,
	public IOPCAsyncIO2Impl<COPCGroup>,
	public opcData::COPCDataCustomer
{
public:
	DWORD m_UpdateRate;
	CString m_GroupName;
	OPCHANDLE m_ClientHandle;
	OPCHANDLE m_ServerHandle;
	DWORD m_TimeBias;
	FLOAT m_PercentDeadband;
	LCID m_dwLCID;

	COPCServer *m_ParentServer;

	CWin32Timer< COPCGroup >   m_Timer;
    //CTimerThunk< COPCGroup >   m_Timer;

	bool m_Deleted;

	// <server,client>
	ItemsInGroupMap m_ItemsAdded;

	// paramters list for async reading
	CComAutoCriticalSection m_AsyncReadSect;
	std::set<OPCHANDLE> m_AsyncRead;


	// перенесено как наследование 
	//opcData::COPCDataCustomer m_DataCustomer;

	COPCGroup() :
		m_Timer( this )
		//m_Timer( g_TimerHost, this, OnTimer )
	{
		m_ClientHandle = 0;
		m_UpdateRate = 1000; // 1 second
		m_ServerHandle = 0;
		m_TimeBias = 0;

		m_Deleted = false;
		// m_FlagChangedValues = FALSE;
		m_ParentServer = NULL;

		// OPC group don't accept parameters without subscription
		/*m_DataCustomer.*/SetAcceptAll( false ); 
	}

	virtual ~COPCGroup()
	{
	
	}

	BEGIN_COM_MAP(COPCGroup)
		COM_INTERFACE_ENTRY(IOPCItemMgt)
		COM_INTERFACE_ENTRY(IOPCGroupStateMgt)
		COM_INTERFACE_ENTRY(IErrorInfo)
		COM_INTERFACE_ENTRY(IConnectionPointContainer)
		COM_INTERFACE_ENTRY(IOPCSyncIO)
		COM_INTERFACE_ENTRY(IOPCAsyncIO2)
	END_COM_MAP()

	BEGIN_CONNECTION_POINT_MAP(COPCGroup)
		CONNECTION_POINT_ENTRY(__uuidof(IOPCDataCallback))
	END_CONNECTION_POINT_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease();

public:
	/// timer ivent -> now need call clients callbacks
    void OnTimer(/*DWORD dwTime*/)
	{
		sendChangedDataToClient();
		//m_Timer.KillTimer();
		//	RemoveOldDumps( m_WriteParametrs.m_ArchiveLength );
		//setTimer( -1 );
	}

	void sendChangedDataToClient();
};

template <typename T>
T* allocate_buffer( size_t count )
{
	T* p = (T*)CoTaskMemAlloc( sizeof(T) * count );
	ZeroMemory( p, sizeof(T) * count );
	return p;
}

