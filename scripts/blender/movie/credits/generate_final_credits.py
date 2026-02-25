#!/usr/bin/env python3
"""
Generates the scrolling final credits KDenlive XML project for GreenhouseMD.
"""
import xml.etree.ElementTree as ET
from pathlib import Path

# ── PRODUCTION CONFIGURATION ────────────────────────────────────────────────
CONFIG = {
    "film_title":        "The Greenhouse",          # appears in header C and credits
    "studio_name":       "GreenhouseMD Production Studio",
    "co_production":     "GreenhouseMD / GreenhouseMHD Production",
    "year":              2026,
    "fps":               25,
    "width":             1920,
    "height":            1080,
    "output_dir":        "output",                  # relative to scripts/blender/movie/credits/

    # Header segment durations in seconds
    "header_segment_a_duration": 12,
    "header_segment_b_duration": 15,
    "header_segment_c_duration": 15,

    # Credits scroll duration in seconds
    "credits_scroll_duration": 90,

    # Colours
    "background_dark":   "#1a1a1a",
    "background_black":  "#000000",
    "text_white":        "#ffffff",
    "text_gold":         "#c8a84b",

    # Cast — { "Character Name": "Performer / Voice" }
    "cast": {
        "Herbaceous":  "AI",
        "Arbor":       "AI",
        "GloomGnome":  "AI",
    },
}

def indent(elem, level=0):
    """Recursively indents XML elements for pretty-printing.

    Args:
        elem: The element to indent.
        level: Current nesting level.
    """
    i = "\n" + level * "  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for sub_elem in elem:
            indent(sub_elem, level + 1)
        if not sub_elem.tail or not sub_elem.tail.strip():
            sub_elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i

def hex_to_rgba_list(hex_color):
    """Converts hex color string to 'R,G,B,A' string.

    Args:
        hex_color: Hex color string (e.g. #ffffff).
    Returns:
        String in 'R,G,B,A' format.
    """
    hex_color = hex_color.lstrip('#')
    r = int(hex_color[0:2], 16)
    g = int(hex_color[2:4], 16)
    b = int(hex_color[4:6], 16)
    return f"{r},{g},{b},255"

def create_complex_title_xml(items, block_height):
    """Creates the internal XML content for a kdenlivetitle producer with multiple items.

    Args:
        items: List of dictionaries describing each text item.
        block_height: The total height of the text block.
    Returns:
        A string of the title XML.
    """
    root = ET.Element("kdenlivetitle", {
        "width": str(CONFIG["width"]),
        "height": str(block_height),
        "out": "9999"
    })
    for i, item in enumerate(items):
        item_el = ET.SubElement(root, "item", {"type": "QGraphicsTextItem", "z-index": str(i)})
        content = ET.SubElement(item_el, "content", {
            "font": "DejaVu Sans",
            "font-size": str(item["size"]),
            "font-weight": str(item["weight"]),
            "fill": item["color"]
        })
        content.text = item["text"]
        ET.SubElement(item_el, "position", {"x": str(item["x"]), "y": str(item["y"])})
    return ET.tostring(root, encoding="unicode")

def generate_final_credits():
    """Generates the final_credits.kdenlive file with a scrolling credit sequence."""
    fps = CONFIG["fps"]
    width = CONFIG["width"]
    height = CONFIG["height"]
    duration_frames = CONFIG["credits_scroll_duration"] * fps

    root = ET.Element("mlt", {
        "LC_NUMERIC": "C",
        "version": "7.22.0",
        "title": "Final Credits",
        "producer": "main_bin",
        "xmlns:kdenlive": "http://www.kdenlive.org/project"
    })

    # Profile definition
    ET.SubElement(root, "profile", {
        "description": "atsc_1080p_25",
        "frame_rate_num": str(fps),
        "frame_rate_den": "1",
        "width": str(width),
        "height": str(height),
        "progressive": "1",
        "sample_aspect_num": "1",
        "sample_aspect_den": "1",
        "display_aspect_num": "16",
        "display_aspect_den": "9",
        "colorspace": "709"
    })

    # Prepare credit content
    lines = [("CAST", "header"), (" ", "body")]
    for char, actor in CONFIG["cast"].items():
        # Character name and actor/voice
        lines.append((f"{char}    {actor}", "body"))

    lines.extend([
        (" ", "body"), (" ", "body"),
        ("EXECUTIVE PRODUCER", "header"), ("Tamaro Green", "body"), (" ", "body"),
        ("PRODUCER", "header"), ("Tamaro Green", "body"), (" ", "body"),
        ("LIGHTING DIRECTOR", "header"), ("AI", "body"), (" ", "body"),
        ("WRITER", "header"), ("AI", "body"), (" ", "body"),
        ("COSTUME DESIGNER", "header"), ("AI", "body"), (" ", "body"), (" ", "body"),
        (CONFIG["film_title"], "body"), (" ", "body"),
        (CONFIG["co_production"], "body"),
        (f"© {CONFIG['year']} {CONFIG['studio_name']}", "body"),
        ("All rights reserved.", "body")
    ])

    # Convert lines to XML items
    items = []
    curr_y = 0
    gold_rgba = hex_to_rgba_list(CONFIG["text_gold"])
    white_rgba = hex_to_rgba_list(CONFIG["text_white"])

    for text, style in lines:
        size = 48 if style == "header" else 36
        weight = 700 if style == "header" else 400
        color = gold_rgba if style == "header" else white_rgba

        # Approximate horizontal centering
        est_w = len(text) * (size * 0.5)
        x = (width - est_w) / 2

        items.append({
            "text": text,
            "size": size,
            "weight": weight,
            "color": color,
            "x": x,
            "y": curr_y
        })
        curr_y += 80 if style == "header" else 60

    text_block_height = max(curr_y, height) # Ensure at least screen height
    title_xml = create_complex_title_xml(items, text_block_height)

    # --- PRODUCERS ---

    bg_black = ET.SubElement(root, "producer", {"id": "bg_black", "in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(bg_black, "property", {"name": "mlt_service"}).text = "color"
    ET.SubElement(bg_black, "property", {"name": "resource"}).text = CONFIG["background_black"]

    prod_credits = ET.SubElement(root, "producer", {"id": "prod_credits", "in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(prod_credits, "property", {"name": "mlt_service"}).text = "kdenlivetitle"
    ET.SubElement(prod_credits, "property", {"name": "xmldata"}).text = title_xml

    # --- MAIN TRACTOR ---
    tractor = ET.SubElement(root, "tractor", {"id": "main_tractor", "in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(tractor, "track", {"producer": "bg_black"})
    ET.SubElement(tractor, "track", {"producer": "prod_credits"})

    # Compositing and Scrolling via Transition
    trans = ET.SubElement(tractor, "transition", {"in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(trans, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(trans, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(trans, "property", {"name": "b_track"}).text = "1"

    # Animated scroll geometry: start at bottom, end above top
    geom = f"0=0/{height}:{width}x{text_block_height}:100; {duration_frames-1}=0/-{text_block_height}:{width}x{text_block_height}:100"
    ET.SubElement(trans, "property", {"name": "geometry"}).text = geom

    # Film grain overlay
    f_grain = ET.SubElement(tractor, "filter", {"in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(f_grain, "property", {"name": "mlt_service"}).text = "frei0r.film"
    ET.SubElement(f_grain, "property", {"name": "0"}).text = "0.1" # Grain intensity
    ET.SubElement(f_grain, "property", {"name": "1"}).text = "0.05" # Scratch intensity

    # Cinematic vignette
    f_vignette = ET.SubElement(tractor, "filter", {"in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(f_vignette, "property", {"name": "mlt_service"}).text = "frei0r.vignette"

    # Final Output Structure
    main_bin = ET.SubElement(root, "playlist", {"id": "main_bin", "in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(main_bin, "entry", {"producer": "main_tractor"})

    # Write File
    output_dir = Path(__file__).parent / CONFIG["output_dir"]
    output_dir.mkdir(parents=True, exist_ok=True)
    output_file = output_dir / "final_credits.kdenlive"

    indent(root)
    tree = ET.ElementTree(root)
    tree.write(output_file, encoding="utf-8", xml_declaration=True)

    print(f"[credits] final_credits.kdenlive written — {duration_frames} frames, 2 producers, 1 filter")

if __name__ == "__main__":
    generate_final_credits()
