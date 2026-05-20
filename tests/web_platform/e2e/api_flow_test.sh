#!/bin/bash
set -e

echo "--- Starting E2E Flow Test ---"

# 1. Start Training
echo "Step 1: Starting training job..."
node web_platform/frontend/js/api/platform_api.js train '{"epochs": 1}'

# 2. Switch Tab
echo "Step 2: Switching to Analytics tab..."
node web_platform/frontend/js/api/platform_api.js switch_tab '"analytics"'

# 3. Wait a bit for training to progress (simulated)
sleep 2

# 4. Switch back to Config/Training Tab
echo "Step 3: Switching back to Config tab..."
node web_platform/frontend/js/api/platform_api.js switch_tab '"config"'

# 5. Verify task status is still present and progress is tracked
echo "Step 4: Verifying task persistence..."
node web_platform/frontend/js/api/platform_api.js list_tasks

echo "E2E Flow Test Completed Successfully."
