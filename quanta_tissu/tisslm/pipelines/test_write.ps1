$ErrorActionPreference = "Stop"

$testOutputFile = Join-Path $PSScriptRoot "test_output_simple.txt"

Write-Host "Attempting to create and write to $testOutputFile" -ForegroundColor Yellow
Add-Content -Path $testOutputFile -Value "This is a test line written at $(Get-Date)."
Write-Host "Successfully wrote to $testOutputFile" -ForegroundColor Green

Write-Host "Script finished." -ForegroundColor Cyan
