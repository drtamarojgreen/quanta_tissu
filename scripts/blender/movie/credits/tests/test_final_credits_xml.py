import unittest
import xml.etree.ElementTree as ET
from pathlib import Path
import sys

# Add the parent directory to sys.path to import generate_final_credits
sys.path.append(str(Path(__file__).parent.parent))
from generate_final_credits import generate_final_credits, CONFIG

class TestFinalCreditsXML(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Ensure the final_credits.kdenlive is generated before tests
        generate_final_credits()
        cls.output_path = Path(__file__).parent.parent / CONFIG["output_dir"] / "final_credits.kdenlive"

    def test_file_exists(self):
        self.assertTrue(self.output_path.exists(), "final_credits.kdenlive was not generated")

    def test_is_valid_xml(self):
        try:
            tree = ET.parse(self.output_path)
            self.root = tree.getroot()
        except ET.ParseError:
            self.fail("final_credits.kdenlive is not a valid XML file")

    def test_root_is_mlt(self):
        tree = ET.parse(self.output_path)
        root = tree.getroot()
        self.assertEqual(root.tag, "mlt")

    def test_cast_presence(self):
        with open(self.output_path, 'r', encoding='utf-8') as f:
            content = f.read()
        self.assertIn("CAST", content)
        self.assertIn("Tamaro Green", content)
        for char in CONFIG["cast"]:
            self.assertIn(char, content)

    def test_scroll_geometry(self):
        tree = ET.parse(self.output_path)
        root = tree.getroot()
        # Find the producer that handles the scroll
        producer = root.find(".//producer[@id='credits_text']")
        self.assertIsNotNone(producer)
        geom = producer.find("property[@name='geometry']")
        self.assertIsNotNone(geom)
        geom_text = geom.text
        # Check for two keyframes (0=...; last=...)
        self.assertIn(";", geom_text)
        # Check for Y movement (scroll)
        # We expect something like 0=0/1080:... ; end=0/-2500:...
        self.assertIn("0/", geom_text)
        self.assertIn("-", geom_text)

    def test_total_duration(self):
        tree = ET.parse(self.output_path)
        root = tree.getroot()
        bg = root.find(".//producer[@id='bg']")
        out_val = int(bg.get("out"))
        # 90s * 25fps = 2250 frames. out is duration-1.
        self.assertGreaterEqual(out_val, 2249)

    def test_profile_settings(self):
        tree = ET.parse(self.output_path)
        root = tree.getroot()
        profile = root.find("profile")
        self.assertIsNotNone(profile)
        self.assertEqual(profile.get("width"), str(CONFIG["width"]))
        self.assertEqual(profile.get("height"), str(CONFIG["height"]))
        self.assertEqual(profile.get("frame_rate_num"), str(CONFIG["fps"]))

    def test_tractor_structure(self):
        tree = ET.parse(self.output_path)
        root = tree.getroot()
        tractor = root.find(".//tractor[@id='main_tractor']")
        self.assertIsNotNone(tractor)
        tracks = tractor.findall("track")
        self.assertGreaterEqual(len(tracks), 2)

        # Check for composite transition
        transition = tractor.find("transition")
        self.assertIsNotNone(transition)
        service = transition.find("property[@name='mlt_service']")
        self.assertEqual(service.text, "composite")

if __name__ == "__main__":
    unittest.main()
