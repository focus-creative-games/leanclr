@echo off
setlocal
call "%~dp0..\lib\repo-root.bat"
cd /d "%REPO_ROOT%\src\generator"
python collect_icalls_intrinsics.py
exit /b %ERRORLEVEL%
