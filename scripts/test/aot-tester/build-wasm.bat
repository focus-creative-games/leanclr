@echo off
setlocal
rem Build aot-tester for WebAssembly (Emscripten).
rem Usage: build-wasm.bat [Config]
rem Requires emcmake/emmake on PATH.

call "%~dp0..\..\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"

set "AOT_TESTER_SRC=%REPO_ROOT%\src\tests\aot-tester"
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"

call "%REPO_ROOT%\scripts\lib\cmake-dir.bat" "tests\aot-tester" "%CONFIG%" "wasm"

echo === Config: %CONFIG% ^| Arch: wasm ===
echo Build dir: %CMAKE_BUILD_DIR%

if not exist "%CMAKE_BUILD_DIR%" mkdir "%CMAKE_BUILD_DIR%"
if errorlevel 1 goto :fail

echo [1/2] CMake configure (Emscripten)...
call emcmake cmake -S "%AOT_TESTER_SRC%" -B "%CMAKE_BUILD_DIR%" -DCMAKE_BUILD_TYPE=%CONFIG%
if errorlevel 1 goto :fail

echo [2/2] Build target 'aot-tester'...
call emmake cmake --build "%CMAKE_BUILD_DIR%" --parallel --target aot-tester
if errorlevel 1 goto :fail

echo Done. Output: "%CMAKE_BUILD_DIR%\bin\aot-tester.js" and aot-tester.wasm
endlocal
exit /b 0

:fail
echo build-wasm failed with error code %ERRORLEVEL%.
endlocal & exit /b %ERRORLEVEL%

