#!/usr/bin/env pwsh
# ------------------------------------------------------------------------------
# Build the S3000XL-VST plugin with CMake.
# Assumes MAME libraries have already been built (see build-mame-libs.bat).
# ------------------------------------------------------------------------------
param(
    [ValidateSet("Release", "Debug", "RelWithDebInfo")]
    [string] $Config = "Release",

    [string] $BuildDir = "build",

    [string] $JuceRoot = $env:JUCE_ROOT,
    [string] $MameRoot = $env:MAME_ROOT
)

$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot ..)

if (-not $JuceRoot) { $JuceRoot = Join-Path $root "JUCE" }
if (-not $MameRoot) { $MameRoot = Join-Path $root "mame" }

if (-not (Test-Path (Join-Path $JuceRoot "CMakeLists.txt"))) {
    throw "JUCE not found at $JuceRoot. Run scripts\setup-dependencies.ps1 first."
}
if (-not (Test-Path (Join-Path $MameRoot "makefile"))) {
    throw "MAME not found at $MameRoot. Run scripts\setup-dependencies.ps1 first."
}

$build = Join-Path $root $BuildDir
Write-Host "Configuring CMake..." -ForegroundColor Cyan
& cmake -B $build -G "Visual Studio 17 2022" -A x64 `
    -DJUCE_ROOT="$JuceRoot" `
    -DMAME_ROOT="$MameRoot"
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

Write-Host "Building plugin ($Config)..." -ForegroundColor Cyan
& cmake --build $build --config $Config --target S3000XL-VST_VST3 --parallel
if ($LASTEXITCODE -ne 0) { throw "CMake build failed" }

$vst3 = Get-ChildItem -Path $build -Recurse -Filter "*.vst3" | Select-Object -First 1
if ($vst3) {
    Write-Host "Built: $($vst3.FullName)" -ForegroundColor Green
} else {
    Write-Warning "Could not locate the generated .vst3 bundle"
}
