@echo off
rem Delivery I - Config on Hat to enable all Spectre versions
rem Packages I's assets into Deliveries\EinsteinHatTiler.I. See DEV.md section 7.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deploy.ps1" -Delivery I
if errorlevel 1 exit /b 1
exit /b 0
