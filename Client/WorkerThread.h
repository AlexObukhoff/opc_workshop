// WorkerThread.h: interface for the CWorkerThread class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "task.h"

#include <vector>
#include <string>
#include <atlcomcli.h>

#define START_THREAD_TIMEOUT	1500

// Класс, управляющий работой в потоке
class COPCWorkerThread
{
protected:
	// Подготовить данные до запуска потока
	virtual bool DoPreThreadTask() = 0;

	// Подготовить данные в потоке после запуска, но до возврата из StartThread
	virtual bool DoPostThreadTask() = 0;

	// 
	virtual bool DoStopThreadTask() = 0;

	virtual void OnTimeout(void) { }
	virtual unsigned ProcessThread(void);

	HANDLE evt_started;
	HANDLE evt_finish;

	HANDLE thr_hdl;

	unsigned wait_timeout;
	// Массив событий, не включает в себя выход из потока
	std::vector<HANDLE> m_waiters;

	bool running;

public:
	virtual std::string DumpStatus();
	COPCWorkerThread(unsigned _wait_timeout = 500) {
		wait_timeout = _wait_timeout;
		running = false;
		evt_started = NULL;
		evt_finish = NULL;
		thr_hdl = NULL;
	}

	virtual ~COPCWorkerThread();

	bool StartThread(unsigned timeout);
	void StopThread(unsigned timeout);

	// Виртуальная функция, вызываемая при поступлении рабочего события
	//  (например, при ожидании )
	virtual void OnWorkerEvent(int event_no) { event_no; }

	friend DWORD WINAPI WorkerThreadRoutine(void *param);

	bool isRunning() { return running; }
};

// Описатель события, возникшего по приходу данных от OPC-сервера
class COPCDataImportEvent
{
public:
	
	enum e_direction { dirInput = 0, dirOutput = 1 };

	bool RepeatLastValue;
	unsigned param_id; // Индекс параметра в таблице параметров

	VARIANT param_value;

	FILETIME stamp; // Нужна большая точность - до долей секунды
	WORD quality;

	unsigned session_id;
	unsigned source_id;

	e_direction direction;

	COPCDataImportEvent(COPCDataImportEvent &ev) {
		param_id = ev.param_id;
		param_value = ev.param_value;
		stamp = ev.stamp;
		RepeatLastValue = ev.RepeatLastValue;
		session_id = ev.session_id;
		source_id = ev.source_id;
		direction = ev.direction;
	}

	COPCDataImportEvent() {
		param_id = 0xFFFFFFFF;
		param_value.vt = VT_EMPTY;

		memset(&stamp, 0, sizeof(stamp));
		RepeatLastValue = false;
		session_id = 0;
		source_id = 0;
		direction = dirInput;
	}

	~COPCDataImportEvent() {	}

	void Value(CComVariant value);
};

