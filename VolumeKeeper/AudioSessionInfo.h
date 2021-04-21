#pragma once
#include <audiopolicy.h>
#include <string>

class AudioSessionInfo sealed:
    public IAudioSessionEvents
{
	LONG _cRef;
    IAudioSessionControl2* _pSessionControl;
    std::wstring _title;
    bool _isSystemSession;
    float _volume;
    float _volumeBeforeJump;

public:
	AudioSessionInfo(IAudioSessionControl2* pSessionControl) :
		_cRef(1),
		_pSessionControl(pSessionControl),
        _isSystemSession(false),
		_volume(0),
        _volumeBeforeJump(0)
	{
	}

	~AudioSessionInfo()
		= default;

	// IUnknown
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(
		REFIID riid,
		VOID** ppvInterface);

	// IAudioSessionEvents
	HRESULT STDMETHODCALLTYPE OnDisplayNameChanged(LPCWSTR NewDisplayName, LPCGUID EventContext) override;
	HRESULT STDMETHODCALLTYPE OnIconPathChanged(LPCWSTR NewIconPath, LPCGUID EventContext) override;
	HRESULT STDMETHODCALLTYPE OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID EventContext) override;
	HRESULT STDMETHODCALLTYPE OnChannelVolumeChanged(DWORD ChannelCount, float NewChannelVolumeArray[], DWORD ChangedChannel,
		LPCGUID EventContext) override;
	HRESULT STDMETHODCALLTYPE OnGroupingParamChanged(LPCGUID NewGroupingParam, LPCGUID EventContext) override;
	HRESULT STDMETHODCALLTYPE OnStateChanged(AudioSessionState NewState) override;
	HRESULT STDMETHODCALLTYPE OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason) override;

    void StartListening();
    void StopListening();
	void RestoreVolume();
};

