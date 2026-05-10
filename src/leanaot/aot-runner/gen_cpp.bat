@echo off
setlocal
cd /d "%~dp0"

echo [1/3] Building leanaot Test project (Debug)...
dotnet build "..\..\leanaot\Test\Test.csproj" -c Debug
if errorlevel 1 (
	echo Test build failed.
	exit /b %ERRORLEVEL%
)

echo [2/3] Building LeanAOT (Debug)...
dotnet build "..\LeanAOT\LeanAOT.csproj" -c Debug
if errorlevel 1 (
	echo LeanAOT build failed.
	exit /b %ERRORLEVEL%
)

echo [3/3] Running LeanAOT...
"..\LeanAOT\bin\Debug\net8.0\LeanAOT.exe" ^
  -o cpp ^
  -d ..\..\libraries\dotnetframework4.x ^
  -d ..\..\leanaot\Test\bin\Debug ^
  --leanaot-aot-rule-file "%~dp0aot-rules-mscorlib.xml" ^
  --leanaot-aot-rule-file "%~dp0aot-rules-test.xml" ^
  -a mscorlib ^
  -a System ^
  -a System.Core ^
  -a Test

if errorlevel 1 exit /b %ERRORLEVEL%

echo Done.
endlocal
