@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
call "%SCRIPT_DIR%..\..\..\scripts\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"

set "ARCH=%~1"
if "%ARCH%"=="" set "ARCH=x64"
set "GENERATOR=Visual Studio 17 2022"

call "%REPO_ROOT%\scripts\lib\cmake-dir.bat" "tools\leanrun" "vs-sln" "%ARCH%"

echo === Generate VS solution (%GENERATOR%, %ARCH%) ===
echo Build dir: %CMAKE_BUILD_DIR%

if not exist "%CMAKE_BUILD_DIR%" mkdir "%CMAKE_BUILD_DIR%"
if errorlevel 1 goto :error

cmake -S "%SCRIPT_DIR%." -B "%CMAKE_BUILD_DIR%" -G "%GENERATOR%" -A %ARCH%
if errorlevel 1 goto :error

set "SLN="
for %%F in ("%CMAKE_BUILD_DIR%\*.sln") do set "SLN=%%~fF"
if defined SLN (
  echo Solution generated: "%SLN%"
) else (
  echo Warning: .sln not found under "%CMAKE_BUILD_DIR%"
)

echo Done.
endlocal
exit /b 0

:error
echo Failed to generate solution. Error code %ERRORLEVEL%.
endlocal & exit /b %ERRORLEVEL%
