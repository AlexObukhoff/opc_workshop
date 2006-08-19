#ifndef __TASK_H_LOADED__
#define __TASK_H_LOADED__

class mutex_t 
{ 
    CRITICAL_SECTION cs;
public:
    void lock() { 
		EnterCriticalSection(&cs);
    } 
    void unlock() {
		LeaveCriticalSection(&cs);
    } 
    mutex_t() { 
		InitializeCriticalSection(&cs);
    }   
    ~mutex_t() { 
		DeleteCriticalSection(&cs);
    }
};

class critical_section 
{ 
    mutex_t& mutex;
public:
    critical_section(mutex_t& m) : mutex(m) {
		m.lock();
    }
    ~critical_section() { 
		mutex.unlock();
    }
};

#endif