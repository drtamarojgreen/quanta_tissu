#!/usr/bin/env python3
import subprocess
import time
import os
import http.server
import socketserver
import threading

# === CONFIG ===
TISSDB_BIN = "./tissdb"       # path to compiled TissDB server
LLM_BIN = "./llama"           # path to llama.cpp binary
MODEL = "./models/7B/ggml-model-q4_0.bin"
OUTPUT_DIR = "./llm_outputs"
PORT = 8000

# === NEXUS FLOW HOOK ===
def nexus_flow_hook(event: str, payload: dict):
    """
    Minimal integration: nexus_flow could be imported or called
    as a function. For now, simulate with logging.
    """
    print(f"[nexus_flow] Event={event}, Payload={payload}")


# === STEP 1: START TISSDB SERVER ===
def start_tissdb():
    print("[server] Starting TissDB...")
    proc = subprocess.Popen([TISSDB_BIN], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    time.sleep(2)  # allow warmup
    nexus_flow_hook("tissdb_started", {"pid": proc.pid})
    return proc


# === STEP 2: TRAIN / RUN LLM ===
def train_llm(prompt: str, session_name: str = "session"):
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    output_file = os.path.join(OUTPUT_DIR, f"{session_name}.txt")

    print(f"[llm] Running training with prompt: {prompt[:50]}...")
    nexus_flow_hook("llm_training_started", {"session": session_name})

    # For simplicity: llama.cpp --prompt "..." > file
    with open(output_file, "w", encoding="utf-8") as f:
        subprocess.run([LLM_BIN,
                        "-m", MODEL,
                        "--prompt", prompt,
                        "--n-predict", "256"], stdout=f)

    nexus_flow_hook("llm_training_complete", {"session": session_name, "file": output_file})
    print(f"[llm] Output saved at {output_file}")
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
    print(f"[http] Server listening on port {PORT}...")
    nexus_flow_hook("http_server_started", {"port": PORT})
    httpd.serve_forever()


# === MAIN ENTRY ===
if __name__ == "__main__":
    # Start TissDB in background
    tiss_proc = start_tissdb()

    # Start HTTP server in separate thread
    threading.Thread(target=start_http_server, daemon=True).start()

    # Run a training session
    train_llm("Hello TissDB, this is a test integration with nexus_flow.", "test1")

    print("[system] Ready. Access LLM output at http://localhost:8000/llm/test1")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("[system] Shutting down...")
        tiss_proc.terminate()
