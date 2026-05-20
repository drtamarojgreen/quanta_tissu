#!/bin/bash

# Color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color
BLUE='\033[0;34m'

SERVER_URL="http://127.0.0.1:8000"

echo -e "${BLUE}=== Starting QuantaTissu Platform API Curl Test Suite ===${NC}\n"

# Helper function to print results
check_response() {
    local endpoint="$1"
    local status_code="$2"
    local expected_code="$3"
    
    if [ "$status_code" -eq "$expected_code" ]; then
        echo -e "${GREEN}[PASS]${NC} $endpoint returned status $status_code"
    else
        echo -e "${RED}[FAIL]${NC} $endpoint returned status $status_code (expected $expected_code)"
    fi
}

# 1. Start a task using POST /api/processes
echo -e "${BLUE}1. Testing: POST /api/processes (start task)${NC}"
RESPONSE=$(curl -s -w "\n%{http_code}" -X POST -H "Content-Type: application/json" \
  -d '{"task_id": "curl_echo_task", "type": "shell", "command": "echo \"Hello from API verification curl test!\""}' \
  "$SERVER_URL/api/processes")

HTTP_STATUS=$(echo "$RESPONSE" | tail -n1)
JSON_BODY=$(echo "$RESPONSE" | head -n -1)
check_response "POST /api/processes" "$HTTP_STATUS" 200
echo "Response: $JSON_BODY"
echo ""

# 2. Get status using GET /api/processes/curl_echo_task
echo -e "${BLUE}2. Testing: GET /api/processes/curl_echo_task (get status)${NC}"
sleep 0.5 # Wait a moment for execution
RESPONSE=$(curl -s -w "\n%{http_code}" "$SERVER_URL/api/processes/curl_echo_task")
HTTP_STATUS=$(echo "$RESPONSE" | tail -n1)
JSON_BODY=$(echo "$RESPONSE" | head -n -1)
check_response "GET /api/processes/curl_echo_task" "$HTTP_STATUS" 200
echo "Response: $JSON_BODY"
echo ""

# 3. Retrieve logs using GET /api/processes/curl_echo_task/logs
echo -e "${BLUE}3. Testing: GET /api/processes/curl_echo_task/logs (get logs)${NC}"
RESPONSE=$(curl -s -w "\n%{http_code}" "$SERVER_URL/api/processes/curl_echo_task/logs")
HTTP_STATUS=$(echo "$RESPONSE" | tail -n1)
JSON_BODY=$(echo "$RESPONSE" | head -n -1)
check_response "GET /api/processes/curl_echo_task/logs" "$HTTP_STATUS" 200
echo "Response: $JSON_BODY"
echo ""

# 4. Verify process using GET /api/processes/verify/curl_echo_task
echo -e "${BLUE}4. Testing: GET /api/processes/verify/curl_echo_task (verify)${NC}"
RESPONSE=$(curl -s -w "\n%{http_code}" "$SERVER_URL/api/processes/verify/curl_echo_task")
HTTP_STATUS=$(echo "$RESPONSE" | tail -n1)
JSON_BODY=$(echo "$RESPONSE" | head -n -1)
check_response "GET /api/processes/verify/curl_echo_task" "$HTTP_STATUS" 200
echo "Response: $JSON_BODY"
echo ""

# 5. Start a sleeping task to test process termination
echo -e "${BLUE}5. Testing: Stop running task with DELETE /api/processes/{taskId}${NC}"
echo "Starting sleep task..."
RESPONSE=$(curl -s -w "\n%{http_code}" -X POST -H "Content-Type: application/json" \
  -d '{"task_id": "curl_sleep_task", "type": "shell", "command": "sleep 30"}' \
  "$SERVER_URL/api/processes")
HTTP_STATUS=$(echo "$RESPONSE" | tail -n1)
check_response "POST /api/processes (start sleep)" "$HTTP_STATUS" 200

echo "Sending stop signal..."
RESPONSE=$(curl -s -w "\n%{http_code}" -X DELETE "$SERVER_URL/api/processes/curl_sleep_task")
HTTP_STATUS=$(echo "$RESPONSE" | tail -n1)
JSON_BODY=$(echo "$RESPONSE" | head -n -1)
check_response "DELETE /api/processes/curl_sleep_task" "$HTTP_STATUS" 200
echo "Response: $JSON_BODY"

# Check if state is STOPPED
RESPONSE=$(curl -s -w "\n%{http_code}" "$SERVER_URL/api/processes/curl_sleep_task")
JSON_BODY=$(echo "$RESPONSE" | head -n -1)
echo "Updated status: $JSON_BODY"
echo ""

# 6. Get legacy tasks list using GET /api/tasks
echo -e "${BLUE}6. Testing: GET /api/tasks (legacy endpoint)${NC}"
RESPONSE=$(curl -s -w "\n%{http_code}" "$SERVER_URL/api/tasks")
HTTP_STATUS=$(echo "$RESPONSE" | tail -n1)
JSON_BODY=$(echo "$RESPONSE" | head -n -1)
check_response "GET /api/tasks" "$HTTP_STATUS" 200
echo "Response: $JSON_BODY"
echo ""

# 7. Edge Case: Missing parameters
echo -e "${BLUE}7. Testing Edge Case: POST /api/processes with missing parameters${NC}"
RESPONSE=$(curl -s -w "\n%{http_code}" -X POST -H "Content-Type: application/json" \
  -d '{"task_id": "incomplete_task"}' \
  "$SERVER_URL/api/processes")
HTTP_STATUS=$(echo "$RESPONSE" | tail -n1)
JSON_BODY=$(echo "$RESPONSE" | head -n -1)
check_response "POST /api/processes (incomplete)" "$HTTP_STATUS" 400
echo "Response: $JSON_BODY"
echo ""

echo -e "${BLUE}=== API Curl Test Suite Complete ===${NC}"
