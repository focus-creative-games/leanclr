@echo off
setlocal

where emcmake >nul 2>nul
if errorlevel 1 (
    call ..\..\..\..\emsdk\emsdk_env.bat
)

call emcmake cmake -B build-wasm -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1

call emmake cmake --build build-wasm --parallel
if errorlevel 1 exit /b 1

endlocal
