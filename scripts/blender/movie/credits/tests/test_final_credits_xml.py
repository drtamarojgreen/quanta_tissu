#!/usr/bin/env python3
"""
Tests for the final_credits.kdenlive XML generation.
"""
import xml.etree.ElementTree as ET
from pathlib import Path
import sys
import re

def test_final_credits_xml():
    """Validates the structure and content of final_credits.kdenlive."""
    print("Running test_final_credits_xml...")

    # Path is relative to this test file
    base_dir = Path(__file__).parent.parent
    filepath = base_dir / "output" / "final_credits.kdenlive"

    if not filepath.exists():
        print(f"FAILED: {filepath} does not exist. Run generate_final_credits.py first.")
        sys.exit(1)

    try:
        tree = ET.parse(filepath)
        root = tree.getroot()
    except ET.ParseError as e:
        print(f"FAILED: {filepath} is not valid XML: {e}")
        sys.exit(1)

    # 1. Root element is <mlt>
    if root.tag != "mlt":
        print(f"FAILED: Root tag is {root.tag}, expected mlt")
        sys.exit(1)

    # 2. Text checks
    xml_str = ET.tostring(root, encoding="unicode")
    if xml_str.count("Tamaro Green") < 2:
        print(f"FAILED: 'Tamaro Green' appears {xml_str.count('Tamaro Green')} times, expected at least 2")
        sys.exit(1)
    if "CAST" not in xml_str:
        print("FAILED: 'CAST' not found in XML")
        sys.exit(1)

    # 3. Geometry keyframe string check
    geom_prop = root.find(".//property[@name='geometry']")
    if geom_prop is None or not geom_prop.text:
        print("FAILED: <property name='geometry'> not found or empty")
        sys.exit(1)

    geom_text = geom_prop.text
    # Look for at least two Y positions (indicated by / or = followed by signed integer then :)
    y_positions = re.findall(r'[=/](-?\d+):', geom_text)
    if len(y_positions) < 2:
        print(f"FAILED: Found {len(y_positions)} Y positions in geometry, expected at least 2 for animation")
        sys.exit(1)
    print(f"Geometry check passed: Y positions {y_positions}")

    # 4. Total timeline duration ≥ 2250 frames
    main_tractor = root.find(".//tractor[@id='main_tractor']")
    if main_tractor is not None:
        # out is 0-indexed, so add 1
        duration = int(main_tractor.get("out", 0)) + 1
        if duration < 2250:
            print(f"FAILED: Duration is {duration} frames, expected ≥ 2250")
            sys.exit(1)
        else:
            print(f"Duration check passed: {duration} frames")
    else:
        print("FAILED: <tractor id='main_tractor'> not found")
        sys.exit(1)

    print("SUCCESS: All tests passed for final_credits.kdenlive")

if __name__ == "__main__":
    test_final_credits_xml()
