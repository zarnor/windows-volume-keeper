#include "VolumeManager.h"

#include <cstddef>
#include <iostream>
#include <ostream>
#include <mmdeviceapi.h>
#include <xlocmon>
#include <fstream>

#include "CSessionNotifications.h"
#include "framework.h"
#include "Macros.h"

extern DWORD g_threadId;

VolumeManager::VolumeManager()
{
    _pSessionManager = nullptr;
    _sessionNotifications = new CSessionNotifications([=](IAudioSessionControl* pSessionControl) { this->BeginListeningSession(pSessionControl); });
}

VolumeManager::~VolumeManager()
{
    delete _sessionNotifications;
}

void VolumeManager::Initialize()
{
    HRESULT hr = S_OK;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;

    // Get enumerator for audio endpoint devices.
    CHECK_HR(CoCreateInstance(__uuidof(MMDeviceEnumerator),
        NULL, CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator),
        (void**)&pEnumerator))

    // Get default audio-rendering device.
    CHECK_HR(pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice))

    // Get the session manager
    CHECK_HR(pDevice->Activate(__uuidof(IAudioSessionManager2),
        CLSCTX_ALL, NULL, (void**)&this->_pSessionManager))

    this->BeginListeningForNewSessions();
    this->BeginListeningActiveSessions();
    this->RestoreSavedVolumes();

Exit:
    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
}

void VolumeManager::Close()
{
	// Stop listening to new session notifications
    if (_pSessionManager) {
        _pSessionManager->UnregisterSessionNotification(_sessionNotifications);
        SAFE_RELEASE(_pSessionManager);
    }

	// Stop listening sesion specific notifications
    for (auto& _listenedSession : _listenedSessions)
    {
	    _listenedSession->StopListening();
        delete _listenedSession;
    }

    _listenedSessions.clear();

    this->SaveSeenVolumes();
}

void VolumeManager::BeginListeningForNewSessions()
{
    _pSessionManager->RegisterSessionNotification(_sessionNotifications);
}

void VolumeManager::BeginListeningActiveSessions()
{
    int cbSessionCount = 0;
    IAudioSessionEnumerator* pSessionList = nullptr;
    IAudioSessionControl* pSessionControl = nullptr;
	
    // Get the current list of sessions.
    CHECK_HR(_pSessionManager->GetSessionEnumerator(&pSessionList));

    // Get the session count.
    CHECK_HR(pSessionList->GetCount(&cbSessionCount));
	
    for (int index = 0; index < cbSessionCount; index++)
    {
        // Get the <n>th session.
        CHECK_HR(pSessionList->GetSession(index, &pSessionControl));

    	BeginListeningSession(pSessionControl);
        pSessionControl = nullptr;
    }
	
Exit:
    SAFE_RELEASE(pSessionControl)
    SAFE_RELEASE(pSessionList)
}

void VolumeManager::SaveSeenVolumes()
{
	std::wofstream file;
    file.open("volumes.dat", std::ios::trunc);
    if (!file) {
        OutputDebugStringA("Cannot save volumes.");
        return;
    }

    for (auto it = _seenVolumes.begin(); it != _seenVolumes.end(); ++it) {
        std::wcout << it->first << ", " << it->second << '\n';

        OutputDebugStringA("Proc: ");
    	OutputDebugStringW(it->first.c_str());

        file << it->first << std::endl;
        file << it->second << std::endl;
    }

    file.close();
}

void VolumeManager::RestoreSavedVolumes()
{
    std::wfstream file;
    file.open("volumes.dat", std::ios::in);
    if (!file) {
    	OutputDebugStringA("No previous volumes found.");
        return;
    }

    std::wstring processs_name;
    std::wstring processs_last_volume_as_string;
    float volume;
	
    while (true) {

        std::getline(file, processs_name);
        std::getline(file, processs_last_volume_as_string);

    	volume = std::stof(processs_last_volume_as_string);

        this->_seenVolumes.insert_or_assign(processs_name, volume);
    	
        if (file.eof())
            break;
    }

    file.close();
}

void VolumeManager::BeginListeningSession(IAudioSessionControl* pSessionControl)
{
    AudioSessionInfo* sessionInfo = nullptr;
    IAudioSessionControl2* pSessionControl2;
    float volume;
    std::wstring title;
	
    // Get the extended session control interface pointer.
    CHECK_HR(pSessionControl->QueryInterface(
        __uuidof(IAudioSessionControl2), (void**)&pSessionControl2));

    sessionInfo = new AudioSessionInfo(this, pSessionControl2);
    sessionInfo->StartListening();
    _listenedSessions.push_back(sessionInfo);

    volume = sessionInfo->getLastVolume();
    title = sessionInfo->getTitle();
    if (volume == 100.0F)
    {
        const bool previous_volume_is_known = _seenVolumes.find(title) != _seenVolumes.end();
    	if  (previous_volume_is_known)
    	{
            const auto last_seen_volume = _seenVolumes[title];
    		
            sessionInfo->setLastVolume(last_seen_volume);
            PostThreadMessage(g_threadId, WM_RESTORE_VOLUME, reinterpret_cast<WPARAM>(this), NULL);

            OutputDebugStringA("Resetting volume of ");
            OutputDebugStringW(title.c_str());
    	}
    }
	else
    {
        _seenVolumes.insert_or_assign(title, volume);
    }
	
Exit:
    ;
}

void VolumeManager::StopListening(AudioSessionInfo* info)
{
    info->StopListening();

    _listenedSessions.erase(
        std::find(
            _listenedSessions.begin(), 
            _listenedSessions.end(), 
            info));
}

void VolumeManager::NotifyVolumeChanged(AudioSessionInfo* info, float old_volume, float new_volume)
{
    const bool volume_at_max = (new_volume == 1.0F);
	
    if (volume_at_max)
    {
        this->_seenVolumes.erase(info->getTitle());
    }
	else
    {
        this->_seenVolumes.insert_or_assign(info->getTitle(), info->getLastVolume());
    }

    this->SaveSeenVolumes();
}
