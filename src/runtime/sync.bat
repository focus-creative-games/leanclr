@echo off
setlocal

for %%I in ("%~dp0.") do set "SRC_DIR=%%~fI"
set "DST_DIR_1=D:\workspace\wasmclr\TestWeb\Library\LeanCLR\LocalIl2CppData-WindowsEditor\il2cpp\libil2cpp"
set "DST_DIR_2=D:\workspace\wasmclr\leanclr4unity\LeanCLR~\runtime"

echo Sync runtime directory...
echo   Source: %SRC_DIR%
echo   Target 1: %DST_DIR_1%
echo   Target 2: %DST_DIR_2%
echo.

if not exist "%DST_DIR_1%" mkdir "%DST_DIR_1%"
if not exist "%DST_DIR_2%" mkdir "%DST_DIR_2%"

robocopy "%SRC_DIR%" "%DST_DIR_1%" /E /XD "%SRC_DIR%\build" /XD "%SRC_DIR%\build-ninja" /XF *.bat *.sh CMakeLists.txt
set "RC=%ERRORLEVEL%"

if %RC% GEQ 8 (
    echo Sync target 1 failed. robocopy exit code: %RC%
    exit /b %RC%
)

robocopy "%SRC_DIR%" "%DST_DIR_2%" /E /XD "%SRC_DIR%\build" /XD "%SRC_DIR%\build-ninja" /XF *.bat *.sh CMakeLists.txt
set "RC=%ERRORLEVEL%"

if %RC% GEQ 8 (
    echo Sync target 2 failed. robocopy exit code: %RC%
    exit /b %RC%
)

echo Sync completed. robocopy exit code: %RC%
exit /b 0

