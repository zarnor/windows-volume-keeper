#define AppName "Windows Volume Keeper"
#ifndef AppVersion
  #define AppVersion "DEV"
#endif
#define AppExeName "VolumeKeeper.exe"

[Setup]
AppId={{C06911DB-CA60-4008-B360-519C2BD7D884}
AppName={#AppName}
AppVersion={#AppVersion}
DisableDirPage=yes
DefaultDirName={autopf}\{#AppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputBaseFilename={#AppName} {#AppVersion} Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "x64\Release\{#AppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "VolumeKeeper\volumes.dat.default"; DestDir: "{app}"; DestName: "volumes.dat"; Flags: ignoreversion onlyifdoesntexist

[Icons]
Name: "{autoprograms}\{#AppName} Configuration"; Filename: "{app}\{#AppExeName}"; Parameters: "--config"

[Run]
Filename: "{app}\{#AppExeName}"; Parameters: "--config"; Flags: nowait postinstall skipifsilent; Description: "Launch {#StringChange(AppName, '&', '&&')} Configuration"

[UninstallRun]
Filename: "{cmd}"; Parameters: "/C ""taskkill /im volumekeeper.exe /f /t"; Flags: runhidden; RunOnceId: "killvolumekeeperprocess"

[Registry]
Root: "HKCU"; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "VolumeKeeper"; ValueData: """{app}\{#AppExeName}"""; Flags: uninsdeletevalue
