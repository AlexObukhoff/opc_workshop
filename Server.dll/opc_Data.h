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

#include "Value.h"
#include "RarefiedArray.h"

/*
Ideology: 
	Data sources transfer data in to COPCDataManager::pushNewData.
	COPCDataManager collect data into solid queue. From main queue date assorting and pushing 
	to subscribers (COPCDataCustomer).
	
	Realization COPCDataCustomer allows what parameters will operate to get in internal queue of parameters. 
	(SetAcceptAll, ResetAcceptList, AcceptParam)
*/


#define MAX_CUSTOMER_BUFFER 10000

/*
	Flag responsible for an opportunity of filtering of identical values of parameter if in 
	a server have written down tag with the same value and the status, but other time.
	
	If USE_NEW_DATA_FILTER == 1 such tag it will not be transferred the client, all will 
	be transferred differently that in a server is written down.
*/

#define USE_NEW_DATA_FILTER 1

namespace opcData {

//extern CRarefiedArray<CAG_Value> g_LastValues;

class COPCDataCustomer;
class COPCDataManager;
class COPCDataSource;

typedef std::list<COPCDataCustomer*> COPCDataCustomerList;

/// base abstract class for date sources and customers
class COPCDataOperator
{
public:
	COPCDataManager *m_Manager;

	COPCDataOperator();
	virtual ~COPCDataOperator();

	virtual bool Subscribe( COPCDataManager *man ) = 0;
	virtual bool Unsubscribe() = 0;
};

/// Template: queue of abstract date type (bloking by critical section)
/// date type must be have copy constructor 
template <class T>
class COPCDataQueue //: private CComAutoCriticalSection
{
	/// queue of parametrs 
	CComAutoCriticalSection m_QueueSect;
	std::list<T*> m_Queue;
public:
	/// push to queue (with copying)
	template< class _it >
	void push_copy ( _it begin, _it end )
	{
		thread::CCritSectLocker locker(&m_QueueSect);
		for( _it it = begin; it != end; ++it)
			m_Queue.push_back( new T(*it) );
	}

	/// push to queue
	template< class _it >
	void push ( _it begin, _it end )
	{
		thread::CCritSectLocker locker(&m_QueueSect);
		m_Queue.insert( m_Queue.end(), begin, end );
	}

	/// push to queue one value
	void push ( T* value )
	{
		thread::CCritSectLocker locker(&m_QueueSect);
		m_Queue.push_back( value );
	}

	/// pop for queue
	T* pop()
	{
		thread::CCritSectLocker locker(&m_QueueSect);
		if( m_Queue.size() ) {
			T* vect = *m_Queue.begin();
			m_Queue.pop_front();
			return vect;
		}
		/// if empty returning NULL value (Therefore it is possible to hold only pointers)
		return NULL;
	}

	/// get queue size
	size_t size()
	{
		thread::CCritSectLocker locker(&m_QueueSect);
		return m_Queue.size();
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// The subscriber on given from COPCDataManager
class COPCDataCustomer :
	public COPCDataOperator,
	private thread::CThreadingObject
{
protected:
	/// The attribute, that the receiver accepts all parameters
	bool m_AcceptAllData;
	CComAutoCriticalSection m_AcceptedParamsSect;
	std::set<OPCHANDLE> m_AcceptedParams;

	/// queue of not filtered parameters
	COPCDataQueue<CAG_Value> m_Queue;

	/// Archive of last values of each parameter
	CRarefiedArray<CAG_Value> m_LastValues;
	/// Pushs value in final queue for sending to the client
	void PushToClient( CAG_Value* adapt );
protected:
	/// The filtered parameters.
	std::map<OPCHANDLE,CAG_Value> m_FilteredData;
	/// The critical section protects a vector m_FilteredData
	CComAutoCriticalSection m_FilteredDataSect;


public:
	COPCDataCustomer();
	virtual ~COPCDataCustomer();
	virtual bool Subscribe( COPCDataManager *man );
	virtual bool Unsubscribe();

	enum CustomerType {
		PUSH_DATA_BY_ONE,
		PUSH_DATA_BY_PACK
	};

	/// to transfer the subscriber the next parameter 
	/// this function should give management as soon as possible		
	virtual int pushData(CAG_Value* adap, bool copy = true);

	/// to transfer the subscriber the next pack of parameters 
	/// this function should give management as soon as possible
	virtual int pushData(CAG_ValueVector* adap);


public:

/// Tag filter acessors 

	/// Whether establishes to accept all parameters to the subscriber
	void SetAcceptAll( bool allow = true );
	/// Reset the list of parameters which the subscriber can accept
	void ResetAcceptList( );
	/// add accepting parametr
	void AcceptParam( OPCHANDLE h );
	/// remove accepting paramrt
	void DeclineParam( OPCHANDLE h );
	/// check parametr (it in accepting list)
	bool isAcceptedParam( OPCHANDLE h );

	/// get last value of parametr 
	bool getLastValue( OPCHANDLE h, CAG_Value& value );

protected:
	/// process tags from input queue through filter to internal queue
	virtual void step();
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// source - customer manager 
class COPCDataManager :
	private thread::CThreadingObject
{
	CRarefiedArray<CAG_Value> m_LastValue;

	CComAutoCriticalSection m_CustomerSect;
	COPCDataCustomerList m_CustomerList;

	COPCDataQueue<CAG_ValueVector> m_Queue;
	//CComAutoCriticalSection m_QueueSect;
	//CAG_ValuesQueue m_Queue;
public:
	COPCDataManager();
	virtual ~COPCDataManager();

private:
	friend class COPCDataCustomer;
	/// subscripe customet to the main tag stream 
	bool AddCustomer( COPCDataCustomer *cust);
	/// remove customer subscription
	bool RemoveCustomer( COPCDataCustomer *cust);

public:
	/// push to subscribers new data pack
	/// object 'vect' must be deliver to manager (after processing, manager call delete vect; )
	virtual HRESULT pushNewData( CAG_ValueVector *vect );

	/// push to subscribers new data
	virtual HRESULT pushNewData( CAG_Value &val );

	/// function processing data to subscribers (customers)
	virtual void step();

	/// get last value for tag
	CAG_Value getLastValue( DWORD nameID );
};

/*
//////////////////////////////////////////////////////////////////////////////////////////////////
/// data source class
class COPCDataSource :
	public COPCDataOperator
{
public:
	COPCDataManager *m_Manager;

	COPCDataSource();
	virtual ~COPCDataSource() { Unsubscribe(); }

	virtual bool Subscribe( COPCDataManager *man );
	virtual bool Unsubscribe();
};

*/
}; // namespace opcData;

using namespace opcData;
