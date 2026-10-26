@echo off
rem Delivery D - iPhone App EinsteinHat - Xcode project
rem Packages D's assets into Deliveries\EinsteinHatTiler.D. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery D
if errorlevel 1 exit /b 1
exit /b 0
