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
 
// Wizard-generated connection point proxy class
// WARNING: This file may be regenerated by the wizard
#pragma once


template<class T>
class CProxy_IOPCDataCallback :
	public IConnectionPointImpl<T, &__uuidof(IOPCDataCallback)>
{
public:

//	std::map<IUnknown*, CComGITPtr<IOPCDataCallback> > GIT_callbacs;

	HRESULT Fire_OnDataChange(
		/*[in]*/ DWORD dwTransid,
		/*[in]*/ OPCHANDLE hGroup,
		/*[in]*/ HRESULT hrMasterquality,
		/*[in]*/ HRESULT hrMastererror,
		/*[in]*/ DWORD dwCount,
		/*[in, sizeis(dwCount)]*/ OPCHANDLE * phClientItems,
		/*[in, sizeis(dwCount)]*/ VARIANT * pvValues,
		/*[in, sizeis(dwCount)]*/ WORD * pwQualities,
		/*[in, sizeis(dwCount)]*/ FILETIME * pftTimeStamps,
		/*[in, sizeis(dwCount)]*/ HRESULT *pErrors )
	{
		HRESULT hr = S_OK;
		T * pThis = static_cast<T *>(this);
		int cConnections = m_vec.GetSize();

		for (int iConnection = 0; iConnection < cConnections; iConnection++)
		{
			pThis->Lock();
			CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
			pThis->Unlock();

			IOPCDataCallback * pConnection = static_cast<IOPCDataCallback *>(punkConnection.p);
			//CComGITPtr<IOPCDataCallback> git( static_cast<IOPCDataCallback *>(punkConnection.p) );
			//CComPtr<IOPCDataCallback> pConnection;
			//hr = git.CopyTo( &pConnection );

			try {
				if (pConnection)
					hr = pConnection->OnDataChange(
								dwTransid,
								hGroup,
								hrMasterquality,
								hrMastererror,
								dwCount,
								phClientItems,
								pvValues,
								pwQualities,
								pftTimeStamps,
								pErrors );
			} catch( ... ) {
				// ������ ��� ������ ��� ����
				// ��������� ��� ��� ����, �� ����� ����� ����� ����� ������
				hr = S_FALSE;
			}
		}
		return hr;
	}

	HRESULT Fire_OnReadComplete(
		/*[in]*/ DWORD dwTransid,
		/*[in]*/ OPCHANDLE hGroup,
		/*[in]*/ HRESULT hrMasterquality,
		/*[in]*/ HRESULT hrMastererror,
		/*[in]*/ DWORD dwCount,
		/*[in, sizeis(dwCount)]*/ OPCHANDLE * phClientItems,
		/*[in, sizeis(dwCount)]*/ VARIANT * pvValues,
		/*[in, sizeis(dwCount)]*/ WORD * pwQualities,
		/*[in, sizeis(dwCount)]*/ FILETIME * pftTimeStamps,
		/*[in, sizeis(dwCount)]*/ HRESULT *pErrors
		)
	{
		HRESULT hr = S_OK;
		T * pThis = static_cast<T *>(this);
		int cConnections = m_vec.GetSize();

		for (int iConnection = 0; iConnection < cConnections; iConnection++)
		{
			pThis->Lock();
			CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
			pThis->Unlock();

			IOPCDataCallback * pConnection = static_cast<IOPCDataCallback *>(punkConnection.p);

			try {
				if (pConnection)
					hr = pConnection->OnReadComplete(dwTransid, hGroup, hrMasterquality,
						hrMastererror, dwCount, phClientItems, pvValues,
						pwQualities,pftTimeStamps, pErrors);
			} catch( ... ) {
				// ������ ��� ������ ��� ����
				// ��������� ��� ��� ����, �� ����� ����� ����� ����� ������
				hr = S_FALSE;
			}
		}
		return hr;
	}

	HRESULT Fire_OnWriteComplete(
		/*[in]*/ DWORD dwTransid,
		/*[in]*/ OPCHANDLE hGroup,
		/*[in]*/ HRESULT hrMasterError,
		/*[in]*/ DWORD dwCount,
		/*[in, sizeis(dwCount)]*/ OPCHANDLE * phClientItems,
		/*[in, sizeis(dwCount)]*/ HRESULT * pError
		)
	{
		HRESULT hr = S_OK;
		T * pThis = static_cast<T *>(this);
		int cConnections = m_vec.GetSize();

		for (int iConnection = 0; iConnection < cConnections; iConnection++)
		{
			pThis->Lock();
			CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
			pThis->Unlock();

			IOPCDataCallback * pConnection = static_cast<IOPCDataCallback *>(punkConnection.p);

			try {
				if (pConnection)
					hr = pConnection->OnWriteComplete( dwTransid, hGroup, hrMasterError, dwCount, phClientItems, pError );
			} catch( ... ) {
				// ������ ��� ������ ��� ����
				// ��������� ��� ��� ����, �� ����� ����� ����� ����� ������
				hr = S_FALSE;
			}
		}
		return hr;
	}

};
