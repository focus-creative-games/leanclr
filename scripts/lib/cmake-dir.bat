@echo off
rem Requires out-dir-init.bat (OUT_ROOT). Sets CMAKE_BUILD_DIR, CMAKE_ARCH_LABEL, CMAKE_GENERATOR_ARCH.
rem Usage: call cmake-dir.bat <module-path> <config> [arch]
rem   arch: x64 (default), x86, or Win32 (CMake VS generator platform for 32-bit)
set "CMAKE_ARCH_INPUT=%~3"
if "%CMAKE_ARCH_INPUT%"=="" set "CMAKE_ARCH_INPUT=x64"

set "CMAKE_MODULE=%~1"
set "CMAKE_CONFIG=%~2"
set "CMAKE_ARCH_LABEL=%CMAKE_ARCH_INPUT%"
set "CMAKE_GENERATOR_ARCH=%CMAKE_ARCH_INPUT%"
if /i "%CMAKE_ARCH_INPUT%"=="x86" set "CMAKE_GENERATOR_ARCH=Win32"

set "CMAKE_BUILD_DIR=%OUT_ROOT%\cmake\%CMAKE_MODULE%\%CMAKE_CONFIG%-%CMAKE_ARCH_LABEL%"
