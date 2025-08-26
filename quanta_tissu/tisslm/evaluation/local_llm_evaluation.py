import subprocess
import time
import requests
import os
import sys

# Script settings
SCRIPT_TO_TEST = "quanta_tissu/tisslm/integrations/tissdb_integration.py"
SERVER_URL = "http://localhost:8000"
# This is the session name used in the main block of the script-under-test
TEST_SESSION = "test1"
EVALUATION_OUTPUT_FILE = f"llm_outputs/{TEST_SESSION}.txt"


def run_evaluation():
    """
    Starts the integration script, tests its HTTP endpoint, and verifies the output.
    """
    print("--- Starting Local LLM Evaluation ---")

    server_process = None
    try:
        # Start the server script in the background
        print(f"Starting server: {SCRIPT_TO_TEST}")
        # We run it from the root of the repo, so it can find './tissdb'
        server_process = subprocess.Popen(
            [sys.executable, SCRIPT_TO_TEST],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

        # Give the server a moment to start up
        print("Waiting for server to initialize...")
        time.sleep(5)

        # --- Test 1: Check if the server is running and accessible ---
        print(f"Checking server endpoint at {SERVER_URL}/llm/{TEST_SESSION}")
        response = requests.get(f"{SERVER_URL}/llm/{TEST_SESSION}")

        if response.status_code == 200:
            print(f"SUCCESS: Server responded with 200 OK.")
        else:
            print(f"FAILURE: Server responded with {response.status_code}.")
            return False

        # --- Test 2: Verify the output file was created and contains expected content ---
        # The test assumes a running llamacpp server that will return a valid response.
        # We can't know the exact content, but we can check if it's not empty and doesn't contain the error message.
        print(f"Verifying output file: {EVALUATION_OUTPUT_FILE}")
        if not os.path.exists(EVALUATION_OUTPUT_FILE):
            print(f"FAILURE: Output file '{EVALUATION_OUTPUT_FILE}' was not found.")
            return False

        with open(EVALUATION_OUTPUT_FILE, "r", encoding="utf-8") as f:
            content = f.read()

        if not content:
            print(f"FAILURE: Output file is empty.")
            return False

        if "Error connecting to LLM server" in content:
            print(f"FAILURE: Output file contains an error message: {content}")
            return False

        print("SUCCESS: Output file exists and is not empty.")
        print(f"Response from server:\n---\n{content[:200]}...\n---")

        return True

    except requests.exceptions.RequestException as e:
        print(f"FAILURE: Could not connect to the server: {e}")
        return False
    finally:
        if server_process:
            print("Shutting down the server...")
            server_process.terminate()
            server_process.wait()

        # Clean up the output file
        if os.path.exists(EVALUATION_OUTPUT_FILE):
            os.remove(EVALUATION_OUTPUT_FILE)
            print(f"Cleaned up {EVALUATION_OUTPUT_FILE}")

        print("--- Evaluation Finished ---")


if __name__ == "__main__":
    success = run_evaluation()
    if not success:
        # Exit with a non-zero status code to indicate failure
        sys.exit(1)
