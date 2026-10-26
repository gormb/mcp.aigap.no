@echo off
rem Delivery A - MVP Hat for JavaScript - HTML single-file
rem Packages A's assets into Deliveries\EinsteinHatTiler.A. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery A
if errorlevel 1 exit /b 1
exit /b 0
