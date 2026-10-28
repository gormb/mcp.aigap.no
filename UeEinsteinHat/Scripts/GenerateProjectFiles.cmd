@echo off
rem Regenerates the Visual Studio 2026 solution and the VS Code IntelliSense database.
rem UE 5.8 has no GenerateProjectFiles.bat, so UnrealBuildTool is invoked directly.
rem Visual Studio 18 is Visual Studio 2026 - passing -2022 fails on this machine.

set UE_ROOT=C:\p\UE_5.8
set PROJECT=%~dp0..\UeEinsteinHat.uproject

dotnet "%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" ^
    -ProjectFiles ^
    -Project="%PROJECT%" ^
    -ProjectFileFormat=VisualStudio2026 ^
    -ProjectFileFormat=VisualStudioCode ^
    -Game ^
    -Engine
