@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..\..") do set "REPO_ROOT=%%~fI"
set "SOURCE_DIR=%REPO_ROOT%\src\tools\leanaot"
set "TARGET_DIR=D:\workspace\wasmclr\leanclr4unity\LeanCLR~\leanaot"

echo Sync LeanAOT output...
echo Source: %SOURCE_DIR%
echo Target: %TARGET_DIR%

if not exist "%SOURCE_DIR%\" (
	echo Source directory not found.
	exit /b 1
)

if not exist "%TARGET_DIR%\" (
	mkdir "%TARGET_DIR%"
	if errorlevel 1 (
		echo Failed to create target directory.
		exit /b 1
	)
)

robocopy "%SOURCE_DIR%" "%TARGET_DIR%" /MIR /R:2 /W:1 /NFL /NDL /NJH /NJS /NP
if errorlevel 8 (
	echo Sync failed.
	exit /b 1
)

echo Sync succeeded.
endlocal
