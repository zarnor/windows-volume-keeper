// VolumeKeeper.cpp : Defines the entry point for the application.
//

#include <Windows.h>
#include <CommCtrl.h>
#include <audiopolicy.h>
#include "framework.h"
#include "VolumeKeeper.h"

#include "Macros.h"
#include "VolumeManager.h"

// Global Variables:
GUID g_guidMyContext = GUID_NULL;
DWORD g_threadId = NULL;

void AddRunOnStartupRegistryKey(HINSTANCE hInstance)
{
    // Get the path of the current executable
    TCHAR szPath[_MAX_PATH];
    GetModuleFileNameW(hInstance, szPath, _MAX_PATH);
    const std::wstring progPath(szPath);

    // Add registry value to run the program on startup
    OutputDebugStringW(L"Installing as run on startup...\n");
    HKEY hkey = nullptr;
    const LONG createStatus = RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hkey); //Creates a key       
    if (createStatus == ERROR_SUCCESS)
    {
        RegSetValueEx(hkey, L"VolumeKeeper", 0, REG_SZ, (BYTE*)progPath.c_str(), (progPath.size() + 1) * sizeof(wchar_t));
        RegCloseKey(hkey);
    }
}

void RemoveRunOnStartupRegistryKey()
{
    OutputDebugStringW(L"Uninstalling as run on startup...\n");
    HKEY hkey = nullptr;

    const LONG openStatus = RegOpenKey(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hkey);
    if (openStatus == ERROR_SUCCESS)
    {
        // Key was found and opened. Try deleting the value.
        RegDeleteValue(hkey, L"VolumeKeeper");
        RegCloseKey(hkey);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(nCmdShow);

    // Get the command line arguments
    LPWSTR* szArgList;
    int num_args;

    szArgList = CommandLineToArgvW(lpCmdLine, &num_args);
    if (szArgList == nullptr)
    {
        return -1;
    }

    // Loop trough parameters to see if we're installing or uninstalling.
	for (int i = 0; i < num_args; i++)
	{
		if (wcscmp(szArgList[i], L"--install") == 0)
		{
            AddRunOnStartupRegistryKey(hInstance);
		}
        else if (wcscmp(szArgList[i], L"--uninstall") == 0)
        {
            RemoveRunOnStartupRegistryKey();
            return 0;
        }
	}

    LocalFree(szArgList);
	
    HRESULT hr = S_OK;
    VolumeManager volumeManager;

    if (hPrevInstance)
    {
        return 0;
    }
	
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    EXIT_ON_ERROR(hr)

    hr = CoCreateGuid(&g_guidMyContext);
    EXIT_ON_ERROR(hr)

    g_threadId = GetCurrentThreadId();
	
    volumeManager.Initialize();

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (msg.message == WM_RESTORE_VOLUME)
        {
            const auto info = reinterpret_cast<AudioSessionInfo*>(msg.wParam);
            info->RestoreVolume();
        }
        else if (msg.message == WM_STOP_LISTENING)
        {
            const auto info = reinterpret_cast<AudioSessionInfo*>(msg.wParam);
            volumeManager.StopListening(info);
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

Exit:
    volumeManager.Close();
	
    if (FAILED(hr))
    {
        MessageBox(nullptr, TEXT("This program requires Windows Vista."),
            TEXT("Error termination"), MB_OK);
    }
	
    CoUninitialize();

	return 0;
}
