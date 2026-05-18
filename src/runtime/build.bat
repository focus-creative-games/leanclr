@echo off
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
call "%SCRIPT_DIR%..\..\scripts\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"

set BUILD_TYPE=Release
set ARCH=x64
set VS_VERSION=17 2022
set CLEAN=0
set BUILD_SHARED=0
set GENERATE_SLN=0

:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="sln" set GENERATE_SLN=1
if /i "%~1"=="Debug" set BUILD_TYPE=Debug
if /i "%~1"=="Release" set BUILD_TYPE=Release
if /i "%~1"=="clean" set CLEAN=1
if /i "%~1"=="-clean" set CLEAN=1
if /i "%~1"=="x86" set ARCH=Win32
if /i "%~1"=="x64" set ARCH=x64
if /i "%~1"=="shared" set BUILD_SHARED=1
shift
goto parse_args
:end_parse

if %GENERATE_SLN%==1 (
    call "%REPO_ROOT%\scripts\lib\cmake-dir.bat" "runtime" "vs-sln" "%ARCH%"
) else (
    call "%REPO_ROOT%\scripts\lib\cmake-dir.bat" "runtime" "%BUILD_TYPE%" "%ARCH%"
)

echo ================================
if %GENERATE_SLN%==1 (
    echo Generate Visual Studio solution
) else (
    echo Building leanclr
)
echo ================================
if %GENERATE_SLN%==1 (
    echo Mode: sln only ^(no compile^)
) else (
    echo Build Type: %BUILD_TYPE%
)
echo Architecture: %ARCH%
echo Build Directory: %CMAKE_BUILD_DIR%
echo ================================

if %CLEAN%==1 (
    echo Cleaning build directory...
    if exist "%CMAKE_BUILD_DIR%" rmdir /s /q "%CMAKE_BUILD_DIR%"
)

if not exist "%CMAKE_BUILD_DIR%" mkdir "%CMAKE_BUILD_DIR%"

echo.
if %GENERATE_SLN%==1 (
    echo CMake configure ^(generate .sln^)...
) else (
    echo [1/2] CMake configure...
)
if %BUILD_SHARED%==1 (
    cmake -S "%SCRIPT_DIR%." -B "%CMAKE_BUILD_DIR%" -G "Visual Studio %VS_VERSION%" -A %ARCH% -DBUILD_SHARED_LEANCLR=ON
) else (
    cmake -S "%SCRIPT_DIR%." -B "%CMAKE_BUILD_DIR%" -G "Visual Studio %VS_VERSION%" -A %ARCH%
)
if errorlevel 1 (
    echo ERROR: CMake generation failed.
    exit /b 1
)

if %GENERATE_SLN%==1 (
    echo.
    set "SLN="
    for %%F in ("%CMAKE_BUILD_DIR%\*.sln") do set "SLN=%%~fF"
    if defined SLN (
        echo Solution generated: "!SLN!"
    ) else (
        echo Warning: .sln not found under "%CMAKE_BUILD_DIR%"
    )
    echo ================================
    endlocal
    exit /b 0
)

echo.
echo [2/2] Building project...
cmake --build "%CMAKE_BUILD_DIR%" --config %BUILD_TYPE% --parallel
if errorlevel 1 (
    echo ERROR: Build failed.
    exit /b 1
)

echo.
echo ================================
echo Build completed successfully!
echo Output: %CMAKE_BUILD_DIR%
echo ================================

endlocal
