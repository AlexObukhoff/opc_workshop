// Client.test.h : main header file for the Client.test application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CClienttestApp:
// See Client.test.cpp for the implementation of this class
//

class CClienttestApp : public CWinApp
{
public:
	CClienttestApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CClienttestApp theApp;