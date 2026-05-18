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

call "%REPO_ROOT%\scripts\lib\cmake-dir.bat" "leanaot\aot-runner" "%CONFIG%" "%ARCH%"

set "RUNNER=%CMAKE_BUILD_DIR%\bin\%CONFIG%\aot-runner.exe"
if not exist "%RUNNER%" (
    echo ERROR: aot-runner not found at "%RUNNER%".
    exit /b 1
)

"%RUNNER%" -l "%REPO_ROOT%\src\libraries\dotnetframework4.x" -l "%OUT_ROOT%\dotnet\Test\Debug" -e App::Main Test
set "EXIT_CODE=%ERRORLEVEL%"

if not "%EXIT_CODE%"=="0" exit /b %EXIT_CODE%
endlocal & exit /b 0
