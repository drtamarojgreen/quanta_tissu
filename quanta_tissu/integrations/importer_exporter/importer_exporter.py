import requests
import json
import csv
import xml.etree.ElementTree as ET
import argparse

# ==============================================================================
# TissDB API Client
# ==============================================================================

class TissDBAPIClient:
    """
    A client for the TissDB HTTP API.
    """
    def __init__(self, base_url="http://localhost:8080"):
        """
        Initializes the client with the base URL of the TissDB server.
        """
        self.base_url = base_url.rstrip('/')

    def get_all_documents(self, collection_name: str) -> list[dict]:
        """
        Retrieves all documents from a specified collection.
        """
        url = f"{self.base_url}/{collection_name}/_query"
        payload = {"query": "SELECT *"}
        response = requests.post(url, json=payload)
        response.raise_for_status()
        return response.json()

    def insert_document(self, collection_name: str, document: dict) -> dict:
        """
        Inserts a single document into a specified collection.
        """
        url = f"{self.base_url}/{collection_name}"
        response = requests.post(url, json=document)
        response.raise_for_status()
        return response.json()

# ==============================================================================
# Helper Functions
# ==============================================================================

def _convert_value(value: str):
    """
    Tries to convert a string value to a more specific type (int, float, bool).
    """
    if not value:
        return None

    # Try converting to int
    try:
        return int(value)
    except ValueError:
        pass

    # Try converting to float
    try:
        return float(value)
    except ValueError:
        pass

    # Check for boolean
    if value.lower() == 'true':
        return True
    if value.lower() == 'false':
        return False

    return value

# ==============================================================================
# Core Import/Export Logic
# ==============================================================================

def export_to_csv(collection_name: str, file_path: str, base_url: str):
    """
    Exports all documents from a TissDB collection to a CSV file.
    """
    print(f"Starting export of collection '{collection_name}' to '{file_path}'...")
    try:
        client = TissDBAPIClient(base_url)
        documents = client.get_all_documents(collection_name)

        if not documents:
            print("No documents found in the collection. Nothing to export.")
            return

        header = set()
        for doc in documents:
            header.update(doc.keys())

        sorted_header = sorted(list(header))

        with open(file_path, 'w', newline='', encoding='utf-8') as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=sorted_header)
            writer.writeheader()
            writer.writerows(documents)

        print(f"Successfully exported {len(documents)} documents to '{file_path}'.")

    except requests.exceptions.RequestException as e:
        print(f"Error connecting to TissDB: {e}")
    except IOError as e:
        print(f"Error writing to file '{file_path}': {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

def import_from_csv(collection_name: str, file_path: str, base_url: str):
    """
    Imports documents from a CSV file into a TissDB collection.
    """
    print(f"Starting import from '{file_path}' to collection '{collection_name}'...")
    try:
        client = TissDBAPIClient(base_url)
        count = 0
        with open(file_path, 'r', encoding='utf-8') as csvfile:
            reader = csv.DictReader(csvfile)
            for row in reader:
                document = {key: _convert_value(value) for key, value in row.items()}
                client.insert_document(collection_name, document)
                count += 1
                if count % 100 == 0:
                    print(f"Imported {count} documents...")

        print(f"Successfully imported {count} documents from '{file_path}'.")

    except FileNotFoundError:
        print(f"Error: The file '{file_path}' was not found.")
    except requests.exceptions.RequestException as e:
        print(f"Error connecting to TissDB: {e}")
    except IOError as e:
        print(f"Error reading file '{file_path}': {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

def export_to_xml(collection_name: str, file_path: str, base_url: str):
    """
    Exports all documents from a TissDB collection to an XML file.
    """
    print(f"Starting export of collection '{collection_name}' to '{file_path}'...")
    try:
        client = TissDBAPIClient(base_url)
        documents = client.get_all_documents(collection_name)

        if not documents:
            print("No documents found in the collection. Nothing to export.")
            return

        root = ET.Element("data")
        for doc in documents:
            item = ET.SubElement(root, "item")
            for key, value in doc.items():
                child = ET.SubElement(item, str(key))
                child.text = str(value)

        ET.indent(root, space="\t")

        tree = ET.ElementTree(root)
        tree.write(file_path, encoding='utf-8', xml_declaration=True)

        print(f"Successfully exported {len(documents)} documents to '{file_path}'.")

    except requests.exceptions.RequestException as e:
        print(f"Error connecting to TissDB: {e}")
    except IOError as e:
        print(f"Error writing to file '{file_path}': {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

def import_from_xml(collection_name: str, file_path: str, base_url: str):
    """
    Imports documents from an XML file into a TissDB collection.
    """
    print(f"Starting import from '{file_path}' to collection '{collection_name}'...")
    try:
        client = TissDBAPIClient(base_url)
        count = 0

        tree = ET.parse(file_path)
        root = tree.getroot()

        for item in root.findall('item'):
            document = {}
            for child in item:
                document[child.tag] = _convert_value(child.text)

            client.insert_document(collection_name, document)
            count += 1
            if count % 100 == 0:
                print(f"Imported {count} documents...")

        print(f"Successfully imported {count} documents from '{file_path}'.")

    except FileNotFoundError:
        print(f"Error: The file '{file_path}' was not found.")
    except ET.ParseError as e:
        print(f"Error parsing XML file '{file_path}': {e}")
    except requests.exceptions.RequestException as e:
        print(f"Error connecting to TissDB: {e}")
    except IOError as e:
        print(f"Error reading file '{file_path}': {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

# ==============================================================================
# Command-Line Interface
# ==============================================================================

def main():
    """
    Main function to run the command-line interface.
    """
    parser = argparse.ArgumentParser(description="TissDB Importer/Exporter Tool")

    parser.add_argument("action", choices=['import', 'export'], help="The action to perform: 'import' or 'export'.")
    parser.add_argument("format", choices=['csv', 'xml'], help="The data format to use: 'csv' or 'xml'.")
    parser.add_argument("collection", help="The name of the TissDB collection.")
    parser.add_argument("filepath", help="The path to the input/output file.")
    parser.add_argument("--base-url", default="http://localhost:8080", help="The base URL of the TissDB server.")

    args = parser.parse_args()

    if args.action == 'import':
        if args.format == 'csv':
            import_from_csv(args.collection, args.filepath, args.base_url)
        elif args.format == 'xml':
            import_from_xml(args.collection, args.filepath, args.base_url)
    elif args.action == 'export':
        if args.format == 'csv':
            export_to_csv(args.collection, args.filepath, args.base_url)
        elif args.format == 'xml':
            export_to_xml(args.collection, args.filepath, args.base_url)

if __name__ == "__main__":
    main()
