@echo off
rem Delivery Q - Payable Unreal Component
rem Packages Q's assets into Deliveries\EinsteinHatTiler.Q. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery Q
if errorlevel 1 exit /b 1
exit /b 0
