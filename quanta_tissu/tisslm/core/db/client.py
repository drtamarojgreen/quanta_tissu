import requests
import logging

from ..system_error_handler import DatabaseConnectionError

logger = logging.getLogger(__name__)

class TissDBClient:
    """
    A client for interacting with the TissDB HTTP API.
    This class encapsulates all direct network requests to the database.
    """
    def __init__(self, db_host='127.0.0.1', db_port=8080, db_name='testdb', token=None):
        self.base_url = f"http://{db_host}:{db_port}"
        self.db_name = db_name
        self.db_url = f"{self.base_url}/{self.db_name}"
        self.token = token

    def _get_headers(self):
        """Returns headers for authentication."""
        if self.token:
            return {'Authorization': f'Bearer {self.token}'}
        return {}

    def ensure_db_setup(self, collections: list):
        """
        Ensures the database and the specified collections exist.

        Args:
            collections (list): A list of collection names to ensure exist.

        Returns:
            bool: True if connection and setup were successful, False otherwise.
        """
        try:
            headers = self._get_headers()
            # Ensure database exists
            response = requests.put(self.db_url, headers=headers)
            # Accept 200 (OK), 201 (Created), 409 (Conflict), or 500 if it's due to "already exists"
            if response.status_code not in [200, 201, 409]:
                # Check if 500 is due to "already exists"
                if response.status_code == 500 and "already exists" in response.text:
                    logger.warning(f"TissDB returned 500 for database creation, but it seems to be due to 'already exists'. Proceeding.")
                else:
                    response.raise_for_status()

            # Ensure collections exist
            for collection_name in collections:
                coll_response = requests.put(f"{self.db_url}/{collection_name}", headers=headers)
                if coll_response.status_code not in [200, 201, 409]:
                    coll_response.raise_for_status()

            logger.info(f"TissDB connection successful to {self.db_url}")
            return True
        except requests.exceptions.HTTPError as e:
            if e.response.status_code == 500:
                logger.warning(f"TissDB setup failed with 500 Internal Server Error: {e}. This might indicate an issue with the TissDB server itself. Client will be in a disconnected state.")
            else:
                logger.warning(f"TissDB setup failed: {e}. Client will be in a disconnected state.")
            raise DatabaseConnectionError(f"Database setup failed: {e}") from e
        except requests.exceptions.RequestException as e:
            logger.warning(f"TissDB setup failed: {e}. Client will be in a disconnected state.")
            raise DatabaseConnectionError(f"Database setup failed: {e}") from e

    def add_document(self, collection: str, document: dict):
        """
        Adds a document to a specified collection.
        """
        try:
            headers = self._get_headers()
            response = requests.post(f"{self.db_url}/{collection}", json=document, headers=headers)
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            raise DatabaseConnectionError(f"Failed to add document to {collection}: {e}") from e

    def get_all_documents(self, collection: str):
        """
        Retrieves all documents from a collection.
        NOTE: The current TissDB HTTP API does not support a direct "get all documents"
        or generic query endpoint. This method will log a warning and return an empty list.
        A proper implementation would require extending the TissDB C++ backend
        to expose such functionality (e.g., a scan endpoint or a query language processor).
        """
        logger.warning(f"TissDB HTTP API does not support 'get_all_documents' for collection '{collection}'. Returning empty list.")
        # To implement this, TissDB C++ backend needs to expose an endpoint
        # that can scan a collection and return all documents or their IDs.
        # For now, we return an empty list to prevent a 404 error.
        return []

    def get_stats(self):
        """
        Retrieves statistics for the database.
        """
        try:
            headers = self._get_headers()
            response = requests.get(f"{self.db_url}/_stats", headers=headers)
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
