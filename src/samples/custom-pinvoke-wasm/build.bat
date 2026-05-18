@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
call "%SCRIPT_DIR%..\..\..\scripts\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

call "%REPO_ROOT%\scripts\lib\cmake-dir.bat" "samples\custom-pinvoke-wasm" "%BUILD_TYPE%" "wasm"

where emcmake >nul 2>nul
if errorlevel 1 (
    call "%REPO_ROOT%\emsdk\emsdk_env.bat"
)

echo Build dir: %CMAKE_BUILD_DIR%
if not exist "%CMAKE_BUILD_DIR%" mkdir "%CMAKE_BUILD_DIR%"

call emcmake cmake -S "%SCRIPT_DIR%." -B "%CMAKE_BUILD_DIR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 exit /b 1

call emmake cmake --build "%CMAKE_BUILD_DIR%" --parallel
if errorlevel 1 exit /b 1

endlocal
