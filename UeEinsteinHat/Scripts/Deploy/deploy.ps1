#Requires -Version 5.1
<#
.SYNOPSIS
    Assembles one milestone delivery package for UeEinsteinHat.

.DESCRIPTION
    Copies the assets listed for a delivery out of the project and into
    Deliveries/EinsteinHatTiler.<letter>/. See DEV.md section 7.

    Exits non-zero when a listed asset is missing or the delivery has no
    manifest yet, so an incomplete package cannot be shipped by accident.

.EXAMPLE
    .\deploy.ps1 -Delivery B
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[A-Ra-r]$')]
    [string] $Delivery
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

# Source path inside the project -> destination path inside the package.
# Content/ assets keep their feature folder; Documentation/ stays top level.
$Manifests = @{
    A = @{
        Title = 'MVP Hat for JavaScript - HTML single-file'
        Files = @(
            @('Content/Maps/ExampleMap.umap', 'Maps/ExampleMap.umap'),
            @('Content/Blueprints/BP_EinsteinHatTilerMvp.uasset', 'Blueprints/BP_EinsteinHatTilerMvp.uasset'),
            @('Content/Blueprints/WBP_EinsteinHatTilerMvp.uasset', 'Blueprints/WBP_EinsteinHatTilerMvp.uasset'),
            @('Documentation/EinsteinHatTilerMvp.html', 'Documentation/EinsteinHatTilerMvp.html')
        )
    }
    B = @{
        Title = 'MVP Hat for Unreal Engine'
        Files = @(
            @('Content/Blueprints/BPC_EinsteinHatTilerMin.uasset', 'Blueprints/BPC_EinsteinHatTilerMin.uasset'),
            @('Content/Maps/ExampleMap.umap', 'Maps/ExampleMap.umap'),
            @('Source/EinsteinHatTiler/Public/EinsteinHatTypes.h', 'Source/EinsteinHatTypes.h'),
            @('Source/EinsteinHatTiler/Public/KiteActor.h', 'Source/KiteActor.h'),
            @('Source/EinsteinHatTiler/Private/KiteActor.cpp', 'Source/KiteActor.cpp'),
            @('Source/EinsteinHatTiler/Public/HatShape.h', 'Source/HatShape.h'),
            @('Source/EinsteinHatTiler/Private/HatShape.cpp', 'Source/HatShape.cpp')
        )
    }
    C = @{
        Title = 'Unreal Marketplace EinsteinHat'
        Files = @(
            @('Content/Blueprints/BPC_EinsteinHatTilerMin.uasset', 'Blueprints/BPC_EinsteinHatTilerMin.uasset'),
            @('Content/Materials/M_HatTileMin.uasset', 'Materials/M_HatTileMin.uasset'),
            @('Content/Maps/ExampleMap.umap', 'Maps/ExampleMap.umap'),
            @('Documentation/README.pdf', 'Documentation/README.pdf'),
            @('Documentation/Icon128.png', 'Documentation/Icon128.png')
        )
    }
    D = @{
        Title = 'iPhone App EinsteinHat - Xcode project'
        Files = @()
    }
    E = @{
        Title = 'KiteShape - HTML, Unreal, C++'
        Files = @(
            @('Content/Blueprints/BPC_EinsteinHatTilerKite.uasset', 'Blueprints/BPC_EinsteinHatTilerKite.uasset'),
            @('Content/Materials/M_HatTileKite.uasset', 'Materials/M_HatTileKite.uasset'),
            @('Content/Maps/ExampleMap.umap', 'Maps/ExampleMap.umap'),
            @('Documentation/README.pdf', 'Documentation/README.pdf')
        )
    }
    F = @{
        Title = 'MetaHat - HTML, Unreal, C++'
        Files = @(
            @('Content/Blueprints/BPC_EinsteinHatTiler.uasset', 'Blueprints/BPC_EinsteinHatTiler.uasset'),
            @('Content/Materials/M_HatTileMeta.uasset', 'Materials/M_HatTileMeta.uasset'),
            @('Content/Maps/ExampleMap.umap', 'Maps/ExampleMap.umap'),
            @('Documentation/README.pdf', 'Documentation/README.pdf')
        )
    }
    G = @{ Title = 'Config on Kite to raise or lower mid-point'; Files = @() }
    H = @{ Title = 'Config on Hat to raise or lower all points'; Files = @() }
    I = @{ Title = 'Config on Hat to enable all Spectre versions, the outliers and the middle'; Files = @() }
    J = @{ Title = 'Meta meta Hat'; Files = @() }
    K = @{ Title = 'Material Penrose Thin and Thick Rhombi'; Files = @() }
    L = @{ Title = 'Penrose Thin and Thick Rhombi working on Kite'; Files = @() }
    M = @{ Title = 'Penrose Thin and Thick Rhombi working on Hat'; Files = @() }
    N = @{ Title = 'Penrose Thin and Thick Rhombi working on MetaHat'; Files = @() }
    O = @{ Title = 'Demo Level that uses Penrose Materials'; Files = @() }
    P = @{ Title = 'Demo Level with Common Unreal materials'; Files = @() }
    Q = @{ Title = 'Payable Unreal Component'; Files = @() }
    R = @{ Title = 'Paying Customer'; Files = @() }
}

$letter = $Delivery.ToUpperInvariant()
$manifest = $Manifests[$letter]

# Scripts/Deploy -> Scripts -> project root
$projectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$packageName = "EinsteinHatTiler.$letter"
$packageRoot = Join-Path (Join-Path $projectRoot 'Deliveries') $packageName

Write-Host "Delivery $letter - $($manifest.Title)"
Write-Host "Package  $packageRoot"
Write-Host ''

if ($manifest.Files.Count -eq 0) {
    Write-Warning "Delivery $letter has no manifest yet (DEV.md section 7). Nothing was copied."
    exit 1
}

$missing = @()
$copied = 0

foreach ($entry in $manifest.Files) {
    $source = Join-Path $projectRoot $entry[0]
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
        $missing += $entry[0]
        continue
    }

    $target = Join-Path $packageRoot $entry[1]
    $targetDirectory = Split-Path -Parent $target
    if (-not (Test-Path -LiteralPath $targetDirectory)) {
        $null = New-Item -ItemType Directory -Force -Path $targetDirectory
    }

    Copy-Item -LiteralPath $source -Destination $target -Force
    $copied++
}

Write-Host "Copied $copied of $($manifest.Files.Count) file(s)."

if ($missing.Count -gt 0) {
    Write-Host ''
    Write-Warning "$($missing.Count) source file(s) have not been built yet:"
    foreach ($path in $missing) {
        Write-Host "  - $path"
    }
    exit 1
}

exit 0
