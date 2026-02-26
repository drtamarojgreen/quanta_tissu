import unittest
import xml.etree.ElementTree as ET
from pathlib import Path
import os
import sys

# Add the parent directory to sys.path to import generate_header
sys.path.append(str(Path(__file__).parent.parent))
from generate_header import generate_header, CONFIG

class TestHeaderXML(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Ensure the header.kdenlive is generated before tests
        generate_header()
        cls.output_path = Path(__file__).parent.parent / CONFIG["output_dir"] / "header.kdenlive"

    def test_file_exists(self):
        self.assertTrue(self.output_path.exists(), "header.kdenlive was not generated")

    def test_is_valid_xml(self):
        try:
            tree = ET.parse(self.output_path)
            self.root = tree.getroot()
        except ET.ParseError:
            self.fail("header.kdenlive is not a valid XML file")

    def test_root_is_mlt(self):
        tree = ET.parse(self.output_path)
        root = tree.getroot()
        self.assertEqual(root.tag, "mlt")

    def test_producer_count(self):
        tree = ET.parse(self.output_path)
        root = tree.getroot()
        producers = root.findall("producer")
        # bg, a1, a2, b1, c_title, and len(cast) producers
        expected_count = 5 + len(CONFIG["cast"])
        self.assertGreaterEqual(len(producers), expected_count)

    def test_required_text_content(self):
        with open(self.output_path, 'r', encoding='utf-8') as f:
            content = f.read()
        self.assertIn("GreenhouseMD", content)
        self.assertIn("Production Studio", content)
        self.assertIn(CONFIG["film_title"], content)

    def test_filter_presence(self):
        tree = ET.parse(self.output_path)
        root = tree.getroot()
        filters = root.findall(".//filter")
        services = [f.find("property[@name='mlt_service']").text for f in filters if f.find("property[@name='mlt_service']") is not None]
        # We expect frei0r.film or frei0r.glow (simulated in my current version)
        self.assertTrue(any("frei0r" in s for s in services), "No frei0r filter found")

    def test_total_duration(self):
        tree = ET.parse(self.output_path)
        root = tree.getroot()
        tractor = root.find(".//tractor[@id='main_tractor']")
        out_val = int(tractor.get("out"))
        # 12+15+15 = 42s * 25fps = 1050 frames. tractor out is duration-1.
        self.assertGreaterEqual(out_val, 999)

if __name__ == "__main__":
    unittest.main()
