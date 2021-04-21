#include "VolumeManager.h"

#include <cstddef>
#include <iostream>
#include <mmdeviceapi.h>
#include <xlocmon>

#include "CSessionNotifications.h"
#include "Macros.h"

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

void VolumeManager::BeginListeningSession(IAudioSessionControl* pSessionControl)
{
    AudioSessionInfo* sessionInfo = nullptr;
    IAudioSessionControl2* pSessionControl2;
	
    // Get the extended session control interface pointer.
    CHECK_HR(pSessionControl->QueryInterface(
        __uuidof(IAudioSessionControl2), (void**)&pSessionControl2));

    sessionInfo = new AudioSessionInfo(pSessionControl2);
    sessionInfo->StartListening();
    _listenedSessions.push_back(sessionInfo);
	
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


