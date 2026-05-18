@echo off
setlocal

if "%~1"=="test" (
    if "%~2"=="build" (
        shift
        shift
        call "%~dp0test\build-all.bat" %*
        exit /b %ERRORLEVEL%
    )
    if "%~2"=="run" (
        shift
        shift
        call "%~dp0test\run.bat" %*
        exit /b %ERRORLEVEL%
    )
    goto :usage
)

if "%~1"=="runtime" (
    shift
    call "%~dp0..\src\runtime\build.bat" %*
    exit /b %ERRORLEVEL%
)

if "%~1"=="leanaot" if "%~2"=="publish" (
    call "%~dp0release\publish-leanaot.bat"
    exit /b %ERRORLEVEL%
)

:usage
echo Usage:
echo   scripts\build.bat test build [Config] [Arch]
echo   scripts\build.bat test run [Config]
echo   scripts\build.bat runtime [Config] [Arch]
echo   scripts\build.bat leanaot publish
exit /b 1
