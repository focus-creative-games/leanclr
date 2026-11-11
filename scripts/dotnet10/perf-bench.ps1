param(
    [string]$RuntimeDir,
    [string]$Configuration = "Release",
    [string]$ReportDir,
    [string]$AotOutputDir,
    [string]$NativeInterpBuildDir,
    [string]$NativeAotBuildDir,
    [string]$CMakeGenerator,
    [string]$CMakeArchitecture,
    [string[]]$Benchmarks = @(),
    [switch]$IncludeCoreRuntimeAot,
    [switch]$IncludeConsoleAot,
    [switch]$SkipAotBuild,
    [switch]$VerboseOutput
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

function Invoke-Captured {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,

        [Parameter(ValueFromRemainingArguments = $true)]
        [string[]]$Arguments
    )

    $output = & $FilePath @Arguments 2>&1
    $exitCode = $LASTEXITCODE
    $lines = @($output | ForEach-Object { $_.ToString() })
    if ($VerboseOutput) {
        $lines | ForEach-Object { Write-Host $_ }
    }
    if ($exitCode -ne 0) {
        $joined = $lines -join [Environment]::NewLine
        throw "$FilePath $($Arguments -join ' ') failed with exit code $exitCode$([Environment]::NewLine)$joined"
    }

    return $lines
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

function Get-NativeRunnerPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$BuildDir,

        [Parameter(Mandatory = $true)]
        [string]$ExeBaseName
    )

    $exeName = if ($env:OS -eq "Windows_NT") { "$ExeBaseName.exe" } else { $ExeBaseName }
    $candidate = [System.IO.Path]::Combine($BuildDir, "bin", $Configuration, $exeName)
    if (Test-Path $candidate) {
        return $candidate
    }

    $candidate = [System.IO.Path]::Combine($BuildDir, "bin", $exeName)
    if (Test-Path $candidate) {
        return $candidate
    }

    throw "$ExeBaseName output not found under $BuildDir"
}

function Get-CMakeConfigureArgs {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourceDir,

        [Parameter(Mandatory = $true)]
        [string]$BuildDir,

        [string[]]$ExtraArgs = @()
    )

    $args = @("-S", $SourceDir, "-B", $BuildDir)
    $args += $ExtraArgs

    if ($env:OS -eq "Windows_NT") {
        if ([string]::IsNullOrWhiteSpace($script:CMakeGenerator)) {
            $script:CMakeGenerator = "Visual Studio 17 2022"
        }
        if ([string]::IsNullOrWhiteSpace($script:CMakeArchitecture)) {
            $script:CMakeArchitecture = "x64"
        }
        if (-not [string]::IsNullOrWhiteSpace($script:CMakeGenerator)) {
            $args += @("-G", $script:CMakeGenerator)
        }
        if (-not [string]::IsNullOrWhiteSpace($script:CMakeArchitecture)) {
            $args += @("-A", $script:CMakeArchitecture)
        }
    }
    else {
        $args += "-DCMAKE_BUILD_TYPE=$Configuration"
    }

    return $args
}

function Parse-BenchmarkOutput {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Runtime,

        [Parameter(Mandatory = $true)]
        [string[]]$Lines
    )

    $results = @()
    foreach ($line in $Lines) {
        if (-not $line.StartsWith("BENCH|", [StringComparison]::Ordinal)) {
            continue
        }

        $parts = $line.Split("|")
        if ($parts.Count -ne 5) {
            throw "Malformed benchmark output from ${Runtime}: $line"
        }

        $results += [pscustomobject]@{
            Runtime = $Runtime
            Name = $parts[1]
            Iterations = [int]::Parse($parts[2], [Globalization.CultureInfo]::InvariantCulture)
            Checksum = [Int64]::Parse($parts[3], [Globalization.CultureInfo]::InvariantCulture)
            ElapsedMs = [double]::Parse($parts[4], [Globalization.CultureInfo]::InvariantCulture)
        }
    }

    if ($results.Count -eq 0) {
        $joined = $Lines -join [Environment]::NewLine
        throw "No BENCH lines were emitted by $Runtime.$([Environment]::NewLine)$joined"
    }

    return $results
}

function Format-Ratio {
    param(
        [double]$Numerator,
        [double]$Denominator
    )

    if ($Denominator -le 0) {
        return "n/a"
    }

    return (($Numerator / $Denominator).ToString("F2", [Globalization.CultureInfo]::InvariantCulture) + "x")
}

function Format-Ms {
    param([double]$Value)
    return $Value.ToString("F3", [Globalization.CultureInfo]::InvariantCulture)
}

function Select-BenchmarkEntries {
    param(
        [Parameter(Mandatory = $true)]
        [object[]]$AllEntries,

        [string[]]$RequestedNames = @()
    )

    $names = @($RequestedNames | Where-Object { -not [string]::IsNullOrWhiteSpace($_) } | ForEach-Object { $_.Trim() })
    if ($names.Count -eq 0) {
        return @($AllEntries)
    }

    $byName = @{}
    foreach ($entry in $AllEntries) {
        $byName[$entry.Name] = $entry
    }

    $selected = @()
    $seen = @{}
    foreach ($name in $names) {
        if (-not $byName.ContainsKey($name)) {
            throw "Unknown benchmark '$name'. Known benchmarks: $((@($AllEntries | ForEach-Object { $_.Name }) -join ', '))"
        }
        if (-not $seen.ContainsKey($name)) {
            $selected += $byName[$name]
            $seen[$name] = $true
        }
    }

    return @($selected)
}

$repoRoot = (Resolve-Path ([System.IO.Path]::Combine($PSScriptRoot, "..", ".."))).Path

if ([string]::IsNullOrWhiteSpace($RuntimeDir)) {
    $RuntimeDir = Get-DotNet10RuntimeDir
}
$RuntimeDir = (Resolve-Path $RuntimeDir).Path

if ([string]::IsNullOrWhiteSpace($ReportDir)) {
    $ReportDir = [System.IO.Path]::Combine($repoRoot, "artifacts", "net10-benchmarks")
}
$ReportDir = [System.IO.Path]::GetFullPath($ReportDir)
[System.IO.Directory]::CreateDirectory($ReportDir) | Out-Null

if ([string]::IsNullOrWhiteSpace($AotOutputDir)) {
    $AotOutputDir = [System.IO.Path]::Combine($repoRoot, "artifacts", "net10-aot", "benchmarks", "generated")
}
$AotOutputDir = [System.IO.Path]::GetFullPath($AotOutputDir)

if ([string]::IsNullOrWhiteSpace($NativeInterpBuildDir)) {
    $NativeInterpBuildDir = [System.IO.Path]::Combine($repoRoot, "out", "cmake", "tools", "leanrun-bench", "$Configuration-x64")
}
$NativeInterpBuildDir = [System.IO.Path]::GetFullPath($NativeInterpBuildDir)

if ([string]::IsNullOrWhiteSpace($NativeAotBuildDir)) {
    $NativeAotBuildDir = [System.IO.Path]::Combine($repoRoot, "out", "cmake", "tests", "net10-aot-benchmarks", "$Configuration-x64")
}
$NativeAotBuildDir = [System.IO.Path]::GetFullPath($NativeAotBuildDir)

$benchmarkProject = [System.IO.Path]::Combine($repoRoot, "src", "tests", "managed-net10", "ManagedNet10.Benchmarks", "ManagedNet10.Benchmarks.csproj")
$solutionPath = [System.IO.Path]::Combine($repoRoot, "src", "tests", "managed-net10", "managed-net10.sln")
$leanAotProject = [System.IO.Path]::Combine($repoRoot, "src", "leanaot", "LeanAOT", "LeanAOT.csproj")
$benchmarkDir = [System.IO.Path]::Combine($repoRoot, "out", "dotnet", "ManagedNet10.Benchmarks", $Configuration, "net10.0")
$benchmarkDll = [System.IO.Path]::Combine($benchmarkDir, "ManagedNet10.Benchmarks.dll")
$allBenchmarkEntries = @(
    [pscustomobject]@{ Name = "IntegerArithmetic"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunIntegerArithmetic" },
    [pscustomobject]@{ Name = "BranchingLoop"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunBranchingLoop" },
    [pscustomobject]@{ Name = "ArrayTraversal"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunArrayTraversal" },
    [pscustomobject]@{ Name = "VirtualDispatch"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunVirtualDispatch" },
    [pscustomobject]@{ Name = "DelegateInvoke"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunDelegateInvoke" },
    [pscustomobject]@{ Name = "ObjectAllocation"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunObjectAllocation" },
    [pscustomobject]@{ Name = "StringScan"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunStringScan" },
    [pscustomobject]@{ Name = "ListAppendAndSum"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunListAppendAndSum" },
    [pscustomobject]@{ Name = "DictionaryLookup"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunDictionaryLookup" },
    [pscustomobject]@{ Name = "StringBuilderBuild"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunStringBuilderBuild" },
    [pscustomobject]@{ Name = "ParseAndFormatNumbers"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunParseAndFormatNumbers" },
    [pscustomobject]@{ Name = "InterfaceTypeChecks"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunInterfaceTypeChecks" },
    [pscustomobject]@{ Name = "GenericEquality"; Entry = "ManagedNet10.Benchmarks.AotBenchmarkHost::RunGenericEquality" }
)
$selectedBenchmarkEntries = Select-BenchmarkEntries -AllEntries $allBenchmarkEntries -RequestedNames $Benchmarks
$selectedBenchmarkNames = @($selectedBenchmarkEntries | ForEach-Object { $_.Name })

Write-Host "Building managed benchmark project..."
Invoke-Checked dotnet build $solutionPath -c $Configuration

if (-not (Test-Path $benchmarkDll)) {
    throw "ManagedNet10.Benchmarks output not found: $benchmarkDll"
}

$cmakePath = Resolve-CMakePath

Write-Host "Building LeanCLR interpreter runner..."
$leanrunSourceDir = [System.IO.Path]::Combine($repoRoot, "src", "tools", "leanrun")
$interpConfigureArgs = Get-CMakeConfigureArgs -SourceDir $leanrunSourceDir -BuildDir $NativeInterpBuildDir
Invoke-Checked -FilePath $cmakePath -Arguments $interpConfigureArgs
Invoke-Checked -FilePath $cmakePath -Arguments @("--build", $NativeInterpBuildDir, "--config", $Configuration, "--target", "leanrun", "--parallel")
$leanrun = Get-NativeRunnerPath -BuildDir $NativeInterpBuildDir -ExeBaseName "leanrun"

if (-not $SkipAotBuild) {
    Write-Host "Generating LeanAOT benchmark C++..."
    Clear-GeneratedOutputDirectory $AotOutputDir
    Invoke-Checked dotnet build $leanAotProject -c $Configuration

    $leanAotDll = [System.IO.Path]::Combine($repoRoot, "out", "dotnet", "LeanAOT", $Configuration, "net8.0", "LeanAOT.dll")
    if (-not (Test-Path $leanAotDll)) {
        throw "LeanAOT output not found: $leanAotDll"
    }

    $leanAotArgs = @(
        $leanAotDll,
        "--leanaot-runtime-api-profile",
        "coreclr-net10",
        "-d",
        $benchmarkDir,
        "-d",
        $RuntimeDir
    )

    if ($IncludeCoreRuntimeAot) {
        $profilePath = [System.IO.Path]::Combine($repoRoot, "src", "leanaot", "LeanAOT", "runtime-apis", "coreclr-net10", "profile.json")
        $profile = Get-Content -Raw $profilePath | ConvertFrom-Json
        foreach ($moduleName in @($profile.coreLibraryModules)) {
            if ($moduleName -eq "System.Console" -and -not $IncludeConsoleAot) {
                continue
            }
            $assemblyPath = [System.IO.Path]::Combine($RuntimeDir, "$moduleName.dll")
            if (Test-Path $assemblyPath) {
                $leanAotArgs += @("-a", $moduleName)
            }
        }
    }

    $leanAotArgs += @(
        "-a",
        "ManagedNet10.Benchmarks",
        "-o",
        $AotOutputDir
    )

    Invoke-Checked -FilePath dotnet -Arguments $leanAotArgs

    foreach ($fileName in @(
        "ManagedNet10_Benchmarks.module_registration.cpp",
        "ManagedNet10_Benchmarks.method_body_part1.cpp",
        "modules_registration.cpp",
        "method_invokers_part0.cpp",
        "method_direct_call_bridges_part0.cpp"
    )) {
        $path = [System.IO.Path]::Combine($AotOutputDir, $fileName)
        if (-not (Test-Path $path)) {
            throw "Expected LeanAOT generated file was not found: $path"
        }
    }

    Write-Host "Building LeanAOT benchmark runner..."
    $aotTesterSourceDir = [System.IO.Path]::Combine($repoRoot, "src", "tests", "aot-tester")
    $aotConfigureArgs = Get-CMakeConfigureArgs `
        -SourceDir $aotTesterSourceDir `
        -BuildDir $NativeAotBuildDir `
        -ExtraArgs @("-DAOT_GENERATED_CPP_DIR=$AotOutputDir")
    Invoke-Checked -FilePath $cmakePath -Arguments $aotConfigureArgs
    Invoke-Checked -FilePath $cmakePath -Arguments @("--build", $NativeAotBuildDir, "--config", $Configuration, "--target", "aot-tester", "--parallel")
}

$aotTester = Get-NativeRunnerPath -BuildDir $NativeAotBuildDir -ExeBaseName "aot-tester"

Write-Host "Running .NET 10 native benchmarks..."
$nativeArgs = @($benchmarkDll)
$nativeArgs += $selectedBenchmarkNames
$nativeLines = Invoke-Captured -FilePath dotnet -Arguments $nativeArgs
$nativeResults = Parse-BenchmarkOutput -Runtime ".NET 10 Native" -Lines $nativeLines

Write-Host "Running LeanCLR interpreter benchmarks..."
$interpArgs = @(
    "-l",
    $benchmarkDir,
    "-l",
    $RuntimeDir,
    "ManagedNet10.Benchmarks"
)
$interpArgs += "--"
$interpArgs += $selectedBenchmarkNames
$interpLines = Invoke-Captured -FilePath $leanrun -Arguments $interpArgs
$interpResults = Parse-BenchmarkOutput -Runtime "LeanCLR Interpreter" -Lines $interpLines

Write-Host "Running LeanCLR AOT benchmarks..."
$aotLines = @()
foreach ($benchmark in $selectedBenchmarkEntries) {
    $aotLines += Invoke-Captured -FilePath $aotTester -Arguments @(
        "-l",
        $benchmarkDir,
        "-l",
        $RuntimeDir,
        "-e",
        $benchmark.Entry,
        "ManagedNet10.Benchmarks"
    )
}
$aotResults = Parse-BenchmarkOutput -Runtime "LeanCLR AOT" -Lines $aotLines

$nativeByName = @{}
$interpByName = @{}
$aotByName = @{}
$nativeResults | ForEach-Object { $nativeByName[$_.Name] = $_ }
$interpResults | ForEach-Object { $interpByName[$_.Name] = $_ }
$aotResults | ForEach-Object { $aotByName[$_.Name] = $_ }

$rows = @()
foreach ($native in $nativeResults) {
    if (-not $interpByName.ContainsKey($native.Name)) {
        throw "Interpreter result missing benchmark: $($native.Name)"
    }
    if (-not $aotByName.ContainsKey($native.Name)) {
        throw "AOT result missing benchmark: $($native.Name)"
    }

    $interp = $interpByName[$native.Name]
    $aot = $aotByName[$native.Name]
    if ($native.Checksum -ne $interp.Checksum -or $native.Checksum -ne $aot.Checksum) {
        throw "Checksum mismatch for $($native.Name): native=$($native.Checksum), interp=$($interp.Checksum), aot=$($aot.Checksum)"
    }

    $rows += [pscustomobject]@{
        Benchmark = $native.Name
        Iterations = $native.Iterations
        Checksum = $native.Checksum
        NativeMs = $native.ElapsedMs
        InterpMs = $interp.ElapsedMs
        AotMs = $aot.ElapsedMs
        InterpVsNative = if ($native.ElapsedMs -gt 0) { $interp.ElapsedMs / $native.ElapsedMs } else { [double]::NaN }
        AotVsNative = if ($native.ElapsedMs -gt 0) { $aot.ElapsedMs / $native.ElapsedMs } else { [double]::NaN }
        AotSpeedupVsInterp = if ($aot.ElapsedMs -gt 0) { $interp.ElapsedMs / $aot.ElapsedMs } else { [double]::NaN }
    }
}

$csvPath = [System.IO.Path]::Combine($ReportDir, "net10-performance.csv")
$jsonPath = [System.IO.Path]::Combine($ReportDir, "net10-performance.json")
$markdownPath = [System.IO.Path]::Combine($ReportDir, "net10-performance.md")

$rows | Export-Csv -NoTypeInformation -Encoding utf8 -Path $csvPath
$rows | ConvertTo-Json -Depth 4 | Set-Content -Encoding utf8 -Path $jsonPath

$markdown = New-Object System.Collections.Generic.List[string]
$markdown.Add("# LeanCLR .NET 10 Performance Benchmarks")
$markdown.Add("")
$markdown.Add("- Generated: $((Get-Date).ToString("yyyy-MM-dd HH:mm:ss zzz"))")
$markdown.Add("- Configuration: ``$Configuration``")
$markdown.Add("- Runtime pack: ``$RuntimeDir``")
$markdown.Add("- Benchmark assembly: ``$benchmarkDll``")
$markdown.Add("- AOT generated C++: ``$AotOutputDir``")
$markdown.Add("- Selected benchmarks: ``$($selectedBenchmarkNames -join ', ')``")
$coreRuntimeAotLabel = if ($IncludeCoreRuntimeAot) {
    if ($IncludeConsoleAot) {
        "included in benchmark generated directory"
    }
    else {
        "included in benchmark generated directory (System.Console excluded for benchmark output)"
    }
}
else {
    "not linked into this benchmark runner"
}
$markdown.Add("- AOT core runtime modules: $coreRuntimeAotLabel")
$markdown.Add("")
$markdown.Add("| Benchmark | Iterations | .NET 10 Native (ms) | LeanCLR Interpreter (ms) | LeanCLR AOT (ms) | Interp/.NET | AOT/.NET | AOT speedup vs interp |")
$markdown.Add("| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |")
foreach ($row in $rows) {
    $markdown.Add(
        "| $($row.Benchmark) | $($row.Iterations) | $(Format-Ms $row.NativeMs) | $(Format-Ms $row.InterpMs) | $(Format-Ms $row.AotMs) | $(Format-Ratio $row.InterpMs $row.NativeMs) | $(Format-Ratio $row.AotMs $row.NativeMs) | $(Format-Ratio $row.InterpMs $row.AotMs) |"
    )
}
$markdown.Add("")
$markdown.Add("Checksums are validated across all three runtimes before this report is written.")
if ($IncludeCoreRuntimeAot) {
    $markdown.Add("LeanCLR AOT runs each benchmark through a small AOT-specific host with checksums matched against the native and interpreter runs.")
}
$markdown | Set-Content -Encoding utf8 -Path $markdownPath

Write-Host ""
Get-Content $markdownPath | ForEach-Object { Write-Host $_ }
Write-Host ""
Write-Host "Wrote benchmark reports:"
Write-Host "  $markdownPath"
Write-Host "  $csvPath"
Write-Host "  $jsonPath"
