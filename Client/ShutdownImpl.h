#pragma once

#include "../include/opcDataReceiver.h"
#include "../include/opccomn.h"
#include <string>
/*
*     Реализация интерфейса событий IOPCShutdown для OPC клиента 
*
*/

class COPCShutdownImpl :
	public IOPCShutdown
{
	CComPtr<IConnectionPoint> point;
	COPCReceiveData **m_res;

public:
	DWORD m_Cookie;
	std::string m_Reason;

	COPCShutdownImpl ( COPCReceiveData **res ) :
	  m_res( res )
	{
		m_Cookie = NULL;
		m_dwRef = 0;
	}

	HRESULT Advise( IUnknown *unk )
	{
		if( m_Cookie || point ) return E_FAIL;

		CComQIPtr<IConnectionPointContainer> container = unk;
		if( container ) {
			container->FindConnectionPoint( __uuidof( IOPCShutdown) , &point );
			if( point ) {
				//if( *res != NULL )
				//	m_hand.Reset();
				return point->Advise( GetUnknown(), &m_Cookie );
			}
		}

		return E_FAIL;
	}
	void Unadvise() 
	{
		try {
			if( point ) {
				point->Unadvise( m_Cookie );
				m_Cookie = NULL;
			}
			point.Release();
		} catch(...) { }
		point.p = NULL;
	}

	virtual HRESULT STDMETHODCALLTYPE ShutdownRequest ( /*[in]*/ LPCWSTR szReason )
	{
		std::string msg = CW2A(szReason);
		if( *m_res != NULL ) {
			(*m_res)->StatusChanged( -1, msg.c_str() );
		}
		else {
			OutputDebugString("OPCClient ShutdownRequest CALLED !!!\n");
		}
		return S_OK;
	}

///  IUnknown implementation.
	ULONG m_dwRef; /// внутренний счетчик ссылок	
	STDMETHOD(QueryInterface) (REFIID riid, void** ppv)
	{
		*ppv = NULL;

		if(riid == IID_IUnknown) {  
			*ppv = GetUnknown();
		}
		else if(riid == __uuidof(IOPCShutdown) )  
		{ 
				*ppv = dynamic_cast<IOPCShutdown*>(this);  ATLASSERT(*ppv); 
		}
		if (NULL != *ppv)
		{
			/*((LPUNKNOWN)*ppv)*/ 	this->AddRef();
			return S_OK;
		}

		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG,AddRef) (void)
	{
		return ++m_dwRef;
	}
	STDMETHOD_(ULONG,Release) (void)
	{
		if (0 != --m_dwRef)
			return m_dwRef;
		return 0;
	}
	IUnknown* GetUnknown()
	{
		 // может это и не правильно, но работает 
		return dynamic_cast<IUnknown*> ( this );
	}
};
