@echo off
rem Delivery C - Unreal Marketplace EinsteinHat
rem Packages C's assets into Deliveries\EinsteinHatTiler.C. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery C
if errorlevel 1 exit /b 1
exit /b 0
