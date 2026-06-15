@echo off
setlocal

call "%~dp0..\..\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"

set "AOT_TESTER_SRC=%REPO_ROOT%\src\tests\aot-tester"
set "LEANAOT_EXE=%OUT_ROOT%\dotnet\LeanAOT\Debug\net8.0\LeanAOT.exe"
set "AOTTEST_DLL_DIR=%OUT_ROOT%\dotnet\AotTests\Debug"
set "COMMON_DLL=%OUT_ROOT%\dotnet\Common\Debug\Common.dll"
set "CPP_OUT=%AOT_TESTER_SRC%\cpp-posix"

echo [1/3] Building managed AotTests project (Debug)...
dotnet build "%REPO_ROOT%\src\tests\managed\AotTests\AotTests.csproj" -c Debug
if errorlevel 1 (
    echo AotTests build failed.
    exit /b %ERRORLEVEL%
)

if exist "%COMMON_DLL%" (
  copy /Y "%COMMON_DLL%" "%AOTTEST_DLL_DIR%\Common.dll" >nul
  if errorlevel 1 exit /b %ERRORLEVEL%
)

echo [2/3] Building LeanAOT (Debug)...
dotnet build "%REPO_ROOT%\src\leanaot\LeanAOT\LeanAOT.csproj" -c Debug
if errorlevel 1 (
    echo LeanAOT build failed.
    exit /b %ERRORLEVEL%
)

echo [3/3] Running LeanAOT...
"%LEANAOT_EXE%" ^
  -o "%CPP_OUT%" ^
  --emit-null-checks ^
  --enable-array-bounds-check ^
  --leanaot-may-throw-exception-in-icall ^
  -d "%REPO_ROOT%\src\libraries\dotnetframework4.x-linux" ^
  -d "%AOTTEST_DLL_DIR%" ^
  --leanaot-aot-rule-file "%AOT_TESTER_SRC%\aot-rules-mscorlib.xml" ^
  --leanaot-aot-rule-file "%AOT_TESTER_SRC%\aot-rules-test.xml" ^
  --leanaot-pgo-rule-file "%AOT_TESTER_SRC%\pgo-aot.xml" ^
  -a mscorlib ^
  -a System ^
  -a System.Core ^
  -a AotTests

if errorlevel 1 exit /b %ERRORLEVEL%

echo Done. Output: %CPP_OUT%
endlocal

