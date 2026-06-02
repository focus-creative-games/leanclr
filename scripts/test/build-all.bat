@echo off
setlocal enabledelayedexpansion

call "%~dp0..\lib\repo-root.bat"
call "%~dp0..\lib\out-dir-init.bat"

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"
set "ARCH=%~2"
if "%ARCH%"=="" set "ARCH=x64"

echo === Config: %CONFIG% ^| Arch: %ARCH% ===

echo build basic-tester
call "%REPO_ROOT%\scripts\test\basic-tester\build.bat" %CONFIG% %ARCH%
if errorlevel 1 (
    echo ERROR: basic-tester build failed.
    exit /b 1
)

echo build managed tests
pushd "%REPO_ROOT%\src\tests\managed"
call dotnet build -c %CONFIG% -p:LeanClrIl2CppOnly=true
if errorlevel 1 (
    echo ERROR: managed tests build failed.
    popd
    exit /b 1
)
popd

call "%~dp0..\lib\cmake-dir.bat" "tests\basic-tester" "%CONFIG%" "%ARCH%"
set "EXE_DIR=%CMAKE_BUILD_DIR%\bin\%CONFIG%"
set "DLLS_DIR=%EXE_DIR%\dlls"

echo stage test DLLs to %DLLS_DIR%
if exist "%DLLS_DIR%" rmdir /s /q "%DLLS_DIR%"
mkdir "%DLLS_DIR%"
if errorlevel 1 (
    echo ERROR: failed to create dlls directory.
    exit /b 1
)

robocopy "%REPO_ROOT%\src\libraries\dotnetframework4.x" "%DLLS_DIR%\dotnetframework4.x" /E /NFL /NDL /NJH /NJS /NP /R:1 /W:1 >nul
set "RC=%ERRORLEVEL%"
if %RC% GEQ 8 (
    echo ERROR: failed to copy dotnetframework4.x libraries.
    exit /b %RC%
)

set "REFNETSTANDARD_DLL=%OUT_ROOT%\dotnet\RefNetstandard\%CONFIG%\RefNetstandard.dll"
if not exist "%REFNETSTANDARD_DLL%" set "REFNETSTANDARD_DLL=%OUT_ROOT%\dotnet\RefNetstandard\Debug\RefNetstandard.dll"

copy /Y "%REFNETSTANDARD_DLL%" "%DLLS_DIR%\" >nul
if errorlevel 1 (
    echo ERROR: failed to copy RefNetstandard.dll from "%REFNETSTANDARD_DLL%".
    exit /b 1
)

set "CORETESTS_DLL=%OUT_ROOT%\dotnet\CoreTests\%CONFIG%\CoreTests.dll"
set "CORLIBTESTS_DLL=%OUT_ROOT%\dotnet\CorlibTests\%CONFIG%\CorlibTests.dll"
if not exist "%CORLIBTESTS_DLL%" set "CORLIBTESTS_DLL=%OUT_ROOT%\dotnet\CorlibTests\Debug\CorlibTests.dll"

copy /Y "%CORETESTS_DLL%" "%DLLS_DIR%\" >nul
if errorlevel 1 (
    echo ERROR: failed to copy CoreTests.dll from "%CORETESTS_DLL%".
    exit /b 1
)

copy /Y "%CORLIBTESTS_DLL%" "%DLLS_DIR%\" >nul
if errorlevel 1 (
    echo ERROR: failed to copy CorlibTests.dll from "%CORLIBTESTS_DLL%".
    exit /b 1
)

set "COMMON_DLL=%OUT_ROOT%\dotnet\Common\%CONFIG%\Common.dll"
if not exist "%COMMON_DLL%" set "COMMON_DLL=%OUT_ROOT%\dotnet\Common\Debug\Common.dll"

copy /Y "%COMMON_DLL%" "%DLLS_DIR%\" >nul
if errorlevel 1 (
    echo ERROR: failed to copy Common.dll from "%COMMON_DLL%".
    exit /b 1
)

set "ILTESTS_DLL=%OUT_ROOT%\dotnet\ILTests\%CONFIG%\ILTests.dll"
set "ILTESTS_NATIVE_DLL=%OUT_ROOT%\dotnet\ILTests\%CONFIG%\ILTests.Native.dll"
if not exist "%ILTESTS_DLL%" set "ILTESTS_DLL=%OUT_ROOT%\dotnet\ILTests\Debug\ILTests.dll"
if not exist "%ILTESTS_NATIVE_DLL%" set "ILTESTS_NATIVE_DLL=%OUT_ROOT%\dotnet\ILTests\Debug\ILTests.Native.dll"

copy /Y "%ILTESTS_DLL%" "%DLLS_DIR%\" >nul
if errorlevel 1 (
    echo ERROR: failed to copy ILTests.dll from "%ILTESTS_DLL%".
    exit /b 1
)

copy /Y "%ILTESTS_NATIVE_DLL%" "%DLLS_DIR%\" >nul
if errorlevel 1 (
    echo ERROR: failed to copy ILTests.Native.dll from "%ILTESTS_NATIVE_DLL%".
    exit /b 1
)

echo All tests built successfully.
endlocal
