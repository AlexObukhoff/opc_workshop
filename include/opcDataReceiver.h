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

#include <wtypes.h>
#include <string>

#define INVALID_CLIENT_ID 0xffffffff

typedef enum 
{
	LogDebugInfo = -1,
	LogMessage = 0,
	LogWarning = 1,
	LogError = 2
} LogLevel;

/// Call back virtual class - receiver data from OPC connections 
class COPCReceiveData
{
protected:
	std::string m_LastMessage;
public:
	/*! Push new data item to client 

		@param name - param name, maybe NULL if clientID is not INVALID_CLIENT_ID 
		@param clientID - client param ID, maybe INVALID_CLIENT_ID if name is not NULL
		@param time - time stamp
		@param value - param value 
		@param Quality - OPC quality flag 
	*/
	virtual void newData(LPCTSTR name, DWORD clientID, FILETIME &time, 
		VARIANT &value, WORD Quality) = 0;
	
	/// after calls newData, called newItemIsReceived
	virtual void newItemIsReceived( DWORD count ) { count; };
	
	/*! This methods is called for log messages for client 
		
		@param fatal :	-1 - debug message 
						0 - default message 
						1 - warning message 
						2 - error message 
						
		@param fmt - format string ( as printf format )

		By default this function formats last message to local string
	*/
	virtual void log_message( LogLevel fatal, const char* fmt, ... )
	{ 
		char str[4096] = {0};

		switch( fatal )
		{
		case LogDebugInfo:	m_LastMessage = "DBG: "; break;
		case LogMessage:	m_LastMessage = "LOG: "; break;
		case LogWarning:	m_LastMessage = "WAR: "; break;
		case LogError:		m_LastMessage = "ERR: "; break;
		};

		va_list args;
		va_start(args, fmt);
		_vsnprintf(str, 4000, fmt, args);
		va_end(args);

		m_LastMessage += str;
	}

	/*! This is an accessor for last log message
		

	*/

	const char *GetLogMessage() {
		return m_LastMessage.c_str();
	}

	/*! For client only, message callback for signal connecting, disconnecting OPC client  
		
		@param status : 0 - client disconnected 
						1 - client connected 
						-1 - server request disconnect ( message -> server reason )
	*/
	virtual void StatusChanged( int status, const char *message ) = 0; // { status };
};
