import xml.etree.ElementTree as ET
from pathlib import Path

# ── PRODUCTION CONFIGURATION ────────────────────────────────────────────────
CONFIG = {
    "film_title":        "The Greenhouse",
    "studio_name":       "GreenhouseMD Production Studio",
    "co_production":     "GreenhouseMD / GreenhouseMHD Production",
    "year":              2026,
    "fps":               25,
    "width":             1920,
    "height":            1080,
    "output_dir":        "output",
    "credits_scroll_duration": 90,
    "background_black":  "#000000",
    "text_white":        "#ffffff",
    "text_gold":         "#c8a84b",
    "cast": {
        "Herbaceous":  "AI",
        "Arbor":       "AI",
        "GloomGnome":  "AI",
    },
}

def generate_final_credits():
    """Generates the final_credits.kdenlive XML file using kdenlivetitle."""
    root = ET.Element("mlt", {
        "LC_NUMERIC": "C",
        "version": "7.22.0"
    })

    width = CONFIG["width"]
    height = CONFIG["height"]
    fps = CONFIG["fps"]
    duration_frames = CONFIG["credits_scroll_duration"] * fps

    ET.SubElement(root, "profile", {
        "description": "atsc_1080p_25",
        "width": str(width),
        "height": str(height),
        "frame_rate_num": str(fps),
        "frame_rate_den": "1",
        "progressive": "1"
    })

    # Background
    bg = ET.SubElement(root, "producer", {"id": "bg", "in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(bg, "property", {"name": "mlt_service"}).text = "color"
    ET.SubElement(bg, "property", {"name": "resource"}).text = CONFIG["background_black"]

    # Kdenlive Title XML
    title_xml = ET.Element("kdenlivetitle", {"width": str(width), "height": str(height), "out": str(duration_frames - 1)})

    # We'll use a single large text item for simplicity, or multiple items.
    # Kdenlive titles handle newlines in content.
    content = "CAST\n\n"
    for char, actor in CONFIG["cast"].items():
        content += f"{char.ljust(20)}  {actor}\n"
    content += "\n\nEXECUTIVE PRODUCER\nTamaro Green\n\nPRODUCER\nTamaro Green\n\n"
    content += "LIGHTING DIRECTOR\nAI\n\nWRITER\nAI\n\nCOSTUME DESIGNER\nAI\n\n"
    content += f"{CONFIG['film_title']}\n\nA GreenhouseMD / GreenhouseMHD Production\n"
    content += f"© {CONFIG['year']} GreenhouseMD Production Studio\nAll rights reserved."

    # In a real kdenlivetitle, each line might be an item, but for a scroll,
    # a single block is often used.
    item = ET.SubElement(title_xml, "item", {"z-index": "0", "type": "QGraphicsTextItem"})
    ET.SubElement(item, "content").text = content
    # Kdenlive title uses specific font properties
    ET.SubElement(item, "font", {"family": "DejaVu Sans", "size": "36", "weight": "50"})
    ET.SubElement(item, "font-color", {"red": "255", "green": "255", "blue": "255", "alpha": "255"})

    # Kdenlive title XML string
    xmldata = ET.tostring(title_xml, encoding="utf-8").decode("utf-8")

    # MLT Producer for the title
    prod = ET.SubElement(root, "producer", {"id": "credits_text", "in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(prod, "property", {"name": "mlt_service"}).text = "kdenlivetitle"
    ET.SubElement(prod, "property", {"name": "xmldata"}).text = xmldata
    # The requirement said to use a geometry keyframe for the scroll
    text_height = 3000 # Estimate
    geom = f"0=0/{height}:{width}x{text_height}:100; {duration_frames-1}=0/-{text_height}:{width}x{text_height}:100"
    ET.SubElement(prod, "property", {"name": "geometry"}).text = geom

    # Tractor
    tractor = ET.SubElement(root, "tractor", {"id": "main_tractor", "in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(tractor, "track", {"producer": "bg"})
    ET.SubElement(tractor, "track", {"producer": "credits_text"})

    # Composite to ensure layering
    tr = ET.SubElement(tractor, "transition", {"in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(tr, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(tr, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(tr, "property", {"name": "b_track"}).text = "1"

    # Global film grain
    f_grain = ET.SubElement(tractor, "filter")
    ET.SubElement(f_grain, "property", {"name": "mlt_service"}).text = "frei0r.film"

    output_dir = Path(__file__).parent / CONFIG["output_dir"]
    output_dir.mkdir(exist_ok=True)
    output_path = output_dir / "final_credits.kdenlive"

    with open(output_path, "wb") as f:
        f.write(b'<?xml version="1.0" encoding="utf-8"?>\n')
        f.write(ET.tostring(root, encoding="utf-8"))

    print(f"[credits] final_credits.kdenlive written — {duration_frames} frames")

if __name__ == "__main__":
    generate_final_credits()
