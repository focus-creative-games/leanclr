param(
    [string]$OutputDir,
    [string]$RepositoryUrl = "https://github.com/dotnet/runtime.git",
    [string]$Ref = "v10.0.9",
    [string[]]$SparsePaths = @(
        "src/coreclr/vm",
        "src/libraries/System.Private.CoreLib/src",
        "docs/design/coreclr"
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

function Invoke-CheckedWithRetry {
    param(
        [int]$Attempts,
        [int]$DelaySeconds,
        [Parameter(Mandatory = $true)]
        [string]$FilePath,
        [string[]]$Arguments
    )

    for ($attempt = 1; $attempt -le $Attempts; $attempt++) {
        try {
            Invoke-Checked -FilePath $FilePath -Arguments $Arguments
            return
        }
        catch {
            if ($attempt -ge $Attempts) {
                throw
            }

            Write-Warning "$FilePath $($Arguments -join ' ') failed on attempt $attempt of $Attempts. Retrying in $DelaySeconds seconds."
            Start-Sleep -Seconds $DelaySeconds
        }
    }
}

$repoRoot = (Resolve-Path ([System.IO.Path]::Combine($PSScriptRoot, "..", ".."))).Path

if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    $OutputDir = [System.IO.Path]::Combine($repoRoot, "artifacts", "dotnet10-runtime-src")
}

$OutputDir = [System.IO.Path]::GetFullPath($OutputDir)
$parentDir = [System.IO.Path]::GetDirectoryName($OutputDir)
New-Item -ItemType Directory -Force $parentDir | Out-Null

if (-not (Test-Path (Join-Path $OutputDir ".git"))) {
    if ((Test-Path $OutputDir) -and ((Get-ChildItem -Force $OutputDir | Measure-Object).Count -gt 0)) {
        throw "OutputDir exists but is not an empty git repository: $OutputDir"
    }

    New-Item -ItemType Directory -Force $OutputDir | Out-Null
    Invoke-Checked git -C $OutputDir init
    Invoke-Checked git -C $OutputDir remote add origin $RepositoryUrl
}
else {
    & git -C $OutputDir rev-parse --is-inside-work-tree | Out-Null
    if ($LASTEXITCODE -ne 0) {
        throw "OutputDir is not a valid git work tree: $OutputDir"
    }

    Invoke-Checked git -C $OutputDir remote set-url origin $RepositoryUrl
}

Invoke-Checked git -C $OutputDir config http.version HTTP/1.1
Invoke-Checked git -C $OutputDir sparse-checkout init --cone
Invoke-Checked git -C $OutputDir sparse-checkout set @SparsePaths
Invoke-CheckedWithRetry -Attempts 3 -DelaySeconds 5 -FilePath git -Arguments @("-C", $OutputDir, "fetch", "--depth", "1", "--filter=blob:none", "--no-tags", "origin", $Ref)
Invoke-CheckedWithRetry -Attempts 3 -DelaySeconds 5 -FilePath git -Arguments @("-C", $OutputDir, "checkout", "--detach", "FETCH_HEAD")

$commit = (& git -C $OutputDir rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0) {
    throw "git rev-parse failed under $OutputDir"
}

foreach ($path in $SparsePaths) {
    $fullPath = Join-Path $OutputDir $path
    if (-not (Test-Path $fullPath)) {
        throw "Sparse checkout path was not found: $fullPath"
    }
}

$manifestPath = Join-Path $OutputDir "leanclr-runtime-source-manifest.txt"
$manifest = @(
    "RepositoryUrl=$RepositoryUrl",
    "Ref=$Ref",
    "Commit=$commit",
    "GeneratedAtUtc=$([System.DateTime]::UtcNow.ToString("o"))",
    "SparsePaths="
)
$manifest += $SparsePaths | ForEach-Object { "  $_" }
$manifest | Set-Content -Path $manifestPath -Encoding utf8

Write-Host "Fetched dotnet/runtime source reference:"
Write-Host "  OutputDir: $OutputDir"
Write-Host "  Ref:       $Ref"
Write-Host "  Commit:    $commit"
Write-Host "  Manifest:  $manifestPath"
