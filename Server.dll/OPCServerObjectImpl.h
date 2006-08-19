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
	Реализация Интерфейсного класса для работы с OPC сервером 
	все функции возвращают 0 если нет ошибок
*/
class COPCServerObjectImpl : public COPCServerObject
{
protected:
	  DWORD m_objid;
public:
	COPCServerObjectImpl(void);
	virtual ~COPCServerObjectImpl(void);

	/// Установить имя объекта сервера
	void setServerProgID(LPCTSTR progID);

	/// установить CLSID сервера 
	void setServerCLSID(REFCLSID progID);

	/// зарегистрировать сервер в системе
	int RegisterServer();

	/// резрегистрировать сервер в системе
	int UnregisterServer();

    /// запустить сервер
	int StartServer(OPCSERVERSTATE state = OPC_STATUS_NOCONFIG);

    /// отстановить процесс сервера
	int StopServer();

	/// 
	void SetServerState(OPCSERVERSTATE state);

	/// указать серверу разделитель в именах параметров
	void SetDelimeter( const char *ch );

	/// добавить тег в базу сервера
	///		при этом указывается тип тега и возможность клиенту писать в этот параметр
	///     Возвращаемое значение - внутренний хэндл или -1 в случае, если такой тэг уже существует
	int AddTag( LPCTSTR name, VARTYPE type, bool readOnly = true);

	/// передать значение параметра
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, const VARIANT &value );
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, int value );
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, float value );
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, double value );
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, int* value , unsigned count );
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, float* value, unsigned count );
	bool WriteValue( LPCTSTR name, FILETIME ft, WORD quality, double* value, unsigned count );

	/// передать значение параметра
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, const VARIANT &value );
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, int value );
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, float value );
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, double value );
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, int* value , unsigned count );
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, float* value, unsigned count );
	virtual bool WriteValue( int hdl, FILETIME ft, WORD quality, double* value, unsigned count );

	/// указать серверу интерфейс по которому сообщать об изменении параметров базы
	void setDataReceiver( COPCReceiveData *receiver);

public:
	// константы 
	virtual WORD QualityGood(void);
	virtual WORD QualityBad(void);
	virtual FILETIME FILETIME_NULL(void);

};

