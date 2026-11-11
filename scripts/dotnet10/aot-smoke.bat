@echo off
setlocal
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0aot-smoke.ps1" %*
exit /b %ERRORLEVEL%
