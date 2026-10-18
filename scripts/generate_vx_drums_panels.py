#!/usr/bin/env python3
"""
Generate the VX Drums / VX Drum Sequencer panel SVGs (light and dark).

Regenerate from the repository root:

    python3 scripts/generate_vx_drums_panels.py                 # SVGs, validation, dark pass
    python3 scripts/generate_vx_drums_panels.py --render-png    # ...plus PNG previews (Windows Inkscape)
    python3 scripts/generate_vx_drums_panels.py --ids-file F    # ...plus a dump of the anchor ids

Outputs (overwritten every run; the output is byte-for-byte deterministic, so a second
run changes nothing):

    res/modules/vx_drums/vx_drums_panel.svg
    res/modules/vx_drums/vx_drums_panel-dark.svg
    res/modules/vx_drum_sequencer/vx_drum_sequencer_panel.svg
    res/modules/vx_drum_sequencer/vx_drum_sequencer_panel-dark.svg
    --render-png: docs/modules/vx-drums/front-panel.png, front-panel-dark.png
                  docs/modules/vx-drum-sequencer/front-panel.png, front-panel-dark.png

Do not hand-edit the SVGs. Change the geometry tables (kit_panel / sequencer_panel) below
and rerun; the widgets read every position from the light SVG through
PanelHelper::findNamed / findNamedRect, so the ids emitted here are the contract with
src/modules/VXDrums/VXDrumsWidget.hpp and src/modules/VXDrumSequencer/VXDrumSequencerWidget.hpp.

This file is the two geometry tables and a main. Everything else -- the house constants,
the embedded Pilat outlines and kerning, the text layout, the SVG emitter, the light and
dark validators, the dark pass, the PNG rendering -- is scripts/panel_gen/, shared with
scripts/generate_euclid_panel.py since 2026-09-16 (docs/implementation_plans/
euclid-migration.md section 7). The panel width is a per-table value there (panel["hp"]
-> panel_gen.constants.PanelSize); this script's N_HP feeds both tables. The move changed
nothing in the output: the files this script writes are byte-identical to the ones the
single-file generator wrote.

How the file is built (brief-panel-svg.md, 2026-09-01):
  * 33 HP = 167.64 mm = 495 px, viewBox 1:1 with Rack pixels (75 dpi), height 128.5 mm as
    every other Voxglitch panel.
  * Labels are Pilat Bold 7 pt outlines emitted glyph by glyph from the embedded outline and
    kerning tables (brief Appendix C; Procedure A, brief section 5.4); the boxed module name
    is Pilat Condensed Bold 6 pt (Appendix D). The font files are NOT needed at run time and
    must not be added to the repository.
  * The VOXGLITCH wordmark and the module-name frame are the house artwork quoted verbatim
    from res/modules/arpseq/arpseq_panel.svg (path50) and
    res/modules/cue_research/cue_research_panel.svg (path6-5) (brief Appendix A / B).
  * Jack anchors are #213d42 circles r 11.764; knob/switch/button/light anchors are #d8d6cf
    dots r 2.5 (res/modules/timeline/timeline_panel.svg idiom); named rects read with
    findNamedRect are plain rects.
  * The light file is validated (unique ids, anchors are untransformed top-level circles /
    rects in the anchors layer with the agreed id suffixes, no anchor centre inside a screw
    square, no <text>, no url(#, only colours scripts/generate_dark_panel.py maps plus
    #d8d6cf / #17171a, width mm == viewBox px / 2.952756), then
    scripts/generate_dark_panel.py is run on it to produce the -dark.svg.
  * generate_dark_panel.py strips every element whose id ends in _display, and the kit's
    KIT readout plate (kit_display, read with findNamedRect) must stay dark in both themes,
    so that rect (#17171a, a colour the dark map passes through) is re-inserted into the
    dark file after the pass; the dark file is then verified (anchors stripped, colours
    mapped, no light-theme colour left). The sequencer has no readout plate.
  * Screw squares: the kit mounts only the top pair and the sequencer only the bottom pair
    (they stack as one machine), but both tables leave panel["screws"] at its default, so
    the validator keeps anchors out of all four squares, as the single-file generator did.

The geometry is DESIGN.md section 3.4 (kit) and 4.8 (sequencer), owner-approved 2026-09-01,
plus the KIT readout and the uniform TUNE / DECAY / SHAPE / LEVEL labels from DESIGN-KITS.md
section 6 (2026-09-02), and the sequencer's external-clock-only re-layout (owner decision
2026-09-02: the INT switch, BPM knob and readout, RUN switch and SWING knob were removed).
"""

import argparse
import sys
from pathlib import Path

from panel_gen import (hrule, PORT_R, DOT_R, COL_PLATE, COL_MARK, COL_DOT, COL_DARK_PLATE,
                       JACK_LABEL_DY, KNOB_LABEL_DY,
                       PanelError, circle, rect, rule, label, label_id_for, slug,
                       self_test, load_dark_generator, build_panel, ids_report_lines)


N_HP = 33                       # was 30; +3 HP on 2026-09-02 for the kit's CV jack column


# ----------------------------------------------------------------------------------------
# Geometry tables (DESIGN.md 3.4 and 4.8). Every number here is a Rack pixel.
# ----------------------------------------------------------------------------------------

def kit_panel():
    """VX Drums (the kit): 7 columns x 4 knob rows, strike lights, jack row. DESIGN.md 3.4."""
    cols = [93, 152, 211, 270, 329, 388, 447]     # +45 px: the CV jack column sits at x 40
    titles = ["BASS DRUM", "SNARE", "CLAP", "PERC", "CLOSED HAT", "OPEN HAT", "MASTER"]
    voices = ["bd", "sd", "cp", "perc", "ch", "oh"]
    # DESIGN-KITS.md 6: every voice column carries the same four labels. The third knob is
    # SHAPE whatever the column's model makes of it (punch, snap, spread, drive, metal...);
    # the tooltip names the model's own parameter.
    knob_params = ["tune", "decay", "shape", "level"]
    # Shifted down 18 px on 2026-09-02 (Bret: the top was cramped, the bottom had room);
    # the bottom screws are gone from this panel, so the jack row may sit this low.
    knob_rows = [102, 156, 210, 264]
    title_baseline = 76
    light_cy = 62
    master_cx = cols[6]
    master = [("m_accent_knob", "ACCENT", knob_rows[0]),
              ("m_vary_knob", "VARY", knob_rows[1]),        # per-hit random spread (Bret, 2026-09-02)
              ("m_drive_knob", "DRIVE", knob_rows[2]),
              ("m_volume_knob", "VOLUME", knob_rows[3])]
    jack_cy = 346
    jack_label_baseline = jack_cy - JACK_LABEL_DY            # 324.7
    # Three inputs at the kit's 40 px jack pitch, centred in the space left of the outputs
    # plate (x 0..157, centre 78.5): TRIG, ACC, and KIT CV (a user request, 2026-09-18).
    inputs = [("trig_input", "TRIG", 38.5), ("acc_input", "ACC", 78.5), ("kit_cv_input", "KIT", 118.5)]
    # Individual outs first, the stereo mix at the right end (Bret, 2026-09-02).
    outputs = [("bd_output", "BD", 177), ("sd_output", "SD", 217), ("cp_output", "CP", 257),
               ("perc_output", "PERC", 297), ("ch_output", "CH", 337), ("oh_output", "OH", 377),
               ("left_output", "L", 417), ("right_output", "R", 457)]
    # Poly CV inputs, one per knob row, in a column left of the knobs (Bret, 2026-09-02):
    # channel c drives column c's knob on that row, channel 7 the master knob, mono normals
    # to all. Labelled below the jack on the knob-label baseline of the row.
    cv_inputs = [("tune_cv_input", "TUNE CV", knob_rows[0]), ("decay_cv_input", "DECAY CV", knob_rows[1]),
                 ("shape_cv_input", "SHAPE CV", knob_rows[2]), ("level_cv_input", "LEVEL CV", knob_rows[3])]
    cv_cx = 40

    # The KIT readout (DESIGN-KITS.md 6): a dark plate top right, read with findNamedRect and
    # painted over by VXDrumsKitDisplay; a KIT label to its left. Dark in BOTH themes, so it
    # is re-inserted after the dark pass (see reinsert_display_rects). It stays
    # clear of the top-right screw square (x 465-480, y 0-15): the rect starts at y 18.
    kit_display = (363, 18, 116, 20)
    kit_label_cx, kit_label_baseline = 348, 30

    # Vertical rules between the six drum columns (Bret, 2026-09-02), at the column
    # midpoints, from just above knob row 1 to just below the LEVEL labels.
    rules = [rule("column_rule_%d" % (i + 1), (cols[i] + cols[i + 1]) / 2.0,
                  knob_rows[0] - 12, knob_rows[3] + KNOB_LABEL_DY + 6)
             for i in range(5)]
    rects = [rect("outputs_plate", 157, 312, 320, 58, 4.5, COL_PLATE, "background"),
             rect("kit_display", kit_display[0], kit_display[1], kit_display[2], kit_display[3], 3,
                  COL_DARK_PLATE, "anchors")]
    anchors = []
    labels = [label("label_kit", "KIT", kit_label_cx, kit_label_baseline)]
    for i, title in enumerate(titles):
        labels.append(label("label_title_" + slug(title), title, cols[i], title_baseline))
    for i, v in enumerate(voices):
        cx = cols[i]
        anchors.append(circle(v + "_light", cx, light_cy, DOT_R, COL_DOT))
        for k, p in enumerate(knob_params):
            cy = knob_rows[k]
            aid = "%s_%s_knob" % (v, p)
            anchors.append(circle(aid, cx, cy, DOT_R, COL_DOT))
            labels.append(label(label_id_for(aid), p.upper(), cx, cy + KNOB_LABEL_DY))
    for aid, text, cy in master:
        anchors.append(circle(aid, master_cx, cy, DOT_R, COL_DOT))
        labels.append(label(label_id_for(aid), text, master_cx, cy + KNOB_LABEL_DY))
    for aid, text, cx in inputs + outputs:
        anchors.append(circle(aid, cx, jack_cy, PORT_R, COL_MARK))
        labels.append(label(label_id_for(aid), text, cx, jack_label_baseline))
    for aid, text, cy in cv_inputs:
        anchors.append(circle(aid, cv_cx, cy, PORT_R, COL_MARK))
        labels.append(label(label_id_for(aid), text, cv_cx, cy + KNOB_LABEL_DY))

    return {
        "name": "VX DRUMS",
        "stem": "vx_drums_panel",
        "res_dir": "vx_drums",
        "docs_dir": "vx-drums",
        "widget": "src/modules/VXDrums/VXDrumsWidget.hpp",
        "generator": "scripts/generate_vx_drums_panels.py",
        "hp": N_HP,                      # screws: default (all four squares checked; see the docstring)
        "rects": rects,
        "rules": rules,
        "anchors": anchors,
        "labels": labels,
        "notes": [
            "Vertical rules (1.2 px ink) between the six drum columns at x 122.5 181.5 240.5 299.5 358.5, y 90-293.",
            "CV column at x 40: tune_cv_input / decay_cv_input / shape_cv_input / level_cv_input on the knob rows (poly).",
            "Kit strip: 7 columns at cx 93 152 211 270 329 388 447 (BD SD CP PERC CH OH MASTER); titles baseline 76.",
            "Strike lights (MediumLight) at (cx, 62) for the six voices. Knob rows cy 102 156 210 264 (RoundBlackKnob),",
            "labels at cy + 23, TUNE DECAY SHAPE LEVEL on every voice column; the master column has ACCENT (row 1),",
            "VARY (row 2), DRIVE (row 3), VOLUME (row 4).",
            "kit_display (363,18) 116 x 20 dark plate, rx 3, top right (findNamedRect; VXDrumsKitDisplay paints it);",
            "KIT label ink-centred at cx 348, baseline 30. The top-right screw square is x 465-480, y 0-15.",
            "Jack row cy 346, labels at 324.7: TRIG 38.5, ACC 78.5, KIT 118.5 (inputs, no plate, centred left of the",
            "plate; KIT = kit select CV, 2026-09-18); outputs on the #d2d2d3 plate",
            "at pitch 40: BD 177, SD 217, CP 257, PERC 297, CH 337, OH 377, L 417, R 457.",
        ],
    }


def sequencer_panel():
    """VX Drum Sequencer: grid area, 16 memory buttons, one control/jack row with the outputs.
    DESIGN.md 4.8 as revised 2026-09-02: external clock only (the INT switch, BPM knob,
    BPM readout, RUN switch and SWING knob are gone), so the freed height goes to the grid."""
    mem_cy = 262
    mem_label_baseline = 282
    ctrl_cy = 340
    ctrl_label_baseline = ctrl_cy - JACK_LABEL_DY            # 318.7
    controls = [("clk_input", "CLK", 40, "jack"), ("rst_input", "RST", 80, "jack"),
                ("mem_input", "MEM", 120, "jack"), ("twk_input", "TWK", 160, "jack"),
                ("rewind_button", "RWD", 216, "dot"), ("random_button", "RND", 256, "dot"),
                ("rnd_input", "", 296, "jack"),     # the RND trigger jack (poly), tied to the button by a rule (2026-09-18)
                ("trig_output", "TRIG", 425, "jack"), ("acc_output", "ACC", 465, "jack")]
    # The link: centre to centre under the two components, so only the 16 px between the
    # button's rim and the jack's rim shows.
    rules = [hrule("rnd_link_rule", ctrl_cy, 256, 296)]

    rects = [rect("grid_area", 15, 42, 465, 200, 4.5, COL_PLATE, "background"),
             rect("outputs_plate", 403, 304, 82, 70, 4.5, COL_PLATE, "background")]   # extended down to y 374 (Bret, 2026-09-02); it may crowd the bottom-right screw
    anchors = []
    labels = []
    for n in range(1, 17):
        cx = 38 + 28 * (n - 1)      # 16 buttons across the 33 HP panel, 38..458
        aid = "mem_%d_button" % n
        anchors.append(circle(aid, cx, mem_cy, DOT_R, COL_DOT))
        labels.append(label(label_id_for(aid), str(n), cx, mem_label_baseline))
    for aid, text, cx, kind in controls:
        anchors.append(circle(aid, cx, ctrl_cy, PORT_R if kind == "jack" else DOT_R,
                              COL_MARK if kind == "jack" else COL_DOT))
        if text:
            labels.append(label(label_id_for(aid), text, cx, ctrl_label_baseline))

    return {
        "name": "VX DRUM SEQ",
        "stem": "vx_drum_sequencer_panel",
        "res_dir": "vx_drum_sequencer",
        "docs_dir": "vx-drum-sequencer",
        "widget": "src/modules/VXDrumSequencer/VXDrumSequencerWidget.hpp",
        "generator": "scripts/generate_vx_drums_panels.py",
        "hp": N_HP,
        "rects": rects,
        "rules": rules,
        "anchors": anchors,
        "labels": labels,
        "notes": [
            "External clock only (owner decision 2026-09-02): no INT switch, BPM knob, BPM readout, RUN switch or SWING knob.",
            "grid_area (15,42) 465 x 200 is painted over by the grid widget; the plate is the browser fallback.",
            "Memory buttons mem_1..mem_16 at cx 38 + 28(n-1), cy 262 (VCVLightBezel), numbers at baseline 282.",
            "One control row cy 340, labels at 318.7: CLK 40, RST 80, MEM 120, TWK 160 (jacks; TWK added 2026-09-08); RWD 216, RND 256 (VCVButton),",
            "rnd_input 296 (poly trigger jack, unlabelled, tied to RND by a 1.2 px rule at cy 340; 2026-09-18);",
            "outputs plate (403,304) 82 x 70 with TRIG 425 (poly) and ACC 465. Only the bottom screw squares",
            "(x 15-30 and 465-480, y 365-380) are on this panel; the top strip holds the wordmark / name box alone.",
        ],
    }



# ----------------------------------------------------------------------------------------
# main
# ----------------------------------------------------------------------------------------

def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--repo-root", default=None, help="repository root (default: parent of this script's directory)")
    ap.add_argument("--render-png", action="store_true", help="render 900 px PNG previews into docs/modules/<kebab>/ via Windows Inkscape")
    ap.add_argument("--ids-file", default=None, help="write the anchor / named-rect id list (grouped per panel) to this path")
    args = ap.parse_args(argv)

    scripts_dir = Path(__file__).resolve().parent
    root = Path(args.repo_root).resolve() if args.repo_root else scripts_dir.parent
    color_map = load_dark_generator(scripts_dir).COLOR_MAP

    try:
        self_test()
        print("self-test: embedded glyph tables reproduce the reference label widths")
        ids_report = []
        for panel in (kit_panel(), sequencer_panel()):
            anchor_ids = build_panel(panel, root, scripts_dir, color_map, render_pngs=args.render_png)
            ids_report.extend(ids_report_lines(panel, anchor_ids))
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
