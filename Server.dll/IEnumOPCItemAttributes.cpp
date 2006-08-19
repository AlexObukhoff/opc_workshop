#include "StdAfx.h"

#include "crEnum.h"
#include "IEnumOpcItemAttributes.h"

#include "ag_thread.h"
#include "opc.h"
#include "opc_Data.h"
#include "opc_impl.h"

#include "OpcGroup.h"


CIEnumOPCItemAttributes::CIEnumOPCItemAttributes(void)
{
	cur_index = 0;
}

void CIEnumOPCItemAttributes::FinalRelease()
{
	for( size_t i=0; i< items.size(); ++i  )
	{
		if( items[i] != NULL ) 
		{
			if( items[i]->szItemID != NULL )
			{
				SysFreeString( items[i]->szItemID );
				items[i]->szItemID = NULL;
			}
			CoTaskMemFree( items[i] );
		}
	}
	items.erase(items.begin(), items.end() );
}

void CIEnumOPCItemAttributes::AddItem( OPCHANDLE first,  ItemsInGroup* i, CItemForBrowse &brI )
{
	OPCITEMATTRIBUTES *oi = (OPCITEMATTRIBUTES *)CoTaskMemAlloc( sizeof(OPCITEMATTRIBUTES) );
	ZeroMemory( oi, sizeof(OPCITEMATTRIBUTES) );

	oi->bActive = i->bActive;
	oi->hClient = i->hClient;
	oi->hServer = first;
	oi->szItemID = brI.name.AllocSysString();
	oi->szAccessPath = SysAllocString(L""); //NULL;
	oi->dwAccessRights = brI.dwAccessRights;
	oi->dwBlobSize = 0;
//	oi->pBlob = NULL;
	oi->vtCanonicalDataType = brI.type;
	oi->vtRequestedDataType = i->type;
//	oi->dwEUType = NULL;

	items.push_back( oi );
}


CIEnumOPCItemAttributes::~CIEnumOPCItemAttributes(void)
{
	FinalRelease();
}

OPCITEMATTRIBUTES *CIEnumOPCItemAttributes::Copy( OPCITEMATTRIBUTES *i )
{
	if( i == NULL )
		return NULL;

	OPCITEMATTRIBUTES *c = (OPCITEMATTRIBUTES *)CoTaskMemAlloc( sizeof(OPCITEMATTRIBUTES) );
	ZeroMemory( c, sizeof(OPCITEMATTRIBUTES) );
	c->bActive = i->bActive;
	c->dwAccessRights = i->dwAccessRights;
	c->dwBlobSize = i->dwBlobSize;
	c->dwEUType = i->dwEUType;
	c->hClient = i->hClient;
	c->hServer = i->hServer;
	c->pBlob = i->pBlob;
	c->szAccessPath = SysAllocString( i->szAccessPath );
	c->szItemID = SysAllocString( i->szItemID );
	VariantCopy(&c->vEUInfo , &i->vEUInfo );
	c->vtCanonicalDataType = i->vtCanonicalDataType;
	c->vtRequestedDataType = i->vtRequestedDataType;
	return c;
}

HRESULT STDMETHODCALLTYPE CIEnumOPCItemAttributes::Next( 
		/* [in] */ ULONG celt,
		/* [size_is][size_is][out] */ OPCITEMATTRIBUTES **ppItemArray,
		/* [out] */ ULONG *pceltFetched)
{
	if( ppItemArray == NULL ) return E_INVALIDARG;
	if( pceltFetched != NULL ) 
		*pceltFetched = 0;
	if (celt == 0)
		return E_INVALIDARG;

	if (ppItemArray == NULL || (celt != 1 && pceltFetched == NULL))
		return E_POINTER;

	for( size_t i = 0; i < celt; ++i )
	{
		if( cur_index >= items.size() ) break;
		ppItemArray[i] = Copy( items[cur_index] );
		if( pceltFetched != NULL ) 
			(*pceltFetched)++;
		cur_index++;
	}

	if( i == celt )
		return S_OK;
	else
		return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CIEnumOPCItemAttributes::Skip( /* [in] */ ULONG celt)
{
	cur_index += celt;

	if( items.size() <= cur_index )
		return S_FALSE;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CIEnumOPCItemAttributes::Reset( void)
{
	cur_index = 0;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CIEnumOPCItemAttributes::Clone( /* [out] */ IEnumOPCItemAttributes **ppEnumItemAttributes)
{
	return E_NOTIMPL;


	//OPCITEMATTRIBUTES oi;
	//ItemsInGroup* i = (*it).second;
	//CItemForBrowse &brI = *g_BrowseItems.find( (*it).first );

	//oi.bActive = i->bActive;
	//oi.hClient = i->hClient;
	//oi.hServer = (*it).first;
	//oi.szItemID = brI.name.AllocSysString();
	//oi.dwAccessRights = brI.dwAccessRights;
	//oi.dwBlobSize = 0;
	//oi.pBlob = NULL;
	//oi.vtCanonicalDataType = brI.type;
	//oi.vtRequestedDataType = i->type;
	//

	//return E_NOTIMPL;
}
