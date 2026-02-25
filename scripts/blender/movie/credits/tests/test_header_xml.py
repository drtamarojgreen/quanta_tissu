#!/usr/bin/env python3
"""
Tests for the header.kdenlive XML generation.
"""
import xml.etree.ElementTree as ET
from pathlib import Path
import sys

def test_header_xml():
    """Validates the structure and content of header.kdenlive."""
    print("Running test_header_xml...")

    # Path is relative to this test file
    base_dir = Path(__file__).parent.parent
    filepath = base_dir / "output" / "header.kdenlive"

    if not filepath.exists():
        print(f"FAILED: {filepath} does not exist. Run generate_header.py first.")
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

    # 2. At least three <producer> elements exist
    producers = root.findall(".//producer")
    if len(producers) < 3:
        print(f"FAILED: Found {len(producers)} producers, expected at least 3")
        sys.exit(1)

    # 3. Specific text exists
    # We check raw string to see if it's anywhere (including xmldata properties)
    xml_str = ET.tostring(root, encoding="unicode")
    if "GreenhouseMD" not in xml_str:
        print("FAILED: 'GreenhouseMD' not found in XML")
        sys.exit(1)
    if "Production Studio" not in xml_str:
        print("FAILED: 'Production Studio' not found in XML")
        sys.exit(1)

    # 4. At least one <filter> element with glow or frei0r
    filters = root.findall(".//filter")
    glow_or_frei0r = False
    for f in filters:
        svc = f.find("property[@name='mlt_service']")
        if svc is not None and svc.text and ("glow" in svc.text or "frei0r" in svc.text):
            glow_or_frei0r = True
            break
    if not glow_or_frei0r:
        print("FAILED: No filter with 'glow' or 'frei0r' service found")
        sys.exit(1)

    # 5. Total timeline duration ≥ 1000 frames
    # Our timeline uses pl_c as the last track in the main_timeline tractor.
    pl_c = root.find(".//playlist[@id='pl_c']")
    if pl_c is not None:
        blank = pl_c.find("blank")
        entry = pl_c.find("entry")
        if blank is not None and entry is not None:
            blank_len = int(blank.get("length", 0))
            prod_id = entry.get("producer")
            # The producer might be a tractor
            prod = root.find(f".//*[@id='{prod_id}']")
            if prod is not None:
                prod_len = int(prod.get("out", 0)) + 1
                total_len = blank_len + prod_len
                if total_len < 1000:
                    print(f"FAILED: Total duration is {total_len} frames, expected ≥ 1000")
                    sys.exit(1)
                else:
                    print(f"Duration check passed: {total_len} frames")
            else:
                print(f"FAILED: Could not find producer/tractor with id '{prod_id}'")
                sys.exit(1)
        else:
            print("FAILED: pl_c structure (blank/entry) not as expected")
            sys.exit(1)
    else:
        print("FAILED: <playlist id='pl_c'> not found in XML")
        sys.exit(1)

    print("SUCCESS: All tests passed for header.kdenlive")

if __name__ == "__main__":
    test_header_xml()
