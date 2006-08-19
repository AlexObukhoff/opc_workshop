#include "StdAfx.h"

#include "Client.h"

#include "OPCDataCallback.h"
#include "OPCDataReceiver.h"

#include "OpcClient.h"

#include <sstream>


BrowseItems::BrowseItems()
{
	client = NULL;
}

BrowseItems::BrowseItems( const BrowseItems& b)
{
	*this = b;
}

BrowseItems::BrowseItems( OPCClient *cl, string name ) : client(cl)
{
	Browse( name );
}

const BrowseItems& BrowseItems::operator =( const BrowseItems& b)
{
	client = b.client;
	items.insert( items.end(), b.items.begin(), b.items.end() );
	browsePosition = b.browsePosition;
	return *this;
}


void BrowseItems::Browse( string name )
{
//	BrowseItems it(client);
	browsePosition = /*browsePosition + "." +*/ name;

	IOPCServer *srv = client->GetServer();
	if( srv != NULL ) 
	{
		CComPtr<IOPCBrowseServerAddressSpace> browse;
		srv->QueryInterface(&browse);
		if( browse != NULL )
			Browse( *this, browse, name );
		else {
			CComPtr<IOPCBrowse> browse;
			srv->QueryInterface(&browse);
			if( browse != NULL ) 
				Browse( *this, browse, name );
			else
				; // TODO - error info - server dosnt support browse interfaces 
		}

	}
//	return it;
}


void BrowseItems::Browse( BrowseItems &it, IOPCBrowseServerAddressSpace* browse, string name )
{
	OPCNAMESPACETYPE nmType = OPC_NS_HIERARCHIAL;
	browse->QueryOrganization( &nmType );
	if( nmType == OPC_NS_FLAT ) {
		CComPtr<IEnumString> strs;
		browse->BrowseOPCItemIDs( OPC_FLAT, L"", VT_EMPTY, OPC_READABLE | OPC_WRITEABLE, &strs );
		strs->Reset();
		LPOLESTR str = NULL;
		ULONG fetched = 0;
		while( strs->Next( 1, &str, &fetched ) == S_OK && fetched == 1 ) {
			Item i;
			i.flag = OPC_BROWSE_ISITEM;
			i.name = CW2A( str );
			LPOLESTR ItemId = NULL;
			browse->GetItemID( str, &ItemId );
			i.itemId = CW2A(ItemId);
			it.items.push_back( i );
		}
	}
	else if( nmType == OPC_NS_HIERARCHIAL ) 
	{
		browse->ChangeBrowsePosition( OPC_BROWSE_TO, CA2W(it.browsePosition.c_str()) );

		CComPtr<IEnumString> strs;
		browse->BrowseOPCItemIDs( OPC_BRANCH, L"", VT_EMPTY, OPC_READABLE | OPC_WRITEABLE, &strs );
		strs->Reset();
		LPOLESTR str = NULL;
		ULONG fetched = 0;
		while( strs->Next( 1, &str, &fetched ) == S_OK && fetched == 1 ) {
			Item i;
			i.flag = OPC_BROWSE_HASCHILDREN;
			i.name = CW2A( str );
			LPOLESTR ItemId = NULL;
			browse->GetItemID( str, &ItemId );
			i.itemId = CW2A(ItemId);
			it.items.push_back( i );
			CoTaskMemFree( str );
			str = NULL;
		}
		strs.Release();

		browse->BrowseOPCItemIDs( OPC_LEAF, L"", VT_EMPTY, OPC_READABLE | OPC_WRITEABLE, &strs );
		strs->Reset();
		while( strs->Next( 1, &str, &fetched ) == S_OK && fetched == 1 ) {
			Item i;
			i.flag = OPC_BROWSE_ISITEM;
			i.name = CW2A( str );
			LPOLESTR ItemId = NULL;
			browse->GetItemID( str, &ItemId );
			i.itemId = CW2A(ItemId);
			it.items.push_back( i );
			CoTaskMemFree( str );
			str = NULL;
		}
		strs.Release();

	}
}

void BrowseItems::Browse( BrowseItems &it, IOPCBrowse* browse, string name )
{
	// эта функция понадобиться для поддержки OPC DA 3.0 
	if( browse == NULL ) return;

	LPWSTR pszContinuationPoint = NULL;
	OPCBROWSEELEMENT *pBrowseElements = NULL; 
	BOOL moreElements = FALSE;
	DWORD count = 0;
	DWORD pdwPropertyIDs = 2;


	HRESULT hr = browse->Browse( CA2W(name.c_str()), 
			&pszContinuationPoint, 
			0, 
			OPC_BROWSE_FILTER_ALL
			, CA2W("") // [in, string] LPWSTR szElementNameFilter
			, CA2W("") //[in, string] LPWSTR szVendorFilter
			, TRUE //[in] BOOL bReturnAllProperties
			, FALSE //[in] BOOL bReturnPropertyValues
			, 1 //[in] DWORD dwPropertyCount
			, &pdwPropertyIDs //[in, size_is(dwPropertyCount)] DWORD * pdwPropertyIDs
			, &moreElements //[out] BOOL * pbMoreElements
			, &count  //[out] DWORD * pdwCount
			, &pBrowseElements //[out, size_is(,*pdwCount)] OPCBROWSEELEMENT ** ppBrowseElements 
			);

	if( SUCCEEDED(hr) ) {
		for( DWORD i=0;i< count; ++i ) {
			BrowseItems::Item item;
			item.flag = pBrowseElements[i].dwFlagValue;
			item.name = CW2A(pBrowseElements[i].szName);
			item.itemId = CW2A(pBrowseElements[i].szItemID);
			it.items.push_back( item );
		}
		CoTaskMemFree( pBrowseElements );
	}
}

