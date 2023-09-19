// fishpet.cpp : Implementation of WinMain


#include "stdafx.h"
#include "app.h"
#include "pet.h"


CExeModule _Module;


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

            SHELLEXECUTEINFO sei = { 0 };
            sei.cbSize = sizeof(sei);
            sei.hwnd = ::GetActiveWindow();
            sei.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;
            sei.lpVerb = _T("runas");
            sei.lpFile = path;
            sei.lpParameters = args;
            sei.nShow = SW_SHOWNORMAL;

            Log(_T("elevate : %s"), args);
            ::ShellExecuteEx(&sei);
        }
        else
            Msg(_T("Server registration failure."), *phr);
    }

    return false;
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
    TCHAR dir[MAX_PATH + 1] = { 0 };
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
    // @TODO: km 20230919 - check pet windows collection
/*
    if (POINT pt; ::GetCursorPos(&pt))
    {
        CRect rc;
        GetWindowRect(rc);
        BOOL inside = rc.PtInRect(pt);

        if (m_MouseInside != inside)
        {
            m_MouseInside = inside;
            ATLTRACE2(atlTraceUtil, 0, _T("%S inside: %d\n"), __FUNCTION__, inside);
            return PostMessage(inside ? WM_MOUSEHOVER : WM_MOUSELEAVE);
        }
    }
*/
    return FALSE;
}

LRESULT CExeModule::LowLevelMouse(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        switch (wParam)
        {
        case WM_MOUSEMOVE:
            DoMouseTracking();
            break;
        case WM_LBUTTONDOWN:
        {
            auto p = (LPMSLLHOOKSTRUCT)lParam;
            ATLTRACE2(atlTraceUtil, 0, _T("mouse pos: %d, %d\n"), p->pt.x, p->pt.y);
            break;
        }
        default:
            ATLTRACE2(atlTraceUtil, 0, _T("mouse wParam: 0x%04X\n"), wParam);
            break;
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
    CTrace::SetLevel(ATL_TRACE_LEVEL);
    ATLTRACE2(atlTraceWindowing, 0, _T("App started.(%S)\n"), __FUNCTION__);
    _Module.Log(_T("--"));
    return _Module.WinMain(nShowCmd);
}
