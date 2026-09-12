param(
    [string]$Configuration = "Release",
    [string]$NativeBuildDir,
    [string]$CMakeGenerator,
    [string]$CMakeArchitecture,
    [ValidateSet("AbiSkeleton", "HandleRegistry", "Dispatcher", "EventCallback", "EngineAdapter", "ValueMarshal", "Diagnostics")]
    [string]$Scenario = "AbiSkeleton",
    [switch]$BuildOnly
)

$ErrorActionPreference = "Stop"

function Invoke-Checked {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,

        [Parameter(ValueFromRemainingArguments = $true)]
        [string[]]$Arguments
    )

    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$FilePath $($Arguments -join ' ') failed with exit code $LASTEXITCODE"
    }
}

function Resolve-CMakePath {
    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    if ($null -ne $cmake) {
        return $cmake.Source
    }

    $candidatePaths = @()
    if (-not [string]::IsNullOrWhiteSpace(${env:ProgramFiles(x86)})) {
        $candidatePaths += Get-ChildItem -Path (Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\2022\*\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe") -ErrorAction SilentlyContinue
    }
    if (-not [string]::IsNullOrWhiteSpace($env:ProgramFiles)) {
        $candidatePaths += Get-ChildItem -Path (Join-Path $env:ProgramFiles "Microsoft Visual Studio\2022\*\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe") -ErrorAction SilentlyContinue
        $standalonePath = Join-Path $env:ProgramFiles "CMake\bin\cmake.exe"
        if (Test-Path $standalonePath) {
            $candidatePaths += Get-Item $standalonePath
        }
    }

    $candidate = $candidatePaths | Sort-Object FullName | Select-Object -First 1
    if ($null -eq $candidate) {
        throw "cmake was not found in PATH or Visual Studio 2022 Build Tools. Install CMake or pass a PATH that contains cmake."
    }

    return $candidate.FullName
}

$repoRoot = (Resolve-Path ([System.IO.Path]::Combine($PSScriptRoot, "..", ".."))).Path
$sourceDir = [System.IO.Path]::Combine($repoRoot, "src", "tests", "host-bridge-smoke")

if ([string]::IsNullOrWhiteSpace($NativeBuildDir)) {
    $NativeBuildDir = [System.IO.Path]::Combine($repoRoot, "out", "cmake", "tests", "host-bridge-smoke", "$Configuration-x64")
}
$NativeBuildDir = [System.IO.Path]::GetFullPath($NativeBuildDir)

$configureArgs = @(
    "-S",
    $sourceDir,
    "-B",
    $NativeBuildDir
)

$isWindowsHost = $env:OS -eq "Windows_NT"
if ($isWindowsHost) {
    if ([string]::IsNullOrWhiteSpace($CMakeGenerator)) {
        $CMakeGenerator = "Visual Studio 17 2022"
    }
    if ([string]::IsNullOrWhiteSpace($CMakeArchitecture)) {
        $CMakeArchitecture = "x64"
    }
    if (-not [string]::IsNullOrWhiteSpace($CMakeGenerator)) {
        $configureArgs += @("-G", $CMakeGenerator)
    }
    if (-not [string]::IsNullOrWhiteSpace($CMakeArchitecture)) {
        $configureArgs += @("-A", $CMakeArchitecture)
    }
}
else {
    $configureArgs += "-DCMAKE_BUILD_TYPE=$Configuration"
}

$cmakePath = Resolve-CMakePath
Invoke-Checked -FilePath $cmakePath -Arguments $configureArgs
Invoke-Checked -FilePath $cmakePath -Arguments @("--build", $NativeBuildDir, "--config", $Configuration, "--target", "host-bridge-smoke", "--parallel")

$exeName = if ($isWindowsHost) { "host-bridge-smoke.exe" } else { "host-bridge-smoke" }
$nativeRunner = [System.IO.Path]::Combine($NativeBuildDir, "bin", $Configuration, $exeName)
if (-not (Test-Path $nativeRunner)) {
    $nativeRunner = [System.IO.Path]::Combine($NativeBuildDir, "bin", $exeName)
}
if (-not (Test-Path $nativeRunner)) {
    throw "host-bridge-smoke output not found under $NativeBuildDir"
}

Write-Host "Built host bridge smoke runner at $nativeRunner"

if ($BuildOnly) {
    return
}

Invoke-Checked -FilePath $nativeRunner -Arguments @("--scenario", $Scenario)
