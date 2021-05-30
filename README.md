# Windows Volume Keeper

Prevents sudden volume jumps to 100 % in Windows 10.

# Features

- Prevents sudden volume jumps
- Restores volumes on program startup and on new application startup

## Usage

1. Compile in Visual Studio
1. Run Volumekeeper.exe --install (Schedules to run on startup)
1. Run Volumekeeper.exe --uninstall (To remove running on startup)

## Motivation

For whatever reason the system volume randomly resets to 100 % in Windows 10. This program detects jumps from below 80 % to 100 % and restores the volume back to its previous value.

## Possibly planned features

1. Set volume for all new applications to specified value
1. Configuration dialog or system tray icon + menu for settings

## License

Garbage licence - Treat it like garbage.
