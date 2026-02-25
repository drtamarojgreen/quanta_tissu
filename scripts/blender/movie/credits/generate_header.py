#!/usr/bin/env python3
"""
Generates the opening header sequence KDenlive XML project for GreenhouseMD.
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

def create_title_xml(text, size, weight=400, color="255,255,255,255", x=0, y=0):
    """Creates the internal XML content for a kdenlivetitle producer.

    Args:
        text: The text to display.
        size: Font size in points.
        weight: Font weight (e.g., 400, 700).
        color: RGBA color string "R,G,B,A".
        x: X position.
        y: Y position.

    Returns:
        A string of the title XML.
    """
    root = ET.Element("kdenlivetitle", {
        "width": str(CONFIG["width"]),
        "height": str(CONFIG["height"]),
        "out": "999"
    })
    item = ET.SubElement(root, "item", {"type": "QGraphicsTextItem", "z-index": "0"})
    content = ET.SubElement(item, "content", {
        "font": "DejaVu Sans",
        "font-size": str(size),
        "font-weight": str(weight),
        "fill": color
    })
    content.text = text
    ET.SubElement(item, "position", {"x": str(x), "y": str(y)})
    return ET.tostring(root, encoding="unicode")

def generate_header():
    """Generates the header.kdenlive file with multi-segment layout and effects."""
    fps = CONFIG["fps"]
    width = CONFIG["width"]
    height = CONFIG["height"]

    root = ET.Element("mlt", {
        "LC_NUMERIC": "C",
        "version": "7.13.0",
        "title": "Header Credits"
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

    # --- PRODUCERS ---

    # Backgrounds
    bg_charcoal = ET.SubElement(root, "producer", {"id": "bg_charcoal"})
    ET.SubElement(bg_charcoal, "property", {"name": "mlt_service"}).text = "color"
    ET.SubElement(bg_charcoal, "property", {"name": "resource"}).text = CONFIG["background_dark"]

    bg_black = ET.SubElement(root, "producer", {"id": "bg_black"})
    ET.SubElement(bg_black, "property", {"name": "mlt_service"}).text = "color"
    ET.SubElement(bg_black, "property", {"name": "resource"}).text = CONFIG["background_black"]

    # Segment A Producers
    dur_a = CONFIG["header_segment_a_duration"] * fps
    title_a1_xml = create_title_xml("GreenhouseMD", 120, 700, x=460, y=400)
    prod_a1 = ET.SubElement(root, "producer", {"id": "prod_a1", "in": "0", "out": str(dur_a - 1)})
    ET.SubElement(prod_a1, "property", {"name": "mlt_service"}).text = "kdenlivetitle"
    ET.SubElement(prod_a1, "property", {"name": "xmldata"}).text = title_a1_xml

    title_a2_xml = create_title_xml("Production Studio", 48, 400, x=750, y=560)
    prod_a2 = ET.SubElement(root, "producer", {"id": "prod_a2", "in": "0", "out": str(dur_a - 1)})
    ET.SubElement(prod_a2, "property", {"name": "mlt_service"}).text = "kdenlivetitle"
    ET.SubElement(prod_a2, "property", {"name": "xmldata"}).text = title_a2_xml

    # Segment B Producer
    dur_b = CONFIG["header_segment_b_duration"] * fps
    title_b_text = f"{CONFIG['studio_name']} presents...\nA {CONFIG['co_production']}"
    title_b_xml = create_title_xml(title_b_text, 60, x=300, y=480)
    prod_b = ET.SubElement(root, "producer", {"id": "prod_b", "in": "0", "out": str(dur_b - 1)})
    ET.SubElement(prod_b, "property", {"name": "mlt_service"}).text = "kdenlivetitle"
    ET.SubElement(prod_b, "property", {"name": "xmldata"}).text = title_b_xml

    # Segment C Producers
    dur_c = CONFIG["header_segment_c_duration"] * fps
    title_c_main_xml = create_title_xml(CONFIG["film_title"], 144, 700, x=400, y=300)
    prod_c_main = ET.SubElement(root, "producer", {"id": "prod_c_main", "in": "0", "out": str(dur_c - 1)})
    ET.SubElement(prod_c_main, "property", {"name": "mlt_service"}).text = "kdenlivetitle"
    ET.SubElement(prod_c_main, "property", {"name": "xmldata"}).text = title_c_main_xml

    cast_pids = []
    for i, (char, actor) in enumerate(CONFIG["cast"].items()):
        pid = f"prod_cast_{i}"
        xml = create_title_xml(f"{char} . . . {actor}", 48, x=600, y=550 + (i * 70))
        p = ET.SubElement(root, "producer", {"id": pid, "in": "0", "out": str(dur_c - 1)})
        ET.SubElement(p, "property", {"name": "mlt_service"}).text = "kdenlivetitle"
        ET.SubElement(p, "property", {"name": "xmldata"}).text = xml
        cast_pids.append(pid)

    # --- SEGMENT TRACTORS ---

    # Segment A Tractor
    tractor_a = ET.SubElement(root, "tractor", {"id": "tractor_a", "in": "0", "out": str(dur_a - 1)})
    ET.SubElement(tractor_a, "track", {"producer": "bg_charcoal"})
    ET.SubElement(tractor_a, "track", {"producer": "prod_a1"})
    ET.SubElement(tractor_a, "track", {"producer": "prod_a2"})

    # Compositing for A
    tr_a1 = ET.SubElement(tractor_a, "transition", {"in": "0", "out": str(dur_a - 1)})
    ET.SubElement(tr_a1, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(tr_a1, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(tr_a1, "property", {"name": "b_track"}).text = "1"

    tr_a2 = ET.SubElement(tractor_a, "transition", {"in": "0", "out": str(dur_a - 1)})
    ET.SubElement(tr_a2, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(tr_a2, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(tr_a2, "property", {"name": "b_track"}).text = "2"

    # CINEMATIC FILTERS FOR A
    f_blur_a = ET.SubElement(tractor_a, "filter", {"in": "0", "out": "50"})
    ET.SubElement(f_blur_a, "property", {"name": "mlt_service"}).text = "boxblur"
    ET.SubElement(f_blur_a, "property", {"name": "hori"}).text = "0=20; 50=0"
    ET.SubElement(f_blur_a, "property", {"name": "vert"}).text = "0=20; 50=0"

    f_vignette_a = ET.SubElement(tractor_a, "filter", {"in": "0", "out": str(dur_a - 1)})
    ET.SubElement(f_vignette_a, "property", {"name": "mlt_service"}).text = "frei0r.vignette"
    ET.SubElement(f_vignette_a, "property", {"name": "0"}).text = "0.5" # Aspect
    ET.SubElement(f_vignette_a, "property", {"name": "1"}).text = "0.2" # Sharpness

    f_glow_a = ET.SubElement(tractor_a, "filter", {"in": "0", "out": str(dur_a - 1)})
    ET.SubElement(f_glow_a, "property", {"name": "mlt_service"}).text = "frei0r.glow"
    ET.SubElement(f_glow_a, "property", {"name": "0"}).text = "0.2" # Blur

    f_scale_a = ET.SubElement(tractor_a, "filter", {"in": "0", "out": str(dur_a - 1)})
    ET.SubElement(f_scale_a, "property", {"name": "mlt_service"}).text = "affine"
    ET.SubElement(f_scale_a, "property", {"name": "rect"}).text = f"0=192 -108 {width*0.8} {height*0.8} 100; {dur_a-1}=0 0 {width} {height} 100"

    # Segment B Tractor
    tractor_b = ET.SubElement(root, "tractor", {"id": "tractor_b", "in": "0", "out": str(dur_b - 1)})
    ET.SubElement(tractor_b, "track", {"producer": "bg_black"})
    ET.SubElement(tractor_b, "track", {"producer": "prod_b"})

    # Compositing for B
    tr_b = ET.SubElement(tractor_b, "transition", {"in": "0", "out": str(dur_b - 1)})
    ET.SubElement(tr_b, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(tr_b, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(tr_b, "property", {"name": "b_track"}).text = "1"

    f_vignette_b = ET.SubElement(tractor_b, "filter", {"in": "0", "out": str(dur_b - 1)})
    ET.SubElement(f_vignette_b, "property", {"name": "mlt_service"}).text = "frei0r.vignette"

    f_drift_b = ET.SubElement(tractor_b, "filter", {"in": "0", "out": str(dur_b - 1)})
    ET.SubElement(f_drift_b, "property", {"name": "mlt_service"}).text = "affine"
    ET.SubElement(f_drift_b, "property", {"name": "transition.rect"}).text = f"0=0 0 {width} {height} 100; {dur_b-1}=0 -20 {width} {height} 100"

    # Segment C Tractor
    tractor_c = ET.SubElement(root, "tractor", {"id": "tractor_c", "in": "0", "out": str(dur_c - 1)})
    ET.SubElement(tractor_c, "track", {"producer": "bg_black"})
    ET.SubElement(tractor_c, "track", {"producer": "prod_c_main"})
    for pid in cast_pids:
        ET.SubElement(tractor_c, "track", {"producer": pid})

    # Compositing for C Main
    tr_c_main = ET.SubElement(tractor_c, "transition", {"in": "0", "out": str(dur_c - 1)})
    ET.SubElement(tr_c_main, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(tr_c_main, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(tr_c_main, "property", {"name": "b_track"}).text = "1"

    f_vignette_c = ET.SubElement(tractor_c, "filter", {"in": "0", "out": str(dur_c - 1)})
    ET.SubElement(f_vignette_c, "property", {"name": "mlt_service"}).text = "frei0r.vignette"

    f_burst_c = ET.SubElement(tractor_c, "filter", {"in": "0", "out": str(dur_c - 1)})
    ET.SubElement(f_burst_c, "property", {"name": "mlt_service"}).text = "frei0r.light_burst"
    ET.SubElement(f_burst_c, "property", {"name": "0"}).text = "0.2" # Intensity

    for i, pid in enumerate(cast_pids):
        start = i * 8
        trans = ET.SubElement(tractor_c, "transition", {"in": str(start), "out": str(dur_c - 1)})
        ET.SubElement(trans, "property", {"name": "mlt_service"}).text = "composite"
        ET.SubElement(trans, "property", {"name": "a_track"}).text = "0"
        ET.SubElement(trans, "property", {"name": "b_track"}).text = str(i + 2)
        ET.SubElement(trans, "property", {"name": "geometry"}).text = "0=0/0:100%x100%:0; 25=0/0:100%x100%:100"

    # --- MAIN TIMELINE ---
    overlap = 25

    pl_a = ET.SubElement(root, "playlist", {"id": "pl_a"})
    ET.SubElement(pl_a, "entry", {"producer": "tractor_a"})

    pl_b = ET.SubElement(root, "playlist", {"id": "pl_b"})
    ET.SubElement(pl_b, "blank", {"length": str(dur_a - overlap)})
    ET.SubElement(pl_b, "entry", {"producer": "tractor_b"})

    pl_c = ET.SubElement(root, "playlist", {"id": "pl_c"})
    ET.SubElement(pl_c, "blank", {"length": str(dur_a + dur_b - 2 * overlap)})
    ET.SubElement(pl_c, "entry", {"producer": "tractor_c"})

    main_timeline = ET.SubElement(root, "tractor", {"id": "main_timeline"})
    ET.SubElement(main_timeline, "track", {"producer": "pl_a"})
    ET.SubElement(main_timeline, "track", {"producer": "pl_b"})
    ET.SubElement(main_timeline, "track", {"producer": "pl_c"})

    # Segment Transitions
    t1 = ET.SubElement(main_timeline, "transition", {"in": str(dur_a - overlap), "out": str(dur_a - 1)})
    ET.SubElement(t1, "property", {"name": "mlt_service"}).text = "luma"
    ET.SubElement(t1, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(t1, "property", {"name": "b_track"}).text = "1"

    t2 = ET.SubElement(main_timeline, "transition", {"in": str(dur_a + dur_b - 2 * overlap), "out": str(dur_a + dur_b - overlap - 1)})
    ET.SubElement(t2, "property", {"name": "mlt_service"}).text = "luma"
    ET.SubElement(t2, "property", {"name": "a_track"}).text = "1"
    ET.SubElement(t2, "property", {"name": "b_track"}).text = "2"

    # Final Output
    output_dir = Path(__file__).parent / CONFIG["output_dir"]
    output_dir.mkdir(parents=True, exist_ok=True)
    output_file = output_dir / "header.kdenlive"

    indent(root)
    tree = ET.ElementTree(root)
    tree.write(output_file, encoding="utf-8", xml_declaration=True)

    total_frames = dur_a + dur_b + dur_c - 2 * overlap
    print(f"[credits] header.kdenlive written — {total_frames} frames, {4 + len(cast_pids)} producers, 3 filters")

if __name__ == "__main__":
    generate_header()
