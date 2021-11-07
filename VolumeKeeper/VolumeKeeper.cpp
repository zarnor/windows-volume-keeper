// VolumeKeeper.cpp : Defines the entry point for the application.
//

#include <Windows.h>
#include <CommCtrl.h>
#include <audiopolicy.h>
#include "framework.h"
#include "VolumeKeeper.h"

#include "Macros.h"
#include "VolumeManager.h"

#define MAX_LOADSTRING 100

// Global Variables:
GUID g_guidMyContext = GUID_NULL;
DWORD g_threadId = NULL;
VolumeManager* g_VolumeManager = NULL;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
UINT WM_OPEN_CFG_DIALOG;
HANDLE ghMutex;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    CfgDialogProc(HWND, UINT, WPARAM, LPARAM);

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
        RegSetValueEx(hkey, L"VolumeKeeper", 0, REG_SZ, (const BYTE*)progPath.c_str(), static_cast<DWORD>((progPath.size() + 1) * sizeof(wchar_t)));
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
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    // Get the command line arguments
    LPWSTR* szArgList;
    int num_args;

    szArgList = CommandLineToArgvW(lpCmdLine, &num_args);
    if (szArgList == nullptr)
    {
        return -1;
    }

    WM_OPEN_CFG_DIALOG = RegisterWindowMessage(L"WM_OPEN_CFG_DIALOG");

    // Create mutex to reliably detect previous instance
    ghMutex = CreateMutex(NULL, TRUE, L"VOLUMEKEEPER_MUTEX");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        ghMutex = NULL;
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
        else if (wcscmp(szArgList[i], L"--config") == 0)
        {
            if (!ghMutex)
            {
                // Send message to existing instance to open configuration dialog
                PostMessage(HWND_BROADCAST, WM_OPEN_CFG_DIALOG, NULL, NULL);
            }
            else
            {
                PostThreadMessage(GetCurrentThreadId(), WM_OPEN_CFG_DIALOG, NULL, NULL);
            }
        }
	}

    LocalFree(szArgList);
	
    HRESULT hr = S_OK;
    VolumeManager volumeManager;
    g_VolumeManager = &volumeManager;

    if (!ghMutex)
    {
        // Previous instance is already running.
        return 0;
    }

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VOLUMEKEEPER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
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
        else if (msg.message == WM_OPEN_CFG_DIALOG)
        {
            DialogBox(hInstance, MAKEINTRESOURCE(IDD_CFG_DIALOG), NULL, CfgDialogProc);
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

    CloseHandle(ghMutex);

	return 0;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    ZeroMemory(&wcex, sizeof(WNDCLASSEX));
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_CLASSDC;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.lpszClassName = szWindowClass;

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//  PURPOSE: Processes messages for the main window.
//  WM_DESTROY  - post a quit message and return
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for configuration dialog
INT_PTR CALLBACK CfgDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        auto volume = g_VolumeManager->getDefaultVolume() * 100.0F;
        auto szVolume = std::to_wstring((int)volume);

        SetDlgItemText(hDlg, IDC_NEW_APP_VOLUME, szVolume.c_str());

        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            WCHAR szVolume[5];
            GetDlgItemText(hDlg, IDC_NEW_APP_VOLUME, szVolume, 5);

            auto volume = std::stof(szVolume);
            if (volume < 0.0F || volume > 100.0F)
            {
                MessageBox(hDlg, TEXT("Volume must be an integer between 0 and 100."),
                    TEXT("Error"), MB_OK);
            }
            else
            {
                g_VolumeManager->setDefaultVolume(volume * 0.01F);
                EndDialog(hDlg, LOWORD(wParam));
            }

            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}