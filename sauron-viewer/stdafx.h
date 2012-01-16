/************************************************************************
*                                                                       *
*   stdafx.h -- include file for standard system and project includes   *
*                                                                       *
*   Copyright (c) Microsoft Corp. All rights reserved.                  *
*                                                                       *
* This code is licensed under the terms of the                          *
* Microsoft Kinect for Windows SDK (Beta) from Microsoft Research       *
* License Agreement: http://research.microsoft.com/KinectSDK-ToU        *
*                                                                       *
************************************************************************/

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <ole2.h>

// autodetect CRT memory leaks
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

// init CRT memory leak detection before main is called
struct init {
  init() { _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); }
} _init;

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

extern TCHAR g_szAppTitle[];

#pragma comment (lib, "d3d9.lib")
#pragma comment ( lib, "winmm.lib" )

#include <assert.h>
