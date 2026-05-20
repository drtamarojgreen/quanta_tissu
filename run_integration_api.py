#!/usr/bin/env python3
import urllib.request
import urllib.parse
import json
import time
import sys

SERVER_URL = "http://127.0.0.1:8000"

def make_request(path, method="GET", data=None):
    url = f"{SERVER_URL}{path}"
    req_data = json.dumps(data).encode("utf-8") if data else None
    headers = {"Content-Type": "application/json"} if data else {}
    req = urllib.request.Request(url, data=req_data, headers=headers, method=method)
    
    try:
        with urllib.request.urlopen(req) as res:
            return res.status, json.loads(res.read().decode("utf-8"))
    except urllib.error.HTTPError as e:
        try:
            err_body = json.loads(e.read().decode("utf-8"))
            return e.code, err_body
        except Exception:
            return e.code, {"error": e.reason}
    except Exception as e:
        return 500, {"error": str(e)}

def poll_process(task_id, description, interval=1.0, timeout=120):
    print(f"Polling {description} (Task ID: {task_id})...")
    start_time = time.time()
    while time.time() - start_time < timeout:
        status, res = make_request(f"/api/processes/{task_id}")
        if status != 200:
            print(f"Error fetching status for {task_id}: {res}")
            return False
        
        state = res.get("state")
        print(f"  Current state: {state}")
        if state in ("COMPLETED", "FAILED", "STOPPED"):
            if state == "COMPLETED":
                print(f"  {description} finished successfully.")
                return True
            else:
                print(f"  {description} finished with state: {state}")
                # Print last logs
                print("  Last logs:")
                for line in res.get("logs", [])[-10:]:
                    print(f"    [LOG] {line}")
                return False
        time.sleep(interval)
    print(f"Timeout waiting for {description}")
    return False

def main():
    print("=========================================================")
    print("   QuantaTissu End-to-End API Integration Runner   ")
    print("=========================================================\n")

    # 1. Build and compile TissDB
    print("[1/8] Compiling TissDB via API...")
    status, res = make_request("/api/db/build", "POST")
    if status != 200 or not res.get("success"):
        print(f"Failed to build TissDB: {res}")
        sys.exit(1)
    print("TissDB built successfully.\n")

    # 2. Start TissDB
    print("[2/8] Starting TissDB server via API...")
    status, res = make_request("/api/db/start", "POST")
    if status != 200 or not res.get("success"):
        print(f"Failed to start TissDB: {res}")
        sys.exit(1)
    print(f"TissDB started (PID: {res.get('pid')}).\n")
    time.sleep(2) # Allow database to initialize

    # 3. Build Analyzer
    print("[3/8] Compiling Analyzer binary via API...")
    status, res = make_request("/api/analyzer/build", "POST")
    if status != 200 or not res.get("success"):
        print(f"Failed to build analyzer: {res}")
        sys.exit(1)
    print("Analyzer binary built successfully.\n")

    # 4. Start Analyzer
    session_id = 42
    print(f"[4/8] Starting Analyzer (Session: {session_id}) via API...")
    status, res = make_request("/api/analyzer/start", "POST", {"session_id": session_id})
    if status != 200 or not res.get("success"):
        print(f"Failed to start analyzer: {res}")
        sys.exit(1)
    print("Analyzer process started.\n")

    # 5. Compile C++ Training Model
    print("[5/8] Compiling C++ Model Training binary via API...")
    compile_cmd = (
        "g++ -std=c++17 -o train_model_exe "
        "tests/model/program/train_model.cpp "
        "quanta_tissu/tisslm/program/core/matrix.cpp "
        "quanta_tissu/tisslm/program/core/parameter.cpp "
        "quanta_tissu/tisslm/program/core/layernorm.cpp "
        "quanta_tissu/tisslm/program/core/dropout.cpp "
        "quanta_tissu/tisslm/program/core/feedforward.cpp "
        "quanta_tissu/tisslm/program/core/multiheadattention.cpp "
        "quanta_tissu/tisslm/program/core/transformerblock.cpp "
        "quanta_tissu/tisslm/program/core/positionalencoding.cpp "
        "quanta_tissu/tisslm/program/core/embedding.cpp "
        "quanta_tissu/tisslm/program/core/transformer_model.cpp "
        "quanta_tissu/tisslm/program/generation/generator.cpp "
        "quanta_tissu/tisslm/program/tokenizer/tokenizer.cpp "
        "quanta_tissu/tisslm/program/tokenizer/pre_tokenizer.cpp "
        "quanta_tissu/tisslm/program/training/optimizer.cpp "
        "quanta_tissu/tisslm/program/training/loss_function.cpp "
        "quanta_tissu/tisslm/program/training/trainer.cpp "
        "quanta_tissu/tisslm/program/retrieval/retrieval_strategy.cpp "
        "tissdb/json/json.cpp "
        "-Itests/model/program -Iquanta_tissu/tisslm/program -Iquanta_tissu/tisslm/program/core "
        "-Iquanta_tissu/tisslm/program/generation -Iquanta_tissu/tisslm/program/tokenizer "
        "-Iquanta_tissu/tisslm/program/training -I. -lpthread -lrt"
    )
    status, res = make_request("/api/processes", "POST", {
        "task_id": "compile_trainer",
        "type": "shell",
        "command": compile_cmd
    })
    if status != 200:
        print(f"Failed to dispatch compilation command: {res}")
        sys.exit(1)
    
    if not poll_process("compile_trainer", "C++ Trainer Compilation"):
        print("Compilation of the C++ training model failed.")
        sys.exit(1)
    print("")

    # 6. Run C++ Training process
    print("[6/8] Running C++ Training process via API...")
    status, res = make_request("/api/processes", "POST", {
        "task_id": "trainer_process",
        "type": "shell",
        "command": f"./train_model_exe {session_id}"
    })
    if status != 200:
        print(f"Failed to dispatch training command: {res}")
        sys.exit(1)
        
    if not poll_process("trainer_process", "C++ Trainer Process", interval=2.0, timeout=180):
        print("C++ model training process failed.")
        sys.exit(1)
    print("")

    # 7. Print analyzer results
    print("[7/8] Retrieving Analyzer results from API...")
    time.sleep(1.0) # Wait for analyzer buffer to flush
    status, res = make_request("/api/analyzer/logs")
    if status == 200:
        print("\n================== ANALYZER RESULTS ==================")
        logs = res.get("logs", [])
        if logs:
            for log in logs:
                print(log)
        else:
            print("[No logs recorded by the analyzer]")
        print("======================================================\n")
    else:
        print(f"Failed to retrieve analyzer logs: {res}")

    # 8. Clean up
    print("[8/8] Cleaning up running servers...")
    # Stop Analyzer
    make_request("/api/analyzer/stop", "POST")
    # Stop TissDB
    make_request("/api/db/stop", "POST")
    
    # Remove temporary executables
    if os.path.exists("train_model_exe"):
        os.remove("train_model_exe")
    print("Cleanup complete. Integration run finished successfully.")

if __name__ == "__main__":
    main()
