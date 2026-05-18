@echo off
setlocal
rem Build LeanAOT to the default out/dotnet layout (Directory.Build.props).
rem Usage: build.bat [Debug|Release]

call "%~dp0..\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
if /i not "%CONFIG%"=="Debug" if /i not "%CONFIG%"=="Release" (
    echo ERROR: Configuration must be Debug or Release.
    exit /b 1
)

set "LEANAOT_PROJ=%REPO_ROOT%\src\leanaot\LeanAOT\LeanAOT.csproj"
set "OUT_DIR=%OUT_ROOT%\dotnet\LeanAOT\%CONFIG%\net8.0"

echo Building LeanAOT (%CONFIG%)...
echo Output: %OUT_DIR%\

dotnet build "%LEANAOT_PROJ%" -c %CONFIG%
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo Build succeeded.
endlocal
