// App.h : Declaration of the CExeModule

#pragma once

#include "resource.h"       // main symbols
#include "fishpet.h"

// We should derive CMessageLoop class in order to 
// override idle processing
//
class CMessageLoopEx : public CMessageLoop
{
    static const UINT SCREEN_SAVER = 1000; // ms

public:
    BOOL OnIdle(int /*nIdleCount*/) override
    {
        /*
                if (m_screensaverState)
                {
                    ::Sleep(SCREEN_SAVER);

                    if (IsScreenSaverActive()) return TRUE;

                    m_screensaverState = false;
                }
                else
                {
                    static UINT uptick = 0;

                    if ((UINT)(::GetTickCount() - uptick) > SCREEN_SAVER)
                    {
                        uptick = ::GetTickCount();

                        if (IsScreenSaverActive())
                            m_screensaverState = true;
                    }
                }
        */

        int cnt = 0;
        for (int i = 0; i < m_aIdleHandler.GetSize(); i++)
        {
            CIdleHandler* pIdleHandler = m_aIdleHandler[i];
            if (pIdleHandler && pIdleHandler->OnIdle())
                ++cnt;
        }
        //::Sleep(1);

        return (cnt > 0);   // continue if nonzero
    }

protected:
    bool IsScreenSaverActive()
    {
        BOOL bRun = FALSE;
        ::SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &bRun, 0);

        return !!bRun;
    }

private:
    bool m_screensaverState = false;
};


class CExeModule : public CAtlExeModuleT<CExeModule>
{
public:
    CExeModule() {}

    DECLARE_LIBID(LIBID_FishPetLib)
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_FISHPET, "{9E79717C-01B9-40F6-8101-4FCE4A76C83C}")

    typedef CAtlExeModuleT<CExeModule> baseClass;
    enum { NOLOG = 0, APPLOG = 1, OVRLOG = 2, OVRLOGBAK = 3, TESTLOG = 99 };

    // Message loop map methods
    BOOL AddMessageLoop(CMessageLoop* pMsgLoop)
    {
        CStaticDataInitCriticalSectionLock lock;
        if (FAILED(lock.Lock()))
        {
            ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CAppModule::AddMessageLoop.\n"));
            ATLASSERT(FALSE);
            return FALSE;
        }

        ATLASSERT(pMsgLoop != NULL);
        ATLASSERT(m_mapMsgLoop.Lookup(::GetCurrentThreadId()) == NULL);   // not in map yet

        BOOL bRet = m_mapMsgLoop.Add(::GetCurrentThreadId(), pMsgLoop);

        lock.Unlock();

        return bRet;
    }

    BOOL RemoveMessageLoop()
    {
        CStaticDataInitCriticalSectionLock lock;
        if (FAILED(lock.Lock()))
        {
            ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CAppModule::RemoveMessageLoop.\n"));
            ATLASSERT(FALSE);
            return FALSE;
        }

        BOOL bRet = m_mapMsgLoop.Remove(::GetCurrentThreadId());

        lock.Unlock();

        return bRet;
    }

    CMessageLoop* GetMessageLoop(DWORD dwThreadID = ::GetCurrentThreadId()) const
    {
        CStaticDataInitCriticalSectionLock lock;
        if (FAILED(lock.Lock()))
        {
            ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CAppModule::GetMessageLoop.\n"));
            ATLASSERT(FALSE);
            return NULL;
        }

        CMessageLoop* pLoop = m_mapMsgLoop.Lookup(dwThreadID);

        lock.Unlock();

        return pLoop;
    }
    bool ParseCommandLine(LPCTSTR pCmdLine, HRESULT* phr) throw();
    HRESULT Run(int nShowCmd = SW_HIDE) throw();
    void Log(LPCTSTR format, ...)
    {
#ifndef __DISABLE_LOGGING_
        if (NOLOG == m_Logging) return;

        if (m_strLog.IsEmpty())
        {
            CPath path;
            if (GetIniFile(path))
            {
                m_Logging = ::GetPrivateProfileInt(_T("pet"), _T("log"), 0, path);
                UINT temp = ::GetPrivateProfileInt(_T("pet"), _T("tmp"), 1, path);

                if (temp)
                {
                    path.m_strPath.GetEnvironmentVariable(_T("TMP"));
                    path.Append(LOG_NAME);
                }
                else
                {
                    path = __targv[0];
                    path.RenameExtension(_T(".log"));
                }
                m_strLog = (LPCTSTR)path; // do not change!
            }
            else
                m_Logging = NOLOG;

            switch (m_Logging)
            {
            case OVRLOGBAK:
                for (int i = 0; !::CopyFile(m_strLog, (LPCTSTR)path, TRUE); i++)
                {
                    path = (LPCTSTR)m_strLog;
                    path.RemoveExtension();
                    path.m_strPath.AppendFormat(_T(".l%02u"), i);
                }
                // @WARNING: fall through next case!!
            case OVRLOG:
                ::DeleteFile(m_strLog);
            default:    // default is APPLOG
                break;
            case NOLOG: return;
            }
        }

        TCHAR buf[LOG_LINE + 1] = { 0 };
        UINT cch = 9;
        _tstrtime_s(buf, 9); buf[8] = _T(' '); // auto timestamp

        va_list args;
        va_start(args, format);

        cch += _vsntprintf_s(buf + cch, LOG_LINE - cch, _TRUNCATE, format, args);
        ATLASSERT(cch < (LOG_LINE));

        va_end(args);

        FILE* fp = NULL;
        //if (0 == _tfopen_s(&fp, m_strLog, _T("at, ccs=UTF-8")))
        if (0 == _tfopen_s(&fp, m_strLog, _T("at")))
        {
            _fputts(buf, fp); _fputtc(_T('\n'), fp); fclose(fp);
        }
#endif // __DISABLE_LOGGING_
    }

private:
    // helper methods
    bool GetIniFile(CPath& path)
    {
        path = __targv[0];
        path.RenameExtension(_T(".ini"));

        return !!path.FileExists();
    }
    void Msg(LPCTSTR msg) throw()
    {
        ::MessageBox(NULL, msg, CString((LPCTSTR)IDR_MAINFRAME), MB_ICONERROR);
    }
    void Msg(LPCTSTR msg, HRESULT hr) throw()
    {
        CString str;
        str.Format(_T("%s\nerror code: %x\n\n"), msg, hr);

        LPTSTR buf;
        DWORD flag = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER;
        if (::FormatMessage(flag, NULL, hr, 0, (LPTSTR)&buf, 0, NULL)) { str += buf; }
        ::LocalFree(buf);
        Msg(str);
    }
    BOOL DoMouseTracking();
    LRESULT LowLevelMouse(int nCode, WPARAM wParam, LPARAM lParam);
    // hooks
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

public:
    HWND m_hWndParent;

private:
    UINT m_Logging = APPLOG;
    BOOL m_MouseInside = FALSE;// @TODO: km 20230919 - move to individual pet items
    HHOOK m_hook = NULL;
    CString m_strLog;

    ATL::CSimpleMap<DWORD, CMessageLoop*> m_mapMsgLoop;
};

extern CExeModule _Module;
