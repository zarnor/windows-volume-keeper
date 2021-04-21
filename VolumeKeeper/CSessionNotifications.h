#pragma once
#include <audiopolicy.h>
#include <functional>

class CSessionNotifications sealed: public IAudioSessionNotification
{
    LONG             m_cRefAll;
    std::function<void (IAudioSessionControl*)> _callback;

public:
    CSessionNotifications(std::function<void(IAudioSessionControl*)> callback);
    ~CSessionNotifications() {};

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvInterface);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    HRESULT STDMETHODCALLTYPE OnSessionCreated(IAudioSessionControl* pNewSession);
};
