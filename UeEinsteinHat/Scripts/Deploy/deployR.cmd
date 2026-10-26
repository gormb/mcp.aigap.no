@echo off
rem Delivery R - Paying Customer
rem Packages R's assets into Deliveries\EinsteinHatTiler.R. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery R
if errorlevel 1 exit /b 1
exit /b 0
