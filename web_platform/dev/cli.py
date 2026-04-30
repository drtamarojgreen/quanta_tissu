#!/usr/bin/env python3
"""
QuantaTissu Framework CLI - Automated Irrigation & Pruning System
Interacts with the web platform API to execute framework steps.
"""

import argparse
import requests
import json
import sys
import logging
import os

# Configure the Greenhouse Log
LOG_FILE = "/var/log/framework.log"

def setup_logging():
    handlers = [logging.StreamHandler(sys.stdout)]
    try:
        # Try to use the file handler if possible (within container)
        if os.access(os.path.dirname(LOG_FILE), os.W_OK) or os.path.exists(LOG_FILE) and os.access(LOG_FILE, os.W_OK):
            handlers.append(logging.FileHandler(LOG_FILE))
    except Exception:
        pass

    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s [%(levelname)s] %(message)s',
        handlers=handlers
    )

setup_logging()

API_BASE = os.environ.get("API_BASE", "http://localhost:8000/api")

def log_api_call(method, endpoint, response):
    logging.info(f"{method} {endpoint} - Status: {response.status_code}")
    if response.status_code >= 400:
        logging.error(f"Error Body: {response.text}")

def db_status(args):
    """Checks the status of TissDB databases."""
    endpoint = f"{API_BASE}/db/databases"
    resp = requests.get(endpoint)
    log_api_call("GET", "/db/databases", resp)
    print(json.dumps(resp.json(), indent=2))

def db_query(args):
    """Queries a collection in TissDB."""
    endpoint = f"{API_BASE}/db/query"
    payload = {"collection": args.collection, "query": args.query}
    resp = requests.post(endpoint, json=payload)
    log_api_call("POST", "/db/query", resp)
    print(json.dumps(resp.json(), indent=2))

def model_generate(args):
    """Generates text using the model."""
    endpoint = f"{API_BASE}/model/generate"
    payload = {
        "prompt": args.prompt,
        "length": args.length,
        "temperature": args.temp,
        "use_rag": args.rag
    }
    resp = requests.post(endpoint, json=payload)
    log_api_call("POST", "/model/generate", resp)
    print(json.dumps(resp.json(), indent=2))

def tisslang_run(args):
    """Executes a TissLang script."""
    endpoint = f"{API_BASE}/tisslang/run"
    script = args.script
    if os.path.exists(script):
        with open(script, 'r') as f:
            script = f.read()

    payload = {"script": script}
    resp = requests.post(endpoint, json=payload)
    log_api_call("POST", "/tisslang/run", resp)
    print(json.dumps(resp.json(), indent=2))

def task_list(args):
    """Lists all background tasks."""
    endpoint = f"{API_BASE}/tasks"
    resp = requests.get(endpoint)
    log_api_call("GET", "/tasks", resp)
    print(json.dumps(resp.json(), indent=2))

def task_status(args):
    """Gets status and logs for a specific task."""
    endpoint = f"{API_BASE}/tasks/{args.id}"
    resp = requests.get(endpoint)
    log_api_call("GET", f"/tasks/{args.id}", resp)
    print(json.dumps(resp.json(), indent=2))

def training_start(args):
    """Starts a new training session."""
    endpoint = f"{API_BASE}/training/start"
    resp = requests.post(endpoint, json={})
    log_api_call("POST", "/training/start", resp)
    print(json.dumps(resp.json(), indent=2))

def training_status(args):
    """Checks the status of the training session."""
    endpoint = f"{API_BASE}/training/status"
    resp = requests.get(endpoint)
    log_api_call("GET", "/training/status", resp)
    print(json.dumps(resp.json(), indent=2))

def main():
    parser = argparse.ArgumentParser(description="QuantaTissu Dev CLI")
    subparsers = parser.add_subparsers(dest="command", help="Command to execute")

    # DB Commands
    db_parser = subparsers.add_parser("db", help="Database operations")
    db_sub = db_parser.add_subparsers(dest="subcommand")
    db_sub.add_parser("status", help="Show database status")
    query_parser = db_sub.add_parser("query", help="Query a collection")
    query_parser.add_argument("collection", help="Collection name")
    query_parser.add_argument("query", help="JSON query string")

    # Model Commands
    model_parser = subparsers.add_parser("model", help="Model operations")
    model_parser.add_argument("prompt", help="Prompt for generation")
    model_parser.add_argument("--length", type=int, default=50, help="Max length")
    model_parser.add_argument("--temp", type=float, default=0.7, help="Temperature")
    model_parser.add_argument("--rag", action="store_true", help="Enable RAG")
    model_parser.set_defaults(func=model_generate)

    # TissLang Commands
    tiss_parser = subparsers.add_parser("tisslang", help="TissLang orchestration")
    tiss_parser.add_argument("script", help="TissLang script or path to script file")
    tiss_parser.set_defaults(func=tisslang_run)

    # Task Commands
    task_parser = subparsers.add_parser("tasks", help="Task management")
    task_sub = task_parser.add_subparsers(dest="subcommand")
    task_sub.add_parser("list", help="List all tasks")
    status_parser = task_sub.add_parser("status", help="Get task status")
    status_parser.add_argument("id", help="Task ID")

    # Training Commands
    train_parser = subparsers.add_parser("training", help="Training control")
    train_sub = train_parser.add_subparsers(dest="subcommand")
    train_sub.add_parser("start", help="Start training")
    train_sub.add_parser("status", help="Get training status")

    args = parser.parse_args()

    if args.command == "db":
        if args.subcommand == "status":
            db_status(args)
        elif args.subcommand == "query":
            db_query(args)
    elif args.command == "tasks":
        if args.subcommand == "list":
            task_list(args)
        elif args.subcommand == "status":
            task_status(args)
    elif args.command == "training":
        if args.subcommand == "start":
            training_start(args)
        elif args.subcommand == "status":
            training_status(args)
    elif hasattr(args, 'func'):
        args.func(args)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
