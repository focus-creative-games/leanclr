@echo off
setlocal
call "%~dp0..\lib\repo-root.bat"
cd /d "%REPO_ROOT%\src\generator"
python gen_cil_opcode_defs.py cil-opcodes.xml ../runtime/interp/il_opcodes.h ../runtime/interp/il_opcodes.cpp
exit /b %ERRORLEVEL%
