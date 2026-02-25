#!/usr/bin/env python3
"""
Generates the opening header sequence KDenlive-compatible XML project for GreenhouseMD.
Uses pango producers with explicit sizes and composite transitions.
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

def create_text_producer(root, pid, text, size, weight=400, color="white", out=1000):
    """Creates a pango text producer."""
    p = ET.SubElement(root, "producer", {"id": pid, "in": "0", "out": str(out)})
    ET.SubElement(p, "property", {"name": "mlt_service"}).text = "pango"
    ET.SubElement(p, "property", {"name": "markup"}).text = f'<span font_family="DejaVu Sans" foreground="{color}" weight="{weight}" size="{size*1024}">{text}</span>'
    ET.SubElement(p, "property", {"name": "align"}).text = "centre"
    ET.SubElement(p, "property", {"name": "width"}).text = str(CONFIG["width"])
    ET.SubElement(p, "property", {"name": "height"}).text = str(CONFIG["height"])
    ET.SubElement(p, "property", {"name": "bgcolour"}).text = "#00000000"
    ET.SubElement(p, "property", {"name": "progressive"}).text = "1"
    return p

def generate_header():
    """Generates the header sequence XML."""
    fps = CONFIG["fps"]
    width = CONFIG["width"]
    height = CONFIG["height"]

    root = ET.Element("mlt", {
        "LC_NUMERIC": "C",
        "version": "7.22.0",
        "title": "Header Credits",
        "producer": "main_timeline",
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

    dur_a = CONFIG["header_segment_a_duration"] * fps
    dur_b = CONFIG["header_segment_b_duration"] * fps
    dur_c = CONFIG["header_segment_c_duration"] * fps
    total_dur = dur_a + dur_b + dur_c

    # --- PRODUCERS ---

    bg_charcoal = ET.SubElement(root, "producer", {"id": "bg_charcoal", "in": "0", "out": str(total_dur)})
    ET.SubElement(bg_charcoal, "property", {"name": "mlt_service"}).text = "color"
    ET.SubElement(bg_charcoal, "property", {"name": "resource"}).text = CONFIG["background_dark"]

    bg_black = ET.SubElement(root, "producer", {"id": "bg_black", "in": "0", "out": str(total_dur)})
    ET.SubElement(bg_black, "property", {"name": "mlt_service"}).text = "color"
    ET.SubElement(bg_black, "property", {"name": "resource"}).text = CONFIG["background_black"]

    create_text_producer(root, "prod_a1", "GreenhouseMD", 120, "bold", out=dur_a)
    create_text_producer(root, "prod_a2", "Production Studio", 48, "normal", out=dur_a)
    create_text_producer(root, "prod_b", f"{CONFIG['studio_name']} presents...\nA {CONFIG['co_production']}", 60, "normal", out=dur_b)
    create_text_producer(root, "prod_c_main", CONFIG["film_title"], 144, "bold", out=dur_c)
    cast_pids = []
    for i, (char, actor) in enumerate(CONFIG["cast"].items()):
        pid = f"prod_cast_{i}"
        create_text_producer(root, pid, f"{char} . . . {actor}", 48, "normal", out=dur_c)
        cast_pids.append(pid)

    # --- TRACTORS ---

    tractor_a = ET.SubElement(root, "tractor", {"id": "tractor_a", "in": "0", "out": str(dur_a - 1)})
    ET.SubElement(tractor_a, "track", {"producer": "bg_charcoal"})
    ET.SubElement(tractor_a, "track", {"producer": "prod_a1"})
    ET.SubElement(tractor_a, "track", {"producer": "prod_a2"})

    for i in [1, 2]:
        tr = ET.SubElement(tractor_a, "transition", {"in": "0", "out": str(dur_a - 1)})
        ET.SubElement(tr, "property", {"name": "mlt_service"}).text = "composite"
        ET.SubElement(tr, "property", {"name": "a_track"}).text = "0"
        ET.SubElement(tr, "property", {"name": "b_track"}).text = str(i)
        y_off = 0 if i == 1 else 200
        ET.SubElement(tr, "property", {"name": "geometry"}).text = f"0: 0/{y_off}:{width}x{height}:100"

    f_blur_a = ET.SubElement(tractor_a, "filter", {"in": "0", "out": "50"})
    ET.SubElement(f_blur_a, "property", {"name": "mlt_service"}).text = "boxblur"
    ET.SubElement(f_blur_a, "property", {"name": "hori"}).text = "0: 30; 50: 0"
    ET.SubElement(f_blur_a, "property", {"name": "vert"}).text = "0: 30; 50: 0"

    f_glow_a = ET.SubElement(tractor_a, "filter", {"in": "0", "out": str(dur_a - 1)})
    ET.SubElement(f_glow_a, "property", {"name": "mlt_service"}).text = "frei0r.glow"
    ET.SubElement(f_glow_a, "property", {"name": "0"}).text = "0.1"

    f_shimmer_a = ET.SubElement(tractor_a, "filter", {"in": "0", "out": str(dur_a - 1)})
    ET.SubElement(f_shimmer_a, "property", {"name": "mlt_service"}).text = "brightness"
    ET.SubElement(f_shimmer_a, "property", {"name": "level"}).text = "0: 1.0; 60: 1.2; 120: 1.0; 180: 1.2; 240: 1.0; 299: 1.2"

    f_scale_a = ET.SubElement(tractor_a, "filter", {"in": "0", "out": str(dur_a - 1)})
    ET.SubElement(f_scale_a, "property", {"name": "mlt_service"}).text = "affine"
    ET.SubElement(f_scale_a, "property", {"name": "rect"}).text = f"0: 192 -108 {width*0.8} {height*0.8} 100; {dur_a-1}: 0 0 {width} {height} 100"

    tractor_b = ET.SubElement(root, "tractor", {"id": "tractor_b", "in": "0", "out": str(dur_b - 1)})
    ET.SubElement(tractor_b, "track", {"producer": "bg_black"})
    ET.SubElement(tractor_b, "track", {"producer": "prod_b"})

    tr_b = ET.SubElement(tractor_b, "transition", {"in": "0", "out": str(dur_b - 1)})
    ET.SubElement(tr_b, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(tr_b, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(tr_b, "property", {"name": "b_track"}).text = "1"
    ET.SubElement(tr_b, "property", {"name": "geometry"}).text = f"0: 0/0:{width}x{height}:100"

    f_drift_b = ET.SubElement(tractor_b, "filter", {"in": "0", "out": str(dur_b - 1)})
    ET.SubElement(f_drift_b, "property", {"name": "mlt_service"}).text = "affine"
    ET.SubElement(f_drift_b, "property", {"name": "transition.rect"}).text = f"0: 0 0 {width} {height} 100; {dur_b-1}: 0 -20 {width} {height} 100"

    tractor_c = ET.SubElement(root, "tractor", {"id": "tractor_c", "in": "0", "out": str(dur_c - 1)})
    ET.SubElement(tractor_c, "track", {"producer": "bg_black"})
    ET.SubElement(tractor_c, "track", {"producer": "prod_c_main"})
    for pid in cast_pids:
        ET.SubElement(tractor_c, "track", {"producer": pid})

    tr_c_main = ET.SubElement(tractor_c, "transition", {"in": "0", "out": str(dur_c - 1)})
    ET.SubElement(tr_c_main, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(tr_c_main, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(tr_c_main, "property", {"name": "b_track"}).text = "1"
    ET.SubElement(tr_c_main, "property", {"name": "geometry"}).text = f"0: 0/-100:{width}x{height}:100"

    for i, pid in enumerate(cast_pids):
        start = i * 8
        trans = ET.SubElement(tractor_c, "transition", {"in": str(start), "out": str(dur_c - 1)})
        ET.SubElement(trans, "property", {"name": "mlt_service"}).text = "composite"
        ET.SubElement(trans, "property", {"name": "a_track"}).text = "0"
        ET.SubElement(trans, "property", {"name": "b_track"}).text = str(i + 2)
        ET.SubElement(trans, "property", {"name": "geometry"}).text = f"0: 0/{150 + i*70}:{width}x{height}:0; 25: 0/{150 + i*70}:{width}x{height}:100"

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

    total_frames = dur_a + dur_b + dur_c - 2 * overlap
    main_timeline = ET.SubElement(root, "tractor", {"id": "main_timeline", "in": "0", "out": str(total_frames - 1)})
    ET.SubElement(main_timeline, "track", {"producer": "pl_a"})
    ET.SubElement(main_timeline, "track", {"producer": "pl_b"})
    ET.SubElement(main_timeline, "track", {"producer": "pl_c"})

    for i in [1, 2]:
        t = ET.SubElement(main_timeline, "transition", {"in": "0", "out": str(total_frames - 1)})
        ET.SubElement(t, "property", {"name": "mlt_service"}).text = "composite"
        ET.SubElement(t, "property", {"name": "a_track"}).text = "0"
        ET.SubElement(t, "property", {"name": "b_track"}).text = str(i)
        ET.SubElement(t, "property", {"name": "geometry"}).text = "0: 0/0:100%x100%:0; 25: 0/0:100%x100%:100"

    output_dir = Path(__file__).parent / CONFIG["output_dir"]
    output_dir.mkdir(parents=True, exist_ok=True)
    output_file = output_dir / "header.kdenlive"
    indent(root)
    ET.ElementTree(root).write(output_file, encoding="utf-8", xml_declaration=True)
    print(f"[credits] header.kdenlive written — {total_frames} frames")

if __name__ == "__main__":
    generate_header()
