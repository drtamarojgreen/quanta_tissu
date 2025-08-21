#!/usr/bin/env python3

"""
This script provides a suite of functions to automate the setup,
population, and testing of the TissDB and TissLM components.
"""

import argparse
import json
import os
import subprocess
import sys
import time
import requests

# Constants
TISSDB_PID_FILE = "tissdb.pid"
TISSDB_EXECUTABLE = "tissdb/tissdb"
TISSDB_URL = "http://localhost:8080"
GENERATE_TEXT_SCRIPT = "quanta_tissu/tisslm/generate_text.py"


def run_unit_tests():
    """Runs the Python unit tests."""
    print("Running unit tests...")
    try:
        subprocess.run([sys.executable, "run_tests.py"], check=True)
        print("Unit tests passed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Unit tests failed: {e}")
        sys.exit(1)
    except FileNotFoundError:
        print("Error: run_tests.py not found. Make sure you are in the root directory.")
        sys.exit(1)


def start_tissdb():
    """Starts the TissDB server in the background."""
    if not os.path.exists(TISSDB_EXECUTABLE):
        print(f"Error: TissDB executable not found at '{TISSDB_EXECUTABLE}'")
        print("Please compile TissDB first.")
        sys.exit(1)

    if not os.access(TISSDB_EXECUTABLE, os.X_OK):
        print(f"Error: TissDB executable at '{TISSDB_EXECUTABLE}' is not executable.")
        sys.exit(1)

    print(f"Starting TissDB server from '{TISSDB_EXECUTABLE}'...")
    # Start the server as a background process
    process = subprocess.Popen([f"./{TISSDB_EXECUTABLE}"], cwd="tissdb/")

    # Save the PID to a file
    with open(TISSDB_PID_FILE, "w") as f:
        f.write(str(process.pid))

    print(f"TissDB server started with PID: {process.pid}")

    # Wait for the server to initialize
    print("Waiting for server to initialize...")
    time.sleep(3)
    print("Server initialized.")


def stop_tissdb():
    """Stops the TissDB server."""
    if not os.path.exists(TISSDB_PID_FILE):
        print("TissDB server not running (no PID file found).")
        return

    with open(TISSDB_PID_FILE, "r") as f:
        pid = int(f.read())

    print(f"Stopping TissDB server with PID: {pid}...")
    try:
        os.kill(pid, 15)  # 15 is the signal for SIGTERM
        print("TissDB server stopped.")
    except ProcessLookupError:
        print(f"Process with PID {pid} not found. It may have already been stopped.")
    except Exception as e:
        print(f"An error occurred while trying to stop the server: {e}")

    # Clean up the PID file
    os.remove(TISSDB_PID_FILE)


def populate_database():
    """Creates a collection and populates it with generated text."""
    collection_name = "tiss_generations"
    collection_url = f"{TISSDB_URL}/{collection_name}"

    # 1. Create a new collection
    print(f"Creating collection '{collection_name}'...")
    try:
        response = requests.put(collection_url)
        response.raise_for_status()
        print("Collection created successfully.")
    except requests.exceptions.RequestException as e:
        print(f"Error creating collection: {e}")
        sys.exit(1)

    # 2. Generate text and populate the collection
    print("Generating and populating 5 documents...")
    for i in range(5):
        print(f"Generating document {i + 1}...")
        try:
            # Call the python script to generate text
            result = subprocess.run(
                [sys.executable, GENERATE_TEXT_SCRIPT, "--prompt", "hello", "--length", "8"],
                capture_output=True,
                text=True,
                check=True,
            )
            generated_text = result.stdout.strip()

            # POST the generated text to the collection
            payload = {"content": generated_text}
            post_response = requests.post(collection_url, json=payload)
            post_response.raise_for_status()
            print(f"  -> Successfully inserted document {i+1}.")

        except FileNotFoundError:
            print(f"Error: The script '{GENERATE_TEXT_SCRIPT}' was not found.")
            sys.exit(1)
        except subprocess.CalledProcessError as e:
            print(f"Error generating text: {e}")
            print(f"Stderr: {e.stderr}")
            sys.exit(1)
        except requests.exceptions.RequestException as e:
            print(f"Error inserting document: {e}")
            sys.exit(1)

    print("Population complete.")


def verify_data():
    """Verifies that the data was inserted correctly."""
    collection_name = "tiss_generations"
    query_url = f"{TISSDB_URL}/{collection_name}/_query"
    query = {"query": "SELECT * FROM tiss_generations"}

    print("Verifying inserted data...")
    try:
        response = requests.post(query_url, json=query)
        response.raise_for_status()
        documents = response.json()

        print(f"Query successful. Response:\n{json.dumps(documents, indent=2)}")

        if len(documents) == 5:
            print("\nVerification successful: 5 documents found.")
        else:
            print(f"\nVerification failed: Expected 5 documents, but found {len(documents)}.")
            sys.exit(1)

    except requests.exceptions.RequestException as e:
        print(f"Error querying database: {e}")
        sys.exit(1)
    except json.JSONDecodeError:
        print("Error: Failed to decode JSON response from the server.")
        sys.exit(1)


def main():
    """Main entry point for the script."""
    parser = argparse.ArgumentParser(description="CI/CD pipeline for QuantaTissu.")
    parser.add_argument(
        "--steps",
        nargs="+",
        default=["all"],
        help="Which pipeline steps to run. "
        "Options: all, test, start_db, stop_db, populate, verify. "
        "Default: all",
    )
    args = parser.parse_args()

    # Change to the root of the repository
    script_dir = os.path.dirname(os.path.realpath(__file__))
    root_dir = os.path.abspath(os.path.join(script_dir, '..', '..'))
    os.chdir(root_dir)
    print(f"Working directory set to: {os.getcwd()}")


    print("Starting QuantaTissu pipeline...")

    if "all" in args.steps or "test" in args.steps:
        print("\n--- Step: Unit Testing ---")
        run_unit_tests()

    if "all" in args.steps or "start_db" in args.steps:
        print("--- Step: Starting TissDB server ---")
        start_tissdb()

    if "all" in args.steps or "populate" in args.steps:
        print("\n--- Step: Populating database ---")
        populate_database()

    if "all" in args.steps or "verify" in args.steps:
        print("\n--- Step: Verifying data ---")
        verify_data()

    if "all" in args.steps or "stop_db" in args.steps:
        print("\n--- Step: Stopping TissDB server ---")
        stop_tissdb()

    print("\nQuantaTissu pipeline finished successfully!")


if __name__ == "__main__":
    main()
