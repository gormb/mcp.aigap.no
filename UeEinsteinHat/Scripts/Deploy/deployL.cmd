@echo off
rem Delivery L - Penrose Thin and Thick Rhombi working on Kite
rem Packages L's assets into Deliveries\EinsteinHatTiler.L. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery L
if errorlevel 1 exit /b 1
exit /b 0
