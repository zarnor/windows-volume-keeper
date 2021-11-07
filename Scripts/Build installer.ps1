$compiler = "cl.exe"
$innoSetupExecutable = "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"

# --- Check prerequisites

$isInDeveloperCommandPrompt = "${env:VSCMD_VER}" -ne ""
if (!$isInDeveloperCommandPrompt) {
    Write-Error 'This script must be run inside Developer Powershell for VS 2019+'
    exit 1
}

$isVCToolsInstalled = "${env:VCToolsVersion}" -ne ""
if (!$isInDeveloperCommandPrompt) {
    Write-Error 'Visual studio with c++ workload must be installed.'
    exit 1
}

$innoSetupExists = Test-Path -Path $innoSetupExecutable -PathType Leaf
if (!$innoSetupExists) {
    Write-Error "Inno Setup 6 must be installed to build the installer."
    Write-Error "Visit: https://jrsoftware.org/isdl.php"
    exit 1
}

# --- Compile solution
Write-Host Compiling executable
msbuild ..\VolumeKeeper.sln /p:Configuration=Release

# --- Build installer
Write-Host Building setup
& $innoSetupExecutable ..\VolumeKeeper.iss
