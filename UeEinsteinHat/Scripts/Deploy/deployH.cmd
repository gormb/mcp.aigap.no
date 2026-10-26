@echo off
rem Delivery H - Config on Hat to raise or lower all points
rem Packages H's assets into Deliveries\EinsteinHatTiler.H. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery H
if errorlevel 1 exit /b 1
exit /b 0
