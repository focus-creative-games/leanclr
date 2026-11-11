param(
    [string]$RuntimeDir,
    [string]$Configuration = "Release",
    [string]$BuildDir = "out/net10-baseline",
    [string]$ContractsBuildDir
)
$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
function Invoke-Checked {
    $FilePath = $args[0]
    $Arguments = $args[1..($args.Length - 1)]
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$FilePath failed with exit code $LASTEXITCODE" }
}
Push-Location $repoRoot
try {
    if (-not $RuntimeDir) {
        $runtime = dotnet --list-runtimes | ForEach-Object {
            if ($_ -match '^Microsoft\.NETCore\.App (10\.0\.\d+) \[(.+)\]$') {
                [pscustomobject]@{Version=[version]$Matches[1]; Path=(Join-Path $Matches[2] $Matches[1])}
            }
        } | Sort-Object Version -Descending | Select-Object -First 1
        if (-not $runtime) { throw "Install a .NET 10 runtime or pass -RuntimeDir" }
        $RuntimeDir = $runtime.Path
    }
    $RuntimeDir = (Resolve-Path $RuntimeDir).Path
    Write-Host "CoreLib under test: $RuntimeDir"
    Get-FileHash (Join-Path $RuntimeDir "System.Private.CoreLib.dll") -Algorithm SHA256 | Format-List
    Invoke-Checked dotnet build src/tests/managed-net10/ManagedNet10.Smoke/ManagedNet10.Smoke.csproj -c $Configuration
    Invoke-Checked cmake -S src/tools/leanrun -B $BuildDir -A x64 -DLEANCLR_BCL_ADAPTER=CORECLR_NET10
    Invoke-Checked cmake --build $BuildDir --config $Configuration --parallel 4
    $runner = Join-Path $BuildDir "bin/$Configuration/leanrun.exe"
    $assemblyDir = "out/dotnet/ManagedNet10.Smoke/$Configuration/net10.0"
    $entries = @(
        "TestBasics", "TestPairArithmetic", "TestBoxOnly", "TestBoxGetTypeOnly",
        "TestTypeOfNoCompare", "TestTypeOfNameOnly", "TestPayloadCtorOnly",
        "TestStaticDelegateInvokeOnly", "TestThrowCatchOnly", "TestExceptionFilterOnly",
        "TestExceptionMessageReadOnly", "TestGenericsDelegatesAndExceptions",
        "TestBoxingMetadata", "TestReflectionTypeOnly", "TestTypeEqualityOnly",
        "TestNestedThrowCatchOnly", "TestThrowTargetInvocationCatchOnly", "TestSpan",
        "TestSpanStackalloc", "TestExceptionFilterControlFlow"
    )
    foreach ($entry in $entries) {
        Write-Host "Running $entry"
        Invoke-Checked $runner -l $assemblyDir -l $RuntimeDir -e "ManagedNet10.Smoke.Program::$entry" ManagedNet10.Smoke
    }
    $contracts = if ($ContractsBuildDir) { $ContractsBuildDir } else { "$BuildDir-contracts" }
    Invoke-Checked cmake -S src/tests/net10-contracts -B $contracts -A x64
    Invoke-Checked cmake --build $contracts --config $Configuration --parallel 4
    Invoke-Checked ctest --test-dir $contracts -C $Configuration --output-on-failure
    Write-Host "$($entries.Count) managed smoke entries and dependent-handle contracts passed"
}
finally { Pop-Location }
