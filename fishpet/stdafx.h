// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once


// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                  // Allow use of features specific to Windows XP or later.
#define WINVER          0x0501  // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT    0x0501  // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS  0x0410  // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE               // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE       0x0600  // Change this to the appropriate value to target other versions of IE.
#endif


#define _ATL_APARTMENT_THREADED

// some CString constructors will be explicit
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
// turns off ATL's hiding of some common and often safely ignored warning messages
#define _ATL_ALL_WARNINGS

// only ATL strings are allowed, not WTL
#define _WTL_NO_CSTRING
// only ATL types are allowed, not WTL
#define _WTL_NO_WTYPES
#define _WTL_NO_UNION_CLASSES

#define _SECURE_ATL 1

#define __GDIPLUS_SUPPORT_
#define __PLAYWAV_SUPPORT_
//#define __DISABLE_LOGGING_

#include "Tools.h"

#include <ShellAPI.h>

#pragma warning(push)
#pragma warning(disable : 4127)
#include <atlpath.h>
#include <atlapp.h>

#include <atlctl.h>
#include <atlcoll.h>
#include <atlframe.h>
#include <atltypes.h>
#pragma warning(pop)

#define LOG_NAME        _T("fishpet.log")
#define LOG_LINE        256

#ifdef __PLAYWAV_SUPPORT_
#pragma warning(push)
#pragma warning(disable : 4201)
#include <MMSystem.h>
#pragma comment(lib, "Winmm.lib")
#pragma warning(pop)
#endif
