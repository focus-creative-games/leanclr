@echo off
setlocal
call "%~dp0..\lib\repo-root.bat"
cd /d "%REPO_ROOT%\src\generator"
python gen_high_level_opcodes.py hl-opcodes.xml ../runtime/interp/hl_opcodes.h
exit /b %ERRORLEVEL%
