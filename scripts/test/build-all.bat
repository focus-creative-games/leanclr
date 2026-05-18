@echo off
setlocal enabledelayedexpansion

call "%~dp0..\lib\repo-root.bat"
call "%~dp0..\lib\out-dir-init.bat"

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"
set "ARCH=%~2"
if "%ARCH%"=="" set "ARCH=x64"

echo === Config: %CONFIG% ^| Arch: %ARCH% ===

echo build basic_test_runner
pushd "%REPO_ROOT%\src\tests\basic_test_runner"
call build.bat %CONFIG% %ARCH%
if errorlevel 1 (
    echo ERROR: basic_test_runner build failed.
    popd
    exit /b 1
)
popd

echo build managed tests
pushd "%REPO_ROOT%\src\tests\managed"
call dotnet build -c %CONFIG%
if errorlevel 1 (
    echo ERROR: managed tests build failed.
    popd
    exit /b 1
)
popd

call "%~dp0..\lib\cmake-dir.bat" "tests\basic_test_runner" "%CONFIG%" "%ARCH%"
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

xcopy /Y /Q "%REPO_ROOT%\src\tests\managed\MiscDlls\*" "%DLLS_DIR%\" >nul
if errorlevel 1 (
    echo ERROR: failed to copy MiscDlls.
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

echo All tests built successfully.
endlocal
