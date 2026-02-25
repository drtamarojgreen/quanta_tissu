import xml.etree.ElementTree as ET
from pathlib import Path

CONFIG = {
    "film_title":        "The Greenhouse",
    "studio_name":       "GreenhouseMD Production Studio",
    "co_production":     "GreenhouseMD / GreenhouseMHD Production",
    "year":              2026,
    "fps":               25,
    "width":             1920,
    "height":            1080,
    "output_dir":        "output",
    "header_segment_a_duration": 12,
    "header_segment_b_duration": 15,
    "header_segment_c_duration": 15,
    "background_dark":   "#1a1a1a",
    "background_black":  "#000000",
    "text_white":        "#ffffff",
}

def create_pango_producer(root, id, text, size, weight="normal", out=100):
    prod = ET.SubElement(root, "producer", {"id": id, "in": "0", "out": str(out - 1)})
    ET.SubElement(prod, "property", {"name": "mlt_service"}).text = "pango"
    ET.SubElement(prod, "property", {"name": "align"}).text = "centre"
    markup = f'<span font_family="DejaVu Sans" foreground="white" weight="{weight}" size="{size*1024}">{text}</span>'
    ET.SubElement(prod, "property", {"name": "markup"}).text = markup
    ET.SubElement(prod, "property", {"name": "width"}).text = str(CONFIG["width"])
    ET.SubElement(prod, "property", {"name": "height"}).text = str(size * 3)
    return prod

def generate_header():
    root = ET.Element("mlt", {"LC_NUMERIC": "C", "version": "7.22.0"})
    width, height, fps = CONFIG["width"], CONFIG["height"], CONFIG["fps"]
    ET.SubElement(root, "profile", {"description": "atsc_1080p_25", "width": str(width), "height": str(height), "frame_rate_num": str(fps)})

    dur_a, dur_b, dur_c = CONFIG["header_segment_a_duration"] * fps, CONFIG["header_segment_b_duration"] * fps, CONFIG["header_segment_c_duration"] * fps
    overlap = fps # 1 second overlap for transitions

    # Segment A Producers
    bg_a = ET.SubElement(root, "producer", {"id": "bg_a", "out": str(dur_a - 1)})
    ET.SubElement(bg_a, "property", {"name": "mlt_service"}).text = "color"
    ET.SubElement(bg_a, "property", {"name": "resource"}).text = CONFIG["background_dark"]
    create_pango_producer(root, "a1", "GreenhouseMD", 120, "bold", out=dur_a)
    create_pango_producer(root, "a2", "Production Studio", 48, "normal", out=dur_a)

    # Segment B Producers
    bg_b = ET.SubElement(root, "producer", {"id": "bg_b", "out": str(dur_b - 1)})
    ET.SubElement(bg_b, "property", {"name": "mlt_service"}).text = "color"
    ET.SubElement(bg_b, "property", {"name": "resource"}).text = CONFIG["background_black"]
    create_pango_producer(root, "b1", f"{CONFIG['studio_name']} presents...\nA {CONFIG['co_production']}", 60, "normal", out=dur_b)

    # Segment C Producers
    bg_c = ET.SubElement(root, "producer", {"id": "bg_c", "out": str(dur_c - 1)})
    ET.SubElement(bg_c, "property", {"name": "mlt_service"}).text = "color"
    ET.SubElement(bg_c, "property", {"name": "resource"}).text = CONFIG["background_black"]
    create_pango_producer(root, "c_title", CONFIG["film_title"], 144, "bold", out=dur_c)

    # Segment A Tractor
    tractor_a = ET.SubElement(root, "tractor", {"id": "tractor_a", "out": str(dur_a - 1)})
    ET.SubElement(tractor_a, "track", {"producer": "bg_a"})
    ET.SubElement(tractor_a, "track", {"producer": "a1"})
    ET.SubElement(tractor_a, "track", {"producer": "a2"})
    tr_a1 = ET.SubElement(tractor_a, "transition", {"in": "0", "out": str(dur_a - 1)})
    ET.SubElement(tr_a1, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(tr_a1, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(tr_a1, "property", {"name": "b_track"}).text = "1"
    # Letter spacing / Scaling animation
    ET.SubElement(tr_a1, "property", {"name": "geometry"}).text = f"0=10%/40%:80%x30%:0; 50=0/40%:100%x30%:100"

    tr_a2 = ET.SubElement(tractor_a, "transition", {"in": "0", "out": str(dur_a - 1)})
    ET.SubElement(tr_a2, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(tr_a2, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(tr_a2, "property", {"name": "b_track"}).text = "2"
    ET.SubElement(tr_a2, "property", {"name": "geometry"}).text = f"0=0/60%:100%x10%:0; 75=0/60%:100%x10%:100"

    # Segment B Tractor
    tractor_b = ET.SubElement(root, "tractor", {"id": "tractor_b", "out": str(dur_b - 1)})
    ET.SubElement(tractor_b, "track", {"producer": "bg_b"})
    ET.SubElement(tractor_b, "track", {"producer": "b1"})
    tr_b = ET.SubElement(tractor_b, "transition", {"in": "0", "out": str(dur_b - 1)})
    ET.SubElement(tr_b, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(tr_b, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(tr_b, "property", {"name": "b_track"}).text = "1"
    # Vertical drift
    ET.SubElement(tr_b, "property", {"name": "geometry"}).text = f"0=0/44%:100%x30%:0; 25=0/44%:100%x30%:100; {dur_b-1}=0/42%:100%x30%:100"

    # Segment C Tractor
    tractor_c = ET.SubElement(root, "tractor", {"id": "tractor_c", "out": str(dur_c - 1)})
    ET.SubElement(tractor_c, "track", {"producer": "bg_c"})
    ET.SubElement(tractor_c, "track", {"producer": "c_title"})
    tr_c = ET.SubElement(tractor_c, "transition", {"in": "0", "out": str(dur_c - 1)})
    ET.SubElement(tr_c, "property", {"name": "mlt_service"}).text = "composite"
    ET.SubElement(tr_c, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(tr_c, "property", {"name": "b_track"}).text = "1"
    ET.SubElement(tr_c, "property", {"name": "geometry"}).text = f"0=0/30%:100%x40%:100"

    # Main Assembly Tractor (Overlapping for transitions)
    main_tractor = ET.SubElement(root, "tractor", {"id": "main_tractor"})
    # Track 0: Segment A and Segment C
    playlist0 = ET.SubElement(main_tractor, "track")
    ET.SubElement(playlist0, "entry", {"producer": "tractor_a", "in": "0", "out": str(dur_a - 1)})
    ET.SubElement(playlist0, "blank", {"length": str(dur_b - overlap)})
    ET.SubElement(playlist0, "entry", {"producer": "tractor_c", "in": "0", "out": str(dur_c - 1)})

    # Track 1: Segment B
    playlist1 = ET.SubElement(main_tractor, "track")
    ET.SubElement(playlist1, "blank", {"length": str(dur_a - overlap)})
    ET.SubElement(playlist1, "entry", {"producer": "tractor_b", "in": "0", "out": str(dur_b - 1)})

    # Crossfade/Wipe transitions in main tractor
    trans1 = ET.SubElement(main_tractor, "transition", {"in": str(dur_a - overlap), "out": str(dur_a)})
    ET.SubElement(trans1, "property", {"name": "mlt_service"}).text = "luma"
    ET.SubElement(trans1, "property", {"name": "a_track"}).text = "0"
    ET.SubElement(trans1, "property", {"name": "b_track"}).text = "1"

    trans2 = ET.SubElement(main_tractor, "transition", {"in": str(dur_a + dur_b - overlap), "out": str(dur_a + dur_b)})
    ET.SubElement(trans2, "property", {"name": "mlt_service"}).text = "luma"
    ET.SubElement(trans2, "property", {"name": "a_track"}).text = "1"
    ET.SubElement(trans2, "property", {"name": "b_track"}).text = "0"

    # Global filters
    f_glow = ET.SubElement(main_tractor, "filter")
    ET.SubElement(f_glow, "property", {"name": "mlt_service"}).text = "frei0r.glow"
    ET.SubElement(f_glow, "property", {"name": "blur"}).text = "0.02"

    output_dir = Path(__file__).parent / CONFIG["output_dir"]
    output_dir.mkdir(exist_ok=True)
    with open(output_dir / "header.kdenlive", "wb") as f:
        f.write(b'<?xml version="1.0" encoding="utf-8"?>\n')
        f.write(ET.tostring(root, encoding="utf-8"))
    print(f"[credits] header.kdenlive written")

if __name__ == "__main__":
    generate_header()
