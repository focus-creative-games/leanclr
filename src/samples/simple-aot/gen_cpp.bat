@echo off
setlocal
cd /d "%~dp0"

echo [1/2] Building leanaot Test project (Debug)...
dotnet build "..\..\leanaot\Test\Test.csproj" -c Debug
if errorlevel 1 (
	echo Test build failed.
	exit /b %ERRORLEVEL%
)

echo [2/2] Running LeanAOT...
..\..\tools\leanaot\LeanAOT.exe ^
  -o cpp ^
  -d ..\..\libraries\dotnetframework4.x ^
  -d ..\..\leanaot\Test\bin\Debug ^
  -a mscorlib ^
  -a Test

if errorlevel 1 exit /b %ERRORLEVEL%

echo Done.
endlocal
