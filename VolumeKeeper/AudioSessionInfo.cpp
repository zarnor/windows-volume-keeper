#include "AudioSessionInfo.h"
#include "Macros.h"
#include "framework.h"
#include <Psapi.h>
#include <Shlwapi.h>
#include "VolumeManager.h"

extern GUID g_guidMyContext;
extern DWORD g_threadId;

AudioSessionInfo::AudioSessionInfo(VolumeManager* volume_manager, IAudioSessionControl2* pSessionControl) :
	_cRef(1),
	_volume_manager(volume_manager),
	_pSessionControl(pSessionControl),
	_isSystemSession(false),
	_volume(0),
	_volumeBeforeJump(0)
{
}

ULONG AudioSessionInfo::AddRef()
{
	return InterlockedIncrement(&_cRef);
}

ULONG AudioSessionInfo::Release()
{
	ULONG ulRef = InterlockedDecrement(&_cRef);
	if (0 == ulRef)
	{
		delete this;
	}
	return ulRef;
}

HRESULT AudioSessionInfo::QueryInterface(const IID& riid, void** ppvInterface)
{
	if (IID_IUnknown == riid)
	{
		AddRef();
		*ppvInterface = (IUnknown*)this;
	}
	else if (__uuidof(IAudioSessionEvents) == riid)
	{
		AddRef();
		*ppvInterface = (IAudioSessionEvents*)this;
	}
	else
	{
		*ppvInterface = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

HRESULT AudioSessionInfo::OnDisplayNameChanged(LPCWSTR NewDisplayName, LPCGUID EventContext)
{
	return S_OK;
}

HRESULT AudioSessionInfo::OnIconPathChanged(LPCWSTR NewIconPath, LPCGUID EventContext)
{
	return S_OK;
}

HRESULT AudioSessionInfo::OnChannelVolumeChanged(DWORD ChannelCount, float NewChannelVolumeArray[],
	DWORD ChangedChannel, LPCGUID EventContext)
{
	return S_OK;
}

HRESULT AudioSessionInfo::OnGroupingParamChanged(LPCGUID NewGroupingParam, LPCGUID EventContext)
{
	return S_OK;
}

HRESULT AudioSessionInfo::OnStateChanged(AudioSessionState NewState)
{
	return S_OK;
}

HRESULT AudioSessionInfo::OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID EventContext)
{
	if (_volume == NewVolume)
	{
		// No change in volume
		return S_OK;
	}

	float old_volume = _volume;
	
	if (*EventContext == g_guidMyContext)
	{
		// Change was triggered from this process.
		_volume = NewVolume;
		return S_OK;
	}
	
	OutputDebugStringA("OnSimpleVolumeChanged: ");
	OutputDebugStringW(_title.c_str());
	OutputDebugStringA(" ");
	OutputDebugStringW(std::to_wstring(_volume).c_str());
	OutputDebugStringA(" --> ");
	OutputDebugStringW(std::to_wstring(NewVolume).c_str());
	
	OutputDebugStringA("\n");

	if (_volume < 0.8F && NewVolume == 1.0F)
	{
		OutputDebugStringA("Volume jump detected\n");
		_volumeBeforeJump = _volume;
		PostThreadMessage(g_threadId, WM_RESTORE_VOLUME, reinterpret_cast<WPARAM>(this), NULL);
	}

	_volume = NewVolume;
	_volume_manager->NotifyVolumeChanged(this, old_volume, NewVolume);
	
	return S_OK;
}

HRESULT AudioSessionInfo::OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason)
{
	OutputDebugStringA("OnSessionDisconnected");
	
	PostThreadMessage(g_threadId, WM_STOP_LISTENING, reinterpret_cast<WPARAM>(this), NULL);
	
	return S_OK;
}

void AudioSessionInfo::StartListening()
{
	if (!_pSessionControl) return;

	_pSessionControl->AddRef();
	_pSessionControl->RegisterAudioSessionNotification(this);

	_isSystemSession = _pSessionControl->IsSystemSoundsSession() == S_OK;

	// Read the initial volume
	ISimpleAudioVolume* volume;
	if (SUCCEEDED(_pSessionControl->QueryInterface(_uuidof(ISimpleAudioVolume), (void**)&volume))) {
		volume->GetMasterVolume(&_volume);
	}

	if (_isSystemSession)
	{
		_title.assign(L"SYSTEM");
	}
	else
	{
		DWORD processId;
		_pSessionControl->GetProcessId(&processId);

		if (processId)
		{
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, processId);
			WCHAR name[4 * MAX_PATH] = L"<unknown>";

			if (hProcess)
			{
				const int len = GetProcessImageFileName(hProcess, name, 4 * MAX_PATH);
				if (len != 0) {
					const LPWSTR fname = PathFindFileName(name);
					_title.assign(fname);
				}

				CloseHandle(hProcess);
			}
		}
	}
	
	OutputDebugStringA("Begin listening: ");
	OutputDebugStringW(_title.c_str());
	OutputDebugStringA("\n");
}

void AudioSessionInfo::StopListening()
{
	if (_pSessionControl)
	{
		OutputDebugStringA("Stop listening: ");
		OutputDebugStringW(_title.c_str());
		OutputDebugStringA("\n");

		_pSessionControl->UnregisterAudioSessionNotification(this);
	}

	SAFE_RELEASE(_pSessionControl);
}

void AudioSessionInfo::RestoreVolume()
{
	OutputDebugStringA("Restoring volume\n");
	
	ISimpleAudioVolume* volume;
	if (SUCCEEDED(_pSessionControl->QueryInterface(_uuidof(ISimpleAudioVolume), reinterpret_cast<void**>(&volume)))) {
		volume->SetMasterVolume(_volumeBeforeJump, &g_guidMyContext);
	}
}
