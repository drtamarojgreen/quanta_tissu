#!/usr/bin/env python3
"""
Generates the scrolling final credits KDenlive-compatible XML project for GreenhouseMD.
Uses pango producers and composite transitions.
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

    # Cast
    "cast": {
        "Herbaceous":  "AI",
        "Arbor":       "AI",
        "GloomGnome":  "AI",
    },
}

def indent(elem, level=0):
    """Recursively indents XML elements."""
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

    # Construct Pango Markup
    markup = ''
    curr_y = 0
    for text, style in lines:
        size = 48 if style == "header" else 36
        weight = "bold" if style == "header" else "normal"
        color = CONFIG["text_gold"] if style == "header" else CONFIG["text_white"]
        markup += f'<span font_family="DejaVu Sans" foreground="{color}" weight="{weight}" size="{size*1024}">{text}</span>\n'
        curr_y += 80 if style == "header" else 60

    text_block_height = max(curr_y, height)

    # --- PRODUCERS ---

    bg_black = ET.SubElement(root, "producer", {"id": "bg_black", "in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(bg_black, "property", {"name": "mlt_service"}).text = "color"
    ET.SubElement(bg_black, "property", {"name": "resource"}).text = CONFIG["background_black"]

    prod_credits = ET.SubElement(root, "producer", {"id": "prod_credits", "in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(prod_credits, "property", {"name": "mlt_service"}).text = "pango"
    ET.SubElement(prod_credits, "property", {"name": "markup"}).text = markup
    ET.SubElement(prod_credits, "property", {"name": "align"}).text = "centre"
    ET.SubElement(prod_credits, "property", {"name": "width"}).text = str(width)
    ET.SubElement(prod_credits, "property", {"name": "bgcolour"}).text = "#00000000"
    ET.SubElement(prod_credits, "property", {"name": "progressive"}).text = "1"

    # --- MAIN TRACTOR ---
    tractor = ET.SubElement(root, "tractor", {"id": "main_tractor", "in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(tractor, "track", {"producer": "bg_black"})
    ET.SubElement(tractor, "track", {"producer": "prod_credits"})

    trans = ET.SubElement(tractor, "transition", {"in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(trans, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(trans, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(trans, "property", {"name": "b_track"}).text = "1"

    geom = f"0: 0/{height}:{width}x{text_block_height}:100; {duration_frames-1}: 0/-{text_block_height}:{width}x{text_block_height}:100"
    ET.SubElement(trans, "property", {"name": "geometry"}).text = geom

    # Film grain and vignette
    f_grain = ET.SubElement(tractor, "filter", {"in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(f_grain, "property", {"name": "mlt_service"}).text = "frei0r.film"
    ET.SubElement(f_grain, "property", {"name": "0"}).text = "0.1"

    f_vignette = ET.SubElement(tractor, "filter", {"in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(f_vignette, "property", {"name": "mlt_service"}).text = "frei0r.vignette"

    # Final Structure
    main_bin = ET.SubElement(root, "playlist", {"id": "main_bin", "in": "0", "out": str(duration_frames - 1)})
    ET.SubElement(main_bin, "entry", {"producer": "main_tractor"})

    # Write File
    output_dir = Path(__file__).parent / CONFIG["output_dir"]
    output_dir.mkdir(parents=True, exist_ok=True)
    output_file = output_dir / "final_credits.kdenlive"
    indent(root)
    ET.ElementTree(root).write(output_file, encoding="utf-8", xml_declaration=True)
    print(f"[credits] final_credits.kdenlive written — {duration_frames} frames")

if __name__ == "__main__":
    generate_final_credits()
