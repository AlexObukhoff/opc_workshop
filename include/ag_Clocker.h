#pragma once

#include <time.h>
//#include "ag_File.h"


class CAG_Clocker
{
	CString name;
	clock_t start_clock;
	bool m_toFile;
	bool m_Stop;
public:
	void start()
	{
		start_clock = clock();
		m_Stop = false;
	}
	double stop()
	{
		m_Stop = true;
		clock_t finish_clock = clock();
		return  (finish_clock - start_clock) / 1000.;
	}

	CAG_Clocker(LPCTSTR filename = NULL, bool toFile = true)
	{
		m_toFile = toFile;
		name = filename;
		start();
	}
	~CAG_Clocker()
	{
		if( m_Stop ) 
			return ;
		CString tstr;

		double duration = stop();
//		CTime::GetCurrentTime().
		tstr.Format( _T("CLOCKER [%s] - Result %2.3f seconds"), (LPCTSTR)name, duration );
		ATLTRACE(_T("%s\n"),tstr);
		_tprintf(_T("%s\n"),tstr);
		
	}
};


#define RELEASE_CLOCK(a)  CAG_Clocker clocker(_T(#a));
#define RELEASE_CLOCK_NOLOG(a)  CAG_Clocker clocker(_T(#a),false);

#ifdef _DEBUG
	#define DEBUG_CLOCK(a)  CAG_Clocker clocker(_T(#a));
#else
	#define DEBUG_CLOCK(a) ;
#endif