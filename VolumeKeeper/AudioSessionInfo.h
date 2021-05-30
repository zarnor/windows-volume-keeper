#pragma once

#include <audiopolicy.h>
#include <string>

class VolumeManager;

class AudioSessionInfo sealed:
    public IAudioSessionEvents
{
	LONG _cRef;
	VolumeManager* _volume_manager;
	IAudioSessionControl2* _pSessionControl;
    std::wstring _title;
    bool _isSystemSession;
    float _volume;
    float _volumeBeforeJump;

public:

	AudioSessionInfo(VolumeManager* volume_manager, IAudioSessionControl2* pSessionControl);

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

	float getLastVolume() const { return _volumeBeforeJump; }
	void setLastVolume(float value) { _volumeBeforeJump = value; }
	
	std::wstring getTitle() const { return _title; }
};

