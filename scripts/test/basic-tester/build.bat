@echo off
setlocal enabledelayedexpansion

call "%~dp0..\..\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"
set "ARCH=%~2"
if "%ARCH%"=="" set "ARCH=x64"

set "RUNNER_SRC_DIR=%REPO_ROOT%\src\tests\basic-tester"

call "%REPO_ROOT%\scripts\lib\cmake-dir.bat" "tests\basic-tester" "%CONFIG%" "%ARCH%"

echo === Config: %CONFIG% ^| Arch: %ARCH% ===
echo Source dir: %RUNNER_SRC_DIR%
echo Build dir: %CMAKE_BUILD_DIR%

if not exist "%CMAKE_BUILD_DIR%" mkdir "%CMAKE_BUILD_DIR%"
if errorlevel 1 goto :error

call "%REPO_ROOT%\scripts\lib\cmake-ensure-platform.bat"

echo [1/2] CMake configure...
cmake -S "%RUNNER_SRC_DIR%" -B "%CMAKE_BUILD_DIR%" -G "Visual Studio 17 2022" -A %CMAKE_GENERATOR_ARCH%
if errorlevel 1 goto :error

echo [2/2] Build target 'test'...
cmake --build "%CMAKE_BUILD_DIR%" --config %CONFIG% --target test -- /m
if errorlevel 1 goto :error

set "EXE=%CMAKE_BUILD_DIR%\bin\%CONFIG%\test.exe"
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
