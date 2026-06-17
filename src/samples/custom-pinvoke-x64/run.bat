
call build.bat

call "%~dp0..\..\..\scripts\lib\repo-root.bat"
call "%REPO_ROOT%\scripts\lib\out-dir-init.bat"
call "%REPO_ROOT%\scripts\lib\cmake-dir.bat" "samples\custom-pinvoke-x64" "Debug" "x64"

"%CMAKE_BUILD_DIR%\bin\Debug\custom-pinvoke.exe" -l "%REPO_ROOT%\src\libraries\mono-4.5" -l "%OUT_ROOT%\dotnet\CoreTests\Debug" -e test.App::CallCustomPInvoke CoreTests

pause
