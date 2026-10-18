"""
panel_gen.text -- label text as glyph outlines (Procedure A, brief-panel-svg.md 5.4).

What: lays a string out glyph by glyph from the embedded Pilat tables (panel_gen.glyphs)
with GPOS pair kerning, tracking and word spacing, and returns it as a <g> of one <path>
per glyph, ink-centred on a given x with the baseline at a given y. No <text> element is
ever emitted: the font files are not in the repository and NanoSVG has no text support.

Why one path per glyph with its own matrix: it is the simplest form that both NanoSVG
(bakes the matrix) and scripts/generate_dark_panel.py (recolours each style="fill:...")
handle without special cases. The group id must never end in a placeholder suffix, or
PanelHelper::findMatched would pick the label up as a widget anchor.

self_test() pins the layout to the widths measured from the shipped, Inkscape-set labels
(brief 5.3 / Appendix C, D): if a table is ever edited, the regression shows up before a
panel is written.

Lifted unchanged from scripts/generate_vx_drums_panels.py (2026-09-01 .. 09-07).
"""

import re

from .constants import (COL_INK, INK_OPACITY, PLACEHOLDER_RE,
                        SIZE_LABEL, TRACK_LABEL, SIZE_NAME, WORD_NAME)
from .glyphs import LABEL_GLYPHS, LABEL_KERN, NAME_GLYPHS, NAME_KERN


class PanelError(Exception):
    """A geometry table, an emitted file or the dark pass broke a house rule."""
    pass


_PATH_CMD_RE = re.compile(r"([MLHVCZ])([^MLHVCZ]*)")
_NUM_RE = re.compile(r"-?\d+(?:\.\d+)?")


def _path_tokens(d):
    """[(command, [numbers...]), ...] for the absolute-only outlines in the tables."""
    return [(cmd, [float(n) for n in _NUM_RE.findall(rest)]) for cmd, rest in _PATH_CMD_RE.findall(d)]


def glyph_ink_x(d):
    """Min/max x of an outline in font units (x coordinates of M/L/C points and H values;
    Pilat keeps its horizontal extrema on-curve, so control points do not widen the box)."""
    xs = []
    for cmd, nums in _path_tokens(d):
        if cmd in "MLC":
            xs.extend(nums[0::2])
        elif cmd == "H":
            xs.extend(nums)
    return min(xs), max(xs)


def text_ink_width(text, glyphs, kern, size, tracking=0.0, word_extra=0.0):
    """Ink width in px of a string laid out exactly as text_paths lays it out (self-test)."""
    _, ink_min, ink_max = _layout(text, glyphs, kern, size, tracking, word_extra)
    return ink_max - ink_min


def label_ink_width(text):
    """Ink width in px of a jack / knob label (Pilat Bold 7 pt, house tracking): the
    number to check against a column pitch before choosing a label."""
    return text_ink_width(text, LABEL_GLYPHS, LABEL_KERN, SIZE_LABEL, TRACK_LABEL, 0.0)


def _layout(text, glyphs, kern, size, tracking, word_extra):
    s = size / 1000.0
    pen_x = 0.0
    placed = []
    ink_min = None
    ink_max = None
    for i, ch in enumerate(text):
        if ch not in glyphs:
            raise PanelError("no outline for %r in %r" % (ch, text))
        adv, d = glyphs[ch]
        if d:  # space has no outline
            placed.append((pen_x, d))
            x0, x1 = glyph_ink_x(d)
            ink_min = pen_x + x0 * s if ink_min is None else min(ink_min, pen_x + x0 * s)
            ink_max = pen_x + x1 * s if ink_max is None else max(ink_max, pen_x + x1 * s)
        if i + 1 < len(text):
            adv += kern.get(ch + text[i + 1], 0)
        pen_x += adv * s + tracking + (word_extra if ch == " " else 0.0)
    if ink_min is None:
        raise PanelError("label %r has no ink" % text)
    return placed, ink_min, ink_max


def text_paths(text, cx, baseline_y, glyphs, kern, size, tracking=0.0, word_extra=0.0,
               fill=COL_INK, opacity=INK_OPACITY, elem_id=None, indent="    "):
    """Return a <g> of one <path> per glyph, ink-centred on cx with the baseline at baseline_y.
    One path per glyph with its own matrix is the simplest correct form: NanoSVG bakes the
    matrix and generate_dark_panel.py recolours each style="fill:...". The group id must never
    end in a placeholder suffix (PanelHelper::findMatched would pick it up)."""
    s = size / 1000.0
    placed, ink_min, ink_max = _layout(text, glyphs, kern, size, tracking, word_extra)
    shift = cx - (ink_min + ink_max) / 2.0
    gid = elem_id or ("label_" + re.sub(r"[^A-Za-z0-9]", "_", text).lower())
    if PLACEHOLDER_RE.match(gid):
        raise PanelError("label id %r ends in a placeholder suffix" % gid)
    out = ['%s<g id="%s" aria-label="%s">' % (indent, gid, text)]
    for x, d in placed:
        out.append('%s  <path transform="matrix(%.6f,0,0,%.6f,%.4f,%.4f)" style="fill:%s;fill-opacity:%s" d="%s"/>'
                   % (indent, s, s, x + shift, baseline_y, fill, opacity, d))
    out.append("%s</g>" % indent)
    return "\n".join(out)


def self_test():
    """The embedded tables must reproduce the shipped label widths (brief 5.3 / Appendix C, D)."""
    refs = [("CLOCK", 27.622), ("CV", 10.786), ("RESET", 25.741), ("RECORD", 33.552)]
    for text, ref in refs:
        got = text_ink_width(text, LABEL_GLYPHS, LABEL_KERN, SIZE_LABEL, TRACK_LABEL, 0.0)
        if abs(got - ref) > 0.002:
            raise PanelError("self-test: %s ink width %.3f, expected %.3f" % (text, got, ref))
    got = text_ink_width("CUE RESEARCH", NAME_GLYPHS, NAME_KERN, SIZE_NAME, 0.0, WORD_NAME)
    if abs(got - 32.569) > 0.002:
        raise PanelError("self-test: CUE RESEARCH ink width %.3f, expected 32.569" % got)
