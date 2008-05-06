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

// OPCServer.cpp : Implementation of COPCServer

#include "stdafx.h"
//#include "resource.h"       // main symbols

#include "ag_thread.h"
#include "NameIndex.h"

#include "opc.h"
#include "opc_Data.h"
#include "opc_impl.h"

//#include "xl_events.h"

#include "ag_utils.h"
#include "ag_clocker.h"

#include "OPCServer.h"
//#include "AG_Value.h"

//#include "sf_timer.h"
//CTimerHost g_TimerHost;

// COPCServer

CComObject<COPCServer> *g_Server = NULL;
CBrowseItemsList g_BrowseItems;
//TCHAR* group_name[GROUP_TABLE_COUNT] = { _T("imp"), _T("clc") };

COPCServer::COPCServer()
{
	m_pBrowserAdapter = &m_BrowserAdapter;
	m_BrowserAdapter.m_server = this;

	//// run process putting in to database
	//for(int i=0;i< GROUP_TABLE_COUNT ; ++i) {
	//	m_DumpWriters[i] = new CDumpWriter( *this, m_DumpFileVector[i] );
	//	m_DumpWriters[i]->SetExtention( group_name[i] );
	//	m_DumpWriters[i]->Execute();
	//}
}

COPCServer::~COPCServer()
{
	//ShutdownStore();
	//delete []m_DumpFileVector;
}

