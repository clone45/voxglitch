"""
panel_gen -- the shared machinery behind Voxglitch's generated panel SVGs.

What: everything a panel generator needs that is not the panel's own geometry. A
generator script (scripts/generate_vx_drums_panels.py, scripts/generate_euclid_panel.py)
is a geometry table built from the vocabulary in panel_gen.elements plus an argparse
main that hands each table to panel_gen.build.build_panel. The output contract is the
one brief-panel-svg.md set on 2026-09-01: a light SVG whose viewBox is 1:1 with Rack
pixels, jack anchors as #213d42 circles r 11.764, control anchors as #d8d6cf dots r 2.5,
labels as embedded Pilat outlines, plates read with findNamedRect, and a dark SVG made by
scripts/generate_dark_panel.py with any *_display plate re-inserted after the pass.

Why a package: the first generator was one 1060-line file with the glyph tables, the
layout code, the emitter, the validators and the dark pass inline, all fixed at 33 HP.
Euclid (8 HP) needed the same pipeline at a different width, and copying the file would
have duplicated ~800 lines that must stay identical. So the shared parts moved here on
2026-09-16 (docs/implementation_plans/euclid-migration.md section 7, option 1), the
panel width became a per-table value (panel_gen.constants.PanelSize), and the VX script
kept only its two tables and its main. The VX output is byte-for-byte what it was.

Modules:
  constants  the house numbers (grid, colours, type, offsets) and PanelSize
  glyphs     the Pilat outline / kerning tables and the wordmark / frame artwork (data)
  text       glyph layout: text_paths, text_ink_width, label_ink_width, self_test, PanelError
  elements   circle / rect / rule / label / label_id_for / slug, fmt, emit_svg
  validate   validate_light, validate_dark, scan_colours, load_dark_generator
  build      run_dark_generator, reinsert_display_rects, render_png, write_if_changed,
             build_panel, ids_report_lines

Everything a generator script uses is re-exported here, so `from panel_gen import *`
style imports of the named symbols work; the scripts import by name.
"""

from .constants import (HP, RACK_GRID_HEIGHT, MM_PER_HP, PANEL_H_MM, MM2PX, RAW2PX, H_PX,
                        PORT_R, DOT_R, SCREW,
                        COL_BG, COL_PLATE, COL_INK, COL_ART, COL_MARK, COL_DOT, COL_DARK_PLATE,
                        INK_OPACITY, SIZE_LABEL, TRACK_LABEL, SIZE_NAME, WORD_NAME,
                        JACK_LABEL_DY, KNOB_LABEL_DY,
                        WORDMARK_X, WORDMARK_Y, FRAME_X, FRAME_Y, WORDMARK_RAW_MIN, FRAME_RAW_BBOX,
                        NAME_CAP_PX, INKSCAPE_WIN, SVG_NS, INK_NS, SODI_NS,
                        PLACEHOLDER_SUFFIXES, PLACEHOLDER_RE, NAMED_RECT_RE,
                        SCREWS_TOP, SCREWS_BOTTOM, SCREWS_ALL, PanelSize, panel_size)
from .glyphs import LABEL_GLYPHS, LABEL_KERN, NAME_GLYPHS, NAME_KERN, WORDMARK_D, FRAME_D
from .text import (PanelError, glyph_ink_x, text_ink_width, label_ink_width, text_paths, self_test)
from .elements import (circle, rect, rule, hrule, label, label_id_for, slug, fmt, frame_geometry,
                       emit_branding, emit_rect, emit_circle, emit_svg)
from .validate import (load_dark_generator, q, local, scan_colours, validate_light, validate_dark)
from .build import (run_dark_generator, reinsert_display_rects, wslpath_w, render_png,
                    write_if_changed, build_panel, ids_report_lines)

__all__ = [
    "HP", "RACK_GRID_HEIGHT", "MM_PER_HP", "PANEL_H_MM", "MM2PX", "RAW2PX", "H_PX",
    "PORT_R", "DOT_R", "SCREW",
    "COL_BG", "COL_PLATE", "COL_INK", "COL_ART", "COL_MARK", "COL_DOT", "COL_DARK_PLATE",
    "INK_OPACITY", "SIZE_LABEL", "TRACK_LABEL", "SIZE_NAME", "WORD_NAME",
    "JACK_LABEL_DY", "KNOB_LABEL_DY",
    "WORDMARK_X", "WORDMARK_Y", "FRAME_X", "FRAME_Y", "WORDMARK_RAW_MIN", "FRAME_RAW_BBOX",
    "NAME_CAP_PX", "INKSCAPE_WIN", "SVG_NS", "INK_NS", "SODI_NS",
    "PLACEHOLDER_SUFFIXES", "PLACEHOLDER_RE", "NAMED_RECT_RE",
    "SCREWS_TOP", "SCREWS_BOTTOM", "SCREWS_ALL", "PanelSize", "panel_size",
    "LABEL_GLYPHS", "LABEL_KERN", "NAME_GLYPHS", "NAME_KERN", "WORDMARK_D", "FRAME_D",
    "PanelError", "glyph_ink_x", "text_ink_width", "label_ink_width", "text_paths", "self_test",
    "circle", "rect", "rule", "hrule", "label", "label_id_for", "slug", "fmt", "frame_geometry",
    "emit_branding", "emit_rect", "emit_circle", "emit_svg",
    "load_dark_generator", "q", "local", "scan_colours", "validate_light", "validate_dark",
    "run_dark_generator", "reinsert_display_rects", "wslpath_w", "render_png",
    "write_if_changed", "build_panel", "ids_report_lines",
]
