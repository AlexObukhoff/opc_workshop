#pragma once
#include "opcda.h"

class ItemsInGroup;
class CItemForBrowse;

class ATL_NO_VTABLE CIEnumOPCItemAttributes :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CIEnumOPCItemAttributes>,
	public IEnumOPCItemAttributes
{

	std::vector<OPCITEMATTRIBUTES*> items;
	size_t cur_index;
public:

	BEGIN_COM_MAP(CIEnumOPCItemAttributes)
		COM_INTERFACE_ENTRY(IEnumOPCItemAttributes)
	END_COM_MAP()

	CIEnumOPCItemAttributes(void);
	virtual ~CIEnumOPCItemAttributes(void);

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		cur_index = 0;
		return S_OK;
	}

	void FinalRelease();

	virtual HRESULT STDMETHODCALLTYPE Next( 
		/* [in] */ ULONG celt,
		/* [size_is][size_is][out] */ OPCITEMATTRIBUTES **ppItemArray,
		/* [out] */ ULONG *pceltFetched);

	virtual HRESULT STDMETHODCALLTYPE Skip( 
	/* [in] */ ULONG celt);

	virtual HRESULT STDMETHODCALLTYPE Reset( void);

	virtual HRESULT STDMETHODCALLTYPE Clone( 
		/* [out] */ IEnumOPCItemAttributes **ppEnumItemAttributes);

	void AddItem( OPCHANDLE first,  ItemsInGroup* i, CItemForBrowse &brI );
private:
	OPCITEMATTRIBUTES *Copy( OPCITEMATTRIBUTES *i );
};
