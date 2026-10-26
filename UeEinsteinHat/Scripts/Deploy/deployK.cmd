@echo off
rem Delivery K - Material Penrose Thin and Thick Rhombi
rem Packages K's assets into Deliveries\EinsteinHatTiler.K. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery K
if errorlevel 1 exit /b 1
exit /b 0
