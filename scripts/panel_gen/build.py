"""
panel_gen.build -- the per-panel pipeline: emit, validate, write, dark pass, verify, render.

What: build_panel(table, ...) does for one geometry table what the first generator's
main() loop did inline for the VX pair (scripts/generate_vx_drums_panels.py, 2026-09-01):

  1. emit_svg -> validate_light -> write_if_changed   res/modules/<res_dir>/<stem>.svg
  2. scripts/generate_dark_panel.py on that file      res/modules/<res_dir>/<stem>-dark.svg
  3. reinsert_display_rects -> validate_dark -> write (the dark pass strips every id
     ending in _display, and a readout plate must stay dark in both themes)
  4. optionally render_png through the Windows Inkscape into docs/modules/<docs_dir>/,
     at PNG_SCALE px per Rack px capped at PNG_MAX_WIDTH_PX, so an 8 HP panel and a
     33 HP panel share a scale in the manuals (a 120 px panel at a fixed 900 px width
     was 7.5x and 2846 px tall)

and prints the same run log the first generator printed. ids_report_lines() gives the
--ids-file block for the table.

Why here: a generator script should be its geometry tables and an argparse main and
nothing else, so a second module's panel is a second short script, not a second copy of
the pipeline. The dark pass is still the separate scripts/generate_dark_panel.py, run as
a subprocess exactly as before: that script is the house tool for every panel, hand-drawn
ones included, and is not touched.

Environment: the Windows Inkscape at INKSCAPE_WIN needs Windows paths (wslpath -w) and
cannot see /tmp; render_png reports instead of raising, so a run without Inkscape still
writes the SVGs.
"""

import os
import shutil
import subprocess
import sys
import re

from .constants import PORT_R, DOT_R, INKSCAPE_WIN, panel_size
from .elements import emit_svg, emit_rect
from .text import PanelError
from .validate import validate_light, validate_dark


# ----------------------------------------------------------------------------------------
# Dark pass
# ----------------------------------------------------------------------------------------

def run_dark_generator(scripts_dir, light_path):
    cmd = [sys.executable, str(scripts_dir / "generate_dark_panel.py"), str(light_path)]
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    if proc.returncode != 0:
        raise PanelError("generate_dark_panel.py failed (rc %d):\n%s" % (proc.returncode, proc.stdout))
    return proc.stdout


def reinsert_display_rects(dark_text, panel):
    """generate_dark_panel.py strips ids ending in _display; the readout plate must be dark in
    both themes, so put it back inside the (now empty) anchors layer. PanelHelper never reads
    the dark file, so the id is cosmetic there; keeping it lets Inkscape work see the same tree."""
    rects = [r for r in panel["rects"] if r["layer"] == "anchors"]
    if not rects:
        return dark_text
    m = re.search(r'<g[^>]*id="layer_anchors"[^>]*>', dark_text)
    if not m:
        raise PanelError("dark file has no layer_anchors group to re-insert into")
    block = "\n    <!-- re-inserted by %s: the dark pass strips *_display ids, and this plate must stay dark in both themes -->" % panel["generator"]
    for r in rects:
        block += "\n" + emit_rect(r)
    return dark_text[:m.end()] + block + dark_text[m.end():]


# ----------------------------------------------------------------------------------------
# PNG previews
# ----------------------------------------------------------------------------------------

PNG_SCALE = 3            # px per Rack px: 120 px -> 360 px, 495 px -> 1485 px before the cap
PNG_MAX_WIDTH_PX = 900   # the width the VX pair has always been rendered at


def png_width_for(panel):
    """The preview width for a geometry table: PNG_SCALE x the panel's pixel width, capped."""
    return min(PNG_MAX_WIDTH_PX, panel_size(panel).w_px * PNG_SCALE)


def wslpath_w(path):
    return subprocess.check_output(["wslpath", "-w", str(path)], universal_newlines=True).strip()


def render_png(svg_path, png_path, width_px):
    """A width_px wide preview through the Windows Inkscape; returns a status string, never raises."""
    if not os.path.exists(INKSCAPE_WIN):
        return "skipped (no %s)" % INKSCAPE_WIN
    if shutil.which("wslpath") is None:
        return "skipped (wslpath not available; not running under WSL)"
    png_path.parent.mkdir(parents=True, exist_ok=True)
    try:
        cmd = [INKSCAPE_WIN, "--export-type=png", "--export-width=%d" % int(width_px),
               "--export-filename=" + wslpath_w(png_path), wslpath_w(svg_path)]
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              universal_newlines=True, timeout=300)
    except (OSError, subprocess.SubprocessError) as exc:
        return "failed: %s" % exc
    if proc.returncode != 0 or not png_path.exists():
        return "failed (rc %d): %s" % (proc.returncode, proc.stdout.strip()[-400:])
    return "ok (%d bytes)" % png_path.stat().st_size


# ----------------------------------------------------------------------------------------
# Files
# ----------------------------------------------------------------------------------------

def write_if_changed(path, text):
    """Write only when the bytes differ, so an unchanged table leaves mtimes alone.
    Returns (created, changed)."""
    path.parent.mkdir(parents=True, exist_ok=True)
    old = path.read_text(encoding="utf-8") if path.exists() else None
    if old != text:
        path.write_text(text, encoding="utf-8")
    return old is None, old is not None and old != text


# ----------------------------------------------------------------------------------------
# The pipeline
# ----------------------------------------------------------------------------------------

def build_panel(panel, root, scripts_dir, color_map, render_pngs=False, png_width_px=None):
    """Emit, validate, write, dark-pass, verify and optionally render one panel.
    root is the repository root, scripts_dir the directory holding generate_dark_panel.py.
    png_width_px overrides the preview width; by default it is png_width_for(panel).
    Returns the anchor ids in file order (for ids_report_lines). Raises PanelError."""
    res_dir = root / "res" / "modules" / panel["res_dir"]
    light_path = res_dir / (panel["stem"] + ".svg")
    dark_path = res_dir / (panel["stem"] + "-dark.svg")

    light = emit_svg(panel)
    anchor_ids = validate_light(light, panel, color_map)
    created, changed = write_if_changed(light_path, light)
    print("\n== %s ==" % panel["name"])
    print("light: %s (%d bytes, %s)" % (light_path, len(light.encode("utf-8")),
                                        "created" if created else ("updated" if changed else "unchanged")))
    jacks = [a for a in panel["anchors"] if a["r"] == PORT_R]
    dots = [a for a in panel["anchors"] if a["r"] == DOT_R]
    print("  anchors: %d jacks, %d dots, %d display rect(s); named rects: %s; labels: %d"
          % (len(jacks), len(dots), sum(1 for r in panel["rects"] if r["layer"] == "anchors"),
             [r["id"] for r in panel["rects"] if r["layer"] == "background"], len(panel["labels"])))
    print("  light validation: OK")

    log = run_dark_generator(scripts_dir, light_path)
    print("  dark pass (scripts/generate_dark_panel.py):")
    for line in log.strip().splitlines():
        print("    " + line)
    dark = reinsert_display_rects(dark_path.read_text(encoding="utf-8"), panel)
    dark_colours = validate_dark(dark, panel, color_map)
    dark_path.write_text(dark, encoding="utf-8")
    print("dark:  %s (%d bytes); colours present: %s" % (dark_path, len(dark.encode("utf-8")), dark_colours))
    print("  dark validation: OK (anchors stripped, colours mapped, nothing unmapped)")

    if render_pngs:
        docs_dir = root / "docs" / "modules" / panel["docs_dir"]
        width_px = png_width_px if png_width_px else png_width_for(panel)
        for svg, png in ((light_path, docs_dir / "front-panel.png"), (dark_path, docs_dir / "front-panel-dark.png")):
            print("  png:   %s <- %s (%d px wide): %s" % (png, svg.name, width_px, render_png(svg, png, width_px)))
    return anchor_ids


def ids_report_lines(panel, anchor_ids):
    """The --ids-file block for one panel: anchors (findNamed) then named rects (findNamedRect)."""
    lines = ["# %s.svg (res/modules/%s): anchors, PanelHelper::findNamed" % (panel["stem"], panel["res_dir"])]
    lines.extend(i for i in anchor_ids if not i.endswith("_display"))
    lines.append("# %s.svg: named rects, PanelHelper::findNamedRect" % panel["stem"])
    lines.extend(r["id"] for r in panel["rects"])
    lines.append("")
    return lines
