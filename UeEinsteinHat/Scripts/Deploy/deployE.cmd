@echo off
rem Delivery E - KiteShape - HTML, Unreal, C++
rem Packages E's assets into Deliveries\EinsteinHatTiler.E. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery E
if errorlevel 1 exit /b 1
exit /b 0
