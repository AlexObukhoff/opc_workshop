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
 
// server->test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <objbase.h>

#include <math.h>
#include <atlsafe.h>
#include <atlstr.h>
#include <ATLComTime.h>

#include "..\server.dll\ag_Clocker.h"
#include "..\server.dll\OPCServerObject.h"

#include "DelayImpHlp.h"

DL_USE_MODULE_BEGIN(OpcServer, "Opcserver.dll")
	DL_DECLARE_FUN(CreateNewOPCServerObject, COPCServerObject*, (void) )
	DL_DECLARE_FUN(DestroyOPCServerObject, int, (COPCServerObject*) )
DL_USE_MODULE_END()

using namespace std;

/// Object received all values, changed inside OPC server 
class dataReceiver :
	public COPCReceiveData
{
public:

	virtual void newData(LPCTSTR name, DWORD /*clientId*/, FILETIME &/*time*/, VARIANT &value, WORD Quality )
	{
		double v = 0.0;
		CComVariant val = value;
		switch( val.vt ) {
			case VT_I2:
			case VT_I4:
			case VT_INT:		v = val.intVal; break;
			case VT_FILETIME:	break;
			case VT_R4:			v = val.fltVal; break;
			case VT_R8:			v = val.dblVal; break;
			case VT_EMPTY:
				v = -1.999999; break;
			default:
				v = 1.999999; break;
		};
		printf("Param changed [%s] value=%f Quality = %d\n", name, v, Quality );	
	}
	/// after calls newData, called newItemIsReceived
	virtual void newItemIsReceived( DWORD count ) { count; }
	virtual void log_message( LogLevel fatal, const char* fmt, ... )
	{
		switch( fatal )
		{
		case LogDebugInfo:	printf("DBG:\t"); break;
		case LogMessage:	printf("LOG:\t"); break;
		case LogWarning:	printf("WAR:\t"); break;
		case LogError:		printf("ERR:\t"); break;
		};
//		static char buffer[4096];
		va_list args;
		va_start(args, fmt);
//		_vnsprintf (buffer, 4096, fmt, args);
		vprintf (fmt, args);
		va_end(args);
	}

	virtual void StatusChanged( int , const char * ) { };

};

void client_open();
void client_close();

GUID CLSID_OPCServerEXE = { 0x4ea2713d, 0xca07, 0x11d4, {0xbe, 0xf5, 0x0, 0x0, 0x21, 0x20, 0xdb, 0x5E} };

int _tmain(int /*argc*/, _TCHAR* /*argv[]*/)
{
	char str[1024] = {0};

	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// define server object 
	COPCServerObject *server = NULL; 
	try {
		server = OpcServer::CreateNewOPCServerObject();
	} catch(...)
	{
		cerr << "Failed load OPCserver->dll\n";
		return -1;
	}

	if( server == NULL ) {
		cerr << "Failed create COPCServerObject\n";
		return -1;
	}


	// define data event receiver 
	dataReceiver receiver;

	// set server name and clsid
	server->setServerProgID( _T("OPC.myTestServer") );
	server->setServerCLSID( CLSID_OPCServerEXE );

	// set delimeter for params name 
	server->SetDelimeter( "." );

	// register server as COM/DCOM object 
	server->RegisterServer();

	// define server values tree
	server->AddTag("Values.flt1", VT_R4 );
	server->AddTag("Values.flt2", VT_R4 );
	server->AddTag("Values.fltArray2", VT_ARRAY|VT_R4 );
	server->AddTag("Values.fltArray2.In", VT_I4, false );

	{
		CAG_Clocker cl("Create COUNT_PARAM tags",false);

		for(int i=0;i<COUNT_PARAM;++i) {
			sprintf(str,"RandomValues.int%d",i+1);
			server->AddTag( str ,VT_I4 );
		}
	}

	// setup object will be received add values change 
	server->setDataReceiver( &receiver );
	
	// create COM class factory and register it 
	server->StartServer( OPC_STATUS_RUNNING );

	printf("\t Client connecting \n");

	client_open();

	printf("\t waiting return\n");
	gets(str);

	// write initial values to OPC params
	for( double x =0.;  x< 50.;x+= .1 ) {
		server->WriteValue( "Values.flt1", server->FILETIME_NULL(), 192, CComVariant( sin(x) ) );
		server->WriteValue( "Values.flt2", server->FILETIME_NULL(), 192, CComVariant( cos(x) ) );
		Sleep(100);
	}

	srand( (unsigned)time( NULL ) );

	for(int i=0;i<COUNT_PARAM;++i) {
		sprintf(str,"RandomValues.int%d",i+1);
		server->WriteValue( str , server->FILETIME_NULL(), 192, CComVariant( rand() ) );
	}

	printf("\t client disconnect  \n");

	client_close();

	printf("\t waiting return for close server \n");
	gets(str);

	server->StopServer();
	server->UnregisterServer();
	OpcServer::DestroyOPCServerObject(server);

	CoUninitialize();
	return 0;
}


