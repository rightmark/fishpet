#if !defined(_GDIPLUSIMAGE_H)
#define _GDIPLUSIMAGE_H


#pragma once

#ifndef __GDIPLUS_SUPPORT_
#error 'GdiplusImage.h requires __GDIPLUS_SUPPORT_ to be globally defined
#endif


#ifndef _GDIPLUS_H
    #error 'GdiplusImage.h requires gdiplus.h to be included first
#endif

#ifndef __ATLALLOC_H__
    #error 'GdiplusImage.h requires atlalloc.h to be included first
#endif


template <class T>
class CGdiplusImageT
{
public:
    CGdiplusImageT() : m_img(NULL) {}
    virtual ~CGdiplusImageT() { Free(); }

    void Free() throw() { DELETE_PTR(m_img); }

    void Create(INT cx, INT cy, Gdiplus::PixelFormat fmt)
    {
        m_img = new Gdiplus::Bitmap(cx, cy, fmt);
        ATLASSERT(NULL != m_img);
    }
    void Create(INT cx, INT cy, Gdiplus::Graphics* g)
    {
        m_img = new Gdiplus::Bitmap(cx, cy, g);
        ATLASSERT(NULL != m_img);
    }
    bool Load(LPCWSTR file) throw()
    {
        Free();
        m_img = T::FromFile(file);
        
        return m_img && m_img->GetLastStatus() == Gdiplus::Ok;
    }
    bool Load(ATL::_U_STRINGorID name, HMODULE hInst) throw();

    operator T*() const throw() { return m_img; }
    T* operator->() const throw()
    {
        ATLASSERT(NULL != m_img);
        return m_img;
    }
    bool operator!() const throw() { return (NULL == m_img); }

protected:
    T* m_img;
};


template <class T>
inline bool CGdiplusImageT<T>::Load(ATL::_U_STRINGorID name, HMODULE hInst)
{
    Free();

    HRSRC hres = ::FindResource(hInst, name.m_lpstr, RT_RCDATA);
    if (!hres) return false;

    DWORD size = ::SizeofResource(hInst, hres);
    if (!size) return false;

    PVOID data = ::LockResource(::LoadResource(hInst, hres));
    if (!data) return false;

    CHeapPtr<BYTE, CGlobalAllocator> pb;
    if (!pb.AllocateBytes(size))
        return false;

    memcpy((PBYTE)pb, data, size);

    IStream* pStream = NULL;
    if (::CreateStreamOnHGlobal((PBYTE)pb, TRUE, &pStream) == S_OK)
    {
        ULARGE_INTEGER len = {size, 0};
        m_img = T::FromStream(pStream);
        pStream->SetSize(len);
        pStream->Release();
        // @WARNING: GdipDisposeImage() frees stream data itself..
        pb.Detach();
    }

    return m_img && m_img->GetLastStatus() == Gdiplus::Ok;
}

typedef CGdiplusImageT<Gdiplus::Image> CGdiplusImage;


class CGdiplusBitmap : public CGdiplusImageT<Gdiplus::Bitmap>
{
public:
    CGdiplusBitmap() : CGdiplusImageT<Gdiplus::Bitmap> () {}    
    CGdiplusBitmap(INT cx, INT cy, Gdiplus::PixelFormat fmt)
    {
        CGdiplusImageT<Gdiplus::Bitmap>::Create(cx, cy, fmt);
    }
    CGdiplusBitmap(INT cx, INT cy, Gdiplus::Graphics* g)
    {
        CGdiplusImageT<Gdiplus::Bitmap>::Create(cx, cy, g);
    }

    operator HBITMAP() const throw()
    { 
        HBITMAP hbm = NULL;
        m_img->GetHBITMAP(Gdiplus::Color(0), &hbm);
        return hbm; 
    }
    bool Draw(WCHAR ch, COLORREF cr, float fsize, int style = Gdiplus::FontStyleRegular) throw()
    {
        using namespace Gdiplus;

        CStringW str(ch);
        RectF rc;
        Unit unit = UnitPixel;
        m_img->GetBounds(&rc, &unit);

        Graphics g(m_img);
        g.SetCompositingQuality(CompositingQualityHighQuality);
        g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);// Glyph anti-alias bitmap with hinting

        Gdiplus::Font font(OLESTR("MS Shell Dlg"), fsize, style);

        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);

        SolidBrush brush((ARGB)Color::Transparent);
        g.FillRectangle(&brush, rc);
        Color clr;
        clr.SetFromCOLORREF(cr);
        brush.SetColor(clr);
        return (Ok == g.DrawString(str, 1, &font, rc, &sf, &brush));
    }
    bool Frame(int n, Gdiplus::Bitmap* bmp) throw()
    {
        using namespace Gdiplus;

        RectF rc;
        Unit unit = UnitPixel;
        bmp->GetBounds(&rc, &unit);
        REAL cxy = rc.Height; // source dimension

        Graphics g(m_img);
        g.SetCompositingQuality(CompositingQualityHighQuality);

        m_img->GetBounds(&rc, &unit); // destination dimension
        REAL cx = min(cxy, rc.Width);
        REAL cy = min(cxy, rc.Height);
        return (Ok == g.DrawImage(bmp, 0.0, 0.0, cxy*n, 0.0, cx, cy, unit));
    }
};

class CGdiplusIcon : public CGdiplusImageT<Gdiplus::Bitmap>
{
public:
    CGdiplusIcon() : CGdiplusImageT<Gdiplus::Bitmap> () {}    
    CGdiplusIcon(INT cx, INT cy, Gdiplus::PixelFormat fmt)
    {
        CGdiplusImageT<Gdiplus::Bitmap>::Create(cx, cy, fmt);
    }
    CGdiplusIcon(INT cx, INT cy, Gdiplus::Graphics* g)
    {
        CGdiplusImageT<Gdiplus::Bitmap>::Create(cx, cy, g);
    }

    operator HICON() const throw()
    { 
        HICON ico = NULL;
        m_img->GetHICON(&ico);
        return ico; 
    }
    bool Draw(Gdiplus::Bitmap* bmp, Gdiplus::Region* rgn = NULL) throw()
    {
        using namespace Gdiplus;

        Graphics g(m_img);

        if (rgn) { g.SetClip(rgn); }

        return (Ok == g.DrawImage(bmp, 0, 0));
    }
    bool Draw(int n, Gdiplus::Bitmap* bmp, Gdiplus::Region* rgn = NULL) throw()
    {
        using namespace Gdiplus;

        Graphics g(m_img);
        g.SetCompositingQuality(CompositingQualityHighQuality);

        if (rgn) { g.SetClip(rgn); }

        RectF rc;
        Unit unit = UnitPixel;
        m_img->GetBounds(&rc, &unit);

        return (Ok == g.DrawImage(bmp, rc, rc.Width*n, 0, rc.Width, rc.Height, unit));
    }
    bool Draw(LPCTSTR text, COLORREF cr, int style = Gdiplus::FontStyleRegular) throw()
    {
        using namespace Gdiplus;

        CStringW str(text);
        //CStringW str("120°");
        //CStringW str("99°");
        RectF rc;
        Unit unit = UnitPixel;
        m_img->GetBounds(&rc, &unit);

        float fsize = 8.0f; //(str.GetLength() > 3)? 8.0f : 8.0f;
        //fsize *= ((rc.Height - 0.5f) / 16.0f);
        PWSTR fname = (str.GetLength() > 3)? OLESTR("Arial Narrow") : OLESTR("MS Shell Dlg");
        Gdiplus::Font font(fname, fsize, style);

        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);

        Graphics g(m_img);
        g.SetCompositingQuality(CompositingQualityHighQuality);
        g.SetTextRenderingHint(TextRenderingHintSingleBitPerPixelGridFit);

        SolidBrush brush((ARGB)Color::Transparent);
        g.FillRectangle(&brush, rc);
        Color clr;
        clr.SetFromCOLORREF(cr);
        brush.SetColor(clr);
        rc.Inflate(4, 0);
        return (Ok == g.DrawString(str, -1, &font, rc, &sf, &brush));
    }
    bool Pie(Gdiplus::Brush* brush, Gdiplus::RectF& rect, Gdiplus::REAL startAngle, Gdiplus::REAL sweepAngle) throw()
    {
        using namespace Gdiplus;

        Graphics g(m_img);
        //g.SetCompositingQuality(CompositingQualityHighQuality);
        g.SetCompositingQuality(CompositingQualityGammaCorrected);
        return (Ok == g.FillPie(brush, rect, startAngle, sweepAngle));
    }
};

#if 0
//////////////////////////////////////////////////////////////////////////
// HLSMAX BEST IF DIVISIBLE BY 6
// RGBMAX, HLSMAX must each fit in a byte.
#define HLSMAX      240     // hue,lum,sat vary over 0-HLSMAX
#define RGBMAX      255     // r,g,b vary over 0-RGBMAX
#define UNDEFINED   (HLSMAX*2/3) 

// macro substitutions
#define HLS(_a,_b,_c)   RGB(_a,_b,_c)
#define GetHValue(_v)   GetRValue(_v)
#define GetLValue(_v)   GetGValue(_v)
#define GetSValue(_v)   GetBValue(_v)

//////////////////////////////////////////////////////////////////////////

struct CColorX
{
    static COLORREF RGB2HLS(COLORREF clr);
    static COLORREF HLS2RGB(BYTE hue, BYTE lum, BYTE sat);
    static DWORD Hue2RGB(DWORD a, DWORD b, DWORD hue);
};

inline COLORREF CColorX::RGB2HLS(COLORREF clr)
{
    DWORD hue, sat, lum;

    BYTE r = GetRValue(clr);
    BYTE g = GetGValue(clr);
    BYTE b = GetBValue(clr);

    // calculate lightness
    BYTE cMax = max(max(r, g), b);
    BYTE cMin = min(min(r, g), b);
    lum = (((cMax + cMin)*HLSMAX) + RGBMAX)/(2*RGBMAX);

    if (cMax == cMin)
    {   // r=g=b --> achromatic case
        sat = 0; // saturation
        hue = UNDEFINED; // hue
    }
    else
    {   // chromatic case
        // saturation
        sat = (lum <= HLSMAX/2)
            ? (((cMax - cMin)*HLSMAX) + ((cMax+cMin)/2))/(cMax + cMin)
            : (((cMax - cMin)*HLSMAX) + ((2*RGBMAX - cMax - cMin)/2))/(2*RGBMAX - cMax - cMin);

        // hue
        DWORD rDelta = (((cMax - r)*(HLSMAX/6)) + ((cMax - cMin)/2))/(cMax - cMin);
        DWORD gDelta = (((cMax - g)*(HLSMAX/6)) + ((cMax - cMin)/2))/(cMax - cMin);
        DWORD bDelta = (((cMax - b)*(HLSMAX/6)) + ((cMax - cMin)/2))/(cMax - cMin);

        hue = (r == cMax)? (bDelta - gDelta)
            : (g == cMax)? (HLSMAX/3) + rDelta - bDelta
            : ((2*HLSMAX)/3) + gDelta - rDelta; // b == cMax

        if (hue < 0)
            hue += HLSMAX;
        else if (hue > HLSMAX)
            hue -= HLSMAX;
    }

    return HLS(hue, lum, sat);
}

inline COLORREF CColorX::HLS2RGB(BYTE hue, BYTE lum, BYTE sat)
{
    DWORD r, g, b;              // RGB component values
    DWORD Magic1, Magic2;       // calculated magic numbers (really!)

    if (sat == 0)
    {   // achromatic case
        r = g = b = (lum*RGBMAX)/HLSMAX;
        if (hue != UNDEFINED) { ; /*ERROR*/ }
    }
    else
    {   // chromatic case
        // set up magic numbers
        if (lum <= (HLSMAX/2))
            Magic2 = (lum*(HLSMAX + sat) + (HLSMAX/2))/HLSMAX;
        else
            Magic2 = lum + sat - ((lum*sat) + (HLSMAX/2))/HLSMAX;

        Magic1 = 2*lum - Magic2;

        // get RGB, change units from HLSMAX to RGBMAX
        r = (Hue2RGB(Magic1, Magic2, hue + (HLSMAX/3))*RGBMAX + (HLSMAX/2))/HLSMAX;
        g = (Hue2RGB(Magic1, Magic2, hue)*RGBMAX + (HLSMAX/2)) / HLSMAX;
        b = (Hue2RGB(Magic1, Magic2, hue - (HLSMAX/3))*RGBMAX + (HLSMAX/2))/HLSMAX;
    }
    return RGB(r, g, b);
}

// helper
inline DWORD CColorX::Hue2RGB(DWORD a, DWORD b, DWORD hue)
{
    // range check: note values passed add/subtract thirds of range
    if (hue < 0)
        hue += HLSMAX;
    else if (hue > HLSMAX)
        hue -= HLSMAX;

    // return r,g, or b value from this triplet
    if (hue < (HLSMAX/6))
        return (a + (((b - a)*hue + (HLSMAX/12))/(HLSMAX/6)));
    if (hue < (HLSMAX/2))
        return b;
    if (hue < ((HLSMAX*2)/3))
        return (a + (((b - a)*(((HLSMAX*2)/3) - hue) + (HLSMAX/12))/(HLSMAX/6)));
    else
        return a;
}
#endif // 0

#endif // _GDIPLUSIMAGE_H
