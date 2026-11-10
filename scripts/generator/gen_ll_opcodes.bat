@echo off
setlocal
call "%~dp0..\lib\repo-root.bat"
cd /d "%REPO_ROOT%\src\generator"
python gen_low_level_opcodes.py ll-opcodes.xml ../runtime/interp/generated
exit /b %ERRORLEVEL%
