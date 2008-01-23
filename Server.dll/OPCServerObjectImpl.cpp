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

#include "StdAfx.h"
#include <atlsafe.h>

#include "ag_thread.h"
#include "NameIndex.h"

#include "opc.h"
#include "opc_Data.h"
#include "opc_impl.h"

#include "OPCServer.h"
#include "myCF.h"
#include "OPCServerObjectImpl.h"

#include "opcDataTranslator.h"

static COPCDataTranslator *g_DataTranslator = NULL;

extern "C" 
{
	/*! Class fabric for OPCServer objects */
	ExportClass COPCServerObject* WINAPI CreateNewOPCServerObject(void)
	{
		return ::new COPCServerObjectImpl();
	}
	/*! Destroyer for OPCServer objects */
	ExportClass int WINAPI DestroyOPCServerObject(COPCServerObject* server)
	{
		if( server != NULL ) {
			::delete server;
			return 1;
		}
		return 0;
	}
}

COPCServerObjectImpl::COPCServerObjectImpl(void)
{
	if( !g_DataTranslator )
		g_DataTranslator = new COPCDataTranslator();
}

WORD COPCServerObjectImpl::QualityGood() 
{
	return OPC_QUALITY_GOOD;
}

WORD COPCServerObjectImpl::QualityBad()
{
	return OPC_QUALITY_BAD;
}

FILETIME COPCServerObjectImpl::FILETIME_NULL(void)
{
	static FILETIME ftnull = {0,0};
	return ftnull;
}


COPCServerObjectImpl::~COPCServerObjectImpl(void)
{
	if( g_DataTranslator ) {
		delete g_DataTranslator;
		g_DataTranslator = NULL;
	}
}

/// Установить имя объекта сервера
void COPCServerObjectImpl::setServerProgID(LPCTSTR progID)
{
	OPCServerProgID = progID;
}

/// установить CLSID сервера 
void COPCServerObjectImpl::setServerCLSID(REFCLSID progID)
{
	CLSID_OPCServerEXE  = progID;
}


/// зарегистрировать сервер в системе
int COPCServerObjectImpl::RegisterServer()
{
	char np[FILENAME_MAX + 32] = {0};
//	printf("Registering");
	GetModuleFileName(NULL, np , sizeof(np) - 8);

	return ServerRegister(&CLSID_OPCServerEXE, 
                               OPCServerProgID,
							   OPCServerProgID, np, 0);
}

/// резрегистрировать сервер в системе
int COPCServerObjectImpl::UnregisterServer()
{
	g_BrowseItems.erase( g_BrowseItems.begin(), g_BrowseItems.end() );
	return ServerUnregister(&CLSID_OPCServerEXE, OPCServerProgID);
}

/// запустить сервер
int COPCServerObjectImpl::StartServer(OPCSERVERSTATE state)
{
	HRESULT hr = CoRegisterClassObject(CLSID_OPCServerEXE, &my_CF,
                                        CLSCTX_LOCAL_SERVER |
                                        CLSCTX_REMOTE_SERVER |
                                        CLSCTX_INPROC_SERVER,
	                                    REGCLS_MULTIPLEUSE, &m_objid);
	if(SUCCEEDED(hr))
		my_CF.serverAdd();
	
	my_CF.CreateServer();
	SetServerState( state );

	return FAILED(hr);
}

void COPCServerObjectImpl::SetServerState(OPCSERVERSTATE state)
{
	if( g_Server != NULL )
		g_Server->m_ServerStatus.dwServerState = state;
}

extern CBrowseItemsList g_BrowseItems;

/// отстановить процесс сервера
int COPCServerObjectImpl::StopServer()
{
	if( g_Server != NULL )
		g_Server->m_ServerStatus.dwServerState = OPC_STATUS_FAILED;
	if( g_DataTranslator != NULL )
		g_DataTranslator->Unsubscribe( );

	my_CF.CreateServer( false );
    HRESULT hr = CoRevokeClassObject(m_objid);
	g_BrowseItems.erase(g_BrowseItems.begin(), g_BrowseItems.end());

	return FAILED(hr);
}

void COPCServerObjectImpl::SetDelimeter( const char *ch )
{
	strcpy( g_BrowseNameDelimeter , ch );
}


/// добавить тег в базу сервера
///		при этом указывается тип тега и возможность клиенту писать в этот параметр
int COPCServerObjectImpl::AddTag( LPCTSTR name, VARTYPE type, bool readOnly)
{
	if( g_BrowseItems.find( name ) == g_BrowseItems.end() )  // проверка на существование тега
	{
		DWORD new_index = g_NameIndex[ name ];

//		int ret = g_BrowseItems.size();
		g_BrowseItems.push_back( CItemForBrowse( new_index, name , type, 
			readOnly ? OPC_READABLE : OPC_READABLE | OPC_WRITEABLE ) );

		return new_index;
	}

	return -1;
}

/// передать значение параметра
bool COPCServerObjectImpl::WriteValue( LPCTSTR name, FILETIME ft, WORD quality, const VARIANT &value )
{
	CBrowseItemsList::iterator it = g_BrowseItems.find( name );
	if(it == g_BrowseItems.end()) return false;

	CAG_Value adapt;
	adapt.m_Name = name;
	adapt.m_NameId = g_NameIndex[ name ];
	adapt.m_Value = value;
	adapt.m_Quality = quality;

	if( ft == FILETIME_NULL() )
		CoFileTimeNow( &adapt.m_Time );
	else
		adapt.m_Time = ft;

	adapt.m_Type = value.vt;

	return SUCCEEDED( g_Server->m_DM.pushNewData( adapt ) );
}

/// передать значение параметра
bool COPCServerObjectImpl::WriteValue( int hdl, FILETIME ft, WORD quality, const VARIANT &value )
{
	if (g_Server == NULL)
		return false;

	CAG_Value adapt;
	adapt.m_Name = g_NameIndex[ hdl ];
	adapt.m_NameId = hdl;
	adapt.m_Value = value;
	adapt.m_Quality = quality;

	if( ft == FILETIME_NULL() )
		CoFileTimeNow( &adapt.m_Time );
	else
		adapt.m_Time = ft;

	adapt.m_Type = value.vt;

	return SUCCEEDED( g_Server->m_DM.pushNewData( adapt ) );
}
//////////////////////////////////////////////////////////////////////////
bool COPCServerObjectImpl::WriteValue( LPCTSTR name, FILETIME ft, WORD quality, int value )
{
	CComVariant var = value;
	return WriteValue( name, ft, quality, var );
}

bool COPCServerObjectImpl::WriteValue( int hdl, FILETIME ft, WORD quality, int value )
{
	CComVariant var = value;
	return WriteValue( hdl, ft, quality, var );
}
//////////////////////////////////////////////////////////////////////////
bool COPCServerObjectImpl::WriteValue( LPCTSTR name, FILETIME ft, WORD quality, float value )
{
	CComVariant var = value;
	return WriteValue( name, ft, quality, var );
}

bool COPCServerObjectImpl::WriteValue( int hdl, FILETIME ft, WORD quality, float value )
{
	CComVariant var = value;
	return WriteValue( hdl, ft, quality, var );
}
//////////////////////////////////////////////////////////////////////////
bool COPCServerObjectImpl::WriteValue( LPCTSTR name, FILETIME ft, WORD quality, double value )
{
	CComVariant var = value;
	return WriteValue( name, ft, quality, var );
}

bool COPCServerObjectImpl::WriteValue( int hdl, FILETIME ft, WORD quality, double value )
{
	CComVariant var = value;
	return WriteValue( hdl, ft, quality, var );
}
//////////////////////////////////////////////////////////////////////////

bool COPCServerObjectImpl::WriteValue( LPCTSTR name, FILETIME ft, WORD quality, int* value , unsigned count )
{
	CComSafeArray<int> var;
	var.Add( count, value );

	return WriteValue( name, ft, quality, CComVariant(var) );
}

bool COPCServerObjectImpl::WriteValue( int hdl, FILETIME ft, WORD quality, int* value , unsigned count )
{
	CComSafeArray<int> var;
	var.Add( count, value );

	return WriteValue( hdl, ft, quality, CComVariant(var) );
}
//////////////////////////////////////////////////////////////////////////
bool COPCServerObjectImpl::WriteValue( LPCTSTR name, FILETIME ft, WORD quality, float* value, unsigned count )
{
	CComSafeArray<float> var;
	var.Add( count, value );
	return WriteValue( name, ft, quality,CComVariant(var) );
}

bool COPCServerObjectImpl::WriteValue( int hdl, FILETIME ft, WORD quality, float* value, unsigned count )
{
	CComSafeArray<float> var;
	var.Add( count, value );
	return WriteValue( hdl, ft, quality,CComVariant(var) );
}
//////////////////////////////////////////////////////////////////////////
bool COPCServerObjectImpl::WriteValue( LPCTSTR name, FILETIME ft, WORD quality, double* value, unsigned count )
{
	CComSafeArray<double> var;
	var.Add( count, value );
	return WriteValue( name, ft, quality, CComVariant(var) );
}

bool COPCServerObjectImpl::WriteValue( int hdl, FILETIME ft, WORD quality, double* value, unsigned count )
{
	CComSafeArray<double> var;
	var.Add( count, value );
	return WriteValue( hdl, ft, quality, CComVariant(var) );
}
//////////////////////////////////////////////////////////////////////////
void COPCServerObjectImpl::setDataReceiver( COPCReceiveData *receiver)
{
	if( g_Server == NULL )
		my_CF.CreateServer();

	g_DataTranslator->setDataReceiver( receiver );
	g_DataTranslator->SetAcceptAll( true );
	g_DataTranslator->Subscribe( &g_Server->m_DM );
}

