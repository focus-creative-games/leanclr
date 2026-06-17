
set EXE=bin\Debug\net8.0\ExportExtern.exe
if not exist %EXE% (
    echo "Executable not found: %EXE%"
    exit /b 1
)

set LIBRARIES_DIR=..\..\libraries
if not exist %LIBRARIES_DIR% (
    echo "Libraries directory not found: %LIBRARIES_DIR%"
    exit /b 1
)

set WIN_LIBRARIES_DIR=%LIBRARIES_DIR%\mono-4.5

for %%f in (mscorlib System System.Core) do (
    
%EXE% %WIN_LIBRARIES_DIR%\%%f.dll all %WIN_LIBRARIES_DIR%\%%f_externs.txt
)
