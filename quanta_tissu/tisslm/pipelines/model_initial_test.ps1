$ErrorActionPreference = "Stop"

# Define paths
$projectRoot = (Get-Item $PSScriptRoot).Parent.Parent.FullName
$tisslmDir = Join-Path $projectRoot "quanta_tissu\tisslm"
$pipelinesDir = Join-Path $projectRoot "quanta_tissu\pipelines"
$modelsDir = Join-Path $projectRoot "models"
$corpusDir = Join-Path $projectRoot "corpus"

$trainBpeModule = "quanta_tissu.tisslm.core.train_bpe"
$runTrainingModule = "quanta_tissu.tisslm.core.run_training"
$generateTextModule = "quanta_tissu.tisslm.core.generate_text"

$outputFile = Join-Path $pipelinesDir "test_output.txt"
$tokenizerPrefix = Join-Path $modelsDir "trained_tokenizer"
$checkpointDir = Join-Path $modelsDir "checkpoints"
$checkpointPath = Join-Path $checkpointDir "checkpoint_test_001.npz"

# Ensure checkpoints directory exists
if (-not (Test-Path $checkpointDir)) {
    New-Item -ItemType Directory -Path $checkpointDir | Out-Null
}

# Clear previous output file
Remove-Item $outputFile -ErrorAction SilentlyContinue

# Loop 3 times
for ($i = 1; $i -le 3; $i++) {
    Write-Host "`n--- Starting Run $i/3 ---" -ForegroundColor Green
    Add-Content -Path $outputFile -Value "`n" # Add a newline for separation
    Add-Content -Path $outputFile -Value "--- Starting Run $i/3 ---"
    Add-Content -Path $outputFile -Value "`n"

    # 1. Train BPE tokens
    Write-Host "[Run $i] Invoking BPE Tokenizer Training..." -ForegroundColor Yellow
    Add-Content -Path $outputFile -Value "[Run $i] Invoking BPE Tokenizer Training..."
    try {
        # Run as module with unbuffered output
        python -u -m $trainBpeModule --corpus_path $corpusDir --save_prefix $tokenizerPrefix | Out-File -Append -FilePath $outputFile 2>&1
        if ($LASTEXITCODE -ne 0) { throw "BPE Training failed with exit code $LASTEXITCODE." }
        Write-Host "[Run $i] BPE Tokenizer Training Completed." -ForegroundColor Green
        Add-Content -Path $outputFile -Value "[Run $i] BPE Tokenizer Training Completed."
    } catch {
        Write-Host "[Run $i] Error during BPE Training: $($_.Exception.Message)" -ForegroundColor Red
        Add-Content -Path $outputFile -Value "[Run $i] Error during BPE Training: $($_.Exception.Message)"
        continue # Skip to next run if BPE training fails
    }
    Add-Content -Path $outputFile -Value "`n"

    # 2. Create a checkpoint named checkpoint_test_001
    Write-Host "[Run $i] Invoking Model Training..." -ForegroundColor Yellow
    Add-Content -Path $outputFile -Value "[Run $i] Invoking Model Training..."
    try {
        # Ensure the checkpoint directory is clean for each run if we want a fresh checkpoint_test_001
        Remove-Item $checkpointPath -ErrorAction SilentlyContinue

        # Run as module with unbuffered output
        python -u -m $runTrainingModule --corpus_path $corpusDir --checkpoint_dir $checkpointDir --epochs 1 --save_every 1 | Out-File -Append -FilePath $outputFile 2>&1
        if ($LASTEXITCODE -ne 0) { throw "Model Training failed with exit code $LASTEXITCODE." }
        Write-Host "[Run $i] Model Training Completed. Checkpoint saved to $checkpointPath." -ForegroundColor Green
        Add-Content -Path $outputFile -Value "[Run $i] Model Training Completed. Checkpoint saved to $checkpointPath."
    } catch {
        Write-Host "[Run $i] Error during Model Training: $($_.Exception.Message)" -ForegroundColor Red
        Add-Content -Path $outputFile -Value "[Run $i] Error during Model Training: $($_.Exception.Message)"
        continue # Skip to next run if Model training fails
    }
    Add-Content -Path $outputFile -Value "`n"

    # 3. Use generate_text script to test a prompt
    $prompt = "Do you know this is a test?"
    Write-Host "[Run $i] Invoking Text Generation..." -ForegroundColor Yellow
    Add-Content -Path $outputFile -Value "[Run $i] Invoking Text Generation..."
    try {
        # Run as module with unbuffered output
        python -u -m $generateTextModule --prompt "$prompt" --checkpoint_path $checkpointPath --length 50 | Out-File -Append -FilePath $outputFile 2>&1
        if ($LASTEXITCODE -ne 0) { throw "Text Generation failed with exit code $LASTEXITCODE." }
        Write-Host "[Run $i] Text Generation Completed." -ForegroundColor Green
        Add-Content -Path $outputFile -Value "[Run $i] Text Generation Completed."
    } catch {
        Write-Host "[Run $i] Error during Text Generation: $($_.Exception.Message)" -ForegroundColor Red
        Add-Content -Path $outputFile -Value "[Run $i] Error during Text Generation: $($_.Exception.Message)"
        continue # Skip to next run if Text Generation fails
    }
    Add-Content -Path $outputFile -Value "`n"

    Write-Host "--- Finished Run $i/3 ---" -ForegroundColor Green
    Add-Content -Path $outputFile -Value "--- Finished Run $i/3 ---"
}

Write-Host "`nAll runs complete. Check $outputFile for details." -ForegroundColor Cyan
Add-Content -Path $outputFile -Value "`n" # Add a newline for separation
Add-Content -Path $outputFile -Value "All runs complete. Check $outputFile for details."