@echo off
rem Delivery N - Penrose Thin and Thick Rhombi working on MetaHat
rem Packages N's assets into Deliveries\EinsteinHatTiler.N. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery N
if errorlevel 1 exit /b 1
exit /b 0
