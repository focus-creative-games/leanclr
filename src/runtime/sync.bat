@echo off
setlocal

set "SRC_DIR=%~dp0"
set "DST_DIR=D:\workspace\wasmclr\TestWeb\LeanCLR\LocalIl2CppData-WindowsEditor\il2cpp\libil2cpp"

echo Sync runtime directory...
echo   Source: %SRC_DIR%
echo   Target: %DST_DIR%
echo.

robocopy "%SRC_DIR%" "%DST_DIR%" /E /XD "%SRC_DIR%build"
set "RC=%ERRORLEVEL%"

if %RC% GEQ 8 (
    echo Sync failed. robocopy exit code: %RC%
    exit /b %RC%
)

echo Sync completed. robocopy exit code: %RC%
exit /b 0

