// WorkerThread.cpp: implementation of the COPCWorkerThread class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable : 4786 4100 4512 )
//#pragma warning( disable :  )

#include "StdAfx.h"
#include <Windows.h>

#pragma warning( push )
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#include <ATLComTime.h>
#include <AtlComCli.h>
#include <AtlSync.h>
#pragma warning( pop ) 

#include <assert.h>

#include <vector>
#include <queue>
#include <map>
#include <sstream>
using namespace std;

#include "WorkerThread.h"
#include "opcErr.h"

//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
//#endif

void COPCDataImportEvent::Value(CComVariant value)
{
	param_value = value;
	//switch (value.vt) {
	//case VT_R4:
	//	param_value = value.fltVal;
	//	break;
	//case VT_R8:
	//	param_value = value.dblVal;
	//	break;
	//case VT_I2:
	//case VT_I4:
	//	param_value = (double)value.intVal;
	//	break;
	//default:
	//	throw opcError( "Неправильный тип параметра. тип %d ", value.vt );
	//}
}

DWORD WINAPI WorkerThreadRoutine(void *param)
{
	COPCWorkerThread *thr = (COPCWorkerThread *)param;

	assert (thr != NULL);

	return thr->ProcessThread();	
}

COPCWorkerThread::~COPCWorkerThread()
{
	if (evt_started != NULL)
		CloseHandle(evt_started);

	if (evt_finish != NULL)
		CloseHandle(evt_finish);

	running = false;
	evt_started = NULL;
	evt_finish = NULL;
	thr_hdl = NULL;
}

bool COPCWorkerThread::StartThread(unsigned timeout)
{	
	evt_started = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (evt_started == NULL) {
		DWORD d = GetLastError();
		throw opcError( "COPCWorkerThread::Ошибка создания Event %d", d);
//		return false;
	}
	evt_finish = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (evt_finish == NULL) {
		DWORD d = GetLastError();
		throw opcError( "COPCWorkerThread::Ошибка создания Event", d);
//		return false;
	}

	if (!DoPreThreadTask()) {
		return false;
	}

	DWORD thread_id;

	thr_hdl = CreateThread( NULL, 0, WorkerThreadRoutine, this, 0, &thread_id);

	if (thr_hdl == NULL) {
		DWORD d = GetLastError();
		throw opcError( "Error %d creating thread", d);
//		return false;
	}

	DWORD ret = WaitForSingleObject(evt_started, timeout);
	if (ret == WAIT_OBJECT_0) {
		return true;
	}

	switch (ret){
		case WAIT_FAILED:
			{
				DWORD d = GetLastError();
				throw opcError( "Error %d creating thread", d);
			}
		case WAIT_TIMEOUT:
			throw opcError( "The time-out interval elapsed for creating thread (%d msec)", timeout);
		case WAIT_ABANDONED:
			throw opcError( "Wait creating thread abandoned.");
	}
	return false;
}

void COPCWorkerThread::StopThread(unsigned timeout)
{
	if (!running) {
		return;
	}

	SetEvent(evt_finish);

	DWORD res = WaitForSingleObject(thr_hdl, timeout);

	if (res != WAIT_OBJECT_0) {
		// We have some problem(s) stopping a thread

//		DWORD d = GetLastError();
//		throw opcError( "Ошибка %d остановки потока", d);

		res = TerminateThread(thr_hdl, 1);

		if (res != 0) {
			DWORD d = GetLastError();

			throw opcError( "Ошибка %d аварийной остановки потока", d);

		}
	}

	CloseHandle(thr_hdl);
}

unsigned COPCWorkerThread::ProcessThread(void)
{
unsigned ret_code = 777;

	DoPostThreadTask();

	// Signal to our parent that we are ready

	SetEvent(evt_started);

	// Compose waiters array
	vector<HANDLE> waiters;
	waiters.push_back( evt_finish );
	//int nwaiters = m_waiters.size() + 1;
	//HANDLE * waiters = new HANDLE[nwaiters];

	//waiters[0] = evt_finish;

	for (size_t i = 0; i < m_waiters.size(); i++)
		waiters.push_back( m_waiters[i] );

	for (;;) 
	{
		DWORD wait_res = WaitForMultipleObjects(waiters.size(), &waiters[0], FALSE, wait_timeout);

		if (wait_res == WAIT_OBJECT_0) {
			// We are asked to exit
			ret_code = 1111;
			break;
		} else if (wait_res >= WAIT_OBJECT_0 + 1 && wait_res < WAIT_OBJECT_0 + waiters.size() ) {
			// User-defined object fired

			OnWorkerEvent(wait_res - WAIT_OBJECT_0 - 1);
		} else if (wait_res >= WAIT_ABANDONED_0  && wait_res < WAIT_ABANDONED_0 + waiters.size() ) {
			// Wait abandoned - log the event, finish our task with error !!!!!!!!!


		} else if (wait_res == WAIT_TIMEOUT) {
			OnTimeout();
		} else if  (wait_res == WAIT_FAILED) {
			// Some error occured - log maximum of it, and exit thread

			DWORD d = GetLastError();

			DoStopThreadTask();
			throw opcError( "Ошибка %d ожидания события", d);
			//ret_code = 1112;
			//break;
		} else {
			// MSDN has an error - this should not happen
			DoStopThreadTask();
			throw opcError( "MSDN содержит ошибку в части WaitForMultipleObjects");
			//ret_code = 1113;
			//break;
		}
	}

	DoStopThreadTask();
//	waiters;
	return ret_code;
}

string COPCWorkerThread::DumpStatus()
{
	stringstream ss;

	ss << "\t Thread status: " << (running ? "RUNNING" : "STOPPED" ) << endl;

	return ss.str();
}
