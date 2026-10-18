#!/usr/bin/env python3
"""
Generate the Euclid panel SVGs (light and dark).

Regenerate from the repository root:

    python3 scripts/generate_euclid_panel.py                 # SVGs, validation, dark pass
    python3 scripts/generate_euclid_panel.py --render-png    # ...plus PNG previews (Windows Inkscape)
    python3 scripts/generate_euclid_panel.py --ids-file F    # ...plus a dump of the anchor ids

Outputs (overwritten every run; the output is byte-for-byte deterministic, so a second
run changes nothing):

    res/modules/euclid/euclid_panel.svg
    res/modules/euclid/euclid_panel-dark.svg
    --render-png: docs/modules/euclid/front-panel.png, front-panel-dark.png

Do not hand-edit the SVGs. Change the geometry table (euclid_panel) below and rerun; the
widget reads every position from the light SVG through PanelHelper::findNamed /
findNamedRect, so the ids emitted here are the contract with
src/modules/Euclid/EuclidWidget.hpp:

    hits_knob steps_knob rot_knob                      RoundBlackKnob, 28.3 px
    hits_atten_knob steps_atten_knob rot_atten_knob    Trimpot, 17.9 px
    hits_input steps_input rot_input clk_input rst_input   Voxglitch ports, 23.5 px
    trig_output                                        Voxglitch port, on the outputs plate
    ring_display                                       the ring widget's plate (findNamedRect)

This file is one geometry table and a main; the pipeline (house constants, embedded Pilat
outlines, text layout, SVG emitter, validators, dark pass, PNG rendering) is
scripts/panel_gen/, shared with scripts/generate_vx_drums_panels.py. The output contract
and the house style are that script's (brief-panel-svg.md, 2026-09-01): viewBox 1:1 with
Rack pixels, height 128.5 mm, Pilat Bold 7 pt label outlines, #213d42 jack anchors r 11.764,
#d8d6cf control dots r 2.5, only colours scripts/generate_dark_panel.py maps.

The panel (docs/implementation_plans/euclid-migration.md section 3 decisions 1-2 and
section 7, taken as recommended on 2026-09-16):
  * 9 HP = 45.72 mm = 135 px (8 HP until 2026-09-18, when the owner dropped GATE, moved
    every jack to one bottom row and asked for a bigger ring). A standalone module, so all
    four screws; the validator keeps every anchor out of the four 15 x 15 squares and the
    TRIG plate ends above the bottom pair (y 352 < 365).
  * Module name EUCLID in the house frame, placed to the RIGHT of the wordmark (the frame
    no longer sits under it: that row is what the ring gained), panel["frame_xy"].
  * ring_display: a 117 x 117 plate, #17171a, centred (x 9..126), just below the branding.
    It is dark in BOTH themes like the VX kit's kit_display (the ring's colours are fixed,
    not theme-mapped: plan section 6), so it is re-inserted after the dark pass. The ring
    widget paints it and reads its rect with findNamedRect.
  * Three columns at cx 22.5 / 67.5 / 112.5 (45 px pitch): the knob row HITS / STEPS / ROT with
    its label ABOVE the knob, the attenuverter row, then the CV jack row. Neither the
    attenuverter nor the CV jack carries a label: a 1.2 px vertical rule from the knob
    centre to the jack centre, drawn under the components, ties the three together so a
    column reads name -> knob -> amount -> CV in (owner, 2026-09-18).
  * One bottom row on the same columns: CLK / RST (inputs, no plate, labels above) and
    TRIG on the #d2d2d3 outputs plate (the VX kit's plate idiom).
  * At least 6 px between neighbouring ink at every row boundary (the table's comments
    carry the arithmetic).
"""

import argparse
import sys
from pathlib import Path

from panel_gen import (PORT_R, DOT_R, COL_PLATE, COL_MARK, COL_DOT, COL_DARK_PLATE,
                       JACK_LABEL_DY, SCREWS_ALL,
                       PanelError, circle, rect, rule, label, label_id_for,
                       self_test, load_dark_generator, build_panel, ids_report_lines)


N_HP = 9                        # 135 px, three 45 px columns (was 8 HP until 2026-09-18)

KNOB_R = 28.3 / 2.0             # RoundBlackKnob (componentlibrary.hpp) : 14.15
TRIMPOT_R = 17.9 / 2.0          # Trimpot : 8.95
LABEL_CAP = 7.0 * 75 / 72 * 0.7 # cap height of a 7 pt label: 5.10 px above its baseline

# The wordmark's ink is x 20.7..59.0, y 13.1..22.0 (arpseq path50 through RAW2PX); the
# frame (39.52 x 9.39) sits to its right with the same right margin the wordmark has on
# the left, vertically centred on the wordmark's ink.
FRAME_XY = (74.8, 12.85)


# ----------------------------------------------------------------------------------------
# Geometry table. Every number here is a Rack pixel.
# ----------------------------------------------------------------------------------------

def euclid_panel():
    """Euclid: ring plate, three control columns, one bottom jack row. 9 HP (2026-09-18)."""
    cols = [22.5, 67.5, 112.5]
    columns = [("hits", "HITS"), ("steps", "STEPS"), ("rot", "ROT")]

    # Branding: wordmark and frame side by side, ink bottom 22.24; the ring plate starts
    # 7.76 px below it and is as wide as the panel allows (9 px margins).
    ring_display = (9, 30, 117, 117)                     # x, y, w, h: square, centred on 67.5, y 30..147

    # Knob row, its label ABOVE the knob (owner, 2026-09-18): label ink 164.9..170, 17.9 px
    # below the ring plate; knob top 192 - 14.15 = 177.85, 7.85 px below the label.
    knob_cy = 192
    knob_label_baseline = knob_cy - KNOB_R - 7.85        # 170

    # Attenuverter row: trimpot 223.05..240.95, 16.9 px below the knob.
    atten_cy = 232

    # CV jack row, unlabelled: jack 256.24..279.76, 15.3 px below the trimpot. A 1.2 px rule
    # from the knob centre to the jack centre, under the components, ties each column's
    # knob -> attenuverter -> CV jack together (the VX kit's column-rule idiom).
    cv_cy = 268

    # Bottom row, all three jacks: CLK / RST (inputs, no plate) and TRIG on its own plate.
    # Label ink 301.6..306.7 (21.84 px below the CV jacks), jack 316.24..339.76.
    bottom_cy = 328
    bottom_label_baseline = bottom_cy - JACK_LABEL_DY    # 306.7

    # The TRIG plate y 295..352: the label ink starts 6.6 px inside it, the jack ends 12.24 px
    # above its bottom, and it ends 13 px above the bottom screw squares (y 365..380);
    # x 94..131 leaves 6.74 px beyond each jack edge.
    outputs_plate = (94, 295, 37, 57)

    rects = [rect("outputs_plate", outputs_plate[0], outputs_plate[1], outputs_plate[2], outputs_plate[3],
                  4.5, COL_PLATE, "background"),
             rect("ring_display", ring_display[0], ring_display[1], ring_display[2], ring_display[3],
                  3, COL_DARK_PLATE, "anchors")]
    rules = []
    anchors = []
    labels = []
    for cx, (key, text) in zip(cols, columns):
        aid = key + "_knob"
        anchors.append(circle(aid, cx, knob_cy, DOT_R, COL_DOT))
        labels.append(label(label_id_for(aid), text, cx, knob_label_baseline))
        anchors.append(circle(key + "_atten_knob", cx, atten_cy, DOT_R, COL_DOT))
        anchors.append(circle(key + "_input", cx, cv_cy, PORT_R, COL_MARK))
        rules.append(rule(key + "_column_rule", cx, knob_cy, cv_cy))
    for cx, (aid, text) in zip(cols, [("clk_input", "CLK"), ("rst_input", "RST"), ("trig_output", "TRIG")]):
        anchors.append(circle(aid, cx, bottom_cy, PORT_R, COL_MARK))
        labels.append(label(label_id_for(aid), text, cx, bottom_label_baseline))

    # The arithmetic the docstring promises: no two neighbouring rows closer than 6 px.
    gaps = [
        ("branding -> ring plate", ring_display[1] - (FRAME_XY[1] + 9.39)),
        ("ring plate -> knob label", knob_label_baseline - LABEL_CAP - (ring_display[1] + ring_display[3])),
        ("knob label -> knob", knob_cy - KNOB_R - knob_label_baseline),
        ("knob -> trimpot", atten_cy - TRIMPOT_R - (knob_cy + KNOB_R)),
        ("trimpot -> CV jack", cv_cy - PORT_R - (atten_cy + TRIMPOT_R)),
        ("CV jack -> bottom label", bottom_label_baseline - LABEL_CAP - (cv_cy + PORT_R)),
        ("plate -> TRIG label", bottom_label_baseline - LABEL_CAP - outputs_plate[1]),
        ("TRIG jack -> plate bottom", outputs_plate[1] + outputs_plate[3] - (bottom_cy + PORT_R)),
        ("plate bottom -> bottom screws", 365 - (outputs_plate[1] + outputs_plate[3])),
    ]
    for name, gap in gaps:
        least = 3.0 if name.endswith("screws") else 6.0     # the plate is not ink; 3 px from the screws is enough
        if gap < least:
            raise PanelError("euclid_panel: %s gap is %.2f px, needs %.0f" % (name, gap, least))

    return {
        "name": "EUCLID",
        "stem": "euclid_panel",
        "res_dir": "euclid",
        "docs_dir": "euclid",
        "widget": "src/modules/Euclid/EuclidWidget.hpp",
        "generator": "scripts/generate_euclid_panel.py",
        "hp": N_HP,
        "screws": SCREWS_ALL,           # standalone module: both pairs
        "frame_xy": FRAME_XY,           # the name frame beside the wordmark, not under it
        "rects": rects,
        "rules": rules,
        "anchors": anchors,
        "labels": labels,
        "notes": [
            "Wordmark at its house position (ink x 20.7..59.0); the EUCLID frame beside it at (74.8, 12.85).",
            "ring_display (9,30) 117 x 117 dark plate, rx 3, centred; the ring widget paints it (findNamedRect) and it",
            "stays dark in both themes (re-inserted after the dark pass).",
            "Three columns at cx 22.5 67.5 112.5 (HITS STEPS ROT): labels at 170 ABOVE the knobs, knobs cy 192",
            "(RoundBlackKnob), attenuverters cy 232 (Trimpot), CV jacks cy 268; all three unlabelled below the knob label,",
            "tied together by a 1.2 px column rule from the knob centre to the jack centre (drawn under the components).",
            "Bottom row cy 328, labels at 306.7: CLK RST (inputs, no plate) and TRIG on the (94,295) 37 x 57 plate.",
            "Row gaps (ink to ink): 7.76 17.9 7.85 16.9 15.3 21.84 px; the plate ends 13 px above the bottom screws.",
        ],
    }


# ----------------------------------------------------------------------------------------
# main
# ----------------------------------------------------------------------------------------

def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--repo-root", default=None, help="repository root (default: parent of this script's directory)")
    ap.add_argument("--render-png", action="store_true", help="render PNG previews (3 px per Rack px: 405 x 1138) into docs/modules/euclid/ via Windows Inkscape")
    ap.add_argument("--ids-file", default=None, help="write the anchor / named-rect id list to this path")
    args = ap.parse_args(argv)

    scripts_dir = Path(__file__).resolve().parent
    root = Path(args.repo_root).resolve() if args.repo_root else scripts_dir.parent
    color_map = load_dark_generator(scripts_dir).COLOR_MAP

    try:
        self_test()
        print("self-test: embedded glyph tables reproduce the reference label widths")
        panel = euclid_panel()
        anchor_ids = build_panel(panel, root, scripts_dir, color_map, render_pngs=args.render_png)
        ids_report = ids_report_lines(panel, anchor_ids)
    except PanelError as exc:
        print("\nERROR:\n%s" % exc, file=sys.stderr)
        return 1

    if args.ids_file:
        Path(args.ids_file).write_text("\n".join(ids_report), encoding="utf-8")
        print("\nids written to %s" % args.ids_file)
    print("\nALL OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
