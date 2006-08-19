#pragma once

#ifndef _WIN32_DCOM
#define _WIN32_DCOM
#endif

#include <Windows.h>
#include <ATLComTime.h>
#include <AtlComCli.h>
#include <AtlSync.h>
#include <assert.h>

#include <vector>
#include <queue>
#include <map>
#include <sstream>
using namespace std;

/// by OPCFoundation
#include "OPCError.h"
#include "opcda.h"
#include "opccomn.h"

// exception type 
#include "opcErr.h"

#include "ShutdownImpl.h"
#include "OPCClient.h"
#include "opcmonitor.h"
