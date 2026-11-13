param(
    [string]$OutputDir = "out/calli",
    [string]$MonoBclDir
)
$ErrorActionPreference = "Stop"
$repo = (Resolve-Path (Join-Path $PSScriptRoot "../../..")).Path
function Invoke-Checked {
    param([string]$Command, [string[]]$Arguments)
    & $Command @Arguments
    if ($LASTEXITCODE) { throw "$Command failed: $LASTEXITCODE" }
}
Push-Location $repo
try {
    if (-not $MonoBclDir) { $MonoBclDir = Join-Path $repo "src/libraries/mono-4.5" }
    if (-not [IO.Path]::IsPathRooted($OutputDir)) { $OutputDir = Join-Path $repo $OutputDir }
    $OutputDir = [IO.Path]::GetFullPath($OutputDir)
    $net10 = Join-Path $OutputDir "net10"
    $mono = Join-Path $OutputDir "mono"
    New-Item -ItemType Directory -Force $net10, $mono | Out-Null
    Invoke-Checked dotnet @("build", "src/tests/calli/CalliTests.csproj", "-c", "Release", "-o", $net10)
    $sdk = dotnet --list-sdks | ForEach-Object {
        if ($_ -match '^(10\.\S+) \[(.+)\]$') { Join-Path $Matches[2] "$($Matches[1])/Roslyn/bincore/csc.dll" }
    } | Select-Object -Last 1
    if (-not $sdk) { throw ".NET 10 SDK is required for function-pointer fixtures" }
    $sources = @("src/tests/calli/Program.cs", "src/tests/managed/SharedTests/Instructions/Funcs/TC_calli.cs",
        "src/tests/managed/Common/TestCaseBase.cs", "src/tests/managed/Common/UnitTestAttribute.cs", "src/tests/managed/Common/Assert.cs")
    Invoke-Checked dotnet (@($sdk, "-nologo", "-unsafe", "-target:library", "-nostdlib", "-r:$MonoBclDir/mscorlib.dll", "-r:$MonoBclDir/System.dll", "-out:$mono/CalliTests.dll") + $sources)
    foreach ($dir in @($net10, $mono)) {
        Invoke-Checked dotnet @("run", "--project", "src/tests/calli/il-fixture/CalliIlFixture.csproj", "-c", "Release", "--", "$dir/CalliTests.dll", "$dir/CalliILTests.dll")
    }
} finally { Pop-Location }
