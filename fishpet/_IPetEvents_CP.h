#pragma once

template<class T>
class CProxy_IPetEvents
    : public IConnectionPointImpl<T, &__uuidof(_IPetEvents)>
{
public:
    HRESULT Fire_OnInit(ULONG cx, ULONG cy)
    {
        HRESULT hr = S_OK;
        T* pT = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pT->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pT->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant var[2];
                var[1] = cx;
                var[0] = cy;
                CComVariant varResult;

                DISPPARAMS params = { var, NULL, 2, 0 };
                hr = pConnection->Invoke(1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
            }
        }
        return hr;
    }
    HRESULT Fire_OnShut(SHORT reason)
    {
        HRESULT hr = S_OK;
        T* pT = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pT->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pT->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant var[1];
                var[0] = reason;
                CComVariant varResult;

                DISPPARAMS params = { var, NULL, 1, 0 };
                hr = pConnection->Invoke(2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
            }
        }
        return hr;
    }
    HRESULT Fire_OnTime(ULONG tick)
    {
        HRESULT hr = S_OK;
        T* pT = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pT->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pT->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant var[1];
                var[0] = tick;
                CComVariant varResult;

                DISPPARAMS params = { var, NULL, 1, 0 };
                hr = pConnection->Invoke(3, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
            }
        }
        return hr;
    }
    HRESULT Fire_OnClick(LONG x, LONG y, USHORT key)
    {
        HRESULT hr = S_OK;
        T* pT = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pT->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pT->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant var[3];
                var[2] = x;
                var[1] = y;
                var[0] = key;
                CComVariant varResult;

                DISPPARAMS params = { var, NULL, 3, 0 };
                hr = pConnection->Invoke(4, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
            }
        }
        return hr;
    }
    HRESULT Fire_OnDoubleClick(LONG x, LONG y, USHORT key)
    {
        HRESULT hr = S_OK;
        T* pT = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pT->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pT->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant var[3];
                var[2] = x;
                var[1] = y;
                var[0] = key;
                CComVariant varResult;

                DISPPARAMS params = { var, NULL, 3, 0 };
                hr = pConnection->Invoke(5, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
            }
        }
        return hr;
    }
    HRESULT Fire_OnRightClick(LONG x, LONG y, USHORT key)
    {
        HRESULT hr = S_OK;
        T* pT = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pT->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pT->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant var[3];
                var[2] = x;
                var[1] = y;
                var[0] = key;
                CComVariant varResult;

                DISPPARAMS params = { var, NULL, 3, 0 };
                hr = pConnection->Invoke(6, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
            }
        }
        return hr;
    }
    HRESULT Fire_OnDragEnter()
    {
        HRESULT hr = S_OK;
        T* pT = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pT->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pT->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant varResult;

                DISPPARAMS params = { NULL, NULL, 0, 0 };
                hr = pConnection->Invoke(7, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
            }
        }
        return hr;
    }
    HRESULT Fire_OnDragLeave(FLOAT x, FLOAT y)
    {
        HRESULT hr = S_OK;
        T* pT = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pT->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pT->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant var[2];
                var[1] = x;
                var[0] = y;
                CComVariant varResult;

                DISPPARAMS params = { var, NULL, 2, 0 };
                hr = pConnection->Invoke(8, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
            }
        }
        return hr;
    }
    HRESULT Fire_OnHover(LONG x, LONG y, SHORT key)
    {
        HRESULT hr = S_OK;
        T* pT = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pT->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pT->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant var[3];
                var[2] = x;
                var[1] = y;
                var[0] = key;
                CComVariant varResult;

                DISPPARAMS params = { var, NULL, 3, 0 };
                hr = pConnection->Invoke(9, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
            }
        }
        return hr;
    }
    HRESULT Fire_OnLeave()
    {
        HRESULT hr = S_OK;
        T* pT = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pT->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pT->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant varResult;

                DISPPARAMS params = { NULL, NULL, 0, 0 };
                hr = pConnection->Invoke(10, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
            }
        }
        return hr;
    }
    HRESULT Fire_OnScreenSave(bool active)
    {
        HRESULT hr = S_OK;
        T* pT = static_cast<T*>(this);
        int cConnections = m_vec.GetSize();

        for (int iConnection = 0; iConnection < cConnections; iConnection++)
        {
            pT->Lock();
            CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
            pT->Unlock();

            IDispatch* pConnection = static_cast<IDispatch*>(punkConnection.p);

            if (pConnection)
            {
                CComVariant var[1];
                var[0] = active;
                CComVariant varResult;

                DISPPARAMS params = { var, NULL, 1, 0 };
                hr = pConnection->Invoke(11, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
            }
        }
        return hr;
    }
};
