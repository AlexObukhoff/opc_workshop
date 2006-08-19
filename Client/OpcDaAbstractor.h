#pragma once 

#include "task.h"

namespace AG_OpcDA 
{
//class XFRImportItem;

#pragma pack( push, before_include1,1 )

class Item
{
public:
   CString		name;
   CComVariant	value;
   WORD			quality;
   OPCHANDLE	hServerHandle;
   OPCHANDLE	hClientHandle;

   FILETIME		time;
   VARTYPE		type;

//   XFR_File::XFRImportItem *import_item;

   Item() 
   {
	   type = VT_EMPTY;
	   quality = 0;
	   hServerHandle = 0;
	   hClientHandle = 0;
	   ZeroMemory(&time, sizeof(time));
//	   import_item = NULL;
   }
   Item( const Item& it )
   {
		name =			it.name;
		value =			it.value;
		quality =		it.quality;
		hServerHandle = it.hServerHandle;
		hClientHandle = it.hClientHandle;
		time =			it.time; 
//		import_item =	it.import_item;
		type =			it.type;
   }
};

#pragma pack( pop, before_include1 )


class CItemList : 
	public std::vector<Item*>
{
public:

	CItemList() 
	{ 
		/// заполняем нулевой индекс
		push_back(NULL);
	}
};

//// оператор сравнения для поиска в списке итемов
//inline bool operator ==( const Item* it, const char *str)
//{
//	if( !it ) return false;
//	return ( it->name == str );
//}

class COpcDaAbstractor
{
public:
	/// список элементов, на которые подписан клиент
	CItemList	m_items;
	std::map<OPCHANDLE, Item*> m_ItemsMap;
	/// критическая секция защищающая m_items
	mutex_t		m_Sect;


	DWORD m_ItemsAdded;

	COpcDaAbstractor() :  m_ItemsAdded(0)
	{
	}

	virtual ~COpcDaAbstractor()
	{
		CleanDaAbstractor();
	}
public:

	virtual Item* _findItem(LPCTSTR NameOPC)
	{
		if(!NameOPC)
			return NULL;

		CItemList::iterator it; // = find( m_items.begin(), m_items.end(), NameOPC );

		critical_section locker(m_Sect);

		//if( it != m_items.end() ) 
		//	return *it;
		for( it = m_items.begin(); it != m_items.end(); ++it )
		{
			if( *it != NULL )
				if( (*it)->name == NameOPC ) 
					return *it;
		}

		return NULL;
	}

public:
	/// добавить новый тараметр в таблицу 
	OPCHANDLE addNewItem( Item *item )
	{
		if( item == NULL )
			return 0;

		critical_section locker(m_Sect);
		m_items.push_back( item );  // store this item in the item list

		if( item->hClientHandle == 0 ) 
			item->hClientHandle = (OPCHANDLE)(m_items.size() - 1);
		m_ItemsMap[ item->hClientHandle ] = item;

		return item->hClientHandle;
	}

	/// получить параметр по его хендлу 
	Item* getItemByClientHandle(OPCHANDLE h)
	{
		critical_section locker(m_Sect);
		return m_ItemsMap[ h ];
		//if( h >= m_items.size() )
		//	return NULL; // некорректный параметр
		//
		//return m_items[h];
	}

	size_t ItemsCount()
	{
		critical_section locker(m_Sect);
		return m_items.size() - 1;
	}

	void freeItem( OPCHANDLE h )
	{
		critical_section locker(m_Sect);
		if( h >= m_items.size() )
			return ; // некорректный параметр

		Item *it = m_items[h];
		if( it != NULL ) {
			delete it;
		}

		// Add by KDB 15.01.06: remove element from arrays

		m_items.erase(m_items.begin() + h);

		m_ItemsMap[ h ] = NULL;
		std::map<OPCHANDLE, AG_OpcDA::Item *>::iterator f = m_ItemsMap.find(h);
		if (f != m_ItemsMap.end()) {
			m_ItemsMap.erase(f);
		}
	}

	/// очищает все внутренние структуры класса 
	void CleanDaAbstractor()
	{
		for( size_t i = 0; i < m_items.size(); ++i )
		{
			Item *it = m_items[i];
			if( it != NULL )
			{
				delete it;
				m_items[i] = NULL;
			}
		}
		m_items.erase( m_items.begin(), m_items.end() );
		m_ItemsMap.erase( m_ItemsMap.begin(), m_ItemsMap.end() );
	}
};

};