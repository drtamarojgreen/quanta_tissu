import requests
import json
import csv
import xml.etree.ElementTree as ET
import argparse
import sqlparse
from sqlparse.sql import IdentifierList, Identifier
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

def export_to_sql(collection_name: str, file_path: str, base_url: str):
    """
    Exports all documents from a TissDB collection to a SQL file.
    """
    print(f"Starting export of collection '{collection_name}' to '{file_path}'...")
    try:
        client = TissDBAPIClient(base_url)
        documents = client.get_all_documents(collection_name)

        if not documents:
            print("No documents found in the collection. Nothing to export.")
            return

        # Infer schema (column names and types)
        header = set()
        for doc in documents:
            header.update(doc.keys())

        sorted_header = sorted(list(header))

        # Determine column types by checking all values
        column_types = {}
        for column in sorted_header:
            has_real = False
            has_integer = False
            has_boolean = False
            has_string = False

            for doc in documents:
                value = doc.get(column)
                if value is None:
                    continue

                if isinstance(value, bool):
                    has_boolean = True
                elif isinstance(value, float):
                    has_real = True
                elif isinstance(value, int):
                    has_integer = True
                else:
                    has_string = True

            if has_string:
                column_types[column] = "TEXT"
            elif has_real:
                column_types[column] = "REAL"
            elif has_integer:
                if has_boolean:
                    column_types[column] = "TEXT"
                else:
                    column_types[column] = "INTEGER"
            elif has_boolean:
                column_types[column] = "BOOLEAN"
            else:
                column_types[column] = "TEXT"

        # Generate CREATE TABLE statement
        create_table_statement = f"CREATE TABLE IF NOT EXISTS `{collection_name}` (\n"
        columns_definitions = []
        for column in sorted_header:
            columns_definitions.append(f"  `{column}` {column_types[column]}")
        create_table_statement += ",\n".join(columns_definitions)
        create_table_statement += "\n);"

        # Generate INSERT statements
        insert_statements = []
        for doc in documents:
            columns = []
            values = []
            for column in sorted_header:
                if column in doc and doc[column] is not None:
                    columns.append(f"`{column}`")
                    value = doc[column]
                    if isinstance(value, str):
                        value_str = value.replace("'", "''")
                        values.append(f"'{value_str}'")
                    elif isinstance(value, bool):
                        values.append("TRUE" if value else "FALSE")
                    else:
                        values.append(str(value))

            if not columns:
                continue

            insert_statement = f"INSERT INTO `{collection_name}` ({', '.join(columns)}) VALUES ({', '.join(values)});"
            insert_statements.append(insert_statement)

        # Write to file
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(create_table_statement)
            f.write("\n\n")
            for stmt in insert_statements:
                f.write(stmt)
                f.write("\n")

        print(f"Successfully exported {len(documents)} documents to '{file_path}'.")

    except requests.exceptions.RequestException as e:
        print(f"Error connecting to TissDB: {e}")
    except IOError as e:
        print(f"Error writing to file '{file_path}': {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

def import_from_sql(collection_name: str, file_path: str, base_url: str):
    """
    Imports documents from a SQL file into a TissDB collection.
    """
    print(f"Starting import from '{file_path}' to collection '{collection_name}'...")
    try:
        client = TissDBAPIClient(base_url)
        count = 0

        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        statements = sqlparse.parse(content)

        for stmt in statements:
            if stmt.get_type() != 'INSERT':
                continue

            # Find the parenthesis tokens, expecting one for columns and one for values
            parentheses = [t for t in stmt.tokens if isinstance(t, sqlparse.sql.Parenthesis)]
            if len(parentheses) < 2:
                continue

            columns_part = None
            values_part = None

            # Find the VALUES keyword to correctly identify columns and values parentheses
            values_keyword_found = False
            for token in stmt.tokens:
                if token.is_keyword and token.normalized == 'VALUES':
                    values_keyword_found = True
                if isinstance(token, sqlparse.sql.Parenthesis):
                    if not values_keyword_found:
                        columns_part = token
                    else:
                        values_part = token
                        break # Found both, can stop

            if not columns_part or not values_part:
                continue

            # Extract column names from the first parenthesis
            identifiers = [t for t in columns_part.tokens if isinstance(t, Identifier)]
            if not identifiers: # Check inside an IdentifierList
                id_list = [t for t in columns_part.tokens if isinstance(t, IdentifierList)]
                if id_list:
                    identifiers = id_list[0].get_identifiers()

            column_names = [i.get_real_name() for i in identifiers]

            # Extract values from the second parenthesis
            def _parse_sql_value(token):
                if token.ttype in sqlparse.tokens.Literal.String.Single:
                    return token.value[1:-1].replace("''", "'")
                return token.value

            value_tokens = [t for t in values_part.tokens if t.ttype is not None and not t.is_whitespace and t.value not in ('(', ')', ',')]
            values = [_convert_value(_parse_sql_.value(t)) for t in value_tokens]

            if len(column_names) != len(values):
                print(f"Warning: Mismatch between column count ({len(column_names)}) and value count ({len(values)}) in statement: {stmt}")
                continue

            document = dict(zip(column_names, values))
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

# ==============================================================================
# Command-Line Interface
# ==============================================================================

def main():
    """
    Main function to run the command-line interface.
    """
    parser = argparse.ArgumentParser(description="TissDB Importer/Exporter Tool")

    parser.add_argument("action", choices=['import', 'export'], help="The action to perform: 'import' or 'export'.")
    parser.add_argument("format", choices=['csv', 'xml', 'sql'], help="The data format to use: 'csv', 'xml', or 'sql'.")
    parser.add_argument("collection", help="The name of the TissDB collection.")
    parser.add_argument("filepath", help="The path to the input/output file.")
    parser.add_argument("--base-url", default="http://localhost:8080", help="The base URL of the TissDB server.")

    args = parser.parse_args()

    if args.action == 'import':
        if args.format == 'csv':
            import_from_csv(args.collection, args.filepath, args.base_url)
        elif args.format == 'xml':
            import_from_xml(args.collection, args.filepath, args.base_url)
        elif args.format == 'sql':
            import_from_sql(args.collection, args.filepath, args.base_url)
    elif args.action == 'export':
        if args.format == 'csv':
            export_to_csv(args.collection, args.filepath, args.base_url)
        elif args.format == 'xml':
            export_to_xml(args.collection, args.filepath, args.base_url)
        elif args.format == 'sql':
            export_to_sql(args.collection, args.filepath, args.base_url)

if __name__ == "__main__":
    main()
# Trivial change for staging
# Trivial change for staging
