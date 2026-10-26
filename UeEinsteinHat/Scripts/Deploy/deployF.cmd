@echo off
rem Delivery F - MetaHat - HTML, Unreal, C++
rem Packages F's assets into Deliveries\EinsteinHatTiler.F. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery F
if errorlevel 1 exit /b 1
exit /b 0
