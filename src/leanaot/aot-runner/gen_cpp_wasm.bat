@echo off
setlocal
cd /d "%~dp0"

call "%~dp0..\..\..\scripts\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"

set "LEANAOT_EXE=%OUT_ROOT%\dotnet\LeanAOT\Debug\net8.0\LeanAOT.exe"
set "TEST_DLL_DIR=%OUT_ROOT%\dotnet\Test\Debug"

echo [1/3] Building leanaot Test project (Debug)...
dotnet build "%REPO_ROOT%\src\leanaot\Test\Test.csproj" -c Debug
if errorlevel 1 (
	echo Test build failed.
	exit /b %ERRORLEVEL%
)

echo [2/3] Building LeanAOT (Debug)...
dotnet build "%REPO_ROOT%\src\leanaot\LeanAOT\LeanAOT.csproj" -c Debug
if errorlevel 1 (
	echo LeanAOT build failed.
	exit /b %ERRORLEVEL%
)

echo [3/3] Running LeanAOT...
"%LEANAOT_EXE%" ^
  -o cpp ^
  --emit-null-checks ^
  --enable-array-bounds-check ^
  -d "%REPO_ROOT%\src\libraries\dotnetframework4.x-linux" ^
  -d "%TEST_DLL_DIR%" ^
  --leanaot-aot-rule-file "%~dp0aot-rules-mscorlib.xml" ^
  --leanaot-aot-rule-file "%~dp0aot-rules-test.xml" ^
  -a mscorlib ^
  -a System ^
  -a System.Core ^
  -a Test

if errorlevel 1 exit /b %ERRORLEVEL%

echo Done.
endlocal
