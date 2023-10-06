///////////////////////////////////////////////////////////////////////
// Header file: "Tools.h"
//
// Copyright (c) 2001, RightMark Gathering.  All Rights Reserved.
//
// @Author: Konstantin Mironovich (km) develop@rightmark.org
//   derivation work from original code by Alex Simkin (1998)
//   adler32 and crc32 code derived from the original zlib code
//   written by Mark Adler. See original Copyright notice below.
// 
// Purpose: 
//   Checks system setup, declares some useful helper macros
//
// History: 
//   2001/05/10 created. (km)
//   2002/04/22 modified. (km) added adler32/crc32. thanks to Mark Adler.
//   2007/09/24 modified. (km) warning 4127 disabled
//   2008/08/19 modified. (km) CEventLog added
//   2008/11/14 modified. (km) typedef void VOID; added 
//   2023/07/21 modified. (km) multi-pet support improved. C++14
//
///////////////////////////////////////////////////////////////////////

#pragma once

// Verify compiler settings

#if !defined(__cplusplus)
#error 'C++ compilation required
#endif

#if (_MSC_VER < 1300)
#error 'Compilation requires at least Visual C++ 7.0
#endif

// @WARNING: km 20081223 - In Visual C++ 2008, all ATL projects are minimally
// dependent on CRT regardless of whether ATL_MIN_CRT is defined. 
#if (_MSC_VER >= 1500) && defined(ATL_MIN_CRT)
#undef ATL_MIN_CRT 
#endif

#if !defined(_WIN32) || !defined(_MT)
#error 'Compilation requires Win-32 multi-threaded environment
#endif

#if !defined(_M_IX86) && !defined(_M_X64) // defined(_M_IA64)
#error 'The only supported compilations are x86 or x64
#endif

#if defined(_CHAR_UNSIGNED)
#error 'Char type must be signed
#endif

#if !defined(_CPPUNWIND)
#error 'Exception handling must be enabled
#endif

#if defined(_CPPRTTI) && !defined(_DEBUG)
#error 'Run-Time Type Information should be disabled for Release build
#endif

// Platform SDK Libraries

#if defined(_WINDOWS_)
    #error 'windows.h must not be included
#endif

#if !defined(STRICT)
    #define STRICT  1
#endif

// If your application hosts a third-party DLL, extension, plug-in, 
// or control panel, you may want to enable themes or visual styles 
// in your application, but without also enabling the updated common 
// controls for hosted components.
//
//#define ISOLATION_AWARE_ENABLED 1

// Exclude rarely-used stuff from Windows headers
#if !defined(WIN32_LEAN_AND_MEAN)
    #define WIN32_LEAN_AND_MEAN
#endif

#include <sdkddkver.h>

#if defined(_WIN32_WINNT_WIN10)
// define BUILD_WINDOWS to turn off deprecation warning.
#define BUILD_WINDOWS
#endif

#include <windows.h>
#include <objbase.h>

#if defined(OLE2ANSI)
    #error 'COM must support Unicode 
#endif

#if !defined(__RM_FORCE_ANSI)
#if !defined(UNICODE) || !defined(_UNICODE)
    #error 'Compilation for Unicode must be enabled
#endif
#else
#if !defined(_MBCS)
    #error 'Multi-byte Character Set must be enabled
#endif

#include <mbctype.h>
#endif
#include <tchar.h>

#define __TFILE__       _T(__FILE__)
#define __TDATE__       _T(__DATE__)
#define __TTIME__       _T(__TIME__)

#define __TTIMESTAMP__  _T(__TIMESTAMP__)
#define __TFUNCTION__   _T(__FUNCTION__)

#ifdef VOID
#undef VOID
#endif
typedef void VOID;


// C++ Libraries

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <climits>
#include <cstddef>
#include <cstdarg>

#include <cmath>

// MS <math.h> file does not define M_PI...
#if !defined(M_PI)
#define M_PI            3.14159265358979323846
#endif

#define M_PI_inv        (1/M_PI)
#define M_PI_hlf        (M_PI/2)
#define M_PI_dbl        (M_PI*2)
#define DEG2RAD(d)      ((d)*(M_PI/180.0))
#define RAD2DEG(r)      ((r)*(180.0/M_PI))

// Tools library helper macros

#define DLL_IMPORT      __declspec(dllimport)
#define DLL_EXPORT      __declspec(dllexport)

//#pragma warning(disable : 4127) // conditional expression is constant
#define CHECK(_f_, _rc_) \
    __pragma(warning(push)) \
    __pragma(warning(disable : 4127)) \
    do { if (!(_f_)) return (_rc_); } while (0) \
    __pragma(warning(pop))

#define CHECK_NULL(_p_, _rc_) \
    __pragma(warning(push)) \
    __pragma(warning(disable : 4127)) \
    do { if (NULL == (_p_)) return (_rc_); } while (0) \
    __pragma(warning(pop))

#if defined(_DEBUG)
#define ATTEMPT(_hr_) \
    __pragma(warning(push)) \
    __pragma(warning(disable : 4127)) \
    do { if (FAILED(_hr_)) ThrowException(_hr_, __TFILE__, __LINE__); } while (0) \
    __pragma(warning(pop))

#define ENDEAVOR(_hr_) \
    __pragma(warning(push)) \
    __pragma(warning(disable : 4127)) \
    do { if (S_OK != (_hr_)) ThrowException(_hr_, __TFILE__, __LINE__); } while (0) \
    __pragma(warning(pop))
#else
#define ATTEMPT(_hr_) \
    __pragma(warning(push)) \
    __pragma(warning(disable : 4127)) \
    do { if (FAILED(_hr_)) ThrowException(_hr_); } while (0) \
    __pragma(warning(pop))

#define ENDEAVOR(_hr_) \
    __pragma(warning(push)) \
    __pragma(warning(disable : 4127)) \
    do { if (S_OK != (_hr_)) ThrowException(_hr_); } while (0) \
    __pragma(warning(pop))
#endif

// property helper declarators

// @WARNING: don't use both PROPGET*/PROPPUT* macros in the same time. use PROPERTY* macro instead!
#define PROPGETDECL(_type, _name) \
    __declspec(property(get=get_##_name)) _type _name; \
    _type get_##_name(void)

#define PROPPUTDECL(_type, _name) \
    __declspec(property(put=put_##_name)) _type _name; \
    void put_##_name(_type val)

#define PROPERTYDECL(_type, _name) \
    __declspec(property(get=get_##_name, put=put_##_name)) _type _name; \
    _type get_##_name(void); \
    void put_##_name(_type val)

#define PROPGETARRAY(_type, _name) \
    __declspec(property(get=get_##_name)) _type _name[]; \
    _type get_##_name(int i)

#define PROPPUTARRAY(_type, _name) \
    __declspec(property(put=put_##_name)) _type _name[]; \
    void put_##_name(int i, _type val)

#define PROPERTYARRAY(_type, _name) \
    __declspec(property(get=get_##_name, put=put_##_name)) _type _name[]; \
    _type get_##_name(int i); \
    void put_##_name(int i, _type val)


#define PROPGET(_type, _var, _name) \
    __declspec(property(get=get_##_name)) _type _name; \
    _type get_##_name(void) { return _var; }

#define PROPPUT(_type, _var, _name) \
    __declspec(property(put=put_##_name)) _type _name; \
    void put_##_name(_type val) { _var = val; }

#define PROPERTY(_type, _var, _name) \
    __declspec(property(get=get_##_name, put=put_##_name)) _type _name; \
    _type get_##_name(void) const { return _var; } \
    void put_##_name(_type val) { _var = val; }


#if defined(_countof)
#define COUNTOF             _countof
#else
#define COUNTOF(_array_)    (sizeof(_array_)/sizeof(_array_[0]))
#endif

#define DELETE_PTR(_p_) \
    __pragma(warning(push)) \
    __pragma(warning(disable : 4127)) \
    do { if (NULL != (_p_)) { delete (_p_); (_p_) = NULL; } } while(0) \
    __pragma(warning(pop))

#define DELETE_ARR(_p_) \
    __pragma(warning(push)) \
    __pragma(warning(disable : 4127)) \
    do { if (NULL != (_p_)) { delete[] (_p_); (_p_) = NULL; } } while(0) \
    __pragma(warning(pop))

#define RELEASE(_p_) \
    __pragma(warning(push)) \
    __pragma(warning(disable : 4127)) \
    do { if (NULL != (_p_)) { (_p_)->Release(); (_p_) = NULL; } } while(0) \
    __pragma(warning(pop))

#define FREE_TM(_p_) \
    __pragma(warning(push)) \
    __pragma(warning(disable : 4127)) \
    do { if (NULL != (_p_)) ::CoTaskMemFree(_p_); (_p_) = NULL; } while(0) \
    __pragma(warning(pop))

// MS DirectX uuid(x) binding helper macro
// @WARNING: changed from (name, iid)
#define BIND_IID(iid, name)     MIDL_INTERFACE(iid) name


#if defined(TOOLS_DLL)
#if defined(TOOLS_EXPORTS)
#define TOOLS_API       DLL_EXPORT
#else
#define TOOLS_API       DLL_IMPORT
#if defined(UNICODE) || defined(_UNICODE)
#pragma comment(lib, "toolsw.lib")
#else
#pragma comment(lib, "tools.lib")
#endif
#endif
#else
#define TOOLS_API

#if defined(TOOLS_STATIC)
#if defined(UNICODE) || defined(_UNICODE)
#pragma comment(lib, "tools_staticw.lib")
#else
#pragma comment(lib, "tools_static.lib")
#endif
#endif

#endif

// ATL tracing constants
#if defined(_DEBUG)
#undef  ATL_TRACE_LEVEL
#define ATL_TRACE_LEVEL     1
#endif


#define TKLOG_NAME          _T("rm_tools.log")
#define TKLOG_LINE          512
#define DELAY_COUNT         400000

// @WARNING: km 20040204 - processor-dependent code
#if defined(_M_IX86) || defined(_M_X64)     // X86, AMD64 platforms.
#include <intrin.h>
#pragma intrinsic(__rdtsc, __cpuid, _mm_or_ps, _mm_or_pd)

#define READ_TSC(_p_) \
        _p_ = __rdtsc();

#define DO_DELAY_ \
        { \
        int ci[4]; __cpuid(ci, 0); \
        }

#define DO_DELAY(_p_) \
        for (UINT i = _p_; i--;) \
        { \
        int ci[4]; __cpuid(ci, 0); \
        }

#define READ_ID(_a_, _n_) \
        { \
        int ci[4]; \
        __cpuid(ci, _n_); _a_ = ci[0]; \
        }

#define READ_ID4(_a_,_b_,_c_,_d_,_n_) \
        { \
        int ci[4]; \
        __cpuid(ci, _n_); \
        _a_ = ci[0]; _b_ = ci[1]; \
        _c_ = ci[2]; _d_ = ci[3]; \
        }

#define CHECK_SSE_FP \
        { \
        __m128 a = 0; \
        _mm_or_ps(a, a); \
        }

#define CHECK_SSE2_FP \
        { \
        __m128d a = 0; \
        _mm_or_pd(a, a); \
        }

#elif defined(_M_IA64)      // A 64-bit Intel Itanium platform.
#include <intrin.h>
#pragma intrinsic(__getReg)

#define READ_TSC(_p_) \
        _p_ = __getReg(__REG_IA64_ApITC);
// not implemented yet...
#define DO_DELAY_ \
        __getReg(__REG_IA64_CPUID0);

#define DO_DELAY(_p_) \
        for (UINT i = _p_; i--;) \
        { \
        __getReg(__REG_IA64_CPUID0); \
        }

#define READ_ID(_a_, _n_) ATLASSERT(false)
#define READ_ID4(_a_,_b_,_c_,_d_,_n_) ATLASSERT(false)
#define CHECK_SSE ATLASSERT(false)

#endif

///////////////////////////////////////////////////////////////////////
// Helpers
//
class CErrorMode
{
    // flags specify the process error mode. This parameter can be one 
    // or more of the following values:
    //
    // SEM_FAILCRITICALERRORS - System does not display critical-error-handler 
    // message box. Instead, System sends the error to the calling process.
    //
    // SEM_NOGPFAULTERRORBOX - System does not display general-protection-fault
    // message box. This flag should only be set by debugging applications
    // that handle GP faults themselves with an exception handler.
    //
    // SEM_NOOPENFILEERRORBOX - System does not display a message box when it 
    // fails to find a file. Instead, the error is returned to the calling process.
    //
public:
    CErrorMode(UINT flags) : m_em(::SetErrorMode(flags)) {}
   ~CErrorMode() { ::SetErrorMode(m_em); }
private:
    UINT m_em;
};

class CLastError
{
public:
    CLastError() : m_ec(::GetLastError()) {}
   ~CLastError() { ::SetLastError(m_ec); }
private:
    DWORD m_ec;
};

template <bool bInit>
class CTrigger
{
    CTrigger();
public:
    CTrigger(bool& b) : m_b(&b) { b = bInit; }
   ~CTrigger() { *m_b = !bInit; }
private:
    bool *m_b;
};

#ifndef __NO_COM_SUPPORT_
#define INITIALIZE_COM_SUPPORT(_p_) CComInitialize _p_(COINIT_MULTITHREADED)
#include <crtdbg.h>
class CComInitialize
{
public:
    CComInitialize()
    {
        HRESULT hr = ::CoInitialize(NULL);
        _ASSERTE(SUCCEEDED(hr));
        if (FAILED(hr)) throw hr;
    }
    CComInitialize(COINIT flag)
    {
        HRESULT hr = ::CoInitializeEx(NULL, flag);
        _ASSERTE(SUCCEEDED(hr));
        if (FAILED(hr)) throw hr;
    }
   ~CComInitialize() throw() { ::CoUninitialize(); }
    // Registers security and sets the default security values for the process..
    HRESULT Security(DWORD autLevel, DWORD impLevel, DWORD caps = EOAC_NONE)
    {
        return ::CoInitializeSecurity(0, -1, 0, 0, autLevel, impLevel, 0, caps, 0);
    }
    // Unloads DLLs that are no longer in use and whose unload delay has expired.
    // Setting delay=INFINITE uses the system default delay (10 minutes).
    // Setting delay=0 forces the unloading of any DLLs without any delay.
    static void Free(DWORD delay = INFINITE)
    {
        ::CoFreeUnusedLibrariesEx(delay, 0);
    }
};
#else
#define INITIALIZE_COM_SUPPORT(_p_) __noop
#endif

#ifdef __GDIPLUS_SUPPORT_
#define INITIALIZE_GDIPLUS() CGdiPlusInitialize _
#pragma push_macro("new")
#undef new
#pragma warning(push)
#pragma warning(disable : 4458)
#include <gdiplus.h>
#pragma warning(pop)
#pragma pop_macro("new")
#pragma comment(lib, "gdiplus.lib")

class CGdiPlusInitialize
{
public:
    CGdiPlusInitialize()
    {
        Gdiplus::GdiplusStartup(&m_token, &m_input, NULL);
    }
   ~CGdiPlusInitialize() { Gdiplus::GdiplusShutdown(m_token); }
   // @TODO: km 20081211 - add extra ctor to support non-default input/output..
	
private:
    ULONG_PTR m_token;
    Gdiplus::GdiplusStartupInput m_input;
};
#else
#define INITIALIZE_GDIPLUS() __noop
#endif

class CTimeStamp32
{
public:
    CTimeStamp32(DWORD *p) : m_p(p), m_t(::GetTickCount()) {}
   ~CTimeStamp32() { *m_p = ::GetTickCount() - m_t; }
private:
    DWORD m_t, *m_p;
};

class CTimeStamp64
{
public:
    CTimeStamp64(ULONG64 *p) : m_p(p) { READ_TSC(T); m_t = T; }
   ~CTimeStamp64() { READ_TSC(T); *m_p = T - m_t; }
private:
    static ULONG64 T;
    ULONG64 m_t, *m_p;
};


// @TRICKY: km 20080328 - global data item can normally be initialized only once
// in an EXE or DLL project. selectany can be used in initializing global data
// defined by headers, when the same header appears in more than one source file.

#if   (__TIME_MEASURE == 64)
#define __TIMESPAN(_p_) CTimeStamp64 _(&_p_)
#pragma message(">>>>>> CTimeStamp64 is used")
__declspec(selectany) DWORD _D[] = {0, 0, 0, 0};
#elif (__TIME_MEASURE == 32)
#define __TIMESPAN(_p_) CTimeStamp32 _(&_p_)
#pragma message(">>>>>> CTimeStamp32 is used")
__declspec(selectany) DWORD _D[] = {0, 0, 0, 0};
#else
#define __TIMESPAN(_p_)
#endif


#define EVENT_LOG_BUFFER_SIZE   2048

class CEventLog
{
    typedef HANDLE (WINAPI *LPFN_OPENEVENTLOG)(LPCTSTR, LPCTSTR);
    typedef BOOL   (WINAPI *LPFN_CLOSEVENTLOG)(HANDLE);
    typedef BOOL   (WINAPI *LPFN_READEVENTLOG)(HANDLE, DWORD, DWORD, LPVOID, DWORD, PDWORD, PDWORD);

public:
    CEventLog() : m_hlog(NULL)
    {
#if defined(__EVENTLOG_DYNALOAD_)
#if defined(UNICODE) || defined(_UNICODE)
        const char o[] = "OpenEventLogW";
        const char r[] = "ReadEventLogW";
#else
        const char o[] = "OpenEventLogA";
        const char r[] = "ReadEventLogA";
#endif
        const char c[] = "CloseEventLog";

        HMODULE h = ::GetModuleHandle(_T("advapi32"));

        OpenEventLog  = (LPFN_OPENEVENTLOG)::GetProcAddress(h, o);
        ReadEventLog  = (LPFN_READEVENTLOG)::GetProcAddress(h, r);
        CloseEventLog = (LPFN_CLOSEVENTLOG)::GetProcAddress(h, c);
#endif
    }
   ~CEventLog() { Close(); }

    HANDLE Open(LPCTSTR src)
    {
#if defined(__EVENTLOG_DYNALOAD_)
        if (OpenEventLog)
#endif
        m_hlog = OpenEventLog(NULL, src);
        return m_hlog;
    }
    BOOL Close(void)
    {
        BOOL b = FALSE;
#if defined(__EVENTLOG_DYNALOAD_)
        if (!CloseEventLog) return b;
#endif
        if (NULL != m_hlog)
        {
            b = CloseEventLog(m_hlog);
            m_hlog = NULL;
        }
        return b;
    }
    BOOL Read(DWORD flag, PVOID buf, DWORD cb, DWORD& rd)
    {
#if defined(__EVENTLOG_DYNALOAD_)
        if (!ReadEventLog) return FALSE;
#endif
        if (NULL == m_hlog) return FALSE;

        DWORD need; 
        flag |= EVENTLOG_SEQUENTIAL_READ;

        return ReadEventLog(m_hlog, flag, 0, buf, cb, &rd, &need);
    }

private:
#if defined(__EVENTLOG_DYNALOAD_)
    LPFN_OPENEVENTLOG OpenEventLog;
    LPFN_READEVENTLOG ReadEventLog;
    LPFN_CLOSEVENTLOG CloseEventLog;
#endif

    HANDLE m_hlog;
};

class CBindLibrary
{
public:
    CBindLibrary() : hLib(NULL) {}
    CBindLibrary(LPCTSTR s) { load(s); }
    ~CBindLibrary() { free(); }

    bool operator!() const throw() { return (!hLib); }
    operator HMODULE() const throw() { return hLib; }

    bool load(LPCTSTR s) { hLib = ::LoadLibrary(s); return (hLib != NULL); }
    void free(void) { if (hLib) { ::FreeLibrary(hLib); hLib = NULL; } }

    FARPROC WINAPI GetEntry(IN LPCSTR lpProcName)
    {
        return ::GetProcAddress(hLib, lpProcName);
    }

private:
    HMODULE hLib;
};



EXTERN_C void TOOLS_API WINAPI  TkInitialize(void);
EXTERN_C void TOOLS_API WINAPI  TkTerminate(void);
EXTERN_C void TOOLS_API WINAPI  TkDbgLoad(HMODULE hMod);
EXTERN_C void TOOLS_API WINAPI  TkDbgFree(HMODULE hMod);
EXTERN_C void TOOLS_API WINAPI  TkDelay(void);
EXTERN_C void TOOLS_API WINAPI  TkDelayInc(LONG volatile *cnt);
EXTERN_C UINT TOOLS_API WINAPI  TkGetCookie(void);
EXTERN_C UINT TOOLS_API WINAPI  TkGetTickCount(void);
EXTERN_C bool TOOLS_API WINAPI  TkIsNewStyle(void);
EXTERN_C UINT TOOLS_API WINAPIV TkLog(LPCTSTR name, LPCTSTR format, ...);

///////////////////////////////////////////////////////////////////////
//  Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.
//
//  Jean-loup Gailly        Mark Adler
//  jloup@gzip.org          madler@alumni.caltech.edu
//
///////////////////////////////////////////////////////////////////////

EXTERN_C DWORD TOOLS_API WINAPI adler32(DWORD adler, LPBYTE buf, UINT len);
EXTERN_C DWORD TOOLS_API WINAPI crc32(DWORD crc, LPBYTE buf, UINT len);

#define Adler32(p)  adler32(1, (LPBYTE)(p), lstrlen(p)*sizeof(TCHAR))
#define CRC32(p)    crc32(0, (LPBYTE)(p), lstrlen(p)*sizeof(TCHAR))

//
// Create a data source object from dll 
// (by dll name - does not access the registry).
//
EXTERN_C HRESULT TOOLS_API STDMETHODCALLTYPE 
NoRegCoCreate(LPCTSTR dll, REFCLSID rclsid, REFIID riid, void **ppv);

//
// Create a data source object from dll 
// (looks up the class id in the registry).
//
EXTERN_C HRESULT TOOLS_API STDMETHODCALLTYPE 
NoOleCoCreate(REFCLSID rclsid, REFIID riid, void **ppv);


//////////////////////////////////////////////////////////////////////////
// IRandom interface definition and related constants
//
enum { 
    RAND_RANGE_I32 = 1,     // [0, 2_32-1]-interval
    RAND_RANGE_I31,         // [0, 2^31-1]-interval
    RAND_RANGE_I64,         // [0, 2^64-1]-interval
    RAND_RANGE_I63,         // [0, 2^63-1]-interval
    RAND_RANGE_R1,          // [0,1]-real-interval
    RAND_RANGE_R2,          // [0,1)-real-interval
    RAND_RANGE_R3,          // (0,1)-real-interval
    RAND_RANGE_R53          // [0,1) with 53-bit resolution
};

enum { RAND_METHOD_MT19937 = 1, RAND_METHOD_CRYPT };

#if defined _M_IX86
#define RAND_RANGE_MAX  RAND_RANGE_I32
#else
#define RAND_RANGE_MAX  RAND_RANGE_I64
#endif

__interface IRandom
{
    HRESULT Initialize(SIZE_T seed);
    HRESULT Initialize_(SIZE_T key[], int len);
    SIZE_T  RandI(UINT range);
    double  RandR(UINT range);
    void    Destroy(void);
};

EXTERN_C HRESULT TOOLS_API WINAPI TkGetRandom(UINT method, IRandom** pp);