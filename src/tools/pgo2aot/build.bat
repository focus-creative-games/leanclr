@echo off
setlocal
rem Build pgo2aot to the default out/dotnet layout (Directory.Build.props).
rem Usage: build.bat [Debug|Release]

set "SCRIPT_DIR=%~dp0"
call "%SCRIPT_DIR%..\..\..\scripts\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
if /i not "%CONFIG%"=="Debug" if /i not "%CONFIG%"=="Release" (
    echo ERROR: Configuration must be Debug or Release.
    exit /b 1
)

set "PGO2AOT_PROJ=%SCRIPT_DIR%Pgo2Aot.csproj"
set "OUT_DIR=%OUT_ROOT%\dotnet\Pgo2Aot\%CONFIG%\net8.0"

echo Building pgo2aot (%CONFIG%)...
echo Output: %OUT_DIR%\

dotnet build "%PGO2AOT_PROJ%" -c %CONFIG%
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

if exist "%OUT_DIR%\pgo2aot.dll" (
    echo Built: "%OUT_DIR%\pgo2aot.dll"
) else (
    echo Warning: expected dll not found at "%OUT_DIR%\pgo2aot.dll"
)

echo Build succeeded.
endlocal
