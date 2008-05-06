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

/**
 * ag_thread.h
 */
#pragma once

//#include "log.h"

namespace thread {


/// Blocker for critical section 
class CCritSectLocker
{
CComCriticalSection &m_sect;
public:
	CCritSectLocker(CComCriticalSection &sect) : m_sect(sect)
	{
		m_sect.Lock();
	}
	CCritSectLocker(CComCriticalSection *sect) : m_sect(*sect)
	{
		ATLASSERT( sect );
		m_sect.Lock();
	}
	virtual ~CCritSectLocker()
	{
		m_sect.Unlock();
	}
};

/// Class realizing object in a separate thread
class CThreadingObject
{
protected:
	/// thread handle
	HANDLE m_ThreadHandle;
	/// signal for thread to end thread
	HANDLE m_hWantQuit;
	/// signal for thread to wakeup
	HANDLE m_hWakeUp;
	/// critical section for block some code in thread
	CComAutoCriticalSection m_sect;
	/// thread id - indicate for process ending
	volatile DWORD m_threadID;

	/// helper for starting thread 
	static DWORD WINAPI HelperThreadProc(LPVOID p)
	{
		CThreadingObject *obj = static_cast<CThreadingObject*>(p);

		try {
			obj->run();
		} catch(...) {
			ATLTRACE(_T("Exception at %s [%d]\n"),__FILE__,__LINE__);
		}

		CCritSectLocker locker(&(obj->m_sect));
		obj->m_threadID = NULL;
		return 0;
	}

public:

	CThreadingObject() : m_threadID(0), m_ThreadHandle(NULL)
	{
		m_hWakeUp = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_hWantQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	virtual ~CThreadingObject()
	{
		Shutdown();

		CloseHandle(m_hWakeUp); 
		CloseHandle(m_hWantQuit); 
		CloseHandle(m_ThreadHandle);
		m_threadID = 0;
	}

	/*! 
		thread function for process 
		implement by child class 
	*/
	virtual void step() = 0;
	

	/*! 
		main thread function 
	*/
	virtual void run()
	{
		while(1) {
			HANDLE h[2] = {  m_hWakeUp, m_hWantQuit };
			DWORD ret_code = WaitForMultipleObjects( 2, h, FALSE, INFINITE);
			switch(ret_code) {
				case WAIT_OBJECT_0: //  if signalling m_hWakeUp
					try {
						ResetEvent( m_hWakeUp );
						step();
					} catch(...) {
						ATLTRACE(_T("Exception at %s [%d]\n"),__FILE__,__LINE__);
					}
				break;
				default: // to exit thread
					return;
			}
		}
	}

	/// check " thread still running "
	inline bool isRunning()
	{ 
		DWORD th = NULL;
		{
			CCritSectLocker locker(&m_sect);
			th = m_threadID;
		}
		if( th ) {
			FILETIME ft;
			BOOL ret = GetThreadTimes( m_ThreadHandle, &ft, &ft, &ft, &ft );
			return (ret == TRUE);
		}
		return false;
//		return th != NULL; 
	}

	/// execute child thread
	bool Execute(void)
	{
		DWORD threadID;
		
		m_ThreadHandle = CreateThread(NULL, 0, HelperThreadProc, (LPVOID)this, 0, &threadID);
		if(m_ThreadHandle) {
			CCritSectLocker locker(&m_sect);
			m_threadID = threadID;
		}
		return (m_ThreadHandle != NULL);
	}
	
	/// send wakeup signal to thread (for run next step)
	void WakeUp(void) 
	{
		SetEvent( m_hWakeUp );
	}
	/// termination thread working
	void Shutdown(void)
	{
		DWORD count = 0;
		while(isRunning() && ++count < 1000)  
		{
	        SetEvent( m_hWantQuit );
			Sleep(0);
		}
		if( count >= 1000 ) 
		{
			TerminateThread( m_ThreadHandle, 1 );
			CloseHandle( m_ThreadHandle );
			m_ThreadHandle = NULL;
			m_threadID = 0;
		}
	}
};

class CSafeCounterEnter;

/// thread safe counter 
class CSafeCounter 
{
	CComAutoCriticalSection m_sect;
	DWORD m_Value;
public:
	CSafeCounter()
	{
		CCritSectLocker locker(m_sect);
		m_Value = 0;
	}
	operator DWORD()
	{
		CCritSectLocker locker(m_sect);
		return m_Value;
	}
	DWORD inc()
	{
		CCritSectLocker locker(m_sect);
		return ++m_Value;
	}
	DWORD release()
	{
		CCritSectLocker locker(m_sect);
		return --m_Value;
	}
};

class CSafeCounterEnter
{
	CSafeCounter &m_count;
public:
	CSafeCounterEnter(CSafeCounter &count) : m_count(count)
	{
		m_count.inc();
	}
	~CSafeCounterEnter()
	{
		m_count.release();
	}
};



} // namespace thread

//=============================================================================
