param(
    [string]$RuntimeDir,
    [string]$OutputDir,
    [string]$Configuration = "Release",
    [string[]]$Assemblies = @(
        "System.Private.CoreLib",
        "System.Runtime",
        "System.Console",
        "System.Collections",
        "System.Linq",
        "System.Threading",
        "System.Runtime.InteropServices",
        "System.Reflection",
        "System.Private.Uri",
        "System.Text.Json"
    )
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

$repoRoot = (Resolve-Path ([System.IO.Path]::Combine($PSScriptRoot, "..", ".."))).Path

if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    $OutputDir = [System.IO.Path]::Combine($repoRoot, "artifacts", "dotnet10-externs")
}

if ([string]::IsNullOrWhiteSpace($RuntimeDir)) {
    $runtime = dotnet --list-runtimes |
        ForEach-Object {
            if ($_ -match '^Microsoft\.NETCore\.App\s+(?<version>10\.[^\s]+)\s+\[(?<path>.+)\]$') {
                [pscustomobject]@{
                    Version = [version]$Matches.version
                    Path = (Join-Path $Matches.path $Matches.version)
                }
            }
        } |
        Sort-Object Version -Descending |
        Select-Object -First 1

    if ($null -eq $runtime) {
        throw "Microsoft.NETCore.App 10.x runtime was not found. Install .NET 10 or pass -RuntimeDir."
    }

    $RuntimeDir = $runtime.Path
}

$RuntimeDir = (Resolve-Path $RuntimeDir).Path
New-Item -ItemType Directory -Force $OutputDir | Out-Null

Invoke-Checked dotnet build ([System.IO.Path]::Combine($repoRoot, "src", "tools", "exportextern", "ExportExtern.csproj")) -c $Configuration

$toolDll = [System.IO.Path]::Combine($repoRoot, "out", "dotnet", "ExportExtern", $Configuration, "net8.0", "ExportExtern.dll")
if (-not (Test-Path $toolDll)) {
    throw "ExportExtern output not found: $toolDll"
}

$manifestPath = Join-Path $OutputDir "assemblies.txt"
"RuntimeDir=$RuntimeDir" | Set-Content -Path $manifestPath -Encoding utf8

foreach ($assemblyName in $Assemblies) {
    $assemblyPath = Join-Path $RuntimeDir "$assemblyName.dll"
    if (-not (Test-Path $assemblyPath)) {
        Write-Warning "Assembly not found, skipping: $assemblyPath"
        continue
    }

    $outputPath = Join-Path $OutputDir "$($assemblyName)_externs.txt"
    Invoke-Checked dotnet $toolDll $assemblyPath all $outputPath
    "$assemblyName=$assemblyPath" | Add-Content -Path $manifestPath -Encoding utf8
}

Write-Host "Exported .NET 10 externs to $OutputDir"
