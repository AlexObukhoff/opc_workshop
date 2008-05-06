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

#include "OPCServerObject.h"

/*!
	Realisation if interface class for working with opc server object
	all function returning 0 for success
*/
class COPCServerObjectImpl : public COPCServerObject
{
protected:
	  DWORD m_objid;
public:
	COPCServerObjectImpl(void);
	virtual ~COPCServerObjectImpl(void);

	/// Set server name ( e.g. "OPCWorkshop.DemoServer" )
	void setServerProgID(LPCTSTR progID);

	/// set CLSID of server 
	void setServerCLSID(REFCLSID progID);

	/// Register a server in system
	int RegisterServer();

	/// Unregister a server in system
	int UnregisterServer();

	/// To start a server
	int StartServer(OPCSERVERSTATE state = OPC_STATUS_NOCONFIG);

	/// To stop process of a server
	int StopServer();

	/// 
	void SetServerState(OPCSERVERSTATE state);

	/// to specify to a server a divider in names of parameters 
	///   by default it is dot.
	void SetDelimeter( const char *ch );

	/// add tag to server database
	///		return internal handle value or -1 if its parametr already exist 
	int AddTag( LPCTSTR name, VARTYPE type, bool readOnly = true);

	/// Set parametr value
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, const VARIANT &value );
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, int value );
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, float value );
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, double value );
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, int* value , unsigned count );
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, float* value, unsigned count );
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, double* value, unsigned count );

	/// Set parametr value
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, const VARIANT &value );
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, int value );
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, float value );
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, double value );
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, int* value , unsigned count );
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, float* value, unsigned count );
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, double* value, unsigned count );

	/// To specify to a server the interface on which to inform on change of parameters of base
	void setDataReceiver( COPCReceiveData *receiver);

public:
	// constants
	virtual WORD QualityGood(void);
	virtual WORD QualityBad(void);
	virtual FILETIME FILETIME_NULL(void);

};

