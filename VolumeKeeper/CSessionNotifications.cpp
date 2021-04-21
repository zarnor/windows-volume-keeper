#include "CSessionNotifications.h"

CSessionNotifications::CSessionNotifications(std::function<void(IAudioSessionControl*)> callback)
{
	m_cRefAll = 0;
	_callback = callback;
}

HRESULT CSessionNotifications::QueryInterface(const IID& riid, void** ppvInterface)
{
	if (IID_IUnknown == riid)
	{
		AddRef();
		*ppvInterface = (IUnknown*)this;
	}
	else if (__uuidof(IAudioSessionNotification) == riid)
	{
		AddRef();
		*ppvInterface = (IAudioSessionNotification*)this;
	}
	else
	{
		*ppvInterface = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG CSessionNotifications::AddRef()
{
	return InterlockedIncrement(&m_cRefAll);
}

ULONG CSessionNotifications::Release()
{
	ULONG ulRef = InterlockedDecrement(&m_cRefAll);
	if (0 == ulRef)
	{
		delete this;
	}
	return ulRef;
}

HRESULT CSessionNotifications::OnSessionCreated(IAudioSessionControl* pNewSession)
{
	if (pNewSession)
	{
		_callback(pNewSession);
	}

	return S_OK;
}
