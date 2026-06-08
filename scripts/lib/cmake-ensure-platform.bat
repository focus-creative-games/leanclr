@echo off
rem Requires CMAKE_BUILD_DIR and CMAKE_GENERATOR_ARCH from cmake-dir.bat.
rem Drops stale CMake cache when the VS generator platform changed (e.g. x86 -> Win32).
set "CACHE=%CMAKE_BUILD_DIR%\CMakeCache.txt"
if not exist "%CACHE%" exit /b 0

findstr /C:"CMAKE_GENERATOR_PLATFORM:INTERNAL=%CMAKE_GENERATOR_ARCH%" "%CACHE%" >nul 2>&1
if not errorlevel 1 exit /b 0

echo CMake platform mismatch in "%CMAKE_BUILD_DIR%", cleaning cache...
if exist "%CACHE%" del /f /q "%CACHE%"
if exist "%CMAKE_BUILD_DIR%\CMakeFiles" rmdir /s /q "%CMAKE_BUILD_DIR%\CMakeFiles"
exit /b 0
