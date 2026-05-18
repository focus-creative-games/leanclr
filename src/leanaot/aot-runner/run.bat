
call "%~dp0build.bat"

call "%~dp0..\..\..\scripts\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"
call "%REPO_ROOT%\scripts\lib\cmake-dir.bat" "leanaot\aot-runner" "Debug" "x64"

"%CMAKE_BUILD_DIR%\bin\Debug\aot-runner.exe" -l "%REPO_ROOT%\src\libraries\dotnetframework4.x" -l "%OUT_ROOT%\dotnet\Test\Debug" -e App::Main Test

pause
