@echo off
setlocal
call "%~dp0..\lib\repo-root.bat"
cd /d "%REPO_ROOT%\src\generator"
python check_runtime_api_signatures.py ^
  --externs "%REPO_ROOT%\src\libraries\mono-4.5\mscorlib_externs.txt" ^
  --externs "%REPO_ROOT%\src\libraries\mono-4.5\System_externs.txt" ^
  --externs "%REPO_ROOT%\src\libraries\mono-4.5\System.Core_externs.txt" ^
  --icalls "%REPO_ROOT%\src\leanaot\LeanAOT\icalls.json" ^
  --intrinsics "%REPO_ROOT%\src\leanaot\LeanAOT\intrinsics.json" ^
  %*
exit /b %ERRORLEVEL%
