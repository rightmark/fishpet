// fishpet.cpp : Implementation of WinMain


#include "stdafx.h"
#include "App.h"
#include "Pet.h"


CExeModule _Module;

void CMessageLoopEx::ProcessScreenSaving()
{
    _Module.Log(_T("CPet::screen saver - %i"), (int)m_screensave);
    _Module.ScreenSaverProcessing(m_screensave);
}

bool CExeModule::ParseCommandLine(LPCTSTR pCmdLine, HRESULT* phr)
{
    if (baseClass::ParseCommandLine(pCmdLine, phr)) return true;

    if (FAILED(*phr))
    {
        Log(_T("parse param : %p"), *phr);
        if (*phr == E_ACCESSDENIED)
        {
            // @WARNING: km 20091120 - try to elevate for Vista and later OS..
            CPath path(pCmdLine);
            path.RemoveArgs();

            const TCHAR szTokens[] = _T("-/");
            LPCTSTR args = FindOneOf(pCmdLine, szTokens);
            if (args) { args = CharPrev(pCmdLine, args); }

            SHELLEXECUTEINFO sei = {0};
            sei.cbSize       = sizeof(sei);
            sei.hwnd         = ::GetActiveWindow();
            sei.fMask        = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;
            sei.lpVerb       = _T("runas");
            sei.lpFile       = path;
            sei.lpParameters = args;
            sei.nShow        = SW_SHOWNORMAL;

            Log(_T("elevate : %s"), args);
            ::ShellExecuteEx(&sei);
        }
        else
            Msg(_T("Server registration failure."), *phr);
    }

    return false;
}

void CExeModule::ScreenSaverProcessing(bool active)
{
    ForwardMessage(WM_SYSCOMMAND, SC_SCREENSAVE, active);
}

void CExeModule::Log(LPCTSTR format, ...)
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

HRESULT CExeModule::Run(int nShowCmd)
{
    HRESULT hr = S_OK;

    const TCHAR szTokens[] = _T("-/");
    bool bAutomation = false;

    LPCTSTR token = FindOneOf(::GetCommandLine(), szTokens);
    while (NULL != token)
    {
        if (!WordCmpI(token, _T("Automation")) || !WordCmpI(token, _T("Embedding")))
        {
            Log(_T("run : %s"), token);
            bAutomation = true;
            break;
        }
        token = FindOneOf(token, szTokens);
    }

    if (!bAutomation)
    {
        Msg(_T("Bad or missed parameter.\nThe following parameters are available:\n\n")
            _T("-RegServer\n-UnregServer\n"));

        return hr;
    }
#if 0
    TCHAR dir[MAX_PATH+1] = {0};
    ::GetCurrentDirectory(MAX_PATH, dir);
    Log(_T("dir : %s"), dir);
#endif

    hr = PreMessageLoop(nShowCmd);

    // Call RunMessageLoop only if PreMessageLoop returns S_OK.
    while (S_OK == hr)
    {
        INITIALIZE_GDIPLUS()

        m_hWndParent = NULL;
        // @TRICKY: km 20081217 - seems the only way to avoid the application
        // toolwindow to be displayed on the taskbar is to create hidden
        // parental window. recommended WS_EX_APPWINDOW extended style removal
        // does not work under Vista..
        CMainFrame hidden;
        if (!hidden.CreateEx()) break;

        m_hWndParent = hidden;
        hidden.ModifyStyle(WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW, 0);
        hidden.SetWindowPos(NULL, 0, 0, 1, 1, SWP_NOZORDER);

        Log(_T("hidden : %p"), m_hWndParent);

        m_hook = ::SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
        ATLTRACE2(atlTraceUtil, 0, _T("hook: %p\n"), m_hook);

        CMessageLoopEx loop;
        AddMessageLoop(&loop);

        loop.Run();

        if (hidden.IsWindow()) hidden.DestroyWindow();
        Log(_T("hidden : %p, lock=%d"), hidden.m_hWnd, m_nLockCnt);

        RemoveMessageLoop();
        break;
    }
    Log(_T("**loop break"));

    if (m_hook) ::UnhookWindowsHookEx(m_hook);

    // Call PostMessageLoop if PreMessageLoop returns success.
    if (SUCCEEDED(hr))
    {
        hr = PostMessageLoop();
    }

    ATLASSERT(SUCCEEDED(hr));
    return hr;
}

BOOL CExeModule::DoMouseTracking()
{
    POINT pt; ::GetCursorPos(&pt);
    POSITION pos = m_pets.GetHeadPosition();
    while (pos)
    {
        if (m_pets.GetNext(pos)->CheckHit(pt)) return TRUE;
    }
    return FALSE;
}

BOOL CExeModule::TestVirtualKeys(WPARAM& wParam)
{
    BYTE state[256] = { 0 };
    if (!::GetKeyboardState(state)) return FALSE;
    // test keys
    if (state[VK_LBUTTON] & 0x80) wParam |= MK_LBUTTON;
    if (state[VK_RBUTTON] & 0x80) wParam |= MK_RBUTTON;
    if (state[VK_SHIFT]   & 0x80) wParam |= MK_SHIFT;
    if (state[VK_CONTROL] & 0x80) wParam |= MK_CONTROL;

    ATLTRACE2(atlTraceUtil, 0, _T("%S keys: 0x%04x\n"), __FUNCTION__, wParam);
    return TRUE;
}

BOOL CExeModule::ForwardMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    POSITION pos = m_pets.GetHeadPosition();
    while (pos)
    {
        m_pets.GetNext(pos)->PostMessage(message, wParam, lParam);
    }
}

LRESULT CExeModule::LowLevelMouse(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        if (wParam == WM_MOUSEMOVE)
        {
            DoMouseTracking();
        }
        else if (wParam == WM_LBUTTONUP)
        {
            POINT pt; ::GetCursorPos(&pt);
            WPARAM wp = 0;
            TestVirtualKeys(wp);
            ForwardMessage(WM_LBUTTONUP, wp, MAKELPARAM(pt.x, pt.y));
            ATLTRACE2(atlTraceUtil, 0, _T("LLM click: %d, %d\n"), pt.x, pt.y);
        }
    }
    return ::CallNextHookEx(NULL, nCode, wParam, lParam);

}

LRESULT CALLBACK CExeModule::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return _Module.LowLevelMouse(nCode, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////
// entry point

extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int nShowCmd)
{
#ifdef _DEBUG
    CTrace::SetLevel(ATL_TRACE_LEVEL);
#endif
    ATLTRACE2(atlTraceWindowing, 0, _T("App started.(%S)\n"), __FUNCTION__);
    _Module.Log(_T("--"));
    return _Module.WinMain(nShowCmd);
}
