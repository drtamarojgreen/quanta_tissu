import sys
import os
import xml.etree.ElementTree as ET

# Add the project root to the path to allow importing the migration tool
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from quanta_tissu.tisslm.migration_tools.tissdb_migration_tool import (
    convert_xml_to_docs,
    convert_docs_to_xml_root,
)
from tests.helpers.test_utils import assert_equal

def test_xml_to_docs():
    """Tests the conversion of an XML string to a list of dictionaries."""
    xml_string = """
    <data>
        <person id="1">
            <name>Alice</name>
            <city>New York</city>
        </person>
        <person id="2">
            <name>Bob</name>
            <city>London</city>
        </person>
    </data>
    """
    root = ET.fromstring(xml_string)
    documents = convert_xml_to_docs(root)

    expected_docs = [
        {'id': '1', 'name': 'Alice', 'city': 'New York'},
        {'id': '2', 'name': 'Bob', 'city': 'London'}
    ]

    assert_equal(documents, expected_docs, msg="test_xml_to_docs")

def test_docs_to_xml():
    """Tests the conversion of a list of dictionaries to an XML structure."""
    documents = [
        {'id': '1', 'name': 'Alice', 'city': 'New York'},
        {'id': '2', 'name': 'Bob', 'city': 'London'}
    ]

    result_root = convert_docs_to_xml_root(documents)
    result_xml_string = ET.tostring(result_root, 'unicode')

    # Build the expected XML string for a robust comparison
    expected_root = ET.Element("data")
    rec1 = ET.SubElement(expected_root, "record")
    ET.SubElement(rec1, "id").text = "1"
    ET.SubElement(rec1, "name").text = "Alice"
    ET.SubElement(rec1, "city").text = "New York"
    rec2 = ET.SubElement(expected_root, "record")
    ET.SubElement(rec2, "id").text = "2"
    ET.SubElement(rec2, "name").text = "Bob"
    ET.SubElement(rec2, "city").text = "London"
    expected_xml_string = ET.tostring(expected_root, 'unicode')

    assert_equal(result_xml_string, expected_xml_string, msg="test_docs_to_xml")

def test_xml_conversion_empty():
    """Tests that converting an empty list of docs results in an empty XML root."""
    documents = []
    result_root = convert_docs_to_xml_root(documents)

    # Check that the root element has no children
    assert_equal(len(list(result_root)), 0, msg="test_xml_conversion_empty")
    assert_equal(result_root.tag, "data", msg="test_xml_conversion_empty")
