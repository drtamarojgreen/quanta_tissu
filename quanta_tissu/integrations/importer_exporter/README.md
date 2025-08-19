# TissDB Importer/Exporter

This tool allows you to import and export data from a TissDB instance using CSV or XML file formats.

## Features

- Export data from a TissDB collection to a CSV or XML file.
- Import data from a CSV or XML file into a TissDB collection.
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
- `format`: The data format to use. Must be either `csv` or `xml`.
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

#### Using a different TissDB instance
To connect to a TissDB server running on a different host or port, use the `--base-url` option:
```bash
python3 importer_exporter.py export csv users users.csv --base-url http://tissdb.example.com:8000
```

## Sample Data

This directory includes `sample_data.csv` and `sample_data.xml` files that you can use to test the tool.

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
