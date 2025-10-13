import subprocess
import time
import requests
import os
import sys
import logging

# Setup basic logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# Script settings
# Make paths relative to this script's location for robustness
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_PROJECT_ROOT = os.path.abspath(os.path.join(_SCRIPT_DIR, '..', '..', '..'))

SCRIPT_TO_TEST = os.path.join(_PROJECT_ROOT, "quanta_tissu/tisslm/integrations/simple_llm_integration.py")
SERVER_URL = os.environ.get("TISS_SERVER_URL", "http://localhost:8000")
LLAMACPP_URL = os.environ.get("LLAMACPP_URL", "http://127.0.0.1:8080")
# Increase the default wait time and add a timeout for the LLM request
WAIT_TIME_SECONDS = int(os.environ.get("EVAL_WAIT_TIME", "90")) # Wait time for the server to start and generate text
LLM_REQUEST_TIMEOUT = int(os.environ.get("LLM_REQUEST_TIMEOUT", "85")) # Timeout for the actual LLM call

# This is the session name used in the main block of the script-under-test
TEST_SESSION = "test1"
EVALUATION_OUTPUT_DIR = os.path.join(_PROJECT_ROOT, "llm_outputs")
EVALUATION_OUTPUT_FILE = os.path.join(EVALUATION_OUTPUT_DIR, f"{TEST_SESSION}.txt")

def check_llamacpp_server(url):
    """Checks if the Llama.cpp server is running before starting the test."""
    logging.info(f"Checking for Llama.cpp server at {url}...")
    try:
        # The base URL should respond. /completion is a POST endpoint.
        response = requests.get(url, timeout=3)
        if response.status_code == 200:
            logging.info("Llama.cpp server is responsive.")
            return True
        else:
            logging.warning(f"Llama.cpp server responded with status {response.status_code}. The test might fail if it's not working correctly.")
            return True # Continue anyway, maybe it's a different setup
    except requests.exceptions.RequestException:
        logging.error(f"Could not connect to Llama.cpp server at {url}.")
        logging.error("Please ensure the Llama.cpp server is running before executing this evaluation.")
        return False


def run_evaluation():
    """
    Starts the integration script, tests its HTTP endpoint, and verifies the output.
    """
    logging.info("--- Starting Local LLM Evaluation ---")

    if not check_llamacpp_server(LLAMACPP_URL):
        return False

    server_process = None
    try:
        # Start the server script in the background
        logging.info(f"Starting server script: {SCRIPT_TO_TEST}")

        # Ensure the script under test uses the same Llama.cpp URL and timeouts if they're set
        env = os.environ.copy()
        env["LLAMACPP_URL"] = f"{LLAMACPP_URL}/completion"
        env["LLM_REQUEST_TIMEOUT"] = str(LLM_REQUEST_TIMEOUT)

        server_process = subprocess.Popen(
            [sys.executable, SCRIPT_TO_TEST],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding='utf-8',
            env=env
        )

        # Give the server a moment to start up and call the LLM
        logging.info(f"Waiting for server to initialize and generate text (up to {WAIT_TIME_SECONDS}s)...")
        time.sleep(WAIT_TIME_SECONDS)

        # --- Test 1: Check if the server process is still running ---
        if server_process.poll() is not None:
            logging.error("FAILURE: The server process terminated unexpectedly.")
            stdout, stderr = server_process.communicate()
            logging.error(f"--- Server STDOUT ---\n{stdout}")
            logging.error(f"--- Server STDERR ---\n{stderr}")
            return False
        logging.info("Server process is running as expected.")

        # --- Test 2: Check if the server is running and accessible ---
        endpoint_url = f"{SERVER_URL}/llm/{TEST_SESSION}"
        logging.info(f"Checking server endpoint at {endpoint_url}")
        response = requests.get(endpoint_url)

        if response.status_code == 200:
            logging.info(f"SUCCESS: Server responded with 200 OK.")
        else:
            logging.error(f"FAILURE: Server responded with {response.status_code}.")
            return False

        # --- Test 3: Verify the output file was created and contains expected content ---
        logging.info(f"Verifying output file: {EVALUATION_OUTPUT_FILE}")
        if not os.path.exists(EVALUATION_OUTPUT_FILE):
            logging.error(f"FAILURE: Output file '{EVALUATION_OUTPUT_FILE}' was not found.")
            return False

        with open(EVALUATION_OUTPUT_FILE, "r", encoding="utf-8") as f:
            content = f.read()

        if not content:
            logging.error(f"FAILURE: Output file is empty.")
            return False

        if "Error connecting to LLM server" in content:
            logging.error(f"FAILURE: Output file contains an error message: {content}")
            return False

        logging.info("SUCCESS: Output file exists and is not empty.")
        logging.info(f"Response from server:\n---\n{content[:200]}...\n---")

        return True

    except requests.exceptions.RequestException as e:
        logging.error(f"FAILURE: Could not connect to the server: {e}")
        return False
    except FileNotFoundError:
        logging.error(f"FAILURE: Could not find the script to test: {SCRIPT_TO_TEST}")
        return False
    finally:
        if server_process:
            logging.info("Shutting down the server...")
            server_process.terminate()
            try:
                server_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                logging.warning("Server did not terminate gracefully, killing.")
                server_process.kill()
                server_process.wait()

        # Clean up the output file
        if os.path.exists(EVALUATION_OUTPUT_FILE):
            os.remove(EVALUATION_OUTPUT_FILE)
            logging.info(f"Cleaned up {EVALUATION_OUTPUT_FILE}")

        # Clean up the output directory if it's empty
        if os.path.exists(EVALUATION_OUTPUT_DIR) and not os.listdir(EVALUATION_OUTPUT_DIR):
            os.rmdir(EVALUATION_OUTPUT_DIR)
            logging.info(f"Cleaned up empty directory {EVALUATION_OUTPUT_DIR}")

        logging.info("--- Evaluation Finished ---")


if __name__ == "__main__":
    try:
        success = run_evaluation()
        if not success:
            sys.exit(1)
    except Exception as e:
        logging.critical(f"An unhandled exception occurred in the evaluation script: {e}", exc_info=True)
        sys.exit(1)
