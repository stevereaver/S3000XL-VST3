#!/usr/bin/env pwsh
# ------------------------------------------------------------------------------
# Apply the S3000XL-VST patch set to a MAME 0.288 source tree.
# Assumes a sibling directory called 'mame' exists (or use -MameRoot).
# ------------------------------------------------------------------------------
param(
    [string] $MameRoot = (Join-Path (Split-Path $PSScriptRoot) mame)
)

$ErrorActionPreference = "Stop"
$mame = Resolve-Path $MameRoot -ErrorAction SilentlyContinue
if (-not $mame) { throw "MAME directory not found at $MameRoot" }

Get-ChildItem (Join-Path (Split-Path $PSScriptRoot) patches) -Filter *.patch | Sort-Object Name | ForEach-Object {
    Write-Host "Applying $($_.Name) to $mame" -ForegroundColor Cyan
    & git -C $mame apply --verbose $_.FullName
    if ($LASTEXITCODE -ne 0) { throw "Failed to apply $($_.Name)" }
}

Write-Host "All MAME patches applied." -ForegroundColor Green
