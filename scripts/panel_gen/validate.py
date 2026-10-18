"""
panel_gen.validate -- the light-file and dark-file checks.

What: validate_light parses the emitted light SVG back and checks it against the house
rules AND against the geometry table it came from; validate_dark checks the file the dark
pass produced. Both raise PanelError with every finding at once, so one run reports the
whole list.

Why check what we just wrote: the widgets read every control position from the light
file through PanelHelper::findNamed / findNamedRect, and NanoSVG is unforgiving (ids over
63 chars are truncated, transformed anchors are read at the wrong place, a <text> element
renders as nothing). The rules are brief-panel-svg.md sections 2-4 and 6:

  light  root width mm == viewBox px / 2.952756, height 128.5 mm; no <text>/<style>/<use>/
         clip/mask/filter/image/gradient, no url(#), no class; unique ids <= 63 chars; the
         four layers, untransformed; every placeholder-suffixed id is a top-level
         untransformed circle/rect in the anchors layer with the agreed radius and colour,
         on-panel, outside the screw squares the panel mounts, and equal to the table's
         entry; named rects are untransformed #d2d2d3 rects in the background layer;
         every label group is present and made of id-less transformed paths; every colour
         is a key of generate_dark_panel.py COLOR_MAP or one of the two pass-throughs.
  dark   the #18181a dark_background rect is present; no placeholder id survived (except
         the re-inserted *_display plates); no circle survived; no light-only colour is
         left; nothing unmapped appeared; no url(# and no <text>.

The screw squares come from the table's width and screw pairs (panel_gen.constants
.PanelSize), which is the one change from the first generator's validators
(scripts/generate_vx_drums_panels.py, 2026-09-01 .. 09-07); the checks are otherwise its.
"""

import importlib.util
import re
import xml.etree.ElementTree as ET

from .constants import (PANEL_H_MM, MM2PX, PORT_R, DOT_R, COL_PLATE, COL_MARK, COL_DOT,
                        COL_DARK_PLATE, SVG_NS, INK_NS, PLACEHOLDER_RE, NAMED_RECT_RE,
                        panel_size)
from .elements import fmt
from .text import PanelError


def load_dark_generator(scripts_dir):
    """scripts/generate_dark_panel.py as a module, for its COLOR_MAP (the source of truth
    for which colours a light panel may use)."""
    path = scripts_dir / "generate_dark_panel.py"
    spec = importlib.util.spec_from_file_location("generate_dark_panel", str(path))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def q(tag):
    return "{%s}%s" % (SVG_NS, tag)


def local(tag):
    return tag.split("}", 1)[1] if "}" in tag else tag


def scan_colours(text):
    """Every hex colour in the rendered document: XML comments never render and the namedview's
    pagecolor is Inkscape UI state, so both are ignored; everything else (fill/stroke in style
    or attribute form, which is all the dark generator rewrites) is checked."""
    body = re.sub(r"<!--.*?-->", "", text, flags=re.S)
    body = re.sub(r"<sodipodi:namedview.*?</sodipodi:namedview>", "", body, flags=re.S)
    return set(c.lower() for c in re.findall(r"#[0-9a-fA-F]{3,6}\b", body))


def validate_light(text, panel, color_map):
    """Check the emitted light file against the house rules and the table; return the anchor
    ids in file order (the --ids-file report)."""
    size = panel_size(panel)
    errors = []
    root = ET.fromstring(text)

    # root geometry: width mm must equal viewBox px / 2.952756
    width = root.get("width", "")
    vb = root.get("viewBox", "")
    if not width.endswith("mm") or vb != size.viewbox:
        errors.append("root width/viewBox: %r %r" % (width, vb))
    else:
        vb_w = float(vb.split()[2])
        if abs(float(width[:-2]) - vb_w / MM2PX) > 1e-3:
            errors.append("width %s mm != viewBox %s px / 2.952756" % (width, vb_w))
    if root.get("height") != "%smm" % fmt(PANEL_H_MM):
        errors.append("root height %r" % root.get("height"))

    # forbidden content
    for bad in ("text", "style", "use", "clipPath", "mask", "filter", "image", "linearGradient", "radialGradient"):
        if next(root.iter(q(bad)), None) is not None:
            errors.append("forbidden element <%s>" % bad)
    if "url(#" in text:
        errors.append("file contains url(#")
    if any(el.get("class") for el in root.iter()):
        errors.append("class attribute present")

    # ids unique
    seen = {}
    for el in root.iter():
        eid = el.get("id")
        if eid is None:
            continue
        if eid in seen:
            errors.append("duplicate id %r" % eid)
        seen[eid] = el
        if len(eid) > 63:
            errors.append("id %r longer than 63 chars (NanoSVG truncates)" % eid)

    # layers: direct children of the root, untransformed
    layers = {}
    for child in root:
        if child.tag == q("g") and child.get("{%s}groupmode" % INK_NS) == "layer":
            layers[child.get("id")] = child
            if child.get("transform"):
                errors.append("layer %r carries a transform" % child.get("id"))
    for want in ("layer_background", "layer_branding", "layer_anchors", "layer_labels"):
        if want not in layers:
            errors.append("missing layer %r" % want)
    if errors:
        raise PanelError("\n".join(errors))

    # anchors: every placeholder-suffixed id is a top-level untransformed circle/rect in the anchors layer
    anchors_layer = layers["layer_anchors"]
    anchor_ids = []
    table = {a["id"]: a for a in panel["anchors"]}
    table.update({r["id"]: r for r in panel["rects"] if r["layer"] == "anchors"})
    for el in anchors_layer:
        if el.tag not in (q("circle"), q("rect")):
            errors.append("non-anchor element <%s> in the anchors layer" % local(el.tag))
            continue
        eid = el.get("id") or ""
        if not PLACEHOLDER_RE.match(eid) or eid.endswith("_attn"):
            errors.append("anchor id %r does not end in a placeholder suffix" % eid)
        if el.get("transform"):
            errors.append("anchor %r carries a transform" % eid)
        suffix = eid.rsplit("_", 1)[-1]
        fill = el.get("fill")
        if el.tag == q("circle"):
            cx, cy, r = float(el.get("cx")), float(el.get("cy")), float(el.get("r"))
            if suffix in ("input", "output"):
                if r != PORT_R or fill != COL_MARK:
                    errors.append("jack anchor %r must be r %s fill %s" % (eid, PORT_R, COL_MARK))
            elif suffix in ("knob", "button", "switch", "light"):
                if r != DOT_R or fill != COL_DOT:
                    errors.append("dot anchor %r must be r %s fill %s" % (eid, DOT_R, COL_DOT))
            else:
                errors.append("circle anchor %r has suffix %r" % (eid, suffix))
            if not (PORT_R <= cx <= size.w_px - PORT_R and 0 < cy < size.h_px):
                errors.append("anchor %r centre (%s, %s) is off-panel or too close to the edge" % (eid, cx, cy))
        else:
            cx = float(el.get("x")) + float(el.get("width")) / 2.0
            cy = float(el.get("y")) + float(el.get("height")) / 2.0
            if suffix != "display":
                errors.append("rect anchor %r must end in _display" % eid)
            if fill != COL_DARK_PLATE:
                errors.append("display rect %r must be %s" % (eid, COL_DARK_PLATE))
        if size.in_screw_square(cx, cy):
            errors.append("anchor %r centre (%s, %s) is inside a screw square" % (eid, cx, cy))
        ref = table.get(eid)
        if ref is None:
            errors.append("anchor %r is not in the geometry table" % eid)
        elif el.tag == q("circle") and (float(ref["cx"]) != cx or float(ref["cy"]) != cy):
            errors.append("anchor %r round-trip mismatch" % eid)
        anchor_ids.append(eid)
    missing = [k for k in table if k not in anchor_ids]
    if missing:
        errors.append("anchors in the table but not emitted: %s" % missing)
    for eid, el in seen.items():
        if PLACEHOLDER_RE.match(eid) and el not in list(anchors_layer):
            errors.append("placeholder id %r outside the anchors layer (<%s>)" % (eid, local(el.tag)))

    # named rects: untransformed rects in the background layer
    bg = layers["layer_background"]
    for r in panel["rects"]:
        if r["layer"] != "background":
            continue
        el = seen.get(r["id"])
        if el is None or el.tag != q("rect") or el not in list(bg) or el.get("transform"):
            errors.append("named rect %r must be an untransformed <rect> in the background layer" % r["id"])
        elif not NAMED_RECT_RE.match(r["id"]):
            errors.append("named rect id %r should end in _area or _plate" % r["id"])
        elif el.get("fill") != COL_PLATE:
            errors.append("named rect %r must be %s" % (r["id"], COL_PLATE))

    # labels: every label group is present, contains only id-less paths
    labels_layer = layers["layer_labels"]
    for lb in panel["labels"]:
        el = seen.get(lb["id"])
        if el is None or el.tag != q("g") or el not in list(labels_layer):
            errors.append("label group %r missing from the labels layer" % lb["id"])
            continue
        for p in el:
            if p.tag != q("path") or p.get("id") or not p.get("transform", "").startswith("matrix("):
                errors.append("label %r must contain only id-less transformed <path>s" % lb["id"])
                break

    # colours: only what generate_dark_panel.py maps, plus the two deliberate pass-throughs
    allowed = set(color_map.keys()) | {COL_DOT, COL_DARK_PLATE}
    stray = sorted(scan_colours(text) - allowed)
    if stray:
        errors.append("colours not in COLOR_MAP: %s" % stray)

    if errors:
        raise PanelError("\n".join(errors))
    return anchor_ids


def validate_dark(text, panel, color_map):
    """Check the dark file after the pass and the *_display re-insertion; return the sorted
    list of colours it contains (for the run log)."""
    errors = []
    root = ET.fromstring(text)
    reinserted = {r["id"] for r in panel["rects"] if r["layer"] == "anchors"}

    bg = [el for el in root.iter(q("rect")) if el.get("id") == "dark_background"]
    if len(bg) != 1 or "fill:#18181a" not in (bg[0].get("style") or ""):
        errors.append("dark_background rect missing or not #18181a")

    for el in root.iter():
        eid = el.get("id") or ""
        if PLACEHOLDER_RE.match(eid):
            if el.tag == q("rect") and eid in reinserted:
                continue
            errors.append("placeholder %r (<%s>) survived the dark pass" % (eid, local(el.tag)))
    for el in root.iter(q("circle")):
        errors.append("circle %r survived the dark pass" % (el.get("id") or "?"))

    light_only = {k for k, v in color_map.items() if k != v} | {COL_DOT, COL_MARK}
    found = scan_colours(text)
    leftover = sorted(found & light_only)
    if leftover:
        errors.append("light-theme colours left in the dark file: %s" % leftover)
    allowed = set(color_map.values()) | {COL_DARK_PLATE}
    unmapped = sorted(found - allowed - light_only)
    if unmapped:
        errors.append("unmapped colours in the dark file: %s" % unmapped)
    if "url(#" in text or next(root.iter(q("text")), None) is not None:
        errors.append("dark file contains url(# or <text>")

    if errors:
        raise PanelError("\n".join(errors))
    return sorted(found)
