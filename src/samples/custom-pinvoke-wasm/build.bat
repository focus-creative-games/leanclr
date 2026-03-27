@echo off
setlocal

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

where emcmake >nul 2>nul
if errorlevel 1 (
    call ..\..\..\..\emsdk\emsdk_env.bat
)

call emcmake cmake -B build-wasm -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 exit /b 1

call emmake cmake --build build-wasm --parallel
if errorlevel 1 exit /b 1

endlocal
