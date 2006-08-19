/*
* OPCDataCallback.CPP
*/
#include "StdAfx.h"
#include "Client.h"
#include "OpcDataReceiver.h"
#include "OPCDataCallback.h"

COPCDataCallback::COPCDataCallback()
{
	receiver = NULL;
	m_cRef = 0;
	return;
}

COPCDataCallback::~COPCDataCallback(void)
{
	receiver = NULL;
	return;
}


STDMETHODIMP COPCDataCallback::QueryInterface(REFIID riid, void** ppv)
{
	*ppv=NULL;

	if (IID_IUnknown==riid || IID_IOPCDataCallback==riid)
		*ppv=this;

	if (NULL!=*ppv)
	{
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}

	return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) COPCDataCallback::AddRef(void)
{
	return ++m_cRef;
}


STDMETHODIMP_(ULONG) COPCDataCallback::Release(void)
{
	if (0!=--m_cRef)
		return m_cRef;

	delete this;
	return 0;
}


HRESULT  COPCDataCallback::writeParams( DWORD dwCount, 
									   OPCHANDLE __RPC_FAR *phClientItems,
									   VARIANT __RPC_FAR *pvValues,
									   WORD __RPC_FAR *pwQualities,
									   FILETIME __RPC_FAR *pftTimeStamps,
									   HRESULT __RPC_FAR * /*pErrors*/)
{
	if( receiver == NULL ) 
		return S_OK;

	try {
		for (DWORD i = 0; i < dwCount; i++) 
			receiver->newData( NULL, phClientItems[i], pftTimeStamps[i], pvValues[i], pwQualities[i] );

		receiver->newItemIsReceived( dwCount );

	} catch( CAtlException &e ) {
		receiver->log_message( LogError, "EXCEPTION: COPCDataCallback::OnDataChange HRESULT = %d", e.m_hr );
	}
	catch(...)  {
		receiver->log_message( LogError, "EXCEPTION: COPCDataCallback::OnDataChange" );
	}
	return S_OK;
}



STDMETHODIMP COPCDataCallback::OnDataChange( 
	/* [in] */ DWORD /*dwTransid*/,
	/* [in] */ OPCHANDLE /*hGroup*/,
	/* [in] */ HRESULT /*hrMasterquality*/,
	/* [in] */ HRESULT /*hrMastererror*/,
	/* [in] */ DWORD dwCount,
	/* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
	/* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
	/* [size_is][in] */ WORD __RPC_FAR *pwQualities,
	/* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
	/* [size_is][in] */ HRESULT __RPC_FAR *pErrors)
{
	return writeParams( dwCount,  phClientItems, pvValues, pwQualities, pftTimeStamps, pErrors );
}

STDMETHODIMP COPCDataCallback::OnReadComplete( 
	/* [in] */ DWORD /*dwTransid*/,
	/* [in] */ OPCHANDLE /*hGroup*/,
	/* [in] */ HRESULT /*hrMasterquality*/,
	/* [in] */ HRESULT /*hrMastererror*/,
	/* [in] */ DWORD dwCount,
	/* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
	/* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
	/* [size_is][in] */ WORD __RPC_FAR *pwQualities,
	/* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
	/* [size_is][in] */ HRESULT __RPC_FAR *pErrors)
{
	return writeParams( dwCount,  phClientItems, pvValues, pwQualities, pftTimeStamps, pErrors );
}

STDMETHODIMP COPCDataCallback::OnWriteComplete( 
	/* [in] */ DWORD /*dwTransid*/,
	/* [in] */ OPCHANDLE /*hGroup*/,
	/* [in] */ HRESULT /*hrMastererr*/,
	/* [in] */ DWORD /*dwCount*/,
	/* [size_is][in] */ OPCHANDLE __RPC_FAR * /*pClienthandles*/,
	/* [size_is][in] */ HRESULT __RPC_FAR * /*pErrors*/)
{
	ATLTRACE(ATL::atlTraceNotImpl, 0, _T("ATL: %s not implemented.\n"), __FUNCTION__ );
	return S_OK;
}

STDMETHODIMP COPCDataCallback::OnCancelComplete( 
	/* [in] */ DWORD dwTransid,
	/* [in] */ OPCHANDLE hGroup)
{
	ATLTRACE(ATL::atlTraceNotImpl, 0, _T("ATL: %s not implemented.\n"), __FUNCTION__ );
	return S_OK;
}
