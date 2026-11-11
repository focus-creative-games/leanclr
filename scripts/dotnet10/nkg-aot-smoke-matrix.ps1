param(
    [string]$NkgRoot,
    [string]$RuntimeDir,
    [string]$OutputDir,
    [string]$Configuration = "Release",
    [string]$NativeBuildDir,
    [string]$CMakeGenerator,
    [string]$CMakeArchitecture,
    [string]$SourceFile,
    [string[]]$Entries = @(),
    [string[]]$ExcludeEntries = @(),
    [string[]]$AdditionalAssemblyDir = @(),
    [switch]$IncludeNetworkEntries,
    [switch]$IncludeConsoleAot,
    [switch]$SkipNkgBuild,
    [switch]$SkipCoreLibAot,
    [switch]$SkipAotBuild,
    [switch]$ListOnly
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

function Get-NkgSmokeEntries {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    $pattern = '^\s*public\s+static\s+void\s+(?<name>Run[A-Za-z0-9_]+Smoke)\s*\(\s*\)'
    $matches = Select-String -Path $Path -Pattern $pattern
    $names = foreach ($match in $matches) {
        $match.Matches[0].Groups["name"].Value
    }

    $names | Sort-Object -Unique
}

$repoRoot = (Resolve-Path ([System.IO.Path]::Combine($PSScriptRoot, "..", ".."))).Path
$nkgAotSmokeScript = [System.IO.Path]::Combine($PSScriptRoot, "nkg-aot-smoke.ps1")
$assemblyName = "ManagedNet10.NkgSmoke"
$networkEntries = @("RunHostingWebDebugStartSmoke")

if ([string]::IsNullOrWhiteSpace($SourceFile)) {
    $SourceFile = [System.IO.Path]::Combine($repoRoot, "src", "tests", "managed-net10", $assemblyName, "Program.cs")
}
$SourceFile = (Resolve-Path $SourceFile).Path

if ($Entries.Count -eq 0) {
    $Entries = @(Get-NkgSmokeEntries -Path $SourceFile)
}

$excluded = @{}
if (-not $IncludeNetworkEntries) {
    foreach ($entry in $networkEntries) {
        $excluded[$entry] = $true
    }
}
foreach ($entry in $ExcludeEntries) {
    if (-not [string]::IsNullOrWhiteSpace($entry)) {
        $excluded[$entry] = $true
    }
}
if ($excluded.Count -ne 0) {
    $Entries = @($Entries | Where-Object { -not $excluded.ContainsKey($_) })
}
if ($Entries.Count -eq 0) {
    throw "No NKG smoke entries were found in $SourceFile"
}

if ($ListOnly) {
    $Entries
    return
}

if ([string]::IsNullOrWhiteSpace($RuntimeDir)) {
    $RuntimeDir = Get-DotNet10RuntimeDir
}
$RuntimeDir = (Resolve-Path $RuntimeDir).Path

if ([string]::IsNullOrWhiteSpace($NativeBuildDir)) {
    $NativeBuildDir = [System.IO.Path]::Combine($repoRoot, "out", "cmake", "tests", "net10-aot-nkg", "$Configuration-x64")
}
$NativeBuildDir = [System.IO.Path]::GetFullPath($NativeBuildDir)

if (-not $SkipAotBuild) {
    $buildArgs = @(
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        $nkgAotSmokeScript,
        "-Configuration",
        $Configuration,
        "-RuntimeDir",
        $RuntimeDir,
        "-NativeBuildDir",
        $NativeBuildDir,
        "-BuildOnly"
    )

    if (-not [string]::IsNullOrWhiteSpace($NkgRoot)) {
        $buildArgs += @("-NkgRoot", $NkgRoot)
    }
    if (-not [string]::IsNullOrWhiteSpace($OutputDir)) {
        $buildArgs += @("-OutputDir", $OutputDir)
    }
    if (-not [string]::IsNullOrWhiteSpace($CMakeGenerator)) {
        $buildArgs += @("-CMakeGenerator", $CMakeGenerator)
    }
    if (-not [string]::IsNullOrWhiteSpace($CMakeArchitecture)) {
        $buildArgs += @("-CMakeArchitecture", $CMakeArchitecture)
    }
    if ($SkipNkgBuild) {
        $buildArgs += "-SkipNkgBuild"
    }
    if ($SkipCoreLibAot) {
        $buildArgs += "-SkipCoreLibAot"
    }
    if ($IncludeConsoleAot) {
        $buildArgs += "-IncludeConsoleAot"
    }
    if ($IncludeNetworkEntries) {
        $buildArgs += "-IncludeNetworkEntries"
    }

    Invoke-Checked powershell @buildArgs
}

$exeName = if ($env:OS -eq "Windows_NT") { "aot-tester.exe" } else { "aot-tester" }
$nativeRunner = [System.IO.Path]::Combine($NativeBuildDir, "bin", $Configuration, $exeName)
if (-not (Test-Path $nativeRunner)) {
    $nativeRunner = [System.IO.Path]::Combine($NativeBuildDir, "bin", $exeName)
}
if (-not (Test-Path $nativeRunner)) {
    throw "Native aot-tester output not found under $NativeBuildDir"
}

$nkgSmokeDir = [System.IO.Path]::Combine($repoRoot, "out", "dotnet", $assemblyName, $Configuration, "net10.0")
if (-not (Test-Path ([System.IO.Path]::Combine($nkgSmokeDir, "$assemblyName.dll")))) {
    throw "$assemblyName output not found: $nkgSmokeDir"
}

$smokeDependencyDir = [System.IO.Path]::GetFullPath([System.IO.Path]::Combine($repoRoot, "out", "dotnet", "NkgSmokeDependencies", $Configuration, "net10.0"))
if (-not (Test-Path $smokeDependencyDir)) {
    throw "NKG smoke dependency output not found: $smokeDependencyDir"
}

for ($i = 0; $i -lt $Entries.Count; $i++) {
    $entryName = $Entries[$i]
    $entry = "$assemblyName.Program::$entryName"
    Write-Host ("[{0}/{1}] {2}" -f ($i + 1), $Entries.Count, $entry)

    $runArgs = @("-l", $nkgSmokeDir, "-l", $smokeDependencyDir)
    foreach ($dir in $AdditionalAssemblyDir) {
        if ([string]::IsNullOrWhiteSpace($dir)) {
            continue
        }
        $resolvedDir = (Resolve-Path $dir).Path
        $runArgs += @("-l", $resolvedDir)
    }
    $runArgs += @("-l", $RuntimeDir, "-e", $entry, $assemblyName)

    Invoke-Checked -FilePath $nativeRunner -Arguments $runArgs
}

Write-Host "ok! $($Entries.Count) NKG AOT smoke entries passed."
