// Server.Demo.h : main header file for the Server.Demo application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CServerDemoApp:
// See Server.Demo.cpp for the implementation of this class
//

class CServerDemoApp : public CWinApp
{
public:
	CServerDemoApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CServerDemoApp theApp;