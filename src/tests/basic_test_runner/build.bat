@echo off
call "%~dp0..\..\..\scripts\test\basic_test_runner\build.bat" %*
exit /b %ERRORLEVEL%
