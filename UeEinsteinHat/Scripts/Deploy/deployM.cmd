@echo off
rem Delivery M - Penrose Thin and Thick Rhombi working on Hat
rem Packages M's assets into Deliveries\EinsteinHatTiler.M. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery M
if errorlevel 1 exit /b 1
exit /b 0
