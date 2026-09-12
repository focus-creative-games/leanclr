@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0interp-smoke-matrix.ps1" %*
