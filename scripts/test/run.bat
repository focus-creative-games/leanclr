@echo off
setlocal enabledelayedexpansion

call "%~dp0..\lib\repo-root.bat"
call "%~dp0..\lib\out-dir-init.bat"

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"
set "ARCH=%~2"
if "%ARCH%"=="" set "ARCH=x64"

call "%~dp0..\lib\cmake-dir.bat" "tests\basic-tester" "%CONFIG%" "%ARCH%"
set "RUNNER=%CMAKE_BUILD_DIR%\bin\%CONFIG%\test.exe"

if not exist "%RUNNER%" (
    echo ERROR: Test runner not found at "%RUNNER%". Run 'scripts\test\build-all.bat' first.
    endlocal & exit /b 1
)

"%RUNNER%"
set "RUNNER_EXIT_CODE=%ERRORLEVEL%"

if not "%RUNNER_EXIT_CODE%"=="0" (
    echo Some tests failed.
    endlocal & exit /b %RUNNER_EXIT_CODE%
)

echo All tests passed.
endlocal & exit /b 0
