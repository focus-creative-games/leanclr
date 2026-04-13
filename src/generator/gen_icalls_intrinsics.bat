@echo off
setlocal
cd /d "%~dp0"
python collect_icalls_intrinsics.py
exit /b %ERRORLEVEL%
