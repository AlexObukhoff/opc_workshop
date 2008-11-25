// LockedQueue.h: interface for the CLockedQueue class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
//#include "resource.h"

#include "../include/opcErr.h"

#include <deque>

#define QUEUE_RESULT_TIMEOUT 5000

template <class T>
class CLockedQueue  
{
	BOOL faiked;
protected:
	mutex_t mtx;
	std::deque<T> m_queue;

	HANDLE wait_obj;
	HANDLE wait_result;
	BOOL &SyncMode;

	void init()
	{
		faiked = FALSE;
		wait_obj = CreateEvent(NULL, FALSE, FALSE, NULL);

		if (wait_obj == NULL) {
			throw opcError("CWorkerThread:: Event creation error");
		}

		wait_result = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (wait_result == NULL) {
			throw opcError("CWorkerThread:: Result Event creation error");
		}	}
public:

	CLockedQueue() : SyncMode(faiked)
	{
		init();
	}

	CLockedQueue(BOOL &sync) : SyncMode(sync)
	{
		init();
	}

	virtual ~CLockedQueue() 
	{
		if (wait_obj != NULL) {
			CloseHandle(wait_obj);
			wait_obj = NULL;
		}

		if (wait_result != NULL) {
			CloseHandle(wait_result);
			wait_result = NULL;
		}
	}

	void push_back(T &e) {
		{
			critical_section cs(mtx);

			ResetEvent(wait_result);
			m_queue.push_back(e);
			SetEvent(wait_obj);
		}

		// not goog, but use dependence from global configuration ARMImport
		if ( SyncMode ) 
		{
			DWORD ret = WaitForSingleObject(wait_result, QUEUE_RESULT_TIMEOUT);

			switch (ret) {
				case WAIT_OBJECT_0:
					// all ok
					break;
				case WAIT_TIMEOUT:
					opcError( "CWorkerThread:: Result Event timeout ... ");
					break;
				default:
					opcError( "Error running command: %d - %d", ret, GetLastError() );
					break;
			}
		}
	}

	void ResultDone()
	{
		SetEvent(wait_result);
	}

	T pop (void) {
		critical_section cs(mtx);
	
		if (m_queue.size() > 0) {
			T ret_val = m_queue.front();
			m_queue.pop_front();
			return ret_val;
		} else {
			// KDB 30.12.2005: its a normal practic, in loop extraction and process data as 
			//  aomic operation, with out intermediate removal blocking.
			//  But this code only working with pointer data types
			/*
				For example if T=CDataBlock *
				
				CDataBlock *b = NULL;
				while (b = queue.pop()) {
					... 
				}
				// Queue clean, working next
			*/
			return T(NULL);
//			throw opcError( "CLockedQueue - not possible pop from empty queue" );
		}
	}

	HANDLE WaitObj() {
		return wait_obj;
	}
	HANDLE WaitResult() {
		return wait_result;
	}

	size_t Size() 
	{
		critical_section cs(mtx);
		return m_queue.size();
	}

	void clear()
	{
		critical_section cs(mtx);
		m_queue.empty();
	}
};

