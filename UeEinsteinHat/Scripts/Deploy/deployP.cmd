@echo off
rem Delivery P - Demo Level with Common Unreal materials
rem Packages P's assets into Deliveries\EinsteinHatTiler.P. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery P
if errorlevel 1 exit /b 1
exit /b 0
