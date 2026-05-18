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
copy /Y "%OUT_ROOT%\dotnet\CoreTests\%CONFIG%\CoreTests.dll" .\MiscDlls\
copy /Y "%OUT_ROOT%\dotnet\CorlibTests\%CONFIG%\CorlibTests.dll" .\MiscDlls\
popd

echo All tests built successfully.
endlocal
