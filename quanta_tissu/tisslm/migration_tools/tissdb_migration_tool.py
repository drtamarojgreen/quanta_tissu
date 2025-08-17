import argparse
import json
import os
import sys
import xml.etree.ElementTree as ET
import xml.dom.minidom
import requests

# This will be updated by the --url argument in main()
TISSDB_API_URL = ""

# --- Data Conversion Functions ---

def convert_xml_to_docs(root):
    """Converts an XML tree root into a list of document dictionaries."""
    documents = []
    for item in root:
        doc = {}
        if item.attrib:
            doc.update(item.attrib)
        for field in item:
            doc[field.tag] = field.text
        documents.append(doc)
    return documents

def convert_docs_to_xml_root(documents):
    """Converts a list of document dictionaries into an XML tree root."""
    root = ET.Element("data")
    for doc in documents:
        record = ET.SubElement(root, "record")
        for key, val in doc.items():
            field = ET.SubElement(record, key)
            field.text = str(val)
    return root

# --- Core API Functions ---

def import_documents(collection, documents):
    """Imports a list of documents into the specified collection."""
    if not documents:
        print("Warning: No documents to import.")
        return
    url = f"{TISSDB_API_URL}/{collection}"
    success_count = 0
    print(f"Found {len(documents)} documents to import to collection '{collection}'.")
    for i, doc in enumerate(documents):
        try:
            response = requests.post(url, json=doc)
            response.raise_for_status()
            success_count += 1
            print(f"  ({i+1}/{len(documents)}) Successfully imported document.")
        except requests.exceptions.RequestException as e:
            print(f"  ({i+1}/{len(documents)}) Error importing document: {e}")
    print(f"\nImport complete. Successfully imported {success_count} out of {len(documents)} documents.")

def get_documents_from_collection(collection):
    """Queries a collection and returns a list of documents."""
    print(f"Querying documents from collection '{collection}'...")
    url = f"{TISSDB_API_URL}/{collection}/_query"
    query = {"query": "SELECT *"}
    try:
        response = requests.post(url, json=query)
        response.raise_for_status()
        documents = response.json()
    except requests.exceptions.RequestException as e:
        print(f"Error querying collection '{collection}': {e}")
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"Error: Could not decode JSON response from server. Response: {response.text}")
        sys.exit(1)
    if not isinstance(documents, list):
        print(f"Error: Expected a JSON array from the server, but got something else.")
        sys.exit(1)
    print(f"Successfully fetched {len(documents)} documents.")
    return documents

# --- Command Handlers ---

def handle_import_json(args):
    """Handles the JSON import command."""
    print(f"Starting JSON import from '{args.filepath}'...")
    try:
        with open(args.filepath, 'r') as f:
            data = json.load(f)
    except FileNotFoundError:
        print(f"Error: File not found at '{args.filepath}'")
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"Error: Invalid JSON in file '{args.filepath}'")
        sys.exit(1)
    if isinstance(data, dict):
        documents = [data]
    elif isinstance(data, list):
        documents = data
    else:
        print("Error: JSON file must contain a single object or an array of objects.")
        sys.exit(1)
    import_documents(args.collection, documents)

def handle_export_json(args):
    """Handles the JSON export command."""
    documents = get_documents_from_collection(args.collection)
    print(f"Starting JSON export to '{args.filepath}'...")
    try:
        with open(args.filepath, 'w') as f:
            json.dump(documents, f, indent=2)
    except IOError as e:
        print(f"Error writing to file '{args.filepath}': {e}")
        sys.exit(1)
    print(f"\nExport complete. Successfully exported {len(documents)} documents to '{args.filepath}'.")

def handle_import_xml(args):
    """Handles the XML import command."""
    print(f"Starting XML import from '{args.filepath}' to collection '{args.collection}'...")
    try:
        tree = ET.parse(args.filepath)
        root = tree.getroot()
    except FileNotFoundError:
        print(f"Error: File not found at '{args.filepath}'")
        sys.exit(1)
    except ET.ParseError as e:
        print(f"Error: Invalid XML in file '{args.filepath}': {e}")
        sys.exit(1)

    documents = convert_xml_to_docs(root)
    import_documents(args.collection, documents)

def handle_export_xml(args):
    """Handles the XML export command."""
    documents = get_documents_from_collection(args.collection)
    print(f"Starting XML export to '{args.filepath}'...")

    root = convert_docs_to_xml_root(documents)

    try:
        xml_string = ET.tostring(root, 'utf-8')
        pretty_xml = xml.dom.minidom.parseString(xml_string).toprettyxml(indent="  ")
        with open(args.filepath, 'w') as f:
            f.write(pretty_xml)
    except IOError as e:
        print(f"Error writing to file '{args.filepath}': {e}")
        sys.exit(1)
    print(f"\nExport complete. Successfully exported {len(documents)} documents to '{args.filepath}'.")

def main():
    """Main function to run the migration tool."""
    parser = argparse.ArgumentParser(
        description="TissDB Migration Tool: Import and export data from JSON or XML files.",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument(
        "--url",
        default="http://localhost:8080",
        help="The base URL of the TissDB API (default: http://localhost:8080)"
    )

    subparsers = parser.add_subparsers(dest="command", required=True, help="Available commands")

    # --- Import JSON Command ---
    import_json_parser = subparsers.add_parser("import-json", help="Import data from a JSON file.")
    import_json_parser.add_argument("collection", help="The name of the collection to import into.")
    import_json_parser.add_argument("filepath", help="The path to the JSON file.")
    import_json_parser.set_defaults(func=handle_import_json)

    # --- Export JSON Command ---
    export_json_parser = subparsers.add_parser("export-json", help="Export data to a JSON file.")
    export_json_parser.add_argument("collection", help="The name of the collection to export from.")
    export_json_parser.add_argument("filepath", help="The path to the output JSON file.")
    export_json_parser.set_defaults(func=handle_export_json)

    # --- Import XML Command ---
    import_xml_parser = subparsers.add_parser("import-xml", help="Import data from an XML file.")
    import_xml_parser.add_argument("collection", help="The name of the collection to import into.")
    import_xml_parser.add_argument("filepath", help="The path to the XML file.")
    import_xml_parser.set_defaults(func=handle_import_xml)

    # --- Export XML Command ---
    export_xml_parser = subparsers.add_parser("export-xml", help="Export data to an XML file.")
    export_xml_parser.add_argument("collection", help="The name of the collection to export from.")
    export_xml_parser.add_argument("filepath", help="The path to the output XML file.")
    export_xml_parser.set_defaults(func=handle_export_xml)

    args = parser.parse_args()

    # Set the global API URL from the command-line argument
    global TISSDB_API_URL
    TISSDB_API_URL = args.url

    print(f"Using TissDB API at: {TISSDB_API_URL}")

    # Call the appropriate handler function
    args.func(args)

if __name__ == "__main__":
    main()
