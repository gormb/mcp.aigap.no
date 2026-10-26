@echo off
rem Delivery J - Meta meta Hat
rem Packages J's assets into Deliveries\EinsteinHatTiler.J. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery J
if errorlevel 1 exit /b 1
exit /b 0
