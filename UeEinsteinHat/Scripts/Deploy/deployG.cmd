@echo off
rem Delivery G - Config on Kite to raise or lower mid-point
rem Packages G's assets into Deliveries\EinsteinHatTiler.G. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery G
if errorlevel 1 exit /b 1
exit /b 0
