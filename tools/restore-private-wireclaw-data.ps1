$ErrorActionPreference = "Stop"

$root = Resolve-Path (Join-Path $PSScriptRoot "..")
$privateData = Join-Path $root "private\wireclaw-brain\data"
$brainData = Join-Path $root "firmware\wireclaw-brain\data"

if (-not (Test-Path (Join-Path $privateData "config.json"))) {
    throw "No private WireClaw config found at $privateData"
}

New-Item -ItemType Directory -Force $brainData | Out-Null
Copy-Item -Force (Join-Path $privateData "*.json") $brainData
Copy-Item -Force (Join-Path $privateData "system_prompt.txt") $brainData

Write-Host "Restored private WireClaw data into firmware\wireclaw-brain\data"
Write-Host "Run from firmware\wireclaw-brain:"
Write-Host "  python -m platformio run -e esp32-s3 --target uploadfs"
