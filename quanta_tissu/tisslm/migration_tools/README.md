# TissDB Migration Tool

A command-line utility for importing and exporting data to and from a TissDB instance. The tool supports both JSON and XML file formats.

## Prerequisites

This tool is written in Python 3 and requires the `requests` library.

To install the necessary dependencies, run the following command from within the `tissdb_tools` directory:
```bash
pip install -r requirements.txt
```

## Usage

The tool is run from the command line using `python3 tissdb_migration_tool.py`.

### Global Options

- `--url <URL>`: Specify the base URL of the TissDB API. Defaults to `http://localhost:8080`.

### Commands

The tool has four main commands: `import-json`, `export-json`, `import-xml`, and `export-xml`.

---

#### Import JSON

Imports documents from a JSON file into a specified collection. The JSON file can contain a single document object or an array of document objects.

**Usage:**
```bash
python3 tissdb_migration_tool.py import-json <collection_name> <path_to_file.json>
```

**Example:**
```bash
python3 tissdb_migration_tool.py --url http://127.0.0.1:8080 import-json users users_data.json
```

---

#### Export JSON

Exports all documents from a collection into a JSON file. The output file will contain an array of document objects.

**Usage:**
```bash
python3 tissdb_migration_tool.py export-json <collection_name> <path_to_output.json>
```

**Example:**
```bash
python3 tissdb_migration_tool.py export-json products products_export.json
```

---

#### Import XML

Imports documents from an XML file into a specified collection. The tool expects the XML file to have a root element containing a series of child elements, where each child element represents a document. The tags and text of the sub-elements of each document element will be converted into key-value pairs.

**Expected Format:**
```xml
<data>
  <record id="1">
    <name>Product A</name>
    <price>19.99</price>
  </record>
  <record id="2">
    <name>Product B</name>
    <price>29.99</price>
  </record>
</data>
```

**Usage:**
```bash
python3 tissdb_migration_tool.py import-xml <collection_name> <path_to_file.xml>
```

**Example:**
```bash
python3 tissdb_migration_tool.py import-xml inventory inventory_data.xml
```

---

#### Export XML

Exports all documents from a collection into an XML file. Each document will be converted into a `<record>` element within a main `<data>` root element.

**Usage:**
```bash
python3 tissdb_migration_tool.py export-xml <collection_name> <path_to_output.xml>
```

**Example:**
```bash
python3 tissdb_migration_tool.py export-xml users users_export.xml
```
