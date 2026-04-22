@echo off
setlocal

for %%I in ("%~dp0.") do set "SRC_DIR=%%~fI"
set "DST_DIR=D:\workspace\wasmclr\TestWeb\build-win64\Il2CppOutputProject\IL2CPP\libil2cpp"

echo Sync runtime directory...
echo   Source: %SRC_DIR%
echo   Target: %DST_DIR%
echo.

if not exist "%DST_DIR%" mkdir "%DST_DIR%"

robocopy "%SRC_DIR%" "%DST_DIR%" /E /XD "%SRC_DIR%\build" /XD "%SRC_DIR%\build-ninja" /XF *.bat *.sh CMakeLists.txt
set "RC=%ERRORLEVEL%"

if %RC% GEQ 8 (
    echo Sync failed. robocopy exit code: %RC%
    exit /b %RC%
)

echo Sync completed. robocopy exit code: %RC%
exit /b 0

