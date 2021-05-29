#pragma once
#include <map>
#include <vector>

#include "AudioSessionInfo.h"
#include "CSessionNotifications.h"

class VolumeManager
{
	IAudioSessionManager2* _pSessionManager;
	std::vector<AudioSessionInfo*> _listenedSessions;
	std::map<std::wstring, float> _seenVolumes;
	CSessionNotifications* _sessionNotifications;

public:
	VolumeManager();
	~VolumeManager();
	
	void Initialize();
	void Close();
	void BeginListeningSession(IAudioSessionControl* pSessionControl);
	void StopListening(AudioSessionInfo* info);
	void NotifyVolumeChanged(AudioSessionInfo* info, float old_volume, float new_volume);

private:
	
	void BeginListeningForNewSessions();
	void BeginListeningActiveSessions();

	void RestoreSavedVolumes();
	void SaveSeenVolumes();
};

