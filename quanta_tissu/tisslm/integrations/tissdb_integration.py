#!/usr/bin/env python3
import subprocess
import time
import os
import http.server
import socketserver
import threading
import sys
import requests
import logging

# Setup basic logging for better diagnostics
logging.basicConfig(level=logging.INFO, format='[%(asctime)s] [%(levelname)s] %(message)s')

# Determine project root dynamically to make paths more robust
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_PROJECT_ROOT = os.path.abspath(os.path.join(_SCRIPT_DIR, '..', '..', '..'))

# === CONFIG ===
TISSDB_EXECUTABLE_NAME = "tissdb.exe" if sys.platform == "win32" else "tissdb"
TISSDB_DIR = os.path.join(_PROJECT_ROOT, "tissdb")
TISSDB_BIN = os.path.join(TISSDB_DIR, TISSDB_EXECUTABLE_NAME)
OUTPUT_DIR = os.path.join(_PROJECT_ROOT, "llm_outputs")
TISSDB_PORT = os.environ.get("TISSDB_PORT", "9876") # Port for the C++ TissDB server
PORT = 8000 # Port for this Python script's HTTP server
LLAMACPP_URL = os.environ.get("LLAMACPP_URL", "http://127.0.0.1:8080/completion")

# === NEXUS FLOW HOOK ===
def nexus_flow_hook(event: str, payload: dict):
    """
    Minimal integration: nexus_flow could be imported or called
    as a function. For now, simulate with logging.
    """
    logging.info(f"[nexus_flow] Event={event}, Payload={payload}")


# === STEP 1: START TISSDB SERVER ===
def start_tissdb():
    logging.info("Starting TissDB server...")
    if not os.path.isfile(TISSDB_BIN):
        logging.error(f"TissDB executable not found at {TISSDB_BIN}")
        logging.error("Please compile TissDB first by running 'make' in the 'tissdb' directory.")
        return None

    if not os.access(TISSDB_BIN, os.X_OK):
        logging.error(f"TissDB executable at {TISSDB_BIN} is not executable.")
        logging.error("Please run 'chmod +x tissdb/tissdb' to grant execute permissions.")
        return None

    # The TissDB binary should be run from its own directory
    # to ensure it can find any relative files it needs.
    logging.info(f"Attempting to start TissDB on port {TISSDB_PORT}...")
    proc = subprocess.Popen(
        [f"./{TISSDB_EXECUTABLE_NAME}", str(TISSDB_PORT)],
        cwd=TISSDB_DIR,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        encoding='utf-8'
    )
    time.sleep(2)  # allow warmup

    # Check if the process terminated unexpectedly
    if proc.poll() is not None:
        logging.error("TissDB server failed to start or terminated unexpectedly.")
        stdout, stderr = proc.communicate()
        logging.error(f"TissDB STDOUT: {stdout}")
        logging.error(f"TissDB STDERR: {stderr}")
        return None

    nexus_flow_hook("tissdb_started", {"pid": proc.pid})
    return proc


# === STEP 2: GET COMPLETION FROM LLAMACPP SERVER ===
def train_llm(prompt: str, session_name: str = "session"):
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    output_file = os.path.join(OUTPUT_DIR, f"{session_name}.txt")

    logging.info(f"Getting completion for prompt: {prompt[:50]}...")
    nexus_flow_hook("llm_completion_started", {"session": session_name})

    try:
        payload = {
            "prompt": prompt,
            "n_predict": 256,
        }
        # Add a timeout to prevent hanging indefinitely. Must be less than the evaluation script's wait time.
        response = requests.post(LLAMACPP_URL, json=payload, timeout=40)
        response.raise_for_status()  # Raise an exception for bad status codes

        # Assuming the response is JSON and the completion is in the 'content' key
        completion = response.json().get("content", "")

        if not completion:
            logging.warning("LLM server returned a successful response but the 'content' was empty.")

        with open(output_file, "w", encoding="utf-8") as f:
            f.write(completion)

        nexus_flow_hook("llm_completion_complete", {"session": session_name, "file": output_file})
        logging.info(f"Output saved at {output_file}")

    # Catch specific request exceptions first
    except requests.exceptions.RequestException as e:
        error_message = f"Error connecting to LLM server: {e}"
        logging.error(error_message)
        nexus_flow_hook("llm_completion_failed", {"session": session_name, "error": str(e)})
        # Write error to file so the client can see it
        with open(output_file, "w", encoding="utf-8") as f:
            f.write(error_message)
    # Catch any other unexpected exceptions to prevent silent failures
    except Exception as e:
        error_message = f"An unexpected error occurred in train_llm: {e.__class__.__name__}: {e}"
        logging.error(error_message, exc_info=True) # Log the full traceback
        nexus_flow_hook("llm_completion_failed", {"session": session_name, "error": str(e)})
        with open(output_file, "w", encoding="utf-8") as f:
            f.write(error_message)

    return output_file


# === STEP 3: SERVER TO RETRIEVE CONTENT ===
class LLMHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path.startswith("/llm/"):
            _, _, fname = self.path.partition("/llm/")
            fpath = os.path.join(OUTPUT_DIR, fname + ".txt")
            if os.path.exists(fpath):
                self.send_response(200)
                self.send_header("Content-type", "text/plain; charset=utf-8")
                self.end_headers()
                with open(fpath, "r", encoding="utf-8") as f:
                    self.wfile.write(f.read().encode("utf-8"))
            else:
                self.send_response(404)
                self.end_headers()
                self.wfile.write(b"Not found")
        else:
            self.send_response(200)
            self.end_headers()
            self.wfile.write(b"TissDB LLM Server running.")


def start_http_server():
    handler = LLMHandler
    httpd = socketserver.TCPServer(("", PORT), handler)
    logging.info(f"HTTP Server listening on port {PORT}...")
    nexus_flow_hook("http_server_started", {"port": PORT})
    httpd.serve_forever()


# === MAIN ENTRY ===
if __name__ == "__main__":
    # Start TissDB in background
    tiss_proc = start_tissdb()
    if not tiss_proc:
        logging.critical("Could not start TissDB server. Aborting integration script.")
        sys.exit(1)

    # Start HTTP server in separate thread
    threading.Thread(target=start_http_server, daemon=True).start()

    # Run a training session
    train_llm("Hello TissDB, this is a test integration with nexus_flow.", "test1")

    logging.info("Ready. Access LLM output at http://localhost:8000/llm/test1")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        logging.info("Shutting down...")
        tiss_proc.terminate()
