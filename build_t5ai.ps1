# TuyaOpen Build Script for T5AI
# Run this script from PowerShell (as Administrator if needed)

Write-Host "=== TuyaOpen T5AI Build Script ===" -ForegroundColor Cyan

# Check if we're in the right directory
$projectRoot = "E:\TuyaO\TuyaOpen"
if (-not (Test-Path "$projectRoot\tos.py")) {
    Write-Host "Error: Please run this script from the TuyaOpen root directory" -ForegroundColor Red
    exit 1
}

# Step 1: Install dependencies
Write-Host "`n[1/4] Installing Python dependencies..." -ForegroundColor Yellow
cd $projectRoot

# Try to install dependencies
# Option 1: Try user install
Write-Host "Attempting to install dependencies..." -ForegroundColor Gray
python -m pip install --user click-completion click click-option-group cmake ninja kconfiglib PyYAML requests GitPython colorama 2>&1 | Out-Null

# Option 2: If that fails, try virtual environment
if ($LASTEXITCODE -ne 0) {
    Write-Host "User install failed, trying virtual environment..." -ForegroundColor Gray
    if (-not (Test-Path ".venv")) {
        python -m venv .venv
    }
    & ".\.venv\Scripts\activate.ps1"
    pip install -r requirements.txt
}

# Step 2: Navigate to project
Write-Host "`n[2/4] Navigating to your_chat_bot project..." -ForegroundColor Yellow
$projectDir = "$projectRoot\apps\tuya.ai\your_chat_bot"
cd $projectDir

# Verify config is set
if (Test-Path "app_default.config") {
    $configContent = Get-Content "app_default.config" -Raw
    if ($configContent -match "CONFIG_BOARD_CHOICE_T5AI=y") {
        Write-Host "✓ T5AI configuration detected" -ForegroundColor Green
    } else {
        Write-Host "Warning: T5AI config not found in app_default.config" -ForegroundColor Yellow
    }
} else {
    Write-Host "Error: app_default.config not found!" -ForegroundColor Red
    exit 1
}

# Step 3: Build
Write-Host "`n[3/4] Building project..." -ForegroundColor Yellow
Write-Host "Running: python ..\..\..\tos.py build" -ForegroundColor Gray
python ..\..\..\tos.py build

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n✓ Build successful!" -ForegroundColor Green
    
    # Find the output binary
    $buildDir = ".\.build\bin"
    if (Test-Path $buildDir) {
        $binFiles = Get-ChildItem -Path $buildDir -Filter "*.bin" | Select-Object -First 1
        if ($binFiles) {
            Write-Host "`nFirmware binary: $($binFiles.FullName)" -ForegroundColor Cyan
        }
    }
} else {
    Write-Host "`n✗ Build failed!" -ForegroundColor Red
    Write-Host "Please check the error messages above." -ForegroundColor Yellow
    exit 1
}

# Step 4: Instructions for flashing
Write-Host "`n[4/4] Ready to flash!" -ForegroundColor Yellow
Write-Host "`nTo flash the firmware to your device:" -ForegroundColor Cyan
Write-Host "1. Connect your T5AI board via USB" -ForegroundColor White
Write-Host "2. Run: python ..\..\..\tos.py flash" -ForegroundColor White
Write-Host "3. Select the COM port (usually the one with 'A' in the name)" -ForegroundColor White
Write-Host "`nTo monitor logs:" -ForegroundColor Cyan
Write-Host "   python ..\..\..\tos.py monitor" -ForegroundColor White

Write-Host "`n=== Done ===" -ForegroundColor Green
