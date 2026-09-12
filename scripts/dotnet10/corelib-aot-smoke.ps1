param(
    [string]$RuntimeDir,
    [string]$OutputDir,
    [string]$Configuration = "Release",
    [string]$NativeBuildDir,
    [string]$CMakeGenerator,
    [string]$CMakeArchitecture,
    [switch]$IncludeConsoleAot
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

if ([string]::IsNullOrWhiteSpace($RuntimeDir)) {
    $RuntimeDir = Get-DotNet10RuntimeDir
}

if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    $OutputDir = [System.IO.Path]::Combine($repoRoot, "artifacts", "net10-aot", "core", "generated")
}

if ([string]::IsNullOrWhiteSpace($NativeBuildDir)) {
    $NativeBuildDir = [System.IO.Path]::Combine($repoRoot, "out", "cmake", "tests", "net10-aot-core", "$Configuration-x64")
}

$RuntimeDir = (Resolve-Path $RuntimeDir).Path
$OutputDir = [System.IO.Path]::GetFullPath($OutputDir)
$NativeBuildDir = [System.IO.Path]::GetFullPath($NativeBuildDir)
Clear-GeneratedOutputDirectory $OutputDir

Invoke-Checked dotnet build ([System.IO.Path]::Combine($repoRoot, "src", "leanaot", "LeanAOT", "LeanAOT.csproj")) -c $Configuration

$leanAotDll = [System.IO.Path]::Combine($repoRoot, "out", "dotnet", "LeanAOT", $Configuration, "net8.0", "LeanAOT.dll")
if (-not (Test-Path $leanAotDll)) {
    throw "LeanAOT output not found: $leanAotDll"
}

$profilePath = [System.IO.Path]::Combine($repoRoot, "src", "leanaot", "LeanAOT", "runtime-apis", "coreclr-net10", "profile.json")
$profile = Get-Content -Raw $profilePath | ConvertFrom-Json
$coreRuntimeAssemblies = @()
$skippedCoreModules = @()
$defaultExcludedCoreModules = @()
foreach ($moduleName in @($profile.coreLibraryModules)) {
    if ($moduleName -eq "System.Console" -and -not $IncludeConsoleAot) {
        $defaultExcludedCoreModules += $moduleName
        continue
    }

    $assemblyPath = [System.IO.Path]::Combine($RuntimeDir, "$moduleName.dll")
    if (Test-Path $assemblyPath) {
        $coreRuntimeAssemblies += $moduleName
    }
    else {
        $skippedCoreModules += $moduleName
    }
}

if ($coreRuntimeAssemblies.Count -eq 0) {
    throw "No core runtime assemblies from $profilePath were found under $RuntimeDir"
}

$leanAotArgs = @(
    $leanAotDll,
    "--leanaot-runtime-api-profile",
    "coreclr-net10",
    "-d",
    $RuntimeDir
)
foreach ($assemblyName in $coreRuntimeAssemblies) {
    $leanAotArgs += @("-a", $assemblyName)
}
$leanAotArgs += @("-o", $OutputDir)

Invoke-Checked -FilePath dotnet -Arguments $leanAotArgs

$expectedFiles = @(
    "System_Private_CoreLib.method_body_part1.cpp",
    "modules_registration.cpp",
    "method_invokers_part0.cpp",
    "method_direct_call_bridges_part0.cpp"
)
foreach ($assemblyName in $coreRuntimeAssemblies) {
    $expectedFiles += "$($assemblyName.Replace(".", "_")).module_registration.cpp"
}

foreach ($fileName in $expectedFiles) {
    $path = [System.IO.Path]::Combine($OutputDir, $fileName)
    if (-not (Test-Path $path)) {
        throw "Expected LeanAOT generated file was not found: $path"
    }
}

Write-Host "Generated .NET 10 core runtime LeanAOT C++ for $($coreRuntimeAssemblies -join ', ') to $OutputDir"
if ($skippedCoreModules.Count -gt 0) {
    Write-Host "Skipped profile core modules not found in runtime pack: $($skippedCoreModules -join ', ')"
}
if ($defaultExcludedCoreModules.Count -gt 0) {
    Write-Host "Skipped default-excluded core modules: $($defaultExcludedCoreModules -join ', ')"
}

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

Write-Host "Built .NET 10 core runtime LeanAOT native compile smoke under $NativeBuildDir"
