// ClientTestConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/// Object received all values, changed inside OPC server 
class dataReceiverClient :
	public COPCReceiveData
{
public:

	virtual void newData(LPCTSTR name, DWORD clientId, FILETIME &/*time*/, VARIANT &value, WORD Quality )
	{
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

void list( string offset, string name = "" )
{
	BrowseItems brItem( &client, name );

	std::vector<BrowseItems::Item>::iterator it;
	for( it = brItem.items.begin(); it != brItem.items.end(); ++it )
	{
		BrowseItems::Item &item = *it;

		if( item.flag == OPC_BROWSE_HASCHILDREN )
		{
			cout << offset << "[" << item.name << "]\n";
			list( offset + "   ", item.itemId );
		}
		else
			cout << offset << item.name << "\n";
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(NULL);

     client.setDataReceiver( &rec );
     client.Connect( "Matrikon.OPC.Simulation" );

     list("");
	 
	 CoUninitialize();
	 return 0;
}

