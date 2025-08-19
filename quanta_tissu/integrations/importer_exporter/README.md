# TissDB Importer/Exporter

This tool allows you to import and export data from a TissDB instance using CSV or XML file formats.

## Features

- Export data from a TissDB collection to a CSV, XML, or SQL file.
- Import data from a CSV, XML, or SQL file into a TissDB collection.
- Automatic type conversion for imported data (string, integer, float, boolean).

## Prerequisites

Before using this tool, you need to have Python 3 and the `requests` library installed.

You can install the `requests` library using pip:
```bash
pip install requests
```

You also need a running instance of TissDB that is accessible from where you are running the script.

## Usage

The tool is operated from the command line. The basic syntax is:

```bash
python3 importer_exporter.py [action] [format] [collection] [filepath] [options]
```

### Arguments

- `action`: The action to perform. Must be either `import` or `export`.
- `format`: The data format to use. Must be either `csv`, `xml`, or `sql`.
- `collection`: The name of the TissDB collection to interact with.
- `filepath`: The path to the file to be used for importing or exporting.
- `--base-url`: (Optional) The base URL of the TissDB server. Defaults to `http://localhost:8080`.

### Examples

#### Exporting to CSV
To export all documents from the `users` collection to a file named `users.csv`:
```bash
python3 importer_exporter.py export csv users users.csv
```

#### Importing from CSV
To import documents from `users.csv` into the `users` collection:
```bash
python3 importer_exporter.py import csv users users.csv
```

#### Exporting to XML
To export all documents from the `products` collection to a file named `products.xml`:
```bash
python3 importer_exporter.py export xml products products.xml
```

#### Importing from XML
To import documents from `products.xml` into the `products` collection:
```bash
python3 importer_exporter.py import xml products products.xml
```

#### Exporting to SQL
To export all documents from the `users` collection to a file named `users.sql`:
```bash
python3 importer_exporter.py export sql users users.sql
```

#### Importing from SQL
To import documents from `users.sql` into the `users` collection:
```bash
python3 importer_exporter.py import sql users users.sql
```

#### Using a different TissDB instance
To connect to a TissDB server running on a different host or port, use the `--base-url` option:
```bash
python3 importer_exporter.py export csv users users.csv --base-url http://tissdb.example.com:8000
```

## Sample Data

This directory includes `sample_data.csv`, `sample_data.xml`, and `sample_data.sql` files that you can use to test the tool.

**To test the import:**
```bash
# Assuming a running TissDB instance and an empty 'users' collection
python3 importer_exporter.py import csv users sample_data.csv
```

**To test the export:**
```bash
# This will create a new file named 'export_test.csv' with the data from the 'users' collection
python3 importer_exporter.py export csv users export_test.csv
```

## Implementation Details and Challenges

Implementing the SQL import and export functionality involved several challenges. This section provides an overview of the approach taken and the key decisions made during development.

### SQL Export (`export_to_sql`)

The export process generates a `.sql` file containing a `CREATE TABLE` statement and a series of `INSERT` statements based on the documents in a TissDB collection.

**Challenge: Data Type Inference**

TissDB stores data in a JSON-like format, where data types are dynamic (e.g., `string`, `integer`, `float`, `boolean`). SQL, on the other hand, requires a fixed schema with strongly typed columns (`TEXT`, `INTEGER`, `REAL`, `BOOLEAN`).

To address this, the `export_to_sql` function implements a type inference mechanism:
1.  It first scans all documents to gather a set of all unique keys, which become the columns of the SQL table.
2.  For each column, it then iterates through all documents to inspect the type of every value.
3.  A type hierarchy is used to determine the most appropriate SQL type for the column:
    - If any value is a string, or if there's a mix of incompatible types (e.g., integers and booleans), the column type defaults to `TEXT` for maximum compatibility.
    - If a column contains only numbers, it becomes `REAL` if any of them are floats, or `INTEGER` if all are integers.
    - A column with only boolean values is typed as `BOOLEAN`.

This ensures that the generated `CREATE TABLE` statement is as accurate as possible while preventing data loss.

### SQL Import (`import_from_sql`)

The import process reads a `.sql` file and inserts the data into a TissDB collection.

**Challenge: SQL Parsing**

SQL is a complex language, and writing a robust parser from scratch is a significant undertaking. A simple regex-based parser would be too brittle to handle the variations in SQL syntax.

To solve this, the implementation uses the `sqlparse` Python library. This library is excellent at tokenizing SQL statements and identifying their structure without being a full-fledged semantic parser. The `import_from_sql` function uses it to:
1.  Split the content of the `.sql` file into individual statements.
2.  Identify `INSERT` statements.
3.  For each `INSERT` statement, it extracts the list of column names and the corresponding values.
4.  It then reconstructs a JSON document from the columns and values and inserts it into the specified TissDB collection.

The script reuses the `_convert_value` helper function to automatically convert the string values from the SQL file into appropriate Python types (integer, float, boolean, or string).

### General Challenge: Database Dependency

This tool requires a running instance of the TissDB server to function. During development and testing, this can be a hurdle if the server is not available. Users of this script should ensure that the TissDB instance is running and accessible at the specified `--base-url`.
