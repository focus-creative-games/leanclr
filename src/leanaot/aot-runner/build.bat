@echo off
setlocal enabledelayedexpansion
rem If VS Installer does not recognize VS 2022 (e.g. after cleaning Packages): set LEANCLR_VS2022 to the VS root path, e.g.:
rem   set "LEANCLR_VS2022=D:\Program Files\Microsoft Visual Studio\2022\Community"
rem Optionally set LEANCLR_VS2022_VERSION to the 4-part build (e.g. 17.2.32126.315) if auto-detect fails.

rem Directory of this script
set "SCRIPT_DIR=%~dp0"
set "BUILD_DIR=%SCRIPT_DIR%build"

rem Args: CONFIG (Debug/Release), ARCH (x64/x86)
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"
set "ARCH=%~2"
if "%ARCH%"=="" set "ARCH=x64"

echo === Config: %CONFIG% ^| Arch: %ARCH% ===

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if errorlevel 1 goto :error

echo [1/2] CMake configure...
rem Avoid trailing backslash in quoted -S path (Windows arg parsing)
cmake -S "%SCRIPT_DIR%." -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A %ARCH%
if errorlevel 1 goto :error

echo [2/2] Build target 'aot-runner'...
cmake --build "%BUILD_DIR%" --config %CONFIG% --target aot-runner --parallel
if errorlevel 1 goto :error

set "EXE=%BUILD_DIR%\bin\%CONFIG%\aot-runner.exe"
if exist "%EXE%" (
  echo Built: "%EXE%"
) else (
  echo Warning: expected exe not found at "%EXE%"
)

echo Done.
endlocal
exit /b 0

:error
echo Build failed with error code %ERRORLEVEL%.
endlocal & exit /b %ERRORLEVEL%
