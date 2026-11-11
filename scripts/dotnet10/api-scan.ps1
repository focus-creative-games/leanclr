param(
    [string]$NkgRoot,
    [string]$Configuration = "Release",
    [string]$WhitelistPath,
    [string]$ReportDir,
    [switch]$SkipNkg,
    [switch]$WarnOnly
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

function Invoke-Net10ApiScan {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Inputs,

        [Parameter(Mandatory = $true)]
        [string]$ReportPath
    )

    $scanArgs = @(
        "run",
        "--project",
        $scannerProject,
        "-c",
        $Configuration,
        "--"
    )
    foreach ($inputPath in $Inputs) {
        $scanArgs += @("--input", $inputPath)
    }
    $scanArgs += @("--whitelist", $WhitelistPath, "--report", $ReportPath)
    if ($WarnOnly) {
        $scanArgs += "--warn-only"
    }

    Invoke-Checked -FilePath dotnet -Arguments $scanArgs
}

$repoRoot = (Resolve-Path ([System.IO.Path]::Combine($PSScriptRoot, "..", ".."))).Path

if ([string]::IsNullOrWhiteSpace($WhitelistPath)) {
    $WhitelistPath = [System.IO.Path]::Combine($repoRoot, "src", "tools", "net10apiscan", "minimal-net10-whitelist.json")
}
$WhitelistPath = (Resolve-Path $WhitelistPath).Path

if ([string]::IsNullOrWhiteSpace($ReportDir)) {
    $ReportDir = [System.IO.Path]::Combine($repoRoot, "out", "reports", "net10-api-scan")
}
$ReportDir = [System.IO.Path]::GetFullPath($ReportDir)
New-Item -ItemType Directory -Force -Path $ReportDir | Out-Null

$scannerProject = [System.IO.Path]::Combine($repoRoot, "src", "tools", "net10apiscan", "Net10ApiScan.csproj")
Invoke-Checked -FilePath dotnet -Arguments @("build", $scannerProject, "-c", $Configuration)
Invoke-Checked -FilePath dotnet -Arguments @(
    "build",
    [System.IO.Path]::Combine($repoRoot, "src", "tests", "managed-net10", "managed-net10.sln"),
    "-c",
    $Configuration
)

$managedInputs = @(
    [System.IO.Path]::Combine($repoRoot, "out", "dotnet", "ManagedNet10.Smoke", $Configuration, "net10.0", "ManagedNet10.Smoke.dll"),
    [System.IO.Path]::Combine($repoRoot, "out", "dotnet", "ManagedNet10.NkgSmoke", $Configuration, "net10.0", "ManagedNet10.NkgSmoke.dll")
)
Invoke-Net10ApiScan -Inputs $managedInputs -ReportPath ([System.IO.Path]::Combine($ReportDir, "managed-net10-smoke.json"))

if (-not $SkipNkg) {
    if ([string]::IsNullOrWhiteSpace($NkgRoot)) {
        $candidateRoot = [System.IO.Path]::GetFullPath([System.IO.Path]::Combine($repoRoot, "..", "NKGGameFramework"))
        if (Test-Path $candidateRoot) {
            $NkgRoot = $candidateRoot
        }
    }

    if ([string]::IsNullOrWhiteSpace($NkgRoot) -or -not (Test-Path $NkgRoot)) {
        throw "NKGGameFramework root was not found. Pass -NkgRoot or use -SkipNkg."
    }

    $NkgRoot = (Resolve-Path $NkgRoot).Path
    $samplerProject = [System.IO.Path]::Combine($NkgRoot, "samples", "NKGGameFramework.Sampler", "NKGGameFramework.Sampler.csproj")
    Invoke-Checked -FilePath dotnet -Arguments @("build", $samplerProject, "-c", $Configuration)

    $samplerOutputDir = [System.IO.Path]::Combine($NkgRoot, "samples", "NKGGameFramework.Sampler", "bin", $Configuration, "net10.0")
    $nkgInputs = @(
        [System.IO.Path]::Combine($samplerOutputDir, "NKGGameFramework.dll"),
        [System.IO.Path]::Combine($samplerOutputDir, "OdinSerializer.dll"),
        [System.IO.Path]::Combine($samplerOutputDir, "UniTask.dll")
    )
    Invoke-Net10ApiScan -Inputs $nkgInputs -ReportPath ([System.IO.Path]::Combine($ReportDir, "nkg-core.json"))
}
