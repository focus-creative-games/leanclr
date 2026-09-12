param(
    [string]$RuntimeDir,
    [string]$OutputDir,
    [string]$Configuration = "Release",
    [string]$NativeBuildDir,
    [string]$CMakeGenerator,
    [string]$CMakeArchitecture,
    [string[]]$Entries = @(),
    [string[]]$ExcludeEntries = @(),
    [string[]]$AdditionalAssemblyDir = @(),
    [switch]$IncludeFullAssemblyEntry,
    [switch]$IncludeEnvironmentEntries,
    [switch]$IncludeInterpreterUnsupportedEntries,
    [switch]$IncludeConsoleAot,
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

function Get-LegacySmokeEntries {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourceDir
    )

    $pattern = '^\s*public\s+static\s+void\s+(?<name>Run[A-Za-z0-9_]+)\s*\(\s*\)'
    $matches = Get-ChildItem -LiteralPath $SourceDir -Filter "*.cs" -File |
        Select-String -Pattern $pattern
    $names = foreach ($match in $matches) {
        $match.Matches[0].Groups["name"].Value
    }

    $names | Sort-Object -Unique
}

function Split-EntryNames {
    param(
        [string[]]$Names
    )

    @(
        foreach ($name in $Names) {
            if ([string]::IsNullOrWhiteSpace($name)) {
                continue
            }
            foreach ($part in $name.Split(",")) {
                $trimmed = $part.Trim()
                if (-not [string]::IsNullOrWhiteSpace($trimmed)) {
                    $trimmed
                }
            }
        }
    )
}

$repoRoot = (Resolve-Path ([System.IO.Path]::Combine($PSScriptRoot, "..", ".."))).Path
$legacyAotSmokeScript = [System.IO.Path]::Combine($PSScriptRoot, "legacy-aot-smoke.ps1")
$assemblyName = "ManagedNet10.LegacyTests"
$sourceDir = [System.IO.Path]::Combine($repoRoot, "src", "tests", "managed-net10", $assemblyName)
$environmentEntries = @(
    "RunAllPrefixThenRuntimeType",
    "RunAllPrefixThenRuntimeTypeMethod",
    "RunGcFinalizerMethodThenRuntimeType"
)
$interpreterUnsupportedEntries = @(
    "RunCorlibConsole",
    "RunCorlibMonitor",
    "RunCorlibMonitorWaitPulse",
    "RunCorlibReflectionRuntimeModule",
    "RunCorlibRuntimeServices"
)

if ($Entries.Count -eq 0) {
    $Entries = @(Get-LegacySmokeEntries -SourceDir $sourceDir)
}
else {
    $Entries = @(Split-EntryNames -Names $Entries)
}
$ExcludeEntries = @(Split-EntryNames -Names $ExcludeEntries)

$excluded = @{}
if (-not $IncludeFullAssemblyEntry) {
    $excluded["RunAll"] = $true
}
if (-not $IncludeEnvironmentEntries) {
    foreach ($entry in $environmentEntries) {
        $excluded[$entry] = $true
    }
}
if (-not $IncludeInterpreterUnsupportedEntries) {
    foreach ($entry in $interpreterUnsupportedEntries) {
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
    throw "No LegacyTests AOT smoke entries were found in $sourceDir"
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
    $NativeBuildDir = [System.IO.Path]::Combine($repoRoot, "out", "cmake", "tests", "net10-aot-legacy", "$Configuration-x64")
}
$NativeBuildDir = [System.IO.Path]::GetFullPath($NativeBuildDir)

if (-not $SkipAotBuild) {
    $buildArgs = @(
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        $legacyAotSmokeScript,
        "-Configuration",
        $Configuration,
        "-RuntimeDir",
        $RuntimeDir,
        "-NativeBuildDir",
        $NativeBuildDir,
        "-BuildOnly"
    )

    if (-not [string]::IsNullOrWhiteSpace($OutputDir)) {
        $buildArgs += @("-OutputDir", $OutputDir)
    }
    if (-not [string]::IsNullOrWhiteSpace($CMakeGenerator)) {
        $buildArgs += @("-CMakeGenerator", $CMakeGenerator)
    }
    if (-not [string]::IsNullOrWhiteSpace($CMakeArchitecture)) {
        $buildArgs += @("-CMakeArchitecture", $CMakeArchitecture)
    }
    if ($SkipCoreLibAot) {
        $buildArgs += "-SkipCoreLibAot"
    }
    if ($IncludeConsoleAot) {
        $buildArgs += "-IncludeConsoleAot"
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

$assemblyDir = [System.IO.Path]::Combine($repoRoot, "out", "dotnet", $assemblyName, $Configuration, "net10.0")
if (-not (Test-Path ([System.IO.Path]::Combine($assemblyDir, "$assemblyName.dll")))) {
    throw "$assemblyName output not found: $assemblyDir"
}

for ($i = 0; $i -lt $Entries.Count; $i++) {
    $entryName = $Entries[$i]
    $entry = "$assemblyName.Program::$entryName"
    Write-Host ("[{0}/{1}] {2}" -f ($i + 1), $Entries.Count, $entry)

    $runArgs = @("-l", $assemblyDir)
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

Write-Host "ok! $($Entries.Count) LegacyTests AOT smoke entries passed."
