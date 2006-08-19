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

#include "opcDataReceiver.h"
//**************************************************************************
class COPCDataCallback : public IOPCDataCallback
{
public:
    COPCReceiveData *receiver;

   COPCDataCallback();
   ~COPCDataCallback(void);

   STDMETHODIMP QueryInterface(REFIID, void**);
   STDMETHODIMP_(ULONG) AddRef(void);
   STDMETHODIMP_(ULONG) Release(void);

private:
	HRESULT writeParams( DWORD dwCount, 
		OPCHANDLE __RPC_FAR *phClientItems,
		VARIANT __RPC_FAR *pvValues,
		WORD __RPC_FAR *pwQualities,
		FILETIME __RPC_FAR *pftTimeStamps,
        HRESULT __RPC_FAR *pErrors);

public:
    STDMETHODIMP OnDataChange( 
        /* [in] */ DWORD dwTransid,
        /* [in] */ OPCHANDLE hGroup,
        /* [in] */ HRESULT hrMasterquality,
        /* [in] */ HRESULT hrMastererror,
        /* [in] */ DWORD dwCount,
        /* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
        /* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
        /* [size_is][in] */ WORD __RPC_FAR *pwQualities,
        /* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
        /* [size_is][in] */ HRESULT __RPC_FAR *pErrors);
    
    STDMETHODIMP OnReadComplete( 
        /* [in] */ DWORD dwTransid,
        /* [in] */ OPCHANDLE hGroup,
        /* [in] */ HRESULT hrMasterquality,
        /* [in] */ HRESULT hrMastererror,
        /* [in] */ DWORD dwCount,
        /* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
        /* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
        /* [size_is][in] */ WORD __RPC_FAR *pwQualities,
        /* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
        /* [size_is][in] */ HRESULT __RPC_FAR *pErrors);
    
    STDMETHODIMP OnWriteComplete( 
        /* [in] */ DWORD dwTransid,
        /* [in] */ OPCHANDLE hGroup,
        /* [in] */ HRESULT hrMastererr,
        /* [in] */ DWORD dwCount,
        /* [size_is][in] */ OPCHANDLE __RPC_FAR *pClienthandles,
        /* [size_is][in] */ HRESULT __RPC_FAR *pErrors);
    
    STDMETHODIMP OnCancelComplete( 
        /* [in] */ DWORD dwTransid,
        /* [in] */ OPCHANDLE hGroup);

protected:
   ULONG               m_cRef;
};

