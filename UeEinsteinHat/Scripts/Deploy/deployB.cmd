@echo off
rem Delivery B - MVP Hat for Unreal Engine
rem Packages B's assets into Deliveries\EinsteinHatTiler.B. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery B
if errorlevel 1 exit /b 1
exit /b 0
