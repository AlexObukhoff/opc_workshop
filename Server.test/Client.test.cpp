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

// Server.test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <objbase.h>

#include <math.h>
#include <atlsafe.h>
#include <atlstr.h>
#include <ATLComTime.h>

#include "..\Server.dll\ag_Clocker.h"
#include "..\Client\Client.h"


/// Object received all values, changed inside OPC server 
class dataReceiverClient :
	public COPCReceiveData
{
public:

	virtual void newData(LPCTSTR name, DWORD clientId, FILETIME &/*time*/, VARIANT &value, WORD Quality )
	{
		double v = 0.0;
		CComVariant val = value;
		switch( val.vt ) 
		{
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
		printf("CLIENT: Param changed [%s] HANDLE=%d value=%f Quality = %d\n", name, clientId , v, Quality );	
	}
	/// after calls newData, called newItemIsReceived
	virtual void newItemIsReceived( DWORD count ) 
	{ 
		printf( "CLIENT: Received %d params\n", count );
	}

	virtual void log_message( LogLevel fatal, const char* fmt, ... )
	{
		switch( fatal )
		{
		case LogDebugInfo:	printf("DBG:\t"); break;
		case LogMessage:	printf("LOG:\t"); break;
		case LogWarning:	printf("WAR:\t"); break;
		case LogError:		printf("ERR:\t"); break;
		};
		static char buffer[4096];
		va_list args;
		va_start(args, fmt);
		_vsnprintf (buffer, 4096, fmt, args);
		va_end(args);

		CString str = buffer;
		str.AnsiToOem();

		printf("%s\n", (LPCTSTR)str );

	}

	/*! For client only, message callback for signal connecting, disconnecting OPC client  

	@param status : 0 - client disconnected 
	1 - client connected 
	-1 - server request disconnect ( message -> server reason )
	*/
	virtual void StatusChanged( int status, const char *message )
	{
		printf("Client status: ");
		switch( status )
		{
		case 0: printf("DISCONNECTED"); break;
		case 1: printf("CONNECTED"); break;
		case -1: printf("Server request disconnect: "); break;
		};
		printf("[%s]\n", message );
	}
};

OPCClient client;
dataReceiverClient rec;

void client_open()
{
	char str[1024] = {0};

	client.setDataReceiver( &rec );

	OPCHANDLE ch = 1;


	client.AddTag(ch++, "Values.flt1", VT_R4 );
	client.AddTag(ch++, "Values.flt2", VT_R4 );
	client.AddTag(ch++, "Values.fltArray2", VT_ARRAY|VT_R4 );
	client.AddTag(ch++, "Values.fltArray2.In", VT_I4  );

	/// try to add does't existing tag
	client.AddTag(ch++, "Values.fake.flt1", VT_R4 );

	{
		CAG_Clocker cl("CLIENT: Connect to Randoms tags",false);

		for(int i=0;i<COUNT_PARAM;++i) {
			sprintf(str,"RandomValues.int%d",i+1);
			client.AddTag( ch++, str ,VT_I4 );
		}
	}
	client.Connect( "OPC.myTestServer" );
}

void client_close()
{
	client.Disconnect();
}



