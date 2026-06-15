@echo off
setlocal

call "%~dp0build.bat" %~1 %~2
if errorlevel 1 exit /b %ERRORLEVEL%

call "%~dp0..\..\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"
set "ARCH=%~2"
if "%ARCH%"=="" set "ARCH=x64"

call "%REPO_ROOT%\scripts\lib\cmake-dir.bat" "tests\aot-tester" "%CONFIG%" "%ARCH%"

set "RUNNER=%CMAKE_BUILD_DIR%\bin\%CONFIG%\aot-tester.exe"
if not exist "%RUNNER%" (
    echo ERROR: aot-tester not found at "%RUNNER%".
    exit /b 1
)

set "AOTTEST_DLL_DIR=%OUT_ROOT%\dotnet\AotTests\%CONFIG%"
set "COMMON_DLL=%OUT_ROOT%\dotnet\Common\%CONFIG%\Common.dll"
if exist "%COMMON_DLL%" (
    copy /Y "%COMMON_DLL%" "%AOTTEST_DLL_DIR%\Common.dll" >nul
    if errorlevel 1 exit /b %ERRORLEVEL%
)

"%RUNNER%" -l "%REPO_ROOT%\src\libraries\dotnetframework4.x" -l "%AOTTEST_DLL_DIR%" -e AotTests.App::Main AotTests
set "EXIT_CODE=%ERRORLEVEL%"

if not "%EXIT_CODE%"=="0" exit /b %EXIT_CODE%
endlocal & exit /b 0

