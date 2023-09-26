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

    virtual BOOL OnIdle(int /*nIdleCount*/)
    {
        if (IsScreenSaverStateChanged())
        {
            ProcessScreenSaving();
            ::Sleep(SCREEN_SAVER); // @TODO: km 20230925 - ??
            if (m_screensave) return FALSE;
        }

        int cnt = 0;
        for (int i = 0; i < m_aIdleHandler.GetSize(); i++)
        {
            CIdleHandler* pIdleHandler = m_aIdleHandler[i];
            if (pIdleHandler && pIdleHandler->OnIdle())
                ++cnt;
        }
        return (cnt > 0);   // continue if nonzero
    }

protected:
    bool IsScreenSaverActive()
    {
        BOOL bRun = FALSE;
        ::SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &bRun, 0);

        return !!bRun;
    }
    bool IsScreenSaverStateChanged()
    {
        if (m_screensave != IsScreenSaverActive())
        {
            m_screensave = !m_screensave;
            return true;
        }
        return false;
    }
    void ProcessScreenSaving();

private:
    bool m_screensave = false;
};

class CPet; // forward declaration

class CExeModule : public CAtlExeModuleT<CExeModule>
{
public:
    CExeModule(): m_hWndParent(NULL) {}

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

    void AddPetObject(CPet* pet)
    {
        m_pets.AddTail(pet);
    }

    void RemovePetObject(CPet* pet)
    {
        CStaticDataInitCriticalSectionLock lock;
        if (SUCCEEDED(lock.Lock()))
        {
            m_pets.RemoveAt(m_pets.Find(pet));
            lock.Unlock();
        }
    }

    void ScreenSaverProcessing(bool active);
    void Log(LPCTSTR format, ...);
    bool ParseCommandLine(LPCTSTR pCmdLine, HRESULT* phr) throw();
    HRESULT Run(int nShowCmd = SW_HIDE) throw();

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
    BOOL TestVirtualKeys(WPARAM& wParam);
    BOOL ForwardMessage(UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT LowLevelMouse(int nCode, WPARAM wParam, LPARAM lParam);
    // hooks
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

public:
    HWND m_hWndParent;

private:
    UINT m_Logging = APPLOG;
    HHOOK m_hook = NULL;
    CString m_strLog;

    CAtlList<CPet*> m_pets;
    ATL::CSimpleMap<DWORD /*thread ID*/, CMessageLoop*> m_mapMsgLoop;
};

extern CExeModule _Module;
