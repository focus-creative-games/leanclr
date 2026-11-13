param([string]$OutputDir = "out/assembly-stream")
$ErrorActionPreference='Stop'
$root=(Resolve-Path "$PSScriptRoot/../../..").Path
Push-Location $root
try {
function Checked { $exe=$args[0]; $a=$args[1..($args.Length-1)]; & $exe @a; if($LASTEXITCODE -ne 0){throw "Build failed: $exe"} }
Checked dotnet build src/tests/assembly-stream/Fixture/Fixture.csproj -c Release -o "$OutputDir/fixture" --nologo
Checked dotnet build src/tests/assembly-stream/Other/Other.csproj -c Release -o "$OutputDir/other" --nologo
$files=[ordered]@{Fixture="$OutputDir/fixture/StreamFixture.dll";Dependency="$OutputDir/fixture/StreamDependency.dll";Symbols="$OutputDir/fixture/StreamFixture.pdb";Other="$OutputDir/other/StreamFixture.dll"}
$generated='namespace AssemblyStreamTests { public static class Data {'
foreach($name in $files.Keys){$bytes=[System.IO.File]::ReadAllBytes((Resolve-Path $files[$name]).Path);$encoded=[Convert]::ToBase64String($bytes);$generated+="public static byte[] $name(){return System.Convert.FromBase64String(`"$encoded`");}"}
$generated+='}}'; Set-Content "$OutputDir/FixtureData.g.cs" $generated
$generatedPath=(Resolve-Path "$OutputDir/FixtureData.g.cs").Path
Checked dotnet build src/tests/assembly-stream/AssemblyStreamTests.csproj -c Release -o "$OutputDir/managed" "-p:FixtureDataFile=$generatedPath" --nologo
} finally { Pop-Location }

