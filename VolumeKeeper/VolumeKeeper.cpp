// VolumeKeeper.cpp : Defines the entry point for the application.
//

#include <Windows.h>
#include <Commctrl.h>
#include <audiopolicy.h>
#include "framework.h"
#include "VolumeKeeper.h"

#include "Macros.h"
#include "VolumeManager.h"

// Global Variables:
GUID g_guidMyContext = GUID_NULL;
DWORD g_threadId = NULL;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    HRESULT hr = S_OK;
    VolumeManager volumeManager;

    if (hPrevInstance)
    {
        return 0;
    }
	
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    EXIT_ON_ERROR(hr);

    hr = CoCreateGuid(&g_guidMyContext);
    EXIT_ON_ERROR(hr);

    g_threadId = GetCurrentThreadId();
	
    volumeManager.Initialize();

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (msg.message == WM_RESTORE_VOLUME)
        {
            auto info = reinterpret_cast<AudioSessionInfo*>(msg.wParam);
            info->RestoreVolume();
        }
        else if (msg.message == WM_STOP_LISTENING)
        {
            auto info = reinterpret_cast<AudioSessionInfo*>(msg.wParam);
            volumeManager.StopListening(info);
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

Exit:
    volumeManager.Close();
	
    if (FAILED(hr))
    {
        MessageBox(NULL, TEXT("This program requires Windows Vista."),
            TEXT("Error termination"), MB_OK);
    }
	
    CoUninitialize();

	return 0;
}
