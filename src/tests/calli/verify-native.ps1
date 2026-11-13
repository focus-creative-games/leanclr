param(
    [string]$BuildRoot = "out/calli/verification",
    [string]$RuntimeDir,
    [switch]$AbiProbe,
    [switch]$Benchmark
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
    if (-not [IO.Path]::IsPathRooted($BuildRoot)) { $BuildRoot = Join-Path $repo $BuildRoot }
    $BuildRoot = [IO.Path]::GetFullPath($BuildRoot).Replace("\", "/")
    & "$PSScriptRoot/build-fixtures.ps1" -OutputDir $BuildRoot
    Invoke-Checked dotnet @("build", "src/leanaot/LeanAOT/LeanAOT.csproj", "-c", "Release")
    $aot = Join-Path $repo "out/dotnet/LeanAOT/Release/net8.0/LeanAOT.dll"
    if (Test-Path "src/runtime/bcl/coreclr-net10/pinvokes/coreclr_qcall.cpp") {
        $adapter = "CORECLR_NET10"; $profile = "coreclr-net10"; $managed = "$BuildRoot/net10"
        if (-not $RuntimeDir) {
            $RuntimeDir = dotnet --list-runtimes | ForEach-Object {
                if ($_ -match '^Microsoft\.NETCore\.App (10\.0\.\d+) \[(.+)\]$') { Join-Path $Matches[2] $Matches[1] }
            } | Select-Object -Last 1
        }
    } else {
        $adapter = "MONO"; $profile = "mono-4.5"; $managed = "$BuildRoot/mono"
        if (-not $RuntimeDir) { $RuntimeDir = "$repo/src/libraries/mono-4.5" }
    }
    if (-not $RuntimeDir) { throw "A matching runtime directory is required" }
    $RuntimeDir = $RuntimeDir.Replace("\", "/")
    foreach ($mode in @("bridges-only", "aot")) {
        $generated = "$BuildRoot/generated-$mode"
        $arguments = @($aot, "--leanaot-runtime-api-profile", $profile, "-d", $managed, "-d", $RuntimeDir,
            "-a", "CalliTests", "-a", "CalliILTests", "-o", $generated)
        if ($mode -eq "bridges-only") { $arguments += "--calli-bridges-only" }
        Invoke-Checked dotnet $arguments
        $inventory = Get-Content "$generated/calli_bridges.json" -Raw | ConvertFrom-Json
        $unique = @($inventory.sites | Where-Object status -eq generated | Select-Object -ExpandProperty signature -Unique)
        if ($unique.Count -ne $inventory.bridge_count -or "1:9(9)" -notin $unique -or "1:9(9,9)" -notin $unique -or "1:11(11,11)" -notin $unique) { throw "Signature dedup or interpreter-only generic closure census failed" }
        if ($mode -eq "bridges-only" -and (Get-ChildItem $generated -Filter "*.method_body*.cpp")) { throw "Bridge-only generation emitted managed bodies" }
        $build = "$BuildRoot/native-$mode"
        $options = @("-S", "src/tests/calli", "-B", $build, "-DLEANCLR_BCL_ADAPTER=$adapter", "-DLEANCLR_HOST=NATIVE",
            "-DLEANCLR_CALLI_MANAGED_DIR=$managed", "-DLEANCLR_CALLI_RUNTIME_DIR=$RuntimeDir")
        if ($IsWindows -or $env:OS -eq "Windows_NT") { $options += @("-A", "x64") }
        else { $options += @("-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release") }
        if ($mode -eq "aot") { $options += "-DLEANCLR_CALLI_AOT_CPP_DIR=$generated" }
        else { $options += "-DLEANCLR_CALLI_BRIDGE_CPP_DIR=$generated" }
        if ($AbiProbe) { $options += "-DCMAKE_CXX_FLAGS=-DLEANCLR_CALLI_ABI_PROBE=1" }
        Invoke-Checked cmake $options
        Invoke-Checked cmake @("--build", $build, "--config", "Release", "--parallel", "4")
        Invoke-Checked ctest @("--test-dir", $build, "-C", "Release", "--output-on-failure")
        if ($Benchmark) {
            $exe = if ($IsWindows -or $env:OS -eq "Windows_NT") { "$build/Release/calli-native.exe" } else { "$build/calli-native" }
            Invoke-Checked $exe @($managed, $RuntimeDir, "--benchmark")
        }
    }
    Invoke-Checked dotnet @("run", "--project", "src/tests/calli/il-fixture/CalliIlFixture.csproj", "-c", "Release", "--",
        "$BuildRoot/net10/CalliTests.dll", "$BuildRoot/net10/CalliILTests.dll", "verify")
    $fixture = if ($IsWindows -or $env:OS -eq "Windows_NT") { "$BuildRoot/native-aot/Release/calli-fixture-shared.dll" }
        else { "$BuildRoot/native-aot/libcalli-fixture-shared.so" }
    foreach ($entry in @("SharedSuite", "Unmanaged", "NativeStructs", "NativeGeneric")) {
        Invoke-Checked dotnet @("$BuildRoot/net10/CalliTests.dll", $entry, $fixture)
    }
    if ($IsWindows -or $env:OS -eq "Windows_NT") { Invoke-Checked dotnet @("$BuildRoot/net10/CalliTests.dll", "NativeChar", $fixture) }
} finally { Pop-Location }
