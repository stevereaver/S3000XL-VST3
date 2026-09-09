#!/usr/bin/env pwsh
# ------------------------------------------------------------------------------
# Clone the JUCE and MAME source trees needed to build the plugin locally.
# Run this from the repo root, or from anywhere; it resolves paths relative to
# the script's own directory.
# ------------------------------------------------------------------------------
param(
    [string] $JuceTag = "8.0.15",
    [string] $MameTag = "mame0288",
    [switch] $Shallow = $true
)

$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot ..)

function Clone-IfMissing($path, $repo, $tag) {
    if (Test-Path $path) {
        Write-Host "Already exists: $path" -ForegroundColor Green
        return
    }

    Write-Host "Cloning $repo @ $tag into $path" -ForegroundColor Cyan
    $args = @("clone")
    if ($Shallow) { $args += "--depth"; $args += "1" }
    $args += "--branch"; $args += $tag
    $args += $repo
    $args += $path

    & git @args
    if ($LASTEXITCODE -ne 0) { throw "git clone failed for $repo" }
}

Clone-IfMissing (Join-Path $root "JUCE") "https://github.com/juce-framework/JUCE.git" $JuceTag
Clone-IfMissing (Join-Path $root "mame")  "https://github.com/mamedev/mame.git"       $MameTag

Write-Host "Dependencies ready. Next steps:" -ForegroundColor Green
Write-Host "  1. scripts\apply-mame-patches.ps1"
Write-Host "  2. scripts\build-mame-libs.ps1"
Write-Host "  3. scripts\build-plugin.ps1"
