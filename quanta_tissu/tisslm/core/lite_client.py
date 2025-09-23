import json
import logging
import os
import subprocess

script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..', '..')) # Project root is 4 levels up from here

from quanta_tissu.tisslm.core.system_error_handler import DatabaseConnectionError

logger = logging.getLogger(__name__)

class TissDBLiteClient:
    """
    A client for interacting with the TissDBLite (JavaScript in-memory DB) via Node.js.
    """
    def __init__(self):
        self.node_script_path = os.path.join(script_dir, "tissdblite_bridge.js") # tissdblite_bridge.js is now in the same directory
        self._ensure_bridge_script_exists()

class TissDBLiteClient:
    """
    A client for interacting with the TissDBLite (JavaScript in-memory DB) via Node.js.
    """
    def __init__(self):
        self.node_script_path = os.path.join(script_dir, "tissdblite_bridge.js") # tissdblite_bridge.js is now in the same directory
        self._ensure_bridge_script_exists()

    def _ensure_bridge_script_exists(self):
        # This is a placeholder. In a real scenario, you'd generate or ensure this script is present.
        # For now, we'll create a basic one.
        if not os.path.exists(self.node_script_path):
            bridge_code = """
const { TissDBLite } = require('../../../../lite/index.js'); // Adjust path as needed
const db = new TissDBLite();

process.stdin.on('data', (data) => {
    try {
        const command = JSON.parse(data.toString());
        let result;
        switch (command.action) {
            case 'createCollection':
                db.createCollection(command.collectionName);
                result = { status: 'success', message: 'Collection created.' };
                break;
            case 'insert':
                result = { status: 'success', data: db.insert(command.collectionName, command.item) };
                break;
            case 'find':
                result = { status: 'success', data: db.find(command.collectionName, command.condition_string) };
                break;
            case 'update':
                result = { status: 'success', data: db.update(command.collectionName, command.item) };
                break;
            case 'remove':
                db.remove(command.collectionName, command.itemId);
                result = { status: 'success', message: 'Item removed.' };
                break;
            case 'bulkInsert':
                result = { status: 'success', data: db.bulkInsert(command.collectionName, command.items) };
                break;
            case 'exportCollection':
                result = { status: 'success', data: db.exportCollection(command.collectionName) };
                break;
            case 'deleteDb': // For cleanup, though TissDBLite is in-memory
                db.collections = {};
                result = { status: 'success', message: 'In-memory DB cleared.' };
                break;
            default:
                throw new Error(`Unknown action: ${command.action}`);
        }
        process.stdout.write(JSON.stringify(result) + '\n');
    } catch (e) {
        process.stderr.write(JSON.stringify({ status: 'error', message: e.message }) + '\n');
    }
});

process.stdin.on('end', () => {
    // Optional: perform cleanup or final logging
});
"""
            with open(self.node_script_path, "w") as f:
                f.write(bridge_code)

    def ensure_db_setup(self, collections: list):
        # TissDBLite is in-memory, so setup means ensuring collections can be created.
        # We'll just create them here for simplicity.
        for col_name in collections:
            self._execute_node_command({'action': 'createCollection', 'collectionName': col_name})
        return True

    def add_document(self, collection: str, document: dict, doc_id: str = None):
        # TissDBLite generates _id, so we don't pass doc_id directly to insert
        item_to_insert = {**document}
        if doc_id: # If a specific ID is requested, we can try to set it, but TissDBLite will override
            item_to_insert['_id'] = doc_id
        return self._execute_node_command({'action': 'insert', 'collectionName': collection, 'item': item_to_insert})

    def find(self, collection: str, condition_string: str = None):
        return self._execute_node_command({'action': 'find', 'collectionName': collection, 'condition_string': condition_string})

    def update(self, collection: str, item: dict):
        return self._execute_node_command({'action': 'update', 'collectionName': collection, 'item': item})

    def remove(self, collection: str, item_id: str):
        return self._execute_node_command({'action': 'remove', 'collectionName': collection, 'itemId': item_id})

    def export_collection(self, collection: str):
        return self._execute_node_command({'action': 'exportCollection', 'collectionName': collection})

    def delete_db(self):
        # For in-memory, this means clearing all collections
        return self._execute_node_command({'action': 'deleteDb'})

    def _execute_node_command(self, command_obj):
        try:
            json_command = json.dumps(command_obj)
            
            process = subprocess.run(
                ["node", self.node_script_path],
                input=json_command,
                capture_output=True,
                text=True, # Decode stdout/stderr as text
                check=False, # Don't raise an exception for non-zero exit codes
                cwd=project_root # Run from project root
            )

            if process.returncode != 0:
                raise DatabaseConnectionError(f"Node.js command failed with exit code {process.returncode}. Stderr: {process.stderr}")
            
            if process.stdout:
                try:
                    response = json.loads(process.stdout.strip())
                    if response.get('status') == 'error':
                        raise DatabaseConnectionError(f"TissDBLite error: {response.get('message')}")
                    return response.get('data', True) # Return data or True for success
                except json.JSONDecodeError:
                    raise DatabaseConnectionError(f"Invalid JSON response from Node.js: {process.stdout}")
            else:
                raise DatabaseConnectionError("Empty response from Node.js.")

        except FileNotFoundError:
            raise DatabaseConnectionError("Node.js executable not found. Please ensure Node.js is installed and in your PATH.")
        except Exception as e:
            logger.error(f"Error executing Node.js command: {e}", exc_info=True)
            raise DatabaseConnectionError(f"Error communicating with TissDBLite: {e}") from e