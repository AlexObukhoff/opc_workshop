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
#include "stdafx.h"
//#include "resource.h"       // main symbols

#include "ag_thread.h"
#include "NameIndex.h"

#include "opc.h"
#include "opc_Data.h"
#include "opc_impl.h"

#include <fstream>
using namespace std;

namespace opcData {

//////////////////////////////////////////////////////////////////////////////////////////////////
// COPCDataOperator
//////////////////////////////////////////////////////////////////////////////////////////////////

COPCDataOperator::COPCDataOperator()
{
	m_Manager = NULL;
}

COPCDataOperator::~COPCDataOperator()
{
//	ATLASSERT( m_Manager != NULL );
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// COPCDataManager
//////////////////////////////////////////////////////////////////////////////////////////////////

COPCDataManager::COPCDataManager()
{

}

COPCDataManager::~COPCDataManager()
{
	/// free onself from links
	while( m_CustomerList.size() ) 
	{
		COPCDataCustomerList::iterator it = m_CustomerList.begin();
		(*it)->Unsubscribe();
	}
	Shutdown();
}

/// subscribe customer to data flow from manager
bool COPCDataManager::AddCustomer( COPCDataCustomer *cust)
{
thread::CCritSectLocker locker(&m_CustomerSect);
COPCDataCustomerList::iterator it;

	if(cust == NULL) return false;

	for(it = m_CustomerList.begin(); it != m_CustomerList.end(); ++it) {
		if( *it == cust )
			return true;
	}

	m_CustomerList.push_back( cust );
	return true;
}

/// remove customer from subscription list
bool COPCDataManager::RemoveCustomer( COPCDataCustomer *cust)
{
thread::CCritSectLocker locker(&m_CustomerSect);
COPCDataCustomerList::iterator it;
	
	for(it = m_CustomerList.begin(); it != m_CustomerList.end(); ++it) {
		if( *it == cust ) {
			m_CustomerList.erase( it );
			return true;
		}
	}
	return false;
}



/// pass new data pack to manager 
HRESULT COPCDataManager::pushNewData( CAG_ValueVector *vect )
{
	if(vect == NULL) return E_INVALIDARG;

	m_Queue.push( vect );
	// if not running - execute manager thread 
	if(! isRunning() ) 
		Execute();
	// wakeup thread for processing queue
	WakeUp();

	return S_OK;
}

/// pass new data to manager 
HRESULT COPCDataManager::pushNewData( CAG_Value &val )
{
	CAG_ValueVector *vect = new CAG_ValueVector;
	vect->push_back( val );

	m_Queue.push( vect );
	// if not running - execute manager thread 
	if(! isRunning() ) Execute();
	// wakeup thread for processing queue
	WakeUp();

	return S_OK;
}

/// get last value of parametr 
CAG_Value COPCDataManager::getLastValue( DWORD nameID )
{
	CLockWrite locker(m_LastValue);

	return m_LastValue[ nameID ];
}

/// process tags from input queue through filter 
void COPCDataManager::step()
{
	CAG_ValueVector *vect = NULL;
	COPCDataCustomerList::iterator it;

	while((vect = m_Queue.pop()) != NULL) 
	{
		CAG_ValueVector &V = *vect;
		thread::CCritSectLocker locker(&m_CustomerSect);


		try 
		{
			for(size_t i=0;i<V.size();++i)  
			{
				CAG_Value &val = V[i];

				{
					CLockWrite locker(m_LastValue);
					m_LastValue[ val.m_NameId ] = val;
				}

				for( it = m_CustomerList.begin(); it != m_CustomerList.end(); ++it)  
				{
					(*it)->pushData( &val );
				}
			}
		} catch(...) {
			ATLTRACE("Exception: COPCDataManager::step()\n");
		}
		delete vect;
	}

}



//////////////////////////////////////////////////////////////////////////////////////////////////
// COPCDataCustomer
//////////////////////////////////////////////////////////////////////////////////////////////////

COPCDataCustomer::COPCDataCustomer() /*:
	m_FilteredData( MAX_CUSTOMER_BUFFER ) */
{
//	m_FilteredData.reserve( MAX_CUSTOMER_BUFFER );
	m_AcceptAllData = false;
}

COPCDataCustomer::~COPCDataCustomer()
{
	Unsubscribe();
}

bool COPCDataCustomer::Subscribe( COPCDataManager *man )
{
	if(man == NULL) return false;

	if( m_Manager == NULL) {
		if(man->AddCustomer( this )) {
			m_Manager = man;
			return true;
		}
	}
	
	return false;
}

bool COPCDataCustomer::Unsubscribe()
{
	if(m_Manager == NULL) return false;
	if( ! m_Manager->RemoveCustomer( this ) )
		throw(_T("COPCDataCustomer::Unadvise() - error"));
	m_Manager = NULL;
	return true;
}

/// to transfer the subscriber the next parameter 
/// this function should give management as soon as possible		
int COPCDataCustomer::pushData(CAG_Value* adap, bool copy)
{
	if( adap == NULL ) 
		throw( "COPCDataCustomer::pushData(CAG_Value* adap)" );

	/// push date into our queue
	if( copy ) 
		m_Queue.push( new CAG_Value( *adap ) );
	else 
		m_Queue.push( adap );

	// if it wosn`t started - start
	if(! isRunning() ) Execute();
	// wakeup thread for processing date
	WakeUp();
	return 0;
}

/// pass to subscribers vector of date 
int COPCDataCustomer::pushData(CAG_ValueVector* adap)
{
	if( adap == NULL ) 
		throw( "COPCDataCustomer::pushData(CAG_ValueVector* adap)" );

	/// push date into our queue
	m_Queue.push_copy( adap->begin(), adap->end() );

	// if it wosn`t started - start
	if(! isRunning() ) Execute();
	// wakeup thread for processing date
	WakeUp();
	return 0;
}

/// Whether establishes to accept all parameters to the subscriber
void COPCDataCustomer::SetAcceptAll( bool allow )
{
	m_AcceptAllData = allow;
}
/// Reset the list of parameters which the subscriber can accept
void COPCDataCustomer::ResetAcceptList( )
{
	thread::CCritSectLocker locker(&m_AcceptedParamsSect);
	m_AcceptedParams.erase( m_AcceptedParams.begin(), m_AcceptedParams.end() );
}
/// add accepting parametr
void COPCDataCustomer::AcceptParam( OPCHANDLE h )
{
	thread::CCritSectLocker locker(&m_AcceptedParamsSect);
	m_AcceptedParams.insert( h );
}

/// remove accepting paramrt
void COPCDataCustomer::DeclineParam( OPCHANDLE h )
{
	thread::CCritSectLocker locker(&m_AcceptedParamsSect);
	m_AcceptedParams.erase( h );
}

/// check parametr (it in accepting list)
bool COPCDataCustomer::isAcceptedParam( OPCHANDLE h )
{
	thread::CCritSectLocker locker(&m_AcceptedParamsSect);
	return m_AcceptedParams.find( h ) != m_AcceptedParams.end();
}

/// get last value of parametr 
bool COPCDataCustomer::getLastValue( OPCHANDLE h, CAG_Value& value )
{
	CLockRead locker( m_LastValues );
	value = m_LastValues[h];
	return true;
}


/// Pushs value in final queue for sending to the client
void COPCDataCustomer::PushToClient( CAG_Value* adapt )
{
#if USE_NEW_DATA_FILTER == 0
	thread::CCritSectLocker locker(m_FilteredDataSect);
		m_FilteredData[adapt->m_NameId] = *adapt;

	CLockWrite lw(m_LastValues);
		m_LastValues[adapt->m_NameId] = *adapt;
#else
	thread::CCritSectLocker locker(m_FilteredDataSect);
	CLockWrite lw(m_LastValues);
	if( m_LastValues[adapt->m_NameId] != *adapt ) 
	{
		m_FilteredData[adapt->m_NameId] = *adapt;
		m_LastValues[adapt->m_NameId] = *adapt;
	}
#endif
}

/// process tags from input queue through filter to internal queue
void COPCDataCustomer::step()
{
	CAG_Value* adapt = NULL;

	while( (adapt = m_Queue.pop()) != NULL ) {
		if( m_AcceptAllData ) 
		{
			PushToClient( adapt );
		}
		else {
			thread::CCritSectLocker locker(&m_AcceptedParamsSect);
			if( m_AcceptedParams.find( adapt->m_NameId ) != m_AcceptedParams.end()) 
				PushToClient( adapt );
			//	thread::CCritSectLocker locker(m_FilteredDataSect);
			//		m_FilteredData[adapt->m_NameId] = *adapt;

			//	CLockWrite lw(m_LastValues);
			//		m_LastValues[adapt->m_NameId] = *adapt;
			//}
		}
		delete adapt;
	}
}

/*
//////////////////////////////////////////////////////////////////////////////////////////////////
/// data source class
//////////////////////////////////////////////////////////////////////////////////////////////////

COPCDataSource::COPCDataSource()
{
	m_Manager = NULL;
}

bool COPCDataSource::Subscribe( COPCDataManager *man )
{
	if(man == NULL) return false;

	if( m_Manager == NULL) {
		m_Manager = man;
		return true;
	}
	
	return false;
}
bool COPCDataSource::Unsubscribe()
{
	if(m_Manager == NULL) return false;
	m_Manager = NULL;
	return true;
}
*/
};

