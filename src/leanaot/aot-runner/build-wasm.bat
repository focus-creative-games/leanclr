@echo off
setlocal
rem 在脚本所在目录执行，避免工作目录不对导致找不到 CMakeLists / leanaot/Test 下的 js-library
cd /d "%~dp0"

call emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Debug
if errorlevel 1 goto :fail

call emmake cmake --build build-wasm --parallel
if errorlevel 1 goto :fail

echo Done. Output: "%~dp0build-wasm\bin\aot-runner.js" and aot-runner.wasm
echo P/Invoke 验证请先执行 gen_cpp.bat 生成 cpp，再用 node 运行，例如：
echo   node build-wasm\bin\aot-runner.js -l ..\..\libraries\dotnetframework4.x -l ..\..\leanaot\Test\bin\Debug -e WasmPInvokeVerify::Main Test
endlocal
exit /b 0

:fail
echo build-wasm failed with error code %ERRORLEVEL%.
endlocal
exit /b %ERRORLEVEL%
