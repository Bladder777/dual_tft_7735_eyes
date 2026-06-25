param(
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

$root = Resolve-Path (Join-Path $PSScriptRoot "..")
$brain = Join-Path $root "firmware\wireclaw-brain"
$servo = Join-Path $root "firmware\esp32d-servo-bridge"
$python = "python"
$env:PLATFORMIO_OFFLINE = "1"

function Invoke-Pio {
    param(
        [string]$ProjectDir,
        [string[]]$Arguments
    )

    Push-Location $ProjectDir
    try {
        Write-Host ""
        Write-Host "==> $ProjectDir"
        if ($Clean) {
            & $python -m platformio run --target clean
            if ($LASTEXITCODE -ne 0) {
                throw "PlatformIO clean failed in $ProjectDir"
            }
        }
        & $python -m platformio @Arguments
        if ($LASTEXITCODE -ne 0) {
            throw "PlatformIO build failed in $ProjectDir"
        }
    } finally {
        Pop-Location
    }
}

$eyeEnvs = @(
    "esp32-s3-supermini",
    "left_eye_master",
    "left_eye_slave",
    "right_eye_slave"
)

foreach ($envName in $eyeEnvs) {
    Invoke-Pio -ProjectDir $root -Arguments @(
        "run",
        "-e", $envName
    )
}

Invoke-Pio -ProjectDir $brain -Arguments @(
    "run",
    "-e", "esp32-s3"
)

Invoke-Pio -ProjectDir $servo -Arguments @(
    "run",
    "-e", "esp32d-servo-bridge"
)

Write-Host ""
Write-Host "All firmware builds completed."
