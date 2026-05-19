@echo off
setlocal
call "%~dp0..\lib\repo-root.bat"
cd /d "%REPO_ROOT%\src\generator"
python check_runtime_api_signatures.py ^
  --externs "%REPO_ROOT%\src\libraries\dotnetframework4.x\mscorlib_externs.txt" ^
  --externs "%REPO_ROOT%\src\libraries\dotnetframework4.x\System_externs.txt" ^
  --externs "%REPO_ROOT%\src\libraries\dotnetframework4.x\System.Core_externs.txt" ^
  --externs "%REPO_ROOT%\src\libraries\dotnetframework4.x-linux\mscorlib_externs.txt" ^
  --externs "%REPO_ROOT%\src\libraries\dotnetframework4.x-linux\System_externs.txt" ^
  --externs "%REPO_ROOT%\src\libraries\dotnetframework4.x-linux\System.Core_externs.txt" ^
  --icalls "%REPO_ROOT%\src\leanaot\LeanAOT\icalls.json" ^
  --intrinsics "%REPO_ROOT%\src\leanaot\LeanAOT\intrinsics.json" ^
  %*
exit /b %ERRORLEVEL%
