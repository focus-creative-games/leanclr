param(
    [string]$NkgRoot,
    [string]$RuntimeDir,
    [string]$OutputDir,
    [string]$Configuration = "Release",
    [string]$NativeBuildDir,
    [string]$CMakeGenerator,
    [string]$CMakeArchitecture,
    [string]$Entry = "ManagedNet10.NkgSmoke.Program::RunFullWorkloadSurfaceSmoke",
    [switch]$SkipNkgBuild,
    [switch]$SkipCoreLibAot,
    [switch]$IncludeConsoleAot,
    [switch]$IncludeNetworkEntries,
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
$networkEntries = @("ManagedNet10.NkgSmoke.Program::RunHostingWebDebugStartSmoke")

if (-not $IncludeNetworkEntries -and $networkEntries -contains $Entry) {
    throw "$Entry is a network/hosting smoke entry. Pass -IncludeNetworkEntries to include NKG hosting AOT surface."
}

if ([string]::IsNullOrWhiteSpace($NkgRoot)) {
    $candidateRoot = [System.IO.Path]::GetFullPath([System.IO.Path]::Combine($repoRoot, "..", "NKGGameFramework"))
    if (Test-Path $candidateRoot) {
        $NkgRoot = $candidateRoot
    }
}

if ([string]::IsNullOrWhiteSpace($NkgRoot) -or -not (Test-Path $NkgRoot)) {
    throw "NKGGameFramework root was not found. Pass -NkgRoot, for example C:\study\wqaetly\new\NKGGameFramework."
}

if ([string]::IsNullOrWhiteSpace($RuntimeDir)) {
    $RuntimeDir = Get-DotNet10RuntimeDir
}

if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    $OutputDir = [System.IO.Path]::Combine($repoRoot, "artifacts", "net10-aot", "nkg", "generated")
}

if ([string]::IsNullOrWhiteSpace($NativeBuildDir)) {
    $NativeBuildDir = [System.IO.Path]::Combine($repoRoot, "out", "cmake", "tests", "net10-aot-nkg", "$Configuration-x64")
}

$NkgRoot = (Resolve-Path $NkgRoot).Path
$RuntimeDir = (Resolve-Path $RuntimeDir).Path
$OutputDir = [System.IO.Path]::GetFullPath($OutputDir)
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

$samplerProject = [System.IO.Path]::Combine($NkgRoot, "samples", "NKGGameFramework.Sampler", "NKGGameFramework.Sampler.csproj")
if (-not (Test-Path $samplerProject)) {
    throw "NKG sampler project was not found: $samplerProject"
}
$godotPlaneProject = [System.IO.Path]::Combine($NkgRoot, "samples", "NKGGameFramework.GodotPlaneSample", "NKGGameFramework.GodotPlaneSample.csproj")
if (-not (Test-Path $godotPlaneProject)) {
    throw "NKG Godot plane sample project was not found: $godotPlaneProject"
}

if (-not $SkipNkgBuild) {
    Invoke-Checked dotnet build $samplerProject -c $Configuration
    Invoke-Checked dotnet build $godotPlaneProject -c $Configuration
}

$samplerOutputDir = [System.IO.Path]::Combine($NkgRoot, "samples", "NKGGameFramework.Sampler", "bin", $Configuration, "net10.0")
if (-not (Test-Path ([System.IO.Path]::Combine($samplerOutputDir, "NKGGameFramework.dll")))) {
    throw "NKGGameFramework output was not found: $samplerOutputDir"
}
if (-not (Test-Path ([System.IO.Path]::Combine($samplerOutputDir, "UniTask.dll")))) {
    throw "UniTask dependency output was not found: $samplerOutputDir"
}
if (-not (Test-Path ([System.IO.Path]::Combine($samplerOutputDir, "OdinSerializer.dll")))) {
    throw "OdinSerializer dependency output was not found: $samplerOutputDir"
}

$godotPlaneOutputDir = [System.IO.Path]::Combine($NkgRoot, "samples", "NKGGameFramework.GodotPlaneSample", "bin", $Configuration, "net10.0")
if (-not (Test-Path ([System.IO.Path]::Combine($godotPlaneOutputDir, "NKGGameFramework.GodotPlaneSample.dll")))) {
    throw "NKG Godot plane sample output was not found: $godotPlaneOutputDir"
}
if (-not (Test-Path ([System.IO.Path]::Combine($godotPlaneOutputDir, "NKGGameFramework.Diagnostics.dll")))) {
    throw "NKG Diagnostics output was not found: $godotPlaneOutputDir"
}

$smokeDependencyDir = [System.IO.Path]::GetFullPath([System.IO.Path]::Combine($repoRoot, "out", "dotnet", "NkgSmokeDependencies", $Configuration, "net10.0"))
$smokeDependencyRoot = [System.IO.Path]::GetFullPath([System.IO.Path]::Combine($repoRoot, "out", "dotnet"))
if (-not $smokeDependencyDir.StartsWith($smokeDependencyRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Smoke dependency output path escaped repo output: $smokeDependencyDir"
}
[System.IO.Directory]::CreateDirectory($smokeDependencyDir) | Out-Null
Get-ChildItem -Path $smokeDependencyDir -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -in @(".dll", ".pdb", ".json") } |
    ForEach-Object { Remove-Item -LiteralPath $_.FullName -Force }
foreach ($sourceDir in @($samplerOutputDir, $godotPlaneOutputDir)) {
    Get-ChildItem -Path $sourceDir -File |
        Where-Object { $_.Extension -in @(".dll", ".pdb", ".json") } |
        ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $smokeDependencyDir -Force }
}

Invoke-Checked dotnet build ([System.IO.Path]::Combine($repoRoot, "src", "leanaot", "LeanAOT", "LeanAOT.csproj")) -c $Configuration
Invoke-Checked dotnet build ([System.IO.Path]::Combine($repoRoot, "src", "tests", "managed-net10", "managed-net10.sln")) -c $Configuration

$leanAotDll = [System.IO.Path]::Combine($repoRoot, "out", "dotnet", "LeanAOT", $Configuration, "net8.0", "LeanAOT.dll")
if (-not (Test-Path $leanAotDll)) {
    throw "LeanAOT output not found: $leanAotDll"
}

$nkgSmokeDir = [System.IO.Path]::Combine($repoRoot, "out", "dotnet", "ManagedNet10.NkgSmoke", $Configuration, "net10.0")
if (-not (Test-Path ([System.IO.Path]::Combine($nkgSmokeDir, "ManagedNet10.NkgSmoke.dll")))) {
    throw "ManagedNet10.NkgSmoke output not found: $nkgSmokeDir"
}

Clear-GeneratedOutputDirectory $OutputDir

$aotAssemblies = @(
    "ManagedNet10.NkgSmoke",
    "NKGGameFramework",
    "NKGGameFramework.Diagnostics",
    "NKGGameFramework.Adapter.Godot",
    "NKGGameFramework.GodotPlaneSample",
    "NKGGameFramework.Sampler",
    "OdinSerializer",
    "UniTask"
)
if ($IncludeNetworkEntries) {
    $aotAssemblies += "NKGGameFramework.Hosting"
}

$leanAotArgs = @(
    $leanAotDll,
    "--leanaot-runtime-api-profile",
    "coreclr-net10",
    "-d",
    $nkgSmokeDir,
    "-d",
    $smokeDependencyDir,
    "-d",
    $RuntimeDir
)
foreach ($assemblyName in $aotAssemblies) {
    $leanAotArgs += @("-a", $assemblyName)
}
$leanAotArgs += @("-o", $OutputDir)

Invoke-Checked -FilePath dotnet -Arguments $leanAotArgs

$expectedFiles = @(
    "ManagedNet10_NkgSmoke.module_registration.cpp",
    "NKGGameFramework.module_registration.cpp",
    "NKGGameFramework_Diagnostics.module_registration.cpp",
    "NKGGameFramework_Adapter_Godot.module_registration.cpp",
    "NKGGameFramework_GodotPlaneSample.module_registration.cpp",
    "NKGGameFramework_Sampler.module_registration.cpp",
    "OdinSerializer.module_registration.cpp",
    "UniTask.module_registration.cpp",
    "modules_registration.cpp",
    "method_invokers_part0.cpp",
    "method_direct_call_bridges_part0.cpp"
)
if ($IncludeNetworkEntries) {
    $expectedFiles += "NKGGameFramework_Hosting.module_registration.cpp"
}

foreach ($fileName in $expectedFiles) {
    $path = [System.IO.Path]::Combine($OutputDir, $fileName)
    if (-not (Test-Path $path)) {
        throw "Expected LeanAOT generated file was not found: $path"
    }
}

Write-Host "Generated .NET 10 NKG LeanAOT C++ to $OutputDir"

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

Write-Host "Built .NET 10 NKG LeanAOT native smoke runner at $nativeRunner"
if ($BuildOnly) {
    return
}

Invoke-Checked -FilePath $nativeRunner -Arguments @(
    "-l",
    $nkgSmokeDir,
    "-l",
    $smokeDependencyDir,
    "-l",
    $RuntimeDir,
    "-e",
    $Entry,
    "ManagedNet10.NkgSmoke"
)
