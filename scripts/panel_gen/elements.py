"""
panel_gen.elements -- the geometry-table vocabulary and the SVG emitter.

What: a geometry table is a plain dict a generator builds from small helpers (circle,
rect, rule, label) and a few strings; emit_svg turns one into the light-theme file. The
table's shape is the contract with the rest of the package:

    name      the boxed module name (Pilat Condensed Bold, house frame)
    stem      file stem: res/modules/<res_dir>/<stem>.svg and <stem>-dark.svg
    res_dir   directory under res/modules/
    docs_dir  directory under docs/modules/ for the --render-png previews
    widget    the ModuleWidget header that reads the anchors (quoted in the SVG header)
    generator the script that wrote the file (quoted in the SVG header and the dark file)
    hp        width in HP (panel_gen.constants.PanelSize)
    screws    optional: which screw pairs the panel mounts, default all four
    rects     rect(...) entries: layer "background" for #d2d2d3 plates read with
              findNamedRect (ids *_area / *_plate), layer "anchors" for the #17171a
              readout plates (ids *_display) that stay dark in both themes
    rules     optional rule(...) entries: vertical silkscreen lines
    anchors   circle(...) entries: jack markers (r PORT_R, #213d42) and control dots
              (r DOT_R, #d8d6cf), ids suffixed _input/_output/_knob/_button/_switch/_light
    labels    label(...) entries: Pilat Bold outlines ink-centred on cx at a baseline
    notes     free lines for the SVG header comment

Why a dict and not a class: the generators are scripts the owner edits by changing
numbers in a table; the fewer abstractions between the number and the file, the better.
validate_light reads the same dict back against the emitted XML, so a typo in a table is
an error, not a silently misplaced control.

The file layout (four Inkscape layers, top-level untransformed anchors, no <text>, no
url(#), only colours generate_dark_panel.py maps) is brief-panel-svg.md sections 2-4 as
the first generator, scripts/generate_vx_drums_panels.py, wrote it on 2026-09-01; the
emitter is that script's, with the width and the generator name taken from the table
instead of module globals so that a 33 HP and an 8 HP panel come out of the same code.
"""

import re

from .constants import (PANEL_H_MM, RAW2PX, PORT_R, DOT_R,
                        COL_BG, COL_INK, COL_ART, INK_OPACITY,
                        SIZE_LABEL, TRACK_LABEL, SIZE_NAME, WORD_NAME,
                        WORDMARK_X, WORDMARK_Y, FRAME_X, FRAME_Y,
                        WORDMARK_RAW_MIN, FRAME_RAW_BBOX, NAME_CAP_PX,
                        SVG_NS, INK_NS, SODI_NS, PLACEHOLDER_SUFFIXES, panel_size)
from .glyphs import LABEL_GLYPHS, LABEL_KERN, NAME_GLYPHS, NAME_KERN, WORDMARK_D, FRAME_D
from .text import text_paths


# ----------------------------------------------------------------------------------------
# Geometry-table vocabulary. Every number is a Rack pixel.
# ----------------------------------------------------------------------------------------

def circle(eid, cx, cy, r, fill):
    return {"tag": "circle", "id": eid, "cx": cx, "cy": cy, "r": r, "fill": fill}


def rect(eid, x, y, w, h, rx, fill, layer):
    return {"tag": "rect", "id": eid, "x": x, "y": y, "w": w, "h": h, "rx": rx, "fill": fill, "layer": layer}


def rule(eid, x, y1, y2, width=1.2):
    """A vertical silkscreen rule (a <line> in ink colour) in the background layer."""
    return {"id": eid, "x": x, "y1": y1, "y2": y2, "width": width}


def hrule(eid, y, x1, x2, width=1.2):
    """A horizontal silkscreen rule, same layer and ink (VX Drum Sequencer's RND link, 2026-09-18)."""
    return {"id": eid, "x": x1, "x2": x2, "y1": y, "y2": y, "width": width}


def label(eid, text, cx, baseline):
    return {"id": eid, "text": text, "cx": cx, "baseline": baseline}


def label_id_for(anchor_id):
    """label id for an anchor: strip the placeholder suffix so the label never matches one."""
    return "label_" + re.sub(r"_(%s)$" % "|".join(PLACEHOLDER_SUFFIXES), "", anchor_id)


def slug(text):
    return re.sub(r"[^A-Za-z0-9]+", "_", text).strip("_").lower()


# ----------------------------------------------------------------------------------------
# SVG emission
# ----------------------------------------------------------------------------------------

def fmt(v):
    """Compact decimal: 306.7, 11.764, 379.4286, 40 (never scientific notation)."""
    if isinstance(v, int):
        return str(v)
    s = ("%.4f" % v).rstrip("0").rstrip(".")
    return "0" if s in ("", "-0") else s


def frame_geometry(frame_xy=None):
    """Frame placement in Rack px from the raw bbox (Appendix B) and the FRAME_X/Y target.

    frame_xy overrides the house position (a panel too narrow to stack the frame under the
    wordmark puts it to the wordmark's right: Euclid, 2026-09-18)."""
    x0, y0, x1, y1 = FRAME_RAW_BBOX
    w = (x1 - x0) * RAW2PX
    h = (y1 - y0) * RAW2PX
    fx, fy = frame_xy if frame_xy else (FRAME_X, FRAME_Y)
    return fx, fy, w, h


def emit_branding(name, frame_xy=None):
    fx, fy, fw, fh = frame_geometry(frame_xy)
    name_cx = fx + fw / 2.0
    name_baseline = fy + fh / 2.0 + NAME_CAP_PX / 2.0      # name ink centred in the frame
    wm_tx = WORDMARK_X - RAW2PX * WORDMARK_RAW_MIN[0]
    wm_ty = WORDMARK_Y - RAW2PX * WORDMARK_RAW_MIN[1]
    fr_tx = fx - RAW2PX * FRAME_RAW_BBOX[0]
    fr_ty = fy - RAW2PX * FRAME_RAW_BBOX[1]
    lines = []
    lines.append('    <!-- VOXGLITCH wordmark (arpseq path50), ink top-left at (%s, %s) -->' % (fmt(WORDMARK_X), fmt(WORDMARK_Y)))
    lines.append('    <g id="voxglitch_wordmark" transform="matrix(%.7f,0,0,%.7f,%.4f,%.4f)">' % (RAW2PX, RAW2PX, wm_tx, wm_ty))
    lines.append('      <path id="voxglitch_wordmark_path" style="opacity:0.78;fill:%s;fill-opacity:1;stroke:none" d="%s"/>' % (COL_ART, WORDMARK_D))
    lines.append('    </g>')
    lines.append('    <!-- module-name frame (cue_research path6-5) at (%s, %s), %s x %s -->' % (fmt(fx), fmt(fy), fmt(fw), fmt(fh)))
    lines.append('    <g id="module_name_box" transform="matrix(%.7f,0,0,%.7f,%.4f,%.4f)">' % (RAW2PX, RAW2PX, fr_tx, fr_ty))
    lines.append('      <path id="module_name_box_path" style="fill:%s;fill-opacity:%s;stroke:none" d="%s"/>' % (COL_INK, INK_OPACITY, FRAME_D))
    lines.append('    </g>')
    lines.append('    <!-- module name, Pilat Condensed Bold %s px, ink-centred in the frame -->' % fmt(SIZE_NAME))
    lines.append(text_paths(name, name_cx, name_baseline, NAME_GLYPHS, NAME_KERN, SIZE_NAME,
                            tracking=0.0, word_extra=WORD_NAME, elem_id="module_name_text"))
    return lines


def emit_rect(r, indent="    "):
    return '%s<rect id="%s" x="%s" y="%s" width="%s" height="%s" rx="%s" fill="%s"/>' % (
        indent, r["id"], fmt(r["x"]), fmt(r["y"]), fmt(r["w"]), fmt(r["h"]), fmt(r["rx"]), r["fill"])


def emit_circle(c, indent="    "):
    return '%s<circle id="%s" cx="%s" cy="%s" r="%s" fill="%s"/>' % (
        indent, c["id"], fmt(c["cx"]), fmt(c["cy"]), fmt(c["r"]), c["fill"])


def emit_svg(panel):
    """The light-theme file for one geometry table, as a string (deterministic: the same
    table always gives the same bytes, so write_if_changed can leave the file alone)."""
    size = panel_size(panel)
    stem = panel["stem"]
    lines = []
    lines.append('<?xml version="1.0" encoding="UTF-8" standalone="no"?>')
    lines.append('<!--')
    lines.append('  Voxglitch "%s" panel. Generated by %s; do not hand-edit, regenerate.' % (panel["name"], panel["generator"]))
    lines.append('  %d HP = %s mm = %d px (RACK_GRID_WIDTH 15 x %d). Height 128.5 mm to match the other Voxglitch panels.' % (size.n_hp, fmt(size.w_mm), size.w_px, size.n_hp))
    lines.append('  viewBox is 1:1 with Rack pixels, so every coordinate here IS a Rack coordinate.')
    if any(r["layer"] == "anchors" for r in panel["rects"]):
        lines.append('  Dark variant: python3 scripts/generate_dark_panel.py res/modules/%s/%s.svg (the generator script runs it' % (panel["res_dir"], stem))
        lines.append('  and re-inserts the *_display plate, which the dark pass strips).')
    else:
        lines.append('  Dark variant: python3 scripts/generate_dark_panel.py res/modules/%s/%s.svg (the generator script runs it).' % (panel["res_dir"], stem))
    lines.append('  Ids ending _input/_output/_knob/_button/_switch/_light are the contract with %s' % panel["widget"])
    lines.append('  (PanelHelper::findNamed); %s are read with findNamedRect.' % ", ".join(r["id"] for r in panel["rects"]))
    lines.append('  House style: flat background; rounded plates (rx 4.5) behind output clusters only; labels are Pilat Bold 7 pt')
    lines.append('  outlines (no text elements); jack markers r %s; knob/switch/button/light anchor dots r %s (timeline idiom).' % (fmt(PORT_R), fmt(DOT_R)))
    lines.append('  Every colour is a key of the generate_dark_panel.py COLOR_MAP (plus the dot and dark-plate pass-throughs).')
    lines.append('  Screw squares %s are kept clear of anchors.' % size.describe_screw_squares())
    for note in panel["notes"]:
        lines.append('  ' + note)
    lines.append('-->')
    lines.append('<svg version="1.1" id="%s"' % stem)
    lines.append('     width="%smm" height="%smm" viewBox="%s"' % (fmt(size.w_mm), fmt(PANEL_H_MM), size.viewbox))
    lines.append('     xmlns="%s" xmlns:svg="%s"' % (SVG_NS, SVG_NS))
    lines.append('     xmlns:inkscape="%s"' % INK_NS)
    lines.append('     xmlns:sodipodi="%s"' % SODI_NS)
    lines.append('     sodipodi:docname="%s.svg">' % stem)
    lines.append('  <sodipodi:namedview id="namedview1" pagecolor="#ffffff" inkscape:document-units="mm"></sodipodi:namedview>')

    lines.append('  <g inkscape:groupmode="layer" inkscape:label="background" id="layer_background">')
    lines.append('    <rect id="panel_background" x="0" y="0" width="%d" height="%s" fill="%s"/>' % (size.w_px, fmt(size.h_px), COL_BG))
    lines.append('    <!-- plates only behind output clusters (house rule); named rects for code-drawn widgets (findNamedRect) -->')
    for r in panel["rects"]:
        if r["layer"] == "background":
            lines.append(emit_rect(r))
    for rl in panel.get("rules", []):
        lines.append('    <line id="%s" x1="%s" y1="%s" x2="%s" y2="%s" stroke="%s" stroke-opacity="%s" stroke-width="%s"/>' % (
            rl["id"], fmt(rl["x"]), fmt(rl["y1"]), fmt(rl.get("x2", rl["x"])), fmt(rl["y2"]), COL_INK, INK_OPACITY, fmt(rl["width"])))
    lines.append('  </g>')

    lines.append('  <g inkscape:groupmode="layer" inkscape:label="branding" id="layer_branding">')
    lines.extend(emit_branding(panel["name"], panel.get("frame_xy")))
    lines.append('  </g>')

    lines.append('  <g inkscape:groupmode="layer" inkscape:label="anchors" id="layer_anchors">')
    lines.append('    <!-- widget anchors: top-level, untransformed; PanelHelper reads the centre of each id (light file only) -->')
    for a in panel["anchors"]:
        lines.append(emit_circle(a))
    for r in panel["rects"]:
        if r["layer"] == "anchors":
            lines.append(emit_rect(r))
    lines.append('  </g>')

    lines.append('  <g inkscape:groupmode="layer" inkscape:label="labels" id="layer_labels">')
    lines.append('    <!-- Pilat Bold %s px outlines, tracking %s px, GPOS kerning; ink-centred on cx, baseline as given -->' % (fmt(SIZE_LABEL), fmt(TRACK_LABEL)))
    for lb in panel["labels"]:
        lines.append(text_paths(lb["text"], lb["cx"], lb["baseline"], LABEL_GLYPHS, LABEL_KERN, SIZE_LABEL,
                                tracking=TRACK_LABEL, word_extra=0.0, elem_id=lb["id"]))
    lines.append('  </g>')
    lines.append('</svg>')
    return "\n".join(lines) + "\n"
