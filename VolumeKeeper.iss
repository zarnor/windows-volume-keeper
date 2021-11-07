#define MyAppName "Windows Volume Keeper"
#define MyAppVersion "0.9"
#define MyAppExeName "VolumeKeeper.exe"

[Setup]
AppId={{C06911DB-CA60-4008-B360-519C2BD7D884}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
DisableDirPage=yes
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputBaseFilename={#MyAppName} {#MyAppVersion} Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "x64\Release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "VolumeKeeper\volumes.dat.default"; DestDir: "{app}"; DestName: "volumes.dat"; Flags: ignoreversion onlyifdoesntexist

[Icons]
Name: "{autoprograms}\{#MyAppName} Configuration"; Filename: "{app}\{#MyAppExeName}"; Parameters: "--config"

[Run]
Filename: "{app}\{#MyAppExeName}"; Parameters: "--config"; Flags: nowait postinstall skipifsilent; Description: "Launch {#StringChange(MyAppName, '&', '&&')} Configuration"

[UninstallRun]
Filename: "{cmd}"; Parameters: "/C ""taskkill /im volumekeeper.exe /f /t"; Flags: runhidden; RunOnceId: "killvolumekeeperprocess"

[Registry]
Root: "HKCU"; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "VolumeKeeper"; ValueData: "{app}"; Flags: createvalueifdoesntexist uninsdeletevalue
