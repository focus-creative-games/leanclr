param(
    [string]$RuntimeDir,
    [string]$OutputDir,
    [string]$Configuration = "Release",
    [string]$NativeBuildDir,
    [string]$CMakeGenerator,
    [string]$CMakeArchitecture,
    [string]$Entry = "ManagedNet10.LegacyTests.Program::RunLegacyDiscoverySmoke",
    [switch]$IncludeConsoleAot,
    [switch]$SkipCoreLibAot,
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

function Get-DotNet10RuntimeDir {
    $runtime = dotnet --list-runtimes |
        ForEach-Object {
            if ($_ -match '^Microsoft\.NETCore\.App\s+(?<version>10\.[^\s]+)\s+\[(?<path>.+)\]$') {
                [pscustomobject]@{
                    Version = [version]$Matches.version
                    Path = [System.IO.Path]::Combine($Matches.path, $Matches.version)
                }
            }
        } |
        Sort-Object Version -Descending |
        Select-Object -First 1

    if ($null -eq $runtime) {
        throw "Microsoft.NETCore.App 10.x runtime was not found. Install .NET 10 or pass -RuntimeDir."
    }

    return $runtime.Path
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

function Clear-GeneratedOutputDirectory {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Directory
    )

    $fullPath = [System.IO.Path]::GetFullPath($Directory)
    $pathRoot = [System.IO.Path]::GetPathRoot($fullPath)
    if ([string]::IsNullOrWhiteSpace($fullPath) -or $fullPath -eq $pathRoot) {
        throw "Refusing to clean generated output directory: $fullPath"
    }

    [System.IO.Directory]::CreateDirectory($fullPath) | Out-Null
    Get-ChildItem -LiteralPath $fullPath -Force -ErrorAction SilentlyContinue |
        ForEach-Object { Remove-Item -LiteralPath $_.FullName -Recurse -Force }
}

$repoRoot = (Resolve-Path ([System.IO.Path]::Combine($PSScriptRoot, "..", ".."))).Path
$assemblyName = "ManagedNet10.LegacyTests"

if ([string]::IsNullOrWhiteSpace($RuntimeDir)) {
    $RuntimeDir = Get-DotNet10RuntimeDir
}
$RuntimeDir = (Resolve-Path $RuntimeDir).Path

if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    $OutputDir = [System.IO.Path]::Combine($repoRoot, "artifacts", "net10-aot", "legacy", "generated")
}
$OutputDir = [System.IO.Path]::GetFullPath($OutputDir)

if ([string]::IsNullOrWhiteSpace($NativeBuildDir)) {
    $NativeBuildDir = [System.IO.Path]::Combine($repoRoot, "out", "cmake", "tests", "net10-aot-legacy", "$Configuration-x64")
}
$NativeBuildDir = [System.IO.Path]::GetFullPath($NativeBuildDir)

if (-not $SkipCoreLibAot) {
    $corelibSmokeScript = [System.IO.Path]::Combine($PSScriptRoot, "corelib-aot-smoke.ps1")
    $corelibArgs = @(
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        $corelibSmokeScript,
        "-Configuration",
        $Configuration,
        "-RuntimeDir",
        $RuntimeDir
    )
    if (-not [string]::IsNullOrWhiteSpace($CMakeGenerator)) {
        $corelibArgs += @("-CMakeGenerator", $CMakeGenerator)
    }
    if (-not [string]::IsNullOrWhiteSpace($CMakeArchitecture)) {
        $corelibArgs += @("-CMakeArchitecture", $CMakeArchitecture)
    }
    if ($IncludeConsoleAot) {
        $corelibArgs += "-IncludeConsoleAot"
    }

    Invoke-Checked -FilePath powershell -Arguments $corelibArgs
}

Invoke-Checked dotnet build ([System.IO.Path]::Combine($repoRoot, "src", "leanaot", "LeanAOT", "LeanAOT.csproj")) -c $Configuration
Invoke-Checked dotnet build ([System.IO.Path]::Combine($repoRoot, "src", "tests", "managed-net10", "managed-net10.sln")) -c $Configuration

$leanAotDll = [System.IO.Path]::Combine($repoRoot, "out", "dotnet", "LeanAOT", $Configuration, "net8.0", "LeanAOT.dll")
if (-not (Test-Path $leanAotDll)) {
    throw "LeanAOT output not found: $leanAotDll"
}

$legacyDir = [System.IO.Path]::Combine($repoRoot, "out", "dotnet", $assemblyName, $Configuration, "net10.0")
if (-not (Test-Path ([System.IO.Path]::Combine($legacyDir, "$assemblyName.dll")))) {
    throw "$assemblyName output not found: $legacyDir"
}

Clear-GeneratedOutputDirectory $OutputDir

$leanAotArgs = @(
    $leanAotDll,
    "--leanaot-runtime-api-profile",
    "coreclr-net10",
    "-d",
    $legacyDir,
    "-d",
    $RuntimeDir,
    "-a",
    $assemblyName,
    "-o",
    $OutputDir
)

Invoke-Checked -FilePath dotnet -Arguments $leanAotArgs

$expectedFiles = @(
    "ManagedNet10_LegacyTests.module_registration.cpp",
    "modules_registration.cpp",
    "method_invokers_part0.cpp",
    "method_direct_call_bridges_part0.cpp"
)

foreach ($fileName in $expectedFiles) {
    $path = [System.IO.Path]::Combine($OutputDir, $fileName)
    if (-not (Test-Path $path)) {
        throw "Expected LeanAOT generated file was not found: $path"
    }
}

$methodBodyFiles = @(Get-ChildItem -LiteralPath $OutputDir -Filter "ManagedNet10_LegacyTests.method_body_part*.cpp" -File)
if ($methodBodyFiles.Count -eq 0) {
    throw "Expected at least one ManagedNet10.LegacyTests method body file under $OutputDir"
}

Write-Host "Generated .NET 10 LegacyTests LeanAOT C++ to $OutputDir"

$cmakePath = Resolve-CMakePath
$aotTesterSourceDir = [System.IO.Path]::Combine($repoRoot, "src", "tests", "aot-tester")
$configureArgs = @(
    "-S",
    $aotTesterSourceDir,
    "-B",
    $NativeBuildDir,
    "-DAOT_GENERATED_CPP_DIR=$OutputDir"
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

Invoke-Checked -FilePath $cmakePath -Arguments $configureArgs
Invoke-Checked -FilePath $cmakePath -Arguments @("--build", $NativeBuildDir, "--config", $Configuration, "--target", "aot-tester", "--parallel")

$exeName = if ($isWindowsHost) { "aot-tester.exe" } else { "aot-tester" }
$nativeRunner = [System.IO.Path]::Combine($NativeBuildDir, "bin", $Configuration, $exeName)
if (-not (Test-Path $nativeRunner)) {
    $nativeRunner = [System.IO.Path]::Combine($NativeBuildDir, "bin", $exeName)
}
if (-not (Test-Path $nativeRunner)) {
    throw "Native aot-tester output not found under $NativeBuildDir"
}

Write-Host "Built .NET 10 LegacyTests LeanAOT native smoke runner at $nativeRunner"
if ($BuildOnly) {
    return
}

Invoke-Checked -FilePath $nativeRunner -Arguments @(
    "-l",
    $legacyDir,
    "-l",
    $RuntimeDir,
    "-e",
    $Entry,
    $assemblyName
)
