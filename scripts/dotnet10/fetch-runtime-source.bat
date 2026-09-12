@echo off
setlocal
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0fetch-runtime-source.ps1" %*
exit /b %ERRORLEVEL%

