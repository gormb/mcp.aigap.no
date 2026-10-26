@echo off
rem Delivery O - Demo Level that uses Penrose Materials
rem Packages O's assets into Deliveries\EinsteinHatTiler.O. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery O
if errorlevel 1 exit /b 1
exit /b 0
