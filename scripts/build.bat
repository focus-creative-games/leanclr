@echo off
setlocal

rem Do not use "shift" inside ( ) blocks: %* is expanded at parse time and
rem would still pass the original args (e.g. test build) to child scripts.

if /i "%~1"=="test" if /i "%~2"=="build" (
    call "%~dp0test\build-all.bat" %~3 %~4 %~5 %~6 %~7 %~8 %~9
    exit /b %ERRORLEVEL%
)
if /i "%~1"=="test" if /i "%~2"=="run" (
    call "%~dp0test\run.bat" %~3 %~4 %~5 %~6 %~7 %~8 %~9
    exit /b %ERRORLEVEL%
)
if /i "%~1"=="test" goto :usage

if /i "%~1"=="runtime" (
    call "%~dp0runtime\build.bat" %~2 %~3 %~4 %~5 %~6 %~7 %~8 %~9
    exit /b %ERRORLEVEL%
)

if /i "%~1"=="leanaot" if /i "%~2"=="Debug" (
    call "%~dp0leanaot\build.bat" Debug
    exit /b %ERRORLEVEL%
)
if /i "%~1"=="leanaot" if /i "%~2"=="Release" (
    call "%~dp0leanaot\build.bat" Release
    exit /b %ERRORLEVEL%
)

if /i "%~1"=="aot-runner" if /i "%~2"=="gen-cpp" (
    call "%~dp0test\aot-runner\gen_cpp.bat"
    exit /b %ERRORLEVEL%
)
if /i "%~1"=="aot-tester" if /i "%~2"=="gen-cpp" (
    call "%~dp0test\aot-runner\gen_cpp.bat"
    exit /b %ERRORLEVEL%
)
if /i "%~1"=="aot-runner" if /i "%~2"=="gen-cpp-posix" (
    call "%~dp0test\aot-runner\gen_cpp_posix.bat"
    exit /b %ERRORLEVEL%
)
if /i "%~1"=="aot-tester" if /i "%~2"=="gen-cpp-posix" (
    call "%~dp0test\aot-runner\gen_cpp_posix.bat"
    exit /b %ERRORLEVEL%
)
if /i "%~1"=="aot-runner" if /i "%~2"=="run" (
    call "%~dp0test\aot-runner\run.bat" %~3 %~4 %~5 %~6 %~7 %~8 %~9
    exit /b %ERRORLEVEL%
)
if /i "%~1"=="aot-tester" if /i "%~2"=="run" (
    call "%~dp0test\aot-runner\run.bat" %~3 %~4 %~5 %~6 %~7 %~8 %~9
    exit /b %ERRORLEVEL%
)
if /i "%~1"=="aot-runner" if /i "%~2"=="build-wasm" (
    echo ERROR: build-wasm script has been removed.
    exit /b 1
)
if /i "%~1"=="aot-tester" if /i "%~2"=="build-wasm" (
    echo ERROR: build-wasm script has been removed.
    exit /b 1
)
if /i "%~1"=="aot-runner" if /i "%~2"=="build" (
    call "%~dp0test\aot-runner\build.bat" %~3 %~4 %~5 %~6 %~7 %~8 %~9
    exit /b %ERRORLEVEL%
)
if /i "%~1"=="aot-tester" if /i "%~2"=="build" (
    call "%~dp0test\aot-runner\build.bat" %~3 %~4 %~5 %~6 %~7 %~8 %~9
    exit /b %ERRORLEVEL%
)
if /i "%~1"=="aot-runner" (
    call "%~dp0test\aot-runner\build.bat" %~2 %~3 %~4 %~5 %~6 %~7 %~8 %~9
    exit /b %ERRORLEVEL%
)
if /i "%~1"=="aot-tester" (
    call "%~dp0test\aot-runner\build.bat" %~2 %~3 %~4 %~5 %~6 %~7 %~8 %~9
    exit /b %ERRORLEVEL%
)

:usage
echo Usage:
echo   scripts\build.bat test build [Config] [Arch]
echo   scripts\build.bat test run [Config]
echo   scripts\build.bat runtime [Config] [Arch] [clean] [shared] [sln]
echo   scripts\build.bat leanaot Debug^|Release
echo   scripts\build.bat aot-tester [build] [Config] [Arch]
echo   scripts\build.bat aot-tester gen-cpp
echo   scripts\build.bat aot-tester gen-cpp-posix
echo   scripts\build.bat aot-tester run [Config] [Arch]
echo   (aot-runner alias remains supported)
exit /b 1
