@echo off
rem Sets REPO_ROOT to the repository root (parent of src/).
rem Call via relative path from your script directory, e.g.:
rem   call "%SCRIPT_DIR%..\..\scripts\lib\repo-root.bat"     rem src/runtime
rem   call "%SCRIPT_DIR%..\..\..\scripts\lib\repo-root.bat"  rem src/samples/foo
rem   call "%~dp0..\lib\repo-root.bat"                        rem scripts/*
for %%I in ("%~dp0..\..") do set "REPO_ROOT=%%~fI"
