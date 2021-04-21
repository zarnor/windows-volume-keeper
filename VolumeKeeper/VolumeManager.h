#pragma once
#include <vector>

#include "AudioSessionInfo.h"
#include "CSessionNotifications.h"

class VolumeManager
{
	IAudioSessionManager2* _pSessionManager;
	std::vector<AudioSessionInfo*> _listenedSessions;
	CSessionNotifications* _sessionNotifications;

public:
	VolumeManager();
	~VolumeManager();
	
	void Initialize();
	void Close();
	void BeginListeningSession(IAudioSessionControl* pSessionControl);
	void StopListening(AudioSessionInfo* info);

private:
	
	void BeginListeningForNewSessions();
	void BeginListeningActiveSessions();
};

