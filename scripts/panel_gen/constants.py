"""
panel_gen.constants -- the house numbers every generated Voxglitch panel shares, and
PanelSize, the one thing that changes from panel to panel.

What: the Rack grid, the panel height, the colours, the type sizes, the label offsets
and the branding positions that brief-panel-svg.md section 7.1 / 7.2 fixed for the
VX Drums panels and that every later generated panel (Euclid) reuses unchanged.

Why PanelSize: the first generator (scripts/generate_vx_drums_panels.py, 33 HP) kept
N_HP / W_PX / W_MM / VIEWBOX as module globals, so the emitter and the validators could
only ever produce one width. Euclid is 8 HP. The width now travels with the geometry
table (panel["hp"]) and everything derived from it -- pixel width, mm width, viewBox,
the screw squares -- is computed here from that one number. Which screw pairs a panel
mounts is part of the same object (panel["screws"]): the validator keeps anchor centres
out of the squares the panel actually has. The VX pair leaves it at the default (all
four): the kit mounts only the top pair and the sequencer only the bottom pair, but the
first generator validated both against all four squares and their SVG headers say so;
keeping the default keeps their output byte-identical.

Sources: RACK_GRID_WIDTH / RACK_GRID_HEIGHT are Rack-SDK/include/app/common.hpp:18-19;
MM_PER_HP and MM2PX are docs/developer_docs/developer-notes.md:15-16; the colours and
label offsets are brief-panel-svg.md 7.1 and DESIGN.md 3.4 (vx-drums); the branding
block is brief 4.3 / 7.2. Every comment on a constant is the first generator's, kept.
"""

import re


# ----------------------------------------------------------------------------------------
# Rack grid (brief-panel-svg.md section 7.1)
# ----------------------------------------------------------------------------------------

HP = 15.0                       # px per HP (RACK_GRID_WIDTH, Rack-SDK/include/app/common.hpp:18)
RACK_GRID_HEIGHT = 380          # Rack-SDK/include/app/common.hpp:19
MM_PER_HP = 5.08                # docs/developer_docs/developer-notes.md:15
PANEL_H_MM = 128.5              # every Voxglitch panel
MM2PX = 75.0 / 25.4             # 2.952756, developer-notes.md:16 (Rack parses SVG at 75 dpi)
RAW2PX = 75.0 / 72.0            # Illustrator 72-dpi reference files -> Rack px (brief section 1.1)

H_PX = 379.4286                 # DESIGN.md 3.4 / 4.8 (piano_roll_panel.svg uses the same literal)

PORT_R = 11.764                 # Voxglitch port SVGs are 23.528 px square (brief section 2.2)
DOT_R = 2.5                     # non-jack anchor dot (DESIGN.md 3.4: timeline idiom, r 2.5)
SCREW = 15                      # ThemedScrew is 15 x 15 px; keep anchor centres out of the four squares

# ----------------------------------------------------------------------------------------
# Colours (every one a key of scripts/generate_dark_panel.py COLOR_MAP, or a pass-through)
# ----------------------------------------------------------------------------------------

COL_BG = "#e7e7e7"              # panel background
COL_PLATE = "#d2d2d3"           # output plates / content areas, rx 4.5
COL_INK = "#0b1116"             # label ink (Pilat outlines)
COL_ART = "#0c1218"             # wordmark ink
COL_MARK = "#213d42"            # jack markers
COL_DOT = "#d8d6cf"             # knob/switch/button/light anchor dots (timeline_panel.svg:220-268)
COL_DARK_PLATE = "#17171a"      # timeline's dark plate colour; passes through the dark map unchanged
INK_OPACITY = "0.952941"        # fill-opacity of every shipped label (piano_roll_panel.svg:360)

# ----------------------------------------------------------------------------------------
# Type (brief section 5.3 / Appendix C, D)
# ----------------------------------------------------------------------------------------

SIZE_LABEL = 7.0 * 75 / 72      # 7.291667 px  (Pilat Bold 7 pt)
TRACK_LABEL = 0.88 * 0.78125    # 0.6875 px    (letter-spacing .88 Inkscape px)
SIZE_NAME = 6.0 * 75 / 72       # 6.25 px      (Pilat Condensed Bold 6 pt)
WORD_NAME = 0.60 * 0.78125      # 0.46875 px   (word-spacing .6 Inkscape px)

JACK_LABEL_DY = 21.3            # label baseline sits 21.3 px above a jack centre (timeline)
KNOB_LABEL_DY = 23.0            # DESIGN.md 3.4: knob labels at cy + 23

# Branding block (brief section 4.3 / 7.2): wordmark ink top-left at (20.7, 13.1), frame at (20.4, 27.2)
WORDMARK_X, WORDMARK_Y = 20.7, 13.1
FRAME_X, FRAME_Y = 20.4, 27.2
WORDMARK_RAW_MIN = (11.3232, 14.2493)            # raw ink bbox origin of arpseq path50 (Appendix A)
FRAME_RAW_BBOX = (17.5315, 26.0681, 55.4685, 35.0861)   # raw bbox of cue_research path6-5 (Appendix B)
NAME_CAP_PX = 4.5                                # 700/1000 x 6.25 px cap height of the boxed name

INKSCAPE_WIN = "/mnt/c/Program Files/Inkscape/bin/inkscape.com"

SVG_NS = "http://www.w3.org/2000/svg"
INK_NS = "http://www.inkscape.org/namespaces/inkscape"
SODI_NS = "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"

PLACEHOLDER_SUFFIXES = ("input", "output", "knob", "button", "switch", "light", "display", "attn")
PLACEHOLDER_RE = re.compile(r"^[a-z0-9_]+_(%s)$" % "|".join(PLACEHOLDER_SUFFIXES))
NAMED_RECT_RE = re.compile(r"^[a-z0-9_]+_(area|plate)$")


# ----------------------------------------------------------------------------------------
# Panel width (per panel: panel["hp"], panel["screws"])
# ----------------------------------------------------------------------------------------

SCREWS_TOP = "top"
SCREWS_BOTTOM = "bottom"
SCREWS_ALL = (SCREWS_TOP, SCREWS_BOTTOM)


class PanelSize(object):
    """Everything derived from the width in HP, plus which screw pairs the panel mounts.

    w_px is int(hp * 15) and w_mm is hp * 5.08, so the SVG root's width in mm equals
    viewBox px / 2.952756 (validate_light checks that). The viewBox is 1:1 with Rack
    pixels, so every coordinate in a geometry table IS a Rack coordinate."""

    def __init__(self, n_hp, screws=SCREWS_ALL):
        if int(n_hp) != n_hp or n_hp < 1:
            raise ValueError("panel width must be a whole number of HP, got %r" % (n_hp,))
        self.n_hp = int(n_hp)
        self.w_px = int(self.n_hp * HP)
        self.w_mm = self.n_hp * MM_PER_HP
        self.h_px = H_PX
        self.viewbox = "0 0 %d %s" % (self.w_px, H_PX)
        self.screws = tuple(screws)
        for s in self.screws:
            if s not in SCREWS_ALL:
                raise ValueError("unknown screw pair %r (use %r / %r)" % (s, SCREWS_TOP, SCREWS_BOTTOM))
        if not self.screws:
            raise ValueError("a panel mounts at least one screw pair")

    def screw_rows(self):
        """(y0, y1) of each mounted screw row, top first."""
        rows = []
        if SCREWS_TOP in self.screws:
            rows.append((0, SCREW))
        if SCREWS_BOTTOM in self.screws:
            rows.append((RACK_GRID_HEIGHT - SCREW, RACK_GRID_HEIGHT))
        return rows

    def screw_squares(self):
        """(x0, x1, y0, y1) of every 15 x 15 ThemedScrew square the panel mounts, left then
        right per row. Anchor centres must stay out of these (validate_light)."""
        squares = []
        for y0, y1 in self.screw_rows():
            squares.append((SCREW, 2 * SCREW, y0, y1))
            squares.append((self.w_px - 2 * SCREW, self.w_px - SCREW, y0, y1))
        return squares

    def in_screw_square(self, x, y):
        return any(x0 <= x <= x1 and y0 <= y <= y1 for x0, x1, y0, y1 in self.screw_squares())

    def describe_screw_squares(self):
        """The SVG header sentence, e.g. 'x 15..30 and 465..480, y 0..15 and 365..380'."""
        ys = " and ".join("%d..%d" % (y0, y1) for y0, y1 in self.screw_rows())
        return "x %d..%d and %d..%d, y %s" % (SCREW, 2 * SCREW, self.w_px - 2 * SCREW, self.w_px - SCREW, ys)


def panel_size(panel):
    """The PanelSize of a geometry table: panel["hp"] and the optional panel["screws"]."""
    return PanelSize(panel["hp"], panel.get("screws", SCREWS_ALL))
