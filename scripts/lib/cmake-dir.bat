@echo off
rem Requires out-dir-init.bat (OUT_ROOT). Sets CMAKE_BUILD_DIR.
rem Usage: call cmake-dir.bat <module-path> <config> [arch]
set "CMAKE_MODULE=%~1"
set "CMAKE_CONFIG=%~2"
set "CMAKE_ARCH=%~3"
if "%CMAKE_ARCH%"=="" set "CMAKE_ARCH=x64"
set "CMAKE_BUILD_DIR=%OUT_ROOT%\cmake\%CMAKE_MODULE%\%CMAKE_CONFIG%-%CMAKE_ARCH%"
