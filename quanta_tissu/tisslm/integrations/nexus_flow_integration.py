import requests
import logging
import os

# Setup basic logging, if not already configured
logging.basicConfig(level=logging.INFO, format='[%(asctime)s] [%(levelname)s] %(message)s')

# Configuration for TissDB, consistent with other integration scripts
TISSDB_PORT = os.environ.get("TISSDB_PORT", "9876")
TISSDB_URL = f"http://localhost:{TISSDB_PORT}"
DB_NAME = "nexus_flow_db"
COLLECTION_NAME = "graphs"

def get_graphs_by_ids(graph_ids: list[str]) -> list[dict]:
    """
    Retrieves a list of graph documents from TissDB by their specific IDs.

    Args:
        graph_ids: A list of string IDs for the graph documents to retrieve.

    Returns:
        A list of dictionaries, where each dictionary is a parsed JSON graph document.
        Returns an empty list if no graphs are found or an error occurs.
    """
    graphs = []
    if not graph_ids:
        logging.warning("get_graphs_by_ids called with an empty list of IDs.")
        return graphs

    logging.info(f"Attempting to retrieve graphs by IDs: {graph_ids}")
    for doc_id in graph_ids:
        try:
            url = f"{TISSDB_URL}/{DB_NAME}/{COLLECTION_NAME}/{doc_id}"
            response = requests.get(url, timeout=5)

            if response.status_code == 200:
                graphs.append(response.json())
                logging.info(f"Successfully retrieved graph document '{doc_id}'.")
            else:
                # Log a warning for non-200 responses (e.g., 404 Not Found)
                logging.warning(f"Failed to retrieve graph document '{doc_id}'. "
                                f"Status: {response.status_code}, Body: {response.text}")

        except requests.exceptions.RequestException as e:
            # Log an error for connection issues
            logging.error(f"An error occurred while fetching graph document '{doc_id}': {e}")
            # Depending on desired behavior, you might want to stop or continue.
            # Here, we continue to try fetching the other graphs.

    return graphs
