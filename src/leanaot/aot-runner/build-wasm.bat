@echo off
setlocal

cd /d "%~dp0"

call emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Debug
if errorlevel 1 goto :fail

call emmake cmake --build build-wasm --parallel
if errorlevel 1 goto :fail

echo Done. Output: "%~dp0build-wasm\bin\aot-runner.js" and aot-runner.wasm
endlocal
exit /b 0

:fail
echo build-wasm failed with error code %ERRORLEVEL%.
endlocal
exit /b %ERRORLEVEL%
