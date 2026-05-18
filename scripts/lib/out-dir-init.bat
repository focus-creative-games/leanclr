@echo off
rem Sets OUT_ROOT (and REPO_ROOT if unset). Optional: set LEANCLR_OUT_ROOT to override.
if not defined REPO_ROOT (
    call "%~dp0repo-root.bat"
)
if defined LEANCLR_OUT_ROOT (
    set "OUT_ROOT=%LEANCLR_OUT_ROOT%"
) else (
    set "OUT_ROOT=%REPO_ROOT%\out"
)
