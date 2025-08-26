import requests
import logging

from ..system_error_handler import DatabaseConnectionError

logger = logging.getLogger(__name__)

class TissDBClient:
    """
    A client for interacting with the TissDB HTTP API.
    This class encapsulates all direct network requests to the database.
    """
    def __init__(self, db_host='127.0.0.1', db_port=8080, db_name='testdb'):
        self.base_url = f"http://{db_host}:{db_port}"
        self.db_name = db_name
        self.db_url = f"{self.base_url}/{self.db_name}"

    def ensure_db_setup(self, collections: list):
        """
        Ensures the database and the specified collections exist.

        Args:
            collections (list): A list of collection names to ensure exist.

        Returns:
            bool: True if connection and setup were successful, False otherwise.
        """
        try:
            # Ensure database exists
            response = requests.put(self.db_url)
            if response.status_code not in [200, 201, 409]: # OK, Created, Conflict (already exists)
                response.raise_for_status()

            # Ensure collections exist
            for collection_name in collections:
                coll_response = requests.put(f"{self.db_url}/{collection_name}")
                if coll_response.status_code not in [200, 201, 409]:
                    coll_response.raise_for_status()

            logger.info(f"TissDB connection successful to {self.db_url}")
            return True
        except requests.exceptions.RequestException as e:
            logger.warning(f"TissDB setup failed: {e}. Client will be in a disconnected state.")
            raise DatabaseConnectionError(f"Database setup failed: {e}") from e

    def add_document(self, collection: str, document: dict):
        """
        Adds a document to a specified collection.
        """
        try:
            response = requests.post(f"{self.db_url}/{collection}", json=document)
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            raise DatabaseConnectionError(f"Failed to add document to {collection}: {e}") from e

    def get_all_documents(self, collection: str):
        """
        Retrieves all documents from a collection using a simple SELECT query.
        NOTE: This is inefficient and not scalable. A real implementation would
        use the database's own vector search capabilities.
        """
        try:
            query = {"query": f"SELECT id, text, embedding FROM {collection}"}
            response = requests.post(f"{self.db_url}/{collection}/_query", json=query)
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            raise DatabaseConnectionError(f"Failed to retrieve documents from {collection}: {e}") from e

    def get_stats(self):
        """
        Retrieves statistics for the database.
        """
        try:
            response = requests.get(f"{self.db_url}/_stats")
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            raise DatabaseConnectionError(f"Failed to get DB stats: {e}") from e

    def add_feedback(self, feedback_data: dict):
        """
        Adds feedback data to the feedback collection.
        """
        # The original KB had a hardcoded "_feedback" endpoint, which seems wrong.
        # A more RESTful approach would be a 'feedback' collection.
        return self.add_document('feedback', feedback_data)
