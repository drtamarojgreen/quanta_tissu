import requests
import logging
import os
import time

# Setup basic logging
logging.basicConfig(level=logging.INFO, format='[%(asctime)s] [%(levelname)s] %(message)s')

# Configuration for TissDB
TISSDB_PORT = os.environ.get("TISSDB_PORT", "9876")
TISSDB_URL = f"http://localhost:{TISSDB_PORT}"
DB_NAME = "nexus_flow_db"
COLLECTION_NAME = "graphs"

# Graph data translated from graph_logic.cpp
GRAPHS_DATA = [
    {
        "graph_id": "cbt_graph_1",
        "nodes": [
            {"id": 1, "x": 10, "y": 5, "z": 0, "size": 5, "label": "Challenge negative thoughts"},
            {"id": 2, "x": 30, "y": 15, "z": 0, "size": 3, "label": "Cognitive-Behavioral Therapy"},
            {"id": 3, "x": 50, "y": 8, "z": 0, "size": 5, "label": "Practice self-compassion"},
            {"id": 4, "x": 25, "y": 2, "z": 0, "size": 1, "label": "Develop coping strategies"}
        ],
        "edges": [
            {"from": 1, "to": 2},
            {"from": 1, "to": 3},
            {"from": 2, "to": 3},
            {"from": 2, "to": 4}
        ]
    },
    {
        "graph_id": "cbt_graph_2",
        "nodes": [
            {"id": 1, "x": 5, "y": 3, "z": 0, "size": 5, "label": "Mindfulness and relaxation"},
            {"id": 2, "x": 20, "y": 10, "z": 0, "size": 3, "label": "Break harmful patterns"},
            {"id": 3, "x": 18, "y": 9, "z": 0, "size": 1, "label": "A holistic approach"},
            {"id": 4, "x": 40, "y": 5, "z": 0, "size": 5, "label": "Build resilience"},
            {"id": 5, "x": 60, "y": 18, "z": 0, "size": 3, "label": "Emotional regulation"},
            {"id": 6, "x": 70, "y": 2, "z": 0, "size": 1, "label": "Seek professional help"},
            {"id": 7, "x": 35, "y": 20, "z": 0, "size": 3, "label": "It's okay to not be okay"},
            {"id": 8, "x": 5, "y": 20, "z": 0, "size": 5, "label": "Your feelings are valid"}
        ],
        "edges": [
            {"from": 1, "to": 2},
            {"from": 1, "to": 8},
            {"from": 2, "to": 4},
            {"from": 3, "to": 4},
            {"from": 4, "to": 5},
            {"from": 5, "to": 7},
            {"from": 6, "to": 7},
            {"from": 7, "to": 8}
        ]
    }
]

def check_db_connection(retries=5, delay=2):
    """Checks if the TissDB server is reachable, with retries."""
    for i in range(retries):
        try:
            response = requests.get(f"{TISSDB_URL}/_health", timeout=2)
            if response.status_code == 200:
                logging.info("TissDB server is running.")
                return True
        except requests.exceptions.RequestException:
            logging.warning(f"TissDB server not yet available. Retrying in {delay}s... ({i+1}/{retries})")
            time.sleep(delay)
    logging.error(f"Could not connect to TissDB server at {TISSDB_URL} after {retries} retries.")
    return False

def setup_database():
    """Creates the database and collection in TissDB if they don't exist."""
    # Check for database existence and create if not present
    try:
        response = requests.get(f"{TISSDB_URL}/_databases", timeout=5)
        response.raise_for_status()
        databases = response.json()
        if DB_NAME not in databases:
            logging.info(f"Database '{DB_NAME}' not found. Creating...")
            response = requests.put(f"{TISSDB_URL}/{DB_NAME}")
            response.raise_for_status()
            logging.info(f"Database '{DB_NAME}' created successfully.")
        else:
            logging.info(f"Database '{DB_NAME}' already exists.")
    except requests.exceptions.RequestException as e:
        logging.error(f"Error managing database '{DB_NAME}': {e}")
        raise

    # Check for collection existence and create if not present
    try:
        response = requests.get(f"{TISSDB_URL}/{DB_NAME}/_collections", timeout=5)
        # A 404 here can mean the DB exists but is empty, which is fine.
        if response.status_code == 404:
             collections = []
        else:
            response.raise_for_status()
            collections = response.json()

        if COLLECTION_NAME not in collections:
            logging.info(f"Collection '{COLLECTION_NAME}' not found. Creating...")
            response = requests.put(f"{TISSDB_URL}/{DB_NAME}/{COLLECTION_NAME}")
            response.raise_for_status()
            logging.info(f"Collection '{COLLECTION_NAME}' created successfully.")
        else:
            logging.info(f"Collection '{COLLECTION_NAME}' already exists.")
    except requests.exceptions.RequestException as e:
        logging.error(f"Error managing collection '{COLLECTION_NAME}': {e}")
        raise

def populate_graphs():
    """Populates the 'graphs' collection with data using PUT for idempotency."""
    logging.info(f"Populating '{COLLECTION_NAME}' collection...")
    for graph in GRAPHS_DATA:
        doc_id = graph["graph_id"]
        try:
            url = f"{TISSDB_URL}/{DB_NAME}/{COLLECTION_NAME}/{doc_id}"
            # Using PUT to create or update the document is idempotent.
            response = requests.put(url, json=graph, timeout=5)
            response.raise_for_status()
            logging.info(f"Graph document '{doc_id}' created/updated successfully.")
        except requests.exceptions.RequestException as e:
            logging.error(f"Error creating/updating graph document '{doc_id}': {e}")

    logging.info("Graph population complete.")

def main():
    """Main function to run the population script."""
    if not check_db_connection():
        return

    try:
        setup_database()
        populate_graphs()
    except Exception as e:
        logging.error(f"An error occurred during the population process: {e}")

if __name__ == "__main__":
    main()
