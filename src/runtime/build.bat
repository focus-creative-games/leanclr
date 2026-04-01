@echo off
setlocal

:: Parse command line arguments
set BUILD_TYPE=Release
set ARCH=x64
set VS_VERSION=17 2022
set CLEAN=0
set BUILD_SHARED=0

:parse_args
if "%~1"=="" goto end_parse
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

:: Build directory
set BUILD_DIR=%~dp0build
set CMAKE_BUILD_DIR=%BUILD_DIR%\%BUILD_TYPE%

echo ================================
echo Building leanclr
echo ================================
echo Build Type: %BUILD_TYPE%
echo Architecture: %ARCH%
echo Build Directory: %CMAKE_BUILD_DIR%
echo ================================

:: Clean if requested
if %CLEAN%==1 (
    echo Cleaning build directory...
    if exist "%CMAKE_BUILD_DIR%" (
        rmdir /s /q "%CMAKE_BUILD_DIR%"
    )
)

:: Create build directory
if not exist "%CMAKE_BUILD_DIR%" mkdir "%CMAKE_BUILD_DIR%"

pushd "%CMAKE_BUILD_DIR%"

:: Generate Visual Studio solution
echo.
echo [1/2] Generating Visual Studio solution...
if %BUILD_SHARED%==1 (
    cmake -G "Visual Studio %VS_VERSION%" -A %ARCH% -DBUILD_SHARED_LEANCLR=ON  ..\..\
) else (
    cmake -G "Visual Studio %VS_VERSION%" -A %ARCH%  ..\..\
)
if errorlevel 1 (
    echo ERROR: CMake generation failed.
    popd
    exit /b 1
)

:: Build the project
echo.
echo [2/2] Building project...
cmake --build . --config %BUILD_TYPE% --parallel 
if errorlevel 1 (
    echo ERROR: Build failed.
    popd
    exit /b 1
)

popd

echo.
echo ================================
echo Build completed successfully!
echo ================================
echo Output: %CMAKE_BUILD_DIR%\%BUILD_TYPE%
echo ================================

endlocal
