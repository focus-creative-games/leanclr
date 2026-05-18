@echo off
setlocal
call "%~dp0..\lib\repo-root.bat"
cd /d "%REPO_ROOT%\src\generator"
python gen_low_level_opcodes.py ll-opcodes.xml hl-opcodes.xml ../runtime/interp/ll_opcodes.h ../runtime/interp/ll_opcodes.cpp ../runtime/interp/interpreter.cpp
exit /b %ERRORLEVEL%
