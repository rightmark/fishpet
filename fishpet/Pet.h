// Pet.h : Declaration of the CPet

#pragma once

#include "resource.h"       // main symbols

#include "fishpet.h"
#include "_IPetEvents_CP.h"

#include "GdiplusImage.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


// hidden parent window
class CMainFrame : public CFrameWindowImpl<CMainFrame>
{
public:
    DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

    BEGIN_MSG_MAP(CMainFrame)
        CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
    END_MSG_MAP()

    void OnFinalMessage(HWND /*hWnd*/)
    {
        ATLTRACE2(atlTraceWindowing, 0, _T("** CMainFrame::OnFinalMessage()\n"));
    }
};

// CPet

typedef CWinTraits<WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0> CPetWinTraits;

class ATL_NO_VTABLE CPet
    : public CComObjectRootEx<CComSingleThreadModel>
    , public CComCoClass<CPet, &CLSID_Pet>
    , public ISupportErrorInfo
    , public IConnectionPointContainerImpl<CPet>
    , public CProxy_IPetEvents<CPet>
    , public IDispatchImpl<IPet, &IID_IPet, &LIBID_FishPetLib>
    , public IProvideClassInfo2Impl<&CLSID_Pet, &__uuidof(_IPetEvents), &LIBID_FishPetLib>
    , public IObjectSafetyImpl<CPet, INTERFACESAFE_FOR_UNTRUSTED_CALLER>
    , public CFrameWindowImpl<CPet, ATL::CWindow, CPetWinTraits>
    , public CIdleHandler
{
    typedef CFrameWindowImpl<CPet, ATL::CWindow, CPetWinTraits> baseClass;

    typedef Gdiplus::REAL REAL;
    typedef Gdiplus::PointF PointF;
    typedef CGdiplusImage CImage;

    typedef struct tagSHOT
    {
        POINT pt;
        BYTE  alfa;
        bool  flip;
    } SHOT, *LPSHOT;

    enum VAlign { V_CENTER = 0, V_TOP, V_BOTTOM };

    static const UINT DEFLT_FRAMES = 20;
    static const UINT DRAG_EPSILON = 15;
    static const UINT TIMER_ELAPSE = 50; // ms
    static const UINT SCREEN_SAVER = 1000; // ms

public:
    DECLARE_FRAME_WND_CLASS_EX(_T("fishpet_class"), IDR_MAINFRAME, CS_DBLCLKS, COLOR_WINDOW)

    DECLARE_REGISTRY_RESOURCEID(IDR_PET)

    BEGIN_COM_MAP(CPet)
        COM_INTERFACE_ENTRY(IPet)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
        COM_INTERFACE_ENTRY(IConnectionPointContainer)
        COM_INTERFACE_ENTRY(IProvideClassInfo)
        COM_INTERFACE_ENTRY(IProvideClassInfo2)
        COM_INTERFACE_ENTRY(IObjectSafety)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CPet)
        CONNECTION_POINT_ENTRY(__uuidof(_IPetEvents))
    END_CONNECTION_POINT_MAP()

    BEGIN_MSG_MAP(CPet)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnDoubleClick)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseDown)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnMouseUp)
        MESSAGE_HANDLER(WM_RBUTTONUP, OnMouseUpR)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_MOUSEHOVER, OnMouseHover)
        MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
        MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)

        CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        ATLTRACE2(atlTraceRefcount, 0, _T(">> CPet object created (%p)\n"), this);

        _Module.Log(_T("CPet object created (%p)"), this);

        m_initialize = false;
        m_mousehover = false;

        DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
        DWORD exstyle = WS_EX_TOPMOST | WS_EX_LAYERED;

        CRect rc(0, 0, 143, 84);
        if (!CreateEx(_Module.m_hWndParent, rc, style, exstyle))
        {
            HRESULT hr = AtlHresultFromLastError();
            _Module.Log(_T("Pet object window creation failed. (%p)"), hr);
            ATLTRACE2(atlTraceWindowing, 0, _T("Pet object window creation failed. (%p)\n"), hr);
            return hr;
        }
        ShowWindow(SW_SHOW);

        return S_OK;
    }

    void FinalRelease()
    {
        ATLTRACE2(atlTraceRefcount, 0, _T("<< CPet object released (%p)\n"), this);
        _Module.Log(_T("CPet object released (%p)"), this);
    }

    virtual BOOL OnIdle()
    {
        if (m_initialize)
        {
            static UINT uptick = 0;

            UINT tick = ::GetTickCount();

            if ((UINT)(tick - uptick) > m_interval)
            {
                uptick = tick;
                OnTime(tick);

                if (!m_dragging)
                {
                    // adjust pet window position
                    UpdateWindowPos();
                }
                DrawFrame();

                if (++m_curframe >= m_framenum) m_curframe = 0;
            }
            ::Sleep(1);
        }
        return TRUE;
    }
    BOOL CheckHit(POINT& pt, WPARAM vk)
    {
        bool inside = CRect(CPoint((int)m_petpos.X, (int)m_petpos.Y), m_petdim).PtInRect(pt);
        if (m_mousehover != inside)
        {
            m_mousehover = inside;
            inside ? PostMessage(WM_MOUSEHOVER, vk, MAKELPARAM(pt.x, pt.y)) : PostMessage(WM_MOUSELEAVE);
        }
        return inside;
    }

public:
    // ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
    {
        static const IID* arr[] = { &IID_IPet };

        for (int i = 0; i < _countof(arr); i++)
        {
            if (InlineIsEqualGUID(*arr[i], riid))
                return S_OK;
        }
        return S_FALSE;
    }

    // IPet
    STDMETHOD(Init)(VARIANT file, VARIANT fnum)
    {
        Initialize();

        if (VT_BSTR == file.vt)
            m_strFile = file.bstrVal;

        if (VT_I4 == fnum.vt)
            m_framenum = fnum.intVal;

        if (!SetPetImage())
        {
            _Module.Log(_T(">> Pet image loading failed."));
            return E_FAIL; // cannot create pet image..
        }
        // adjust pet window size
        UpdateWindowSize();

        OnInit(m_screen);

        m_initialize = true;

        return S_OK;
    }
    STDMETHOD(Shut)()
    {
        Terminate();

        return S_OK;
    }
    STDMETHOD(Shoot)(LONG x, LONG y)
    {
        if (m_showshot)
        {
            SHOT shot;
            shot.alfa = 255; // opaque
            shot.flip = m_moveback;
            shot.pt.x = x;
            shot.pt.y = y;
            m_shots.Add(shot);
        }

        return S_OK;
    }
    STDMETHOD(SetPos)(FLOAT x, FLOAT y)
    {
        m_petpos.X = x;
        m_petpos.Y = y;

        return S_OK;
    }
    STDMETHOD(get_AreaCX)(ULONG* cx)
    {
        *cx = m_screen.Width();
        return S_OK;
    }
    STDMETHOD(get_AreaCY)(ULONG* cy)
    {
        *cy = m_screen.Height();
        return S_OK;
    }
    STDMETHOD(get_Length)(ULONG* cx)
    {
        *cx = m_petdim.cx;
        return S_OK;
    }
    STDMETHOD(get_Height)(ULONG* cy)
    {
        *cy = m_petdim.cy;
        return S_OK;
    }
    STDMETHOD(get_Damage)(ULONG* pVal)
    {
        *pVal = 0;
        for (int i = m_shots.GetSize(); i--;)
        {
            *pVal += m_shots[i].alfa;// @TODO: km 20230922 - what does it mean?
        }

        return S_OK;
    }
    STDMETHOD(put_Alpha)(SHORT a)
    {
        m_petalpha = (a > 0xff) ? 255
                   : (a < 0x00) ? 0 : (BYTE)a;

        return S_OK;
    }
    STDMETHOD(put_FlipX)(VARIANT_BOOL val)
    {
        m_moveback = (val != VARIANT_FALSE);
        return S_OK;
    }
    STDMETHOD(put_FlipY)(VARIANT_BOOL val)
    {
        m_deadfish = (val != VARIANT_FALSE);
        return S_OK;
    }
    STDMETHOD(put_Wound)(VARIANT_BOOL val)
    {
        m_showshot = (val != VARIANT_FALSE);
        return S_OK;
    }
    STDMETHOD(Message)(BSTR msg, BYTE align)
    {
        if (!PlaySound(msg))
        {
            m_strText = msg;
            m_msgalpha = 255;
            m_msgalign = align;
        }

        return S_OK;
    }
    STDMETHOD(put_Timeout)(ULONG val)
    {
        m_interval = (val > USER_TIMER_MAXIMUM) ? USER_TIMER_MAXIMUM
                   : (val < USER_TIMER_MINIMUM) ? USER_TIMER_MINIMUM : val;

        return S_OK;
    }
    STDMETHOD(put_Huntmode)(VARIANT_BOOL val)
    {
        m_huntmode = (val != VARIANT_FALSE);

        if (m_huntmode)
        {
            SetCursor(IDC_CROSS);
            ModifyStyleEx(WS_EX_TRANSPARENT, 0);
        }
        else
        {
            SetCursor(IDC_ARROW);
            // now the shape of the layered window will be ignored and the mouse events
            // will be passed to other windows underneath the layered window.
            ModifyStyleEx(0, WS_EX_TRANSPARENT);

            m_shots.RemoveAll(); // remove shots history
        }

        return S_OK;
    }
    STDMETHOD(put_Textmode)(VARIANT_BOOL val)
    {
        m_textmode = (val != VARIANT_FALSE);

        return S_OK;
    }

    // _IPetEvents Methods
    STDMETHOD(OnInit)(CRect& rc)
    {
        return Fire_OnInit(rc.left, rc.top, rc.right, rc.bottom);
    }
    STDMETHOD(OnShut)(SHORT reason)
    {
        return Fire_OnShut(reason);
    }
    STDMETHOD(OnTime)(ULONG tick)
    {
        return Fire_OnTime(tick);
    }
    STDMETHOD(OnClick)(LONG x, LONG y, USHORT key)
    {
        return Fire_OnClick(x, y, key);
    }
    STDMETHOD(OnDoubleClick)(LONG x, LONG y, USHORT key)
    {
        return Fire_OnDoubleClick(x, y, key);
    }
    STDMETHOD(OnRightClick)(LONG x, LONG y, USHORT key)
    {
        return Fire_OnRightClick(x, y, key);
    }
    STDMETHOD(OnDragEnter)()
    {
        return Fire_OnDragEnter();
    }
    STDMETHOD(OnDragLeave)(FLOAT x, FLOAT y)
    {
        return Fire_OnDragLeave(x, y);
    }
    STDMETHOD(OnHover)(LONG x, LONG y, USHORT key)
    {
        return Fire_OnHover(x, y, key);
    }
    STDMETHOD(OnLeave)()
    {
        return Fire_OnLeave();
    }
    //     STDMETHOD(OnScreenSave)(bool active)
    //     {
    //         return Fire_OnScreenSave(active);
    //     }

    // Message handlers
    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        ModifyStyle(WS_POPUP, 0);

        // register object for message filtering and idle updates
        CMessageLoop* pl = _Module.GetMessageLoop();
        ATLASSERT(pl != NULL);
        pl->AddIdleHandler(this);

        _Module.AddPetObject(this);
        _Module.Log(_T("create : %p"), m_hWnd);
        return 0;
    }
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        ATLTRACE2(atlTraceWindowing, 0, _T(">> CPet::OnDestroy()\n"));

        // unregister message filtering and idle updates
        CMessageLoop* pl = _Module.GetMessageLoop();
        ATLASSERT(pl != NULL);
        pl->RemoveIdleHandler(this);

        // @TRICKY: km 20090119 - set WS_POPUP to avoid PostQuitMessage(1) in
        // CFrameWindowImplBase::OnDestroy() implementation.
        // this is important when multiple objects created..
        ModifyStyle(0, WS_POPUP);

        _Module.RemovePetObject(this);
        _Module.Log(_T("delete : %p, lock=%d"), m_hWnd, _Module.m_nLockCnt);
        bHandled = FALSE;
        return 1;
    }
    LRESULT OnDoubleClick(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        OnDoubleClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (USHORT)wParam);

        return 0;
    }
    LRESULT OnMouseDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
    {
        m_btnpress = true;
        m_ptDrag.SetPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

        return 0;
    }
    LRESULT OnMouseUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        m_btnpress = false;

        if (m_dragging)
        {
            ReleaseCapture();
            m_dragging = false;
            ATLTRACE(_T("CPet::OnMouseMove() - leave dragging mode\n"));

            OnDragLeave(m_petpos.X, m_petpos.Y);
        }
        else
        {
            OnClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), GET_KEYSTATE_WPARAM(wParam));
        }
        return 0;
    }
    LRESULT OnMouseUpR(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        OnRightClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), GET_KEYSTATE_WPARAM(wParam));

        return 0;
    }
    LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
    {
        if (m_btnpress)
        {
            int dx = GET_X_LPARAM(lParam) - m_ptDrag.x;
            int dy = GET_Y_LPARAM(lParam) - m_ptDrag.y;

            if (!m_dragging && abs(dx) < DRAG_EPSILON && abs(dy) < DRAG_EPSILON)
                return 0;

            if (!m_dragging)
            {
                SetCapture();
                m_dragging = true;
                ATLTRACE(_T("CPet::OnMouseMove() - enter dragging mode\n"));

                OnDragEnter();
            }

            if (m_dragging)
            {
                m_petpos.X += dx;
                m_petpos.Y += dy;

                // adjust pet window position
                UpdateWindowPos();
            }
        }
        return 0;
    }
    LRESULT OnMouseHover(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        OnHover(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (USHORT)wParam);

        return 0;
    }
    LRESULT OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        OnLeave();

        return 0;
    }
    LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        if (wParam == SC_SCREENSAVE)
        {
            Fire_OnScreenSave(lParam);
        }
        return 0;
    }


private:
    // helper methods
    void Initialize()
    {
        // initial pet state..
        m_huntmode = false;
        m_deadfish = false;
        m_moveback = false;
        m_textmode = false;
        m_showshot = false;
        m_btnpress = false;
        m_dragging = false;

        m_interval = TIMER_ELAPSE;
        m_framenum = DEFLT_FRAMES;
        m_curframe = 0;
        m_petalpha = 255;
        m_msgalpha = 255;   // opaque
        m_msgalign = V_CENTER;

        m_petdim.SetSize(0, 0);
        m_ptDrag.SetPoint(0, 0);

        m_petpos.X = 0.0f;
        m_petpos.Y = 0.0f;

        CPoint origin(::GetSystemMetrics(SM_XVIRTUALSCREEN), ::GetSystemMetrics(SM_YVIRTUALSCREEN)); // left/top of the virtual screen
        CSize size(::GetSystemMetrics(SM_CXVIRTUALSCREEN), ::GetSystemMetrics(SM_CYVIRTUALSCREEN)); // width/height of the virtual screen
        m_screen = CRect(origin, size);

        m_strFile.Empty();
        m_shots.RemoveAll();
    }
    void Terminate()
    {
        m_initialize = false;

        PlaySound(NULL); // stop sound

        if (IsWindow()) { PostMessage(WM_CLOSE); }

        OnShut(0); // reason
    }
    bool SetPetImage()
    {
        bool bLoaded = false;
        if (!m_strFile.IsEmpty())
        {
            CErrorMode em(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
            // read image from file..
            bLoaded = m_petpix.Load(m_strFile);
#ifdef _DEBUG
            if (bLoaded)
                ATLTRACE(_T("CPet::SetPetImage() - from file\n"));
#endif
        }

        if (!bLoaded)
        {
            if (!m_petpix.Load(IDR_MAINFRAME, NULL)) return false;
            ATLTRACE(_T("CPet::SetPetImage() - from resource\n"));

            // correct value for default image
            m_framenum = DEFLT_FRAMES;
        }

        if (!m_petpix) return false;

        Gdiplus::PixelFormat pf = m_petpix->GetPixelFormat();
        if (!Gdiplus::IsAlphaPixelFormat(pf) || !Gdiplus::IsCanonicalPixelFormat(pf))
            return false;

        m_petdim.SetSize(m_petpix->GetWidth() / m_framenum, m_petpix->GetHeight());
        m_petpos.X = (REAL)-m_petdim.cx;
        m_petpos.Y = (REAL)m_screen.Width() / 2;

        return true;
    }
    bool PlaySound(BSTR bstr)
    {
#ifndef __PLAYWAV_SUPPORT_
        return false;
#endif
        if (NULL == bstr)
        {
            // all sounds that are playing on behalf of this task are stopped. 
            ::PlaySound(NULL, NULL, SND_PURGE);
        }
        else
        {
            CPath path(bstr);
            if (path.GetExtension().CompareNoCase(_T(".wav")))
                return false;

            if (path.FileExists())
            {
                ::PlaySound((LPCTSTR)path, NULL, SND_FILENAME | SND_ASYNC);
            }
            else
            {
                _Module.Log(_T("file not found : \"%s\""), (LPCTSTR)path);
            }
        }
        return true;
    }
    void DrawFrame()
    {
        CGdiplusBitmap bmp(m_petdim.cx, m_petdim.cy, m_petpix->GetPixelFormat());
        Gdiplus::Graphics g((Gdiplus::Image*)bmp);
        Gdiplus::Rect dstRect(0, 0, m_petdim.cx, m_petdim.cy);
        g.DrawImage(m_petpix, dstRect, m_petdim.cx * m_curframe, 0, m_petdim.cx, m_petdim.cy, Gdiplus::UnitPixel);

        if (m_moveback) bmp->RotateFlip(Gdiplus::RotateNoneFlipX);
        if (m_deadfish) bmp->RotateFlip(Gdiplus::RotateNoneFlipY);

        CWindowDC windc(m_hWnd);
        CDC memdc; memdc.CreateCompatibleDC(windc);

        CBitmap bm(bmp);
        HBITMAP hbm = memdc.SelectBitmap(bm);

        DrawShot(memdc);
        DrawText(memdc, m_petdim.cx / 3 + 20, m_petdim.cy / 3);

        BLENDFUNCTION bf = { 0 };
        bf.BlendOp = AC_SRC_OVER;
        bf.AlphaFormat = AC_SRC_ALPHA;
        bf.SourceConstantAlpha = m_petalpha;

        CPoint pt(0, 0);
        CPoint pos((int)m_petpos.X, (int)m_petpos.Y);
        ::UpdateLayeredWindow(m_hWnd, windc, &pos, &m_petdim, memdc, &pt, 0, &bf, ULW_ALPHA);

        memdc.SelectBitmap(hbm);

        // @TODO: km 20090125 - consider..
        //DrawText(windc, m_petdim.cx/3 + 20, m_petdim.cy/3);
    }
    void FramePixel(int x, int y, COLORREF& c, BYTE& a)
    {
        CGdiplusBitmap bmp(m_petdim.cx, m_petdim.cy, m_petpix->GetPixelFormat());
        Gdiplus::Graphics g((Gdiplus::Image*)bmp);
        Gdiplus::Rect dstRect(0, 0, m_petdim.cx, m_petdim.cy);
        g.DrawImage(m_petpix, dstRect, m_petdim.cx * m_curframe, 0, m_petdim.cx, m_petdim.cy, Gdiplus::UnitPixel);

        if (m_moveback) bmp->RotateFlip(Gdiplus::RotateNoneFlipX);
        if (m_deadfish) bmp->RotateFlip(Gdiplus::RotateNoneFlipY);

        Gdiplus::Color color;
        bmp->GetPixel(x, y, &color);

        c = RGB(color.GetR(), color.GetG(), color.GetB());
        a = color.GetAlpha();
    }
    void DrawShot(HDC hdc)
    {
        for (int i = m_shots.GetSize(); i--;)
        {
            LONG a = (LONG)m_shots[i].alfa - 2;
            if (a > 0)
                m_shots[i].alfa = (BYTE)a;
            else
                m_shots.RemoveAt(i);
        }

        if (!m_showshot || m_deadfish) return;

        Gdiplus::Graphics g(hdc);
        Gdiplus::FontFamily ff(_T("Arial"));
        Gdiplus::Font font(&ff, 20, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

        for (int i = 0; i < m_shots.GetSize(); i++)
        {
            LONG x = m_shots[i].pt.x;
            LONG y = m_shots[i].pt.y;
            BYTE a = m_shots[i].alfa;

            if (m_shots[i].flip != m_moveback) { x = m_petdim.cx - x; }

            Gdiplus::PointF pt((REAL)x, (REAL)y);
            Gdiplus::SolidBrush brush(Gdiplus::Color(a, 64, 0, 0));
            g.DrawString(OLESTR("*"), -1, &font, pt, &brush);

        } // for(i)
    }
    void DrawText(HDC hdc, LONG x, LONG y)
    {
        if (!m_textmode || m_strText.IsEmpty()) return;

        if (m_msgalign == V_CENTER)
        {
            if (m_moveback) x -= 20;
            if (m_deadfish) y += 5;
        }
        else if (m_msgalign == V_TOP)
        {
            y = 0;
        }
        else if (m_msgalign == V_BOTTOM)
        {
            y = m_petdim.cy - 15;
        }
        // @TODO: km 20230727 - if top/bottom then draw on white matte.

        Gdiplus::Graphics g(hdc);
        Gdiplus::FontFamily ff(_T("Arial"));
        Gdiplus::Font font(&ff, 10, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::PointF pt((REAL)x, (REAL)y);
        Gdiplus::SolidBrush brush(Gdiplus::Color(m_msgalpha, 0, 64, 0));
        g.DrawString(m_strText, -1, &font, pt, &brush);

        if (!m_msgalpha--) m_strText.Empty();
    }
    void UpdateWindowPos(void)
    {
        SetWindowPos(HWND_TOPMOST, (int)m_petpos.X, (int)m_petpos.Y, 0, 0, SWP_NOSIZE);
    }
    void UpdateWindowSize(void)
    {
        SetWindowPos(NULL, (int)m_petpos.X, (int)m_petpos.Y, m_petdim.cx, m_petdim.cy, SWP_NOZORDER);
    }

    HCURSOR SetCursor(LPCTSTR cursor)
    {
        return (HCURSOR)::SetClassLongPtr(m_hWnd, GCL_HCURSOR,
            (LONG_PTR)::LoadCursor(NULL, cursor));
    }


public:

private:
    // internal
    bool m_initialize;
    bool m_mousehover;          // cursor is over pet
    bool m_btnpress;            // left button down
    bool m_dragging;            // dragging mode

    // props
    bool m_huntmode;
    bool m_textmode;
    bool m_deadfish;            // upset fish
    bool m_moveback;            // true if moving from right to left
    bool m_showshot;

    UINT m_interval;            // interval between frame updates
    UINT m_framenum;            // total number of frames
    UINT m_curframe;            // current frame selected

    CPoint m_ptDrag;
    CSize  m_petdim;            // pet frame dimensions
    CRect  m_screen;            // screen work area

    PointF m_petpos;            // pet frame origin (left-top)
    CImage m_petpix;

    BYTE m_petalpha;            // alpha transparency. 0 - image is transparent. 
    BYTE m_msgalpha;
    BYTE m_msgalign;            // 0 - center (default), 1 - top, 2 - bottom.

    CStringW m_strText;         // pet' message text
    CStringW m_strFile;         // pet image file

    ATL::CSimpleArray<SHOT> m_shots;
};

OBJECT_ENTRY_AUTO(__uuidof(Pet), CPet)
