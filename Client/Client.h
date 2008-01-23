#pragma once

#ifndef _WIN32_DCOM
#define _WIN32_DCOM
#endif

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

/// by OPCFoundation
#include "../include/opcerror.h"
#include "../include/opcda.h"
#include "../include/opccomn.h"

// exception type 
#include "../include/opcErr.h"

#include "ShutdownImpl.h"
#include "OPCClient.h"
#include "opcmonitor.h"
