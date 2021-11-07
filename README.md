# Windows Volume Keeper

Prevents sudden volume jumps to 100 % in Windows 10.

# Features

- Prevents sudden volume jumps
- Restores volumes on program startup and on new application startup
- Set default volume for new application to a specific value

## Compiling from source

1. Make sure prerequisites are installed:
    1. Microsoft Visual Studio with "Desktop Development with c++" -worload
    1. Inno Setup 6.x (https://jrsoftware.org/isdl.php")
1. Open Developer PowerShell for VS 2019
1. Run the .\Scripts\Build installer.ps1 -script

## Motivation

For whatever reason the system volume randomly resets to 100 % in Windows 10. This program detects jumps from below 80 % to 100 % and restores the volume back to its previous value.

## Possibly planned features

1. Configuration dialog or system tray icon + menu for settings

## License

Garbage licence - Treat it like garbage.
