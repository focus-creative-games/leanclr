@echo off
setlocal
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0interp-smoke.ps1" %*
exit /b %ERRORLEVEL%
