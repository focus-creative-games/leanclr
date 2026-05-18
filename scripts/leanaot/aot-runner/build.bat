@echo off
setlocal enabledelayedexpansion
rem Build aot-runner native executable.
rem Usage: build.bat [Config] [Arch]
rem Optionally set LEANCLR_VS2022 to the VS root path if auto-detect fails.

call "%~dp0..\..\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"

set "AOT_RUNNER_SRC=%REPO_ROOT%\src\leanaot\aot-runner"
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"
set "ARCH=%~2"
if "%ARCH%"=="" set "ARCH=x64"

call "%REPO_ROOT%\scripts\lib\cmake-dir.bat" "leanaot\aot-runner" "%CONFIG%" "%ARCH%"

echo === Config: %CONFIG% ^| Arch: %ARCH% ===
echo Build dir: %CMAKE_BUILD_DIR%

if not exist "%CMAKE_BUILD_DIR%" mkdir "%CMAKE_BUILD_DIR%"
if errorlevel 1 goto :error

echo [1/2] CMake configure...
cmake -S "%AOT_RUNNER_SRC%" -B "%CMAKE_BUILD_DIR%" -G "Visual Studio 17 2022" -A %ARCH%
if errorlevel 1 goto :error

echo [2/2] Build target 'aot-runner'...
cmake --build "%CMAKE_BUILD_DIR%" --config %CONFIG% --target aot-runner --parallel
if errorlevel 1 goto :error

set "EXE=%CMAKE_BUILD_DIR%\bin\%CONFIG%\aot-runner.exe"
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
