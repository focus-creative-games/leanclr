@echo off
setlocal
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0export-bcl-externs.ps1" %*
exit /b %ERRORLEVEL%
