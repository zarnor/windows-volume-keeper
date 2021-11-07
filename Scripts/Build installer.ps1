$executablePath = "..\x64\Release\VolumeKeeper.exe"
$innoSetupExecutable = "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"

# --- Helper functions
function Get-GitTaggedVersion {
    $tag = & git tag --points-at HEAD
    return $tag
}

function Get-GitHasLocalChanges {
    $gitStatusOutput = & git status -s
    return "$gitStatusOutput" -ne ""
}

function Get-FileVersion {
    param ([string] $path)
   (Get-Item $path).VersionInfo.FileVersion
}

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

# --- Get version from git tag if available
$gitVersion = "DEV"
$tag = Get-GitTaggedVersion
$isTagged = "$tag" -ne ""
if ($isTagged) {
    if (Get-GitHasLocalChanges) {
        Write-Warning "Current commit is tagged as $tag but local changes are present. Building version $gitVersion"
    } else {
        Write-Host "Building release version $tag"
        $gitVersion = $tag
    }
} else {
    Write-Warning "Current commit is not tagged. Building version $gitVersion"
}

# --- Compile solution
Write-Host Compiling executable
msbuild ..\VolumeKeeper.sln /p:Configuration=Release -verbosity:minimal

# --- Verify File version of executable (if not DEV build)
if ("$gitVersion" -ne "DEV") {
    $fileVersion = Get-FileVersion $executablePath
    if ("$gitVersion.0" -ne "$fileVersion") {
        Write-Error "File version ($fileVersion) does not match the tagged version ($gitVersion) in git."
    }
}

# --- Build installer
Write-Host "Building VolumeKeeper Setup $gitVersion.exe"
& $innoSetupExecutable ..\VolumeKeeper.iss /Q /DAppVersion=$gitVersion
