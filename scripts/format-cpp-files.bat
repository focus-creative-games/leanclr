@echo off
setlocal enabledelayedexpansion

echo ====================================
echo Formatting all C++ files in src/runtime
echo ====================================
echo.

set CLANG_FORMAT="..\tools\clang-format\clang-format.exe"
set SRC_DIR=%~dp0..\src\runtime

if not exist %CLANG_FORMAT% (
    echo Error: clang-format.exe not found at %CLANG_FORMAT%
    echo Please update the path in this script.
    pause
    exit /b 1
)

if not exist "%SRC_DIR%" (
    echo Error: src directory not found at %SRC_DIR%
    pause
    exit /b 1
)

set COUNT=0

echo Formatting .cpp files...
for /r "%SRC_DIR%" %%f in (*.cpp) do (
    set "file_path=%%f"
    echo !file_path! | findstr /i /c:"\build-" /c:"CMakeFiles" /c:"cmake-build" >nul
    if errorlevel 1 (
        echo Formatting: %%~nxf
        %CLANG_FORMAT% -i "%%f"
        set /a COUNT+=1
    ) else (
        echo Skipping: %%~nxf ^(build directory^)
    )
)

echo.
echo Formatting .h files...
for /r "%SRC_DIR%" %%f in (*.h) do (
    set "file_path=%%f"
    echo !file_path! | findstr /i /c:"\build-" /c:"CMakeFiles" /c:"cmake-build" >nul
    if errorlevel 1 (
        echo Formatting: %%~nxf
        %CLANG_FORMAT% -i "%%f"
        set /a COUNT+=1
    ) else (
        echo Skipping: %%~nxf ^(build directory^)
    )
)

echo.
echo Formatting .hpp files...
for /r "%SRC_DIR%" %%f in (*.hpp) do (
    set "file_path=%%f"
    echo !file_path! | findstr /i /c:"\build-" /c:"CMakeFiles" /c:"cmake-build" >nul
    if errorlevel 1 (
        echo Formatting: %%~nxf
        %CLANG_FORMAT% -i "%%f"
        set /a COUNT+=1
    ) else (
        echo Skipping: %%~nxf ^(build directory^)
    )
)

echo.
echo ====================================
echo Formatting complete!
echo Total files formatted: !COUNT!
echo ====================================

