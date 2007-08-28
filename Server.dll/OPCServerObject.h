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

#ifdef _USRDLL
	#define ExportClass   __declspec( dllexport )
#else
	#define ExportClass   __declspec( dllimport )
#endif

#include "opcDataReceiver.h"
#include "opcda.h"

/*!
	Интерфейсный класс для работы с OPC сервером 
	все функции возвращают 0 если нет ошибок
*/
class COPCServerObject
{
public:
	/// Set server name ( e.g. "OPCWorkshop.DemoServer" )
	virtual void setServerProgID(LPCTSTR progID) = 0;

	/// set CLSID of server 
	virtual void setServerCLSID(REFCLSID progID) = 0;

	/// Register a server in system
	virtual int RegisterServer(void) = 0;

	/// Unregister a server in system
	virtual int UnregisterServer(void) = 0;

    /// To start a server
	virtual int StartServer(OPCSERVERSTATE state = OPC_STATUS_NOCONFIG) = 0;
	
    /// To stop process of a server
	virtual int StopServer(void) = 0;

	/// 
	virtual void SetServerState(OPCSERVERSTATE state) = 0;

	/// to specify to a server a divider in names of parameters 
	///   by default it is dot.
	virtual void SetDelimeter( const char *ch ) = 0;

	/// добавить тег в базу сервера
	///		при этом указывается тип тега и возможность клиенту писать в этот параметр
	///     Возвращаемое значение - внутренний хэндл или -1 в случае, если такой тэг уже существует
	virtual int AddTag( LPCTSTR name, VARTYPE type, bool readOnly = true) = 0;

	/// Set parametr value
	virtual bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, const VARIANT &value ) = 0;
	virtual bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, int value ) = 0;
	virtual bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, float value ) = 0;
	virtual bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, double value ) = 0;
	virtual bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, int* value , unsigned count ) = 0;
	virtual bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, float* value, unsigned count ) = 0;
	virtual bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, double* value, unsigned count ) = 0;

	/// Set parametr value
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, const VARIANT &value ) = 0;
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, int value ) = 0;
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, float value ) = 0;
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, double value ) = 0;
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, int* value , unsigned count ) = 0;
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, float* value, unsigned count ) = 0;
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, double* value, unsigned count ) = 0;

	/// To specify to a server the interface on which to inform on change of parameters of base
	virtual void setDataReceiver( COPCReceiveData *receiver) = 0;

public:
	// constants
	virtual WORD QualityGood(void) = 0;
	virtual WORD QualityBad(void) = 0;
	virtual FILETIME FILETIME_NULL(void) = 0;
};

extern "C" 
{
	/*! Class fabric for OPCServer objects */
	ExportClass COPCServerObject* WINAPI CreateNewOPCServerObject(void);

	/*! Destroyer for OPCServer objects */
	ExportClass int WINAPI DestroyOPCServerObject(COPCServerObject* server);
}
