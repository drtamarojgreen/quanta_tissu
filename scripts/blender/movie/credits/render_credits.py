#!/usr/bin/env python3
"""
Renders GreenhouseMD KDenlive project files into MP4 videos using MELT.
"""
import subprocess
import os
import sys
import argparse
from pathlib import Path

# Attempt to import generation functions from the same directory
try:
    from generate_header import generate_header
    from generate_final_credits import generate_final_credits
except ImportError:
    sys.path.append(str(Path(__file__).parent))
    from generate_header import generate_header
    from generate_final_credits import generate_final_credits

def detect_melt():
    """Detects the MELT binary on the system.

    Checks $MELT_BIN, then 'melt' in PATH, then common system paths.

    Returns:
        The path or command for the MELT binary.
    Raises:
        RuntimeError: If no MELT binary is found.
    """
    candidates = [
        os.environ.get("MELT_BIN"),
        "melt",
        "/usr/bin/melt",
        "/usr/local/bin/melt",
        "melt-7"
    ]
    for c in candidates:
        if not c:
            continue
        try:
            # Check if it runs and responds to version
            subprocess.run([c, "-version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            return c
        except (FileNotFoundError, PermissionError):
            continue

    raise RuntimeError("MELT binary not found. Please install MLT (melt) or set $MELT_BIN.")

import xml.etree.ElementTree as ET

def get_xml_duration(filepath):
    """Gets the duration (out frame) of the main producer in the MLT XML.

    Args:
        filepath: Path to the .kdenlive file.
    Returns:
        The 'out' frame number as an integer.
    """
    try:
        tree = ET.parse(filepath)
        root = tree.getroot()
        main_prod_id = root.get("producer")
        if main_prod_id:
            main_prod = root.find(f".//*[@id='{main_prod_id}']")
            if main_prod is not None:
                return int(main_prod.get("out", 0))
    except Exception:
        pass
    return None

def render_project(melt_bin, input_xml, output_mp4, dry_run=False):
    """Renders a single KDenlive project using MELT.

    Args:
        melt_bin: Path to the MELT binary.
        input_xml: Path to the input .kdenlive file.
        output_mp4: Path to the output .mp4 file.
        dry_run: If True, only prints the command without execution.
    """
    if not input_xml.exists():
        raise FileNotFoundError(f"Input XML {input_xml} does not exist. Run generation first.")

    duration_out = get_xml_duration(input_xml)

    # MELT command construction
    # libx264 with CRF 18 and preset medium for balanced speed/quality in sandbox
    # Use xvfb-run to provide a virtual X server for kdenlivetitle rendering
    cmd = [
        "xvfb-run", "-a",
        melt_bin,
        "-profile", "atsc_1080p_25",
        str(input_xml)
    ]

    if duration_out is not None:
        cmd.extend(["-out", str(duration_out)])

    cmd.extend([
        "-consumer", f"avformat:{output_mp4}",
        "vcodec=libx264", "crf=18", "preset=medium",
        "pix_fmt=yuv420p",
        "acodec=aac", "-audio", "0"
    ])

    print(f"\n--- Rendering {input_xml.name} -> {output_mp4.name} ---")
    if dry_run:
        print(f"[DRY RUN] Would execute: {' '.join(cmd)}")
        return

    print(f"Executing: {' '.join(cmd)}")

    # Run process with real-time stdout passthrough
    with subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True) as proc:
        if proc.stdout:
            for line in proc.stdout:
                print(line, end="")
        proc.wait()

    if proc.returncode != 0:
        raise RuntimeError(f"MELT failed with exit code {proc.returncode}")

    # Post-render validation
    if not output_mp4.exists():
        raise RuntimeError(f"Output file {output_mp4} was not created.")
    if output_mp4.stat().st_size < 100 * 1024: # 100 KB
        raise RuntimeError(f"Output file {output_mp4} is suspiciously small ({output_mp4.stat().st_size} bytes).")

    print(f"SUCCESS: Rendered {output_mp4}")

def main():
    """CLI entry point for rendering credits."""
    parser = argparse.ArgumentParser(description="Render GreenhouseMD credits projects via MELT.")
    parser.add_argument("--dry-run", action="store_true", help="Print MELT commands without executing them.")
    parser.add_argument("--segment", choices=["header", "credits", "all"], default="all",
                        help="Which segment to render (default: all).")
    args = parser.parse_args()

    base_dir = Path(__file__).parent
    output_dir = base_dir / "output"

    # Always regenerate XML before rendering to ensure they are up to date
    print("Regenerating KDenlive XML projects...")
    try:
        if args.segment in ["header", "all"]:
            generate_header()
        if args.segment in ["credits", "all"]:
            generate_final_credits()
    except Exception as e:
        print(f"ERROR during XML generation: {e}")
        sys.exit(1)

    # Detect MELT binary
    try:
        melt_bin = detect_melt()
    except RuntimeError as e:
        if args.dry_run:
            print(f"WARNING: {e} (Continuing dry-run with 'melt')")
            melt_bin = "melt"
        else:
            print(f"WARNING: {e}")
            print("Rendering cannot proceed without MELT. Skipping render stage.")
            return # Graceful degradation

    # Define jobs
    jobs = []
    if args.segment in ["header", "all"]:
        jobs.append((output_dir / "header.kdenlive", output_dir / "header.mp4"))
    if args.segment in ["credits", "all"]:
        jobs.append((output_dir / "final_credits.kdenlive", output_dir / "final_credits.mp4"))

    for input_xml, output_mp4 in jobs:
        try:
            render_project(melt_bin, input_xml, output_mp4, args.dry_run)
        except Exception as e:
            print(f"ERROR rendering {input_xml.name}: {e}")
            sys.exit(1)

if __name__ == "__main__":
    main()
