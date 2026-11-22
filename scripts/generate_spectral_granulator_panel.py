#!/usr/bin/env python3
"""
Generate the Spectral Granulator panel SVGs (light and dark).

Regenerate from the repository root:

    python3 scripts/generate_spectral_granulator_panel.py                 # SVGs, validation, dark pass
    python3 scripts/generate_spectral_granulator_panel.py --render-png    # ...plus PNG previews (Windows Inkscape)
    python3 scripts/generate_spectral_granulator_panel.py --ids-file F    # ...plus a dump of the anchor ids

Outputs (overwritten every run; byte-for-byte deterministic):

    res/modules/spectral_granulator/spectral_granulator_panel.svg
    res/modules/spectral_granulator/spectral_granulator_panel-dark.svg
    --render-png: docs/modules/spectral-granulator/front-panel.png, front-panel-dark.png

Do not hand-edit the SVGs. Change the geometry table below and rerun; the widget
(src/modules/SpectralGranulator/SpectralGranulatorWidget.hpp) reads every position from the light SVG
through PanelHelper::findNamed / findNamedRect, so the ids emitted here are the contract:

    focus_knob width_knob smooth_knob sens_knob pitch_knob tune_knob step_knob resync_knob   RoundBlackKnob, row A
    stretch_knob grain_knob blend_knob feedback_knob jitter_knob density_knob spread_knob imprint_knob   row B
    stretch_atten_knob grain_atten_knob blend_atten_knob                      Trimpot, under their knobs
    <key>_cv_input for each of the sixteen knobs                              Voxglitch ports, two CV rows
    in_l_input in_r_input trig_input                                          Voxglitch ports, bottom row
    onset_output level_output bright_output tonal_output out_l_output out_r_output   on the outputs plate
    spectrum_display                                                          the display's plate (findNamedRect)

The pipeline (house constants, Pilat outlines, emitter, validators, dark pass, PNG rendering)
is scripts/panel_gen/, shared with the Euclid and VX Drums generators.

The panel, 24 HP (360 px). Knobs on top, CV underneath, in the same order (owner, 2026-11-21):

  * Wordmark at its house position with the SPECTRAL GRANULATOR frame beside it (Euclid's placement),
    widened to 60.5 px for the long name.
  * spectrum_display: a 342 x 50 dark plate under the branding, dark in both themes.
  * Eight columns at 44 px pitch (cx 26 .. 334), labels above the knobs (Euclid):
      row A   FOCUS WIDTH SMOOTH SENS | PITCH TUNE STEP RESYNC   (a rule parts the analysis from the grains)
      row B   STRETCH GRAIN BLEND FEEDBACK JITTER DENSITY SPREAD IMPRINT
    and a Trimpot under STRETCH / GRAIN / BLEND, tied to its knob by a 1.2 px rule: the
    steering amount for that knob's CV jack.
  * A full-width rule, then two CV rows on the same eight columns: one jack per knob in the
    knob's own column and row order, each labelled with the knob's name.
  * The bottom row, nine jacks at 38 px pitch: IN L, IN R, TRIG (inputs, no plate), then
    ONSET LEVEL BRIGHT TONAL OUT L OUT R on the outputs plate (the VX Drums idiom).
  * At least 6 px between neighbouring ink at every row boundary; the table checks it.
"""

import argparse
import sys
from pathlib import Path

from panel_gen import (PORT_R, DOT_R, COL_PLATE, COL_MARK, COL_DOT, COL_DARK_PLATE,
                       JACK_LABEL_DY, SCREWS_ALL,
                       PanelError, circle, rect, rule, hrule, label, label_id_for,
                       self_test, load_dark_generator, build_panel, ids_report_lines)


N_HP = 24                       # 360 px

KNOB_R = 28.3 / 2.0             # RoundBlackKnob : 14.15
TRIMPOT_R = 17.9 / 2.0          # Trimpot : 8.95
LABEL_CAP = 7.0 * 75 / 72 * 0.7 # cap height of a 7 pt label: 5.10 px above its baseline

FRAME_XY = (74.8, 12.85)        # the name frame beside the wordmark (Euclid's placement)
FRAME_W = 60.5                  # the name's ink is 52.3 px; the house frame is 39.5 (panel_gen frame_w)


def spectral_granulator_panel():
    """Spectral Granulator: display plate, two knob rows, the steering trimpots, two CV rows, the I/O row."""
    cols = [26 + 44 * i for i in range(8)]               # 26 .. 334

    display = (9, 30, 342, 50)                           # y 30..80

    row_a_cy = 114
    row_a_label = row_a_cy - KNOB_R - 7.85               # 92
    row_b_cy = 162
    row_b_label = row_b_cy - KNOB_R - 7.85               # 140
    atten_cy = 192
    cv_rule_y = 208.3                                    # parts the knobs from the CV jacks
    cv_a_cy = 242
    cv_a_label = cv_a_cy - JACK_LABEL_DY                 # 220.7
    cv_b_cy = 287
    cv_b_label = cv_b_cy - JACK_LABEL_DY                 # 265.7
    io_cy = 338
    io_label = io_cy - JACK_LABEL_DY                     # 316.7
    io_cols = [28 + 38 * i for i in range(9)]            # 28 .. 332
    outputs_plate = (124, 305, 227, 55)                  # around the six outputs, y 305..360

    # FEEDBACK is the widest label (44 px on a 44 px pitch): it sits between BLEND and
    # JITTER, two short ones. SENS, a short one, is the label next to the analysis rule.
    row_a = [("focus", "FOCUS"), ("width", "WIDTH"), ("smooth", "SMOOTH"), ("sens", "SENS"),
             ("pitch", "PITCH"), ("tune", "TUNE"), ("step", "STEP"), ("resync", "RESYNC")]
    row_b = [("stretch", "STRETCH"), ("grain", "GRAIN"), ("blend", "BLEND"), ("feedback", "FEEDBACK"),
             ("jitter", "JITTER"), ("density", "DENSITY"), ("spread", "SPREAD"), ("imprint", "IMPRINT")]
    io = [("in_l_input", "IN L"), ("in_r_input", "IN R"), ("trig_input", "TRIG"),
          ("onset_output", "ONSET"), ("level_output", "LEVEL"), ("bright_output", "BRIGHT"),
          ("tonal_output", "TONAL"), ("out_l_output", "OUT L"), ("out_r_output", "OUT R")]

    rects = [rect("outputs_plate", outputs_plate[0], outputs_plate[1], outputs_plate[2], outputs_plate[3],
                  4.5, COL_PLATE, "background"),
             rect("spectrum_display", display[0], display[1], display[2], display[3],
                  3, COL_DARK_PLATE, "anchors")]
    anchors = []
    labels = []

    # A rule between SENS and PITCH: the four analysis knobs to its left, the grains to its right.
    split_x = (cols[3] + cols[4]) / 2.0
    rules = [rule("analysis_rule", split_x, row_a_label - LABEL_CAP, row_a_cy + KNOB_R),
             hrule("cv_rule", cv_rule_y, display[0], display[0] + display[2])]

    for row, knob_cy, knob_label, cv_cy, cv_label in ((row_a, row_a_cy, row_a_label, cv_a_cy, cv_a_label),
                                                      (row_b, row_b_cy, row_b_label, cv_b_cy, cv_b_label)):
        for cx, (key, text) in zip(cols, row):
            aid = key + "_knob"
            anchors.append(circle(aid, cx, knob_cy, DOT_R, COL_DOT))
            labels.append(label(label_id_for(aid), text, cx, knob_label))
            cid = key + "_cv_input"
            anchors.append(circle(cid, cx, cv_cy, PORT_R, COL_MARK))
            labels.append(label(label_id_for(cid), text, cx, cv_label))

    for cx, (key, _text) in zip(cols[:3], row_b[:3]):     # STRETCH / GRAIN / BLEND: the steering amount
        anchors.append(circle(key + "_atten_knob", cx, atten_cy, DOT_R, COL_DOT))
        rules.append(rule(key + "_column_rule", cx, row_b_cy, atten_cy))

    for cx, (aid, text) in zip(io_cols, io):
        anchors.append(circle(aid, cx, io_cy, PORT_R, COL_MARK))
        labels.append(label(label_id_for(aid), text, cx, io_label))

    gaps = [
        ("branding -> display", display[1] - (FRAME_XY[1] + 9.39)),
        ("display -> row A label", row_a_label - LABEL_CAP - (display[1] + display[3])),
        ("row A label -> knob", row_a_cy - KNOB_R - row_a_label),
        ("row A knob -> row B label", row_b_label - LABEL_CAP - (row_a_cy + KNOB_R)),
        ("row B knob -> trimpot", atten_cy - TRIMPOT_R - (row_b_cy + KNOB_R)),
        ("trimpot -> CV rule", cv_rule_y - 0.6 - (atten_cy + TRIMPOT_R)),
        ("CV rule -> CV row A label", cv_a_label - LABEL_CAP - (cv_rule_y + 0.6)),
        ("CV row A jack -> CV row B label", cv_b_label - LABEL_CAP - (cv_a_cy + PORT_R)),
        ("CV row B jack -> plate", outputs_plate[1] - (cv_b_cy + PORT_R)),
        ("plate -> I/O label", io_label - LABEL_CAP - outputs_plate[1]),
        ("I/O jack -> plate bottom", outputs_plate[1] + outputs_plate[3] - (io_cy + PORT_R)),
        ("plate left -> first output jack", io_cols[3] - PORT_R - outputs_plate[0]),
        ("TRIG jack -> plate left", outputs_plate[0] - (io_cols[2] + PORT_R)),
        ("plate bottom -> bottom screws", 365 - (outputs_plate[1] + outputs_plate[3])),
    ]
    for name, gap in gaps:
        least = 3.0 if (name.endswith("screws") or "plate left" in name) else 6.0
        if gap < least:
            raise PanelError("spectral_granulator_panel: %s gap is %.2f px, needs %.0f" % (name, gap, least))

    return {
        "name": "SPECTRAL GRANULATOR",
        "stem": "spectral_granulator_panel",
        "res_dir": "spectral_granulator",
        "docs_dir": "spectral-granulator",
        "widget": "src/modules/SpectralGranulator/SpectralGranulatorWidget.hpp",
        "generator": "scripts/generate_spectral_granulator_panel.py",
        "hp": N_HP,
        "screws": SCREWS_ALL,
        "frame_xy": FRAME_XY,
        "frame_w": FRAME_W,
        "rects": rects,
        "rules": rules,
        "anchors": anchors,
        "labels": labels,
        "notes": [
            "Wordmark at its house position; the SPECTRAL GRANULATOR frame beside it at (74.8, 12.85), 60.5 px wide.",
            "spectrum_display (9,30) 342 x 50 dark plate, rx 3; the display widget paints it (findNamedRect) and it",
            "stays dark in both themes (re-inserted after the dark pass).",
            "Eight columns at cx 26 + 44 i, labels above the knobs. Row A cy 114: FOCUS WIDTH SMOOTH SENS, a rule at",
            "x 180, PITCH TUNE STEP RESYNC. Row B cy 162: STRETCH GRAIN BLEND FEEDBACK JITTER DENSITY SPREAD IMPRINT.",
            "Trimpots cy 192 under STRETCH GRAIN BLEND, each tied to its knob by a column rule.",
            "A full-width rule at y 208.3, then CV rows cy 242 and 287 on the same columns, one <key>_cv_input per",
            "knob, labelled with the knob's name.",
            "Bottom row cy 338, nine jacks at cx 28 + 38 i: IN L, IN R, TRIG, then ONSET LEVEL BRIGHT TONAL OUT L",
            "OUT R on the (124,305) 227 x 55 outputs plate.",
        ],
    }


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--repo-root", default=None, help="repository root (default: parent of this script's directory)")
    ap.add_argument("--render-png", action="store_true", help="render PNG previews into docs/modules/spectral-granulator/ via Windows Inkscape")
    ap.add_argument("--ids-file", default=None, help="write the anchor / named-rect id list to this path")
    args = ap.parse_args(argv)

    scripts_dir = Path(__file__).resolve().parent
    root = Path(args.repo_root).resolve() if args.repo_root else scripts_dir.parent
    color_map = load_dark_generator(scripts_dir).COLOR_MAP

    try:
        self_test()
        print("self-test: embedded glyph tables reproduce the reference label widths")
        panel = spectral_granulator_panel()
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
