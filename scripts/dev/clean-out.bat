@echo off
setlocal
call "%~dp0..\lib\out-dir-init.bat"
if exist "%OUT_ROOT%" (
    echo Removing %OUT_ROOT% ...
    rmdir /s /q "%OUT_ROOT%"
    echo Done.
) else (
    echo Nothing to clean: %OUT_ROOT%
)
endlocal
