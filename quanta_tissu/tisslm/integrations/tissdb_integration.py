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
from quanta_tissu.tisslm.integrations.nexus_flow_integration import get_graphs_by_ids

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

# Global variable to store retrieved graph data
NEXUS_GRAPH_DATA = []

# === NEXUS FLOW HOOK ===
def nexus_flow_hook(event: str, payload: dict):
    """
    Main integration point for Nexus Flow.
    This hook is called at various points in the application lifecycle.
    """
    global NEXUS_GRAPH_DATA
    logging.info(f"[nexus_flow_hook] Event='{event}', Payload={payload}")

    if event == "tissdb_started":
        # TissDB is running, now we can fetch the graph data.
        # For now, we'll use a hardcoded list of graph IDs we expect to exist.
        graph_ids_to_fetch = ["cbt_graph_1", "cbt_graph_2"]
        NEXUS_GRAPH_DATA = get_graphs_by_ids(graph_ids_to_fetch)
        if NEXUS_GRAPH_DATA:
            logging.info(f"Successfully loaded {len(NEXUS_GRAPH_DATA)} graph(s) from Nexus Flow.")
        else:
            logging.warning("Could not load any graph data from Nexus Flow.")


# === STEP 1: START TISSDB SERVER AND POPULATE DATA ===
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

    # Now that the server process is started, run the population script.
    # The population script has its own retry logic to wait for the server to be ready.
    from quanta_tissu.tisslm.migration_tools.populate_nexus_db import main as populate_nexus_db_main
    populate_nexus_db_main()


    # Check if the process terminated unexpectedly after population attempt
    if proc.poll() is not None:
        logging.error("TissDB server failed to start or terminated unexpectedly.")
        stdout, stderr = proc.communicate()
        logging.error(f"TissDB STDOUT: {stdout}")
        logging.error(f"TissDB STDERR: {stderr}")
        return None

    # With the database populated, we can now trigger the hook to load the data.
    nexus_flow_hook("tissdb_started", {"pid": proc.pid})
    return proc


# === STEP 2: GET COMPLETION FROM LLAMACPP SERVER ===
def train_llm(prompt: str, session_name: str = "session"):
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    output_file = os.path.join(OUTPUT_DIR, f"{session_name}.txt")

    # Format the graph data from the global variable into a string context
    graph_context = ""
    if NEXUS_GRAPH_DATA:
        graph_context += "--- Begin Nexus Flow Graph Context ---\n"
        for graph in NEXUS_GRAPH_DATA:
            graph_id = graph.get('graph_id', 'Unknown Graph')
            graph_context += f"Graph: {graph_id}\n"

            nodes_str = []
            if 'nodes' in graph:
                for node in graph['nodes']:
                    nodes_str.append(f"  - Node {node.get('id')}: {node.get('label')}")
            if nodes_str:
                graph_context += "Nodes:\n" + "\\n".join(nodes_str) + "\n"

            edges_str = []
            if 'edges' in graph:
                for edge in graph['edges']:
                    edges_str.append(f"  - Edge from {edge.get('from')} to {edge.get('to')}")
            if edges_str:
                graph_context += "Edges:\n" + "\\n".join(edges_str) + "\n"

        graph_context += "--- End Nexus Flow Graph Context ---\n\n"

    # Prepend the context to the original prompt
    final_prompt = f"{graph_context}User Prompt: {prompt}"


    logging.info(f"Getting completion for prompt: {final_prompt[:200]}...")
    nexus_flow_hook("llm_completion_started", {"session": session_name})

    try:
        payload = {
            "prompt": final_prompt,
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
    train_llm("Based on the provided context about Cognitive-Behavioral Therapy, what are some strategies for well-being?", "test_rag")

    logging.info("Ready. Access LLM output at http://localhost:8000/llm/test_rag")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        logging.info("Shutting down...")
        tiss_proc.terminate()
