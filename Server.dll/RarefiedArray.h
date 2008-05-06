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
//
// RarefiedArray.h 
//
//  Rarefied array foe memory economy. Maybe must use std::map, but it not always comfortable
// 
template <typename T>
class CRarefiedArray :
	public CRWMonitor
{
	typedef std::map<DWORD,T*> TArray;
	TArray m_vector;
	typedef std::pair<DWORD,T*> pair;
public:
	CRarefiedArray()
	{
	}
	~CRarefiedArray()
	{
		TArray::iterator it;
		for (it = m_vector.begin(); it!= m_vector.end(); ++it)
		{
			if (it->second != NULL)
			{
				delete it->second;
				it->second = NULL;
			}
		}
	}

	T& operator []( DWORD index )
	{
		T* val = NULL;
		//TODO after rewrite with helps of reserve() & operator[]
		//while( index >= m_vector.size() )
		//	m_vector[index] = .insert( std::[pair NULL );
		TArray::iterator it = m_vector.find( index );
		if( it == m_vector.end() ) { // добавляем новый 
			val = new T();
			m_vector.insert( pair( index, val ) );
		}
		else
			val = (*it).second;

		return *val;
	}

	void release( DWORD index )
	{
		iterator it = m_vector.find(index);
		if( it == m_vector.end() ) return;
		T* val = (*it).second;
		m_vector.erase( it );
		delete val;
	}

	T* get_pointer( DWORD index )
	{
		if( m_vector.find(index) == m_vector.end() )
			return NULL;
//		if( index >= m_vector.size() ) return NULL;
		return m_vector[index];
	}
};

