#!/usr/bin/env python3
"""
Generate a dark theme panel SVG from a light theme panel SVG.

This script transforms VCV Rack module panel SVGs from light to dark theme by:
1. Adding a dark background (#18181a)
2. Inverting colors (light → dark, dark → light)
3. Removing widget placeholder elements (inputs, outputs, knobs, etc.)
4. Removing gradient definitions used for light backgrounds

Usage:
    python generate_dark_panel.py <light_panel.svg> [output_dark_panel.svg]

If output path is not specified, outputs to <light_panel>-dark.svg
"""

import sys
import re
import argparse
from pathlib import Path


# Color mapping: light theme → dark theme
COLOR_MAP = {
    # Background colors (light → dark)
    '#e7e7e7': '#18181a',
    '#e9e8e7': '#18181a',
    '#e8e7e7': '#18181a',
    '#ebebeb': '#18181a',

    # Content area colors (light gray → dark gray)
    '#d2d2d3': '#3e3e3e',
    '#d3d2cf': '#414141',
    '#d3d3d3': '#3e3e3e',
    '#d4d4d4': '#414141',

    # Text/artwork colors (dark → light)
    '#0c1117': '#f3eee8',
    '#0c1218': '#f3ede7',
    '#0b1116': '#f3eee8',
    '#1c2729': '#f3ede7',

    # Pure black text → cream/off-white
    '#000000': '#dcd6d2',
    '#010000': '#dcd6d2',
    '#111111': '#dcd6d2',
    '#0c0c0c': '#f3ede7',
    '#1a1a1a': '#dcd6d2',
    '#222222': '#dcd6d2',

    # Widget placeholder fill (teal) - these get removed but just in case
    '#213d42': '#213d42',
}

# Patterns for widget placeholder element IDs to remove
PLACEHOLDER_ID_PATTERNS = [
    r'.*_input$',
    r'.*_output$',
    r'.*_knob$',
    r'.*_button$',
    r'.*_switch$',
    r'.*_light$',
    r'.*_display$',
    r'.*_attn$',
]


def normalize_color(color: str) -> str:
    """Normalize color to lowercase 6-digit hex format."""
    color = color.lower().strip()
    if len(color) == 4 and color.startswith('#'):
        # Expand #rgb to #rrggbb
        return f'#{color[1]*2}{color[2]*2}{color[3]*2}'
    return color


def map_color(color: str) -> str:
    """Map a light theme color to its dark theme equivalent."""
    normalized = normalize_color(color)
    return COLOR_MAP.get(normalized, color)


def is_placeholder_id(element_id: str) -> bool:
    """Check if an element ID matches a widget placeholder pattern."""
    if not element_id:
        return False
    for pattern in PLACEHOLDER_ID_PATTERNS:
        if re.match(pattern, element_id, re.IGNORECASE):
            return True
    return False


def extract_viewbox_dimensions(svg_content: str) -> tuple:
    """Extract viewBox dimensions from SVG."""
    match = re.search(r'viewBox\s*=\s*"([^"]+)"', svg_content)
    if match:
        parts = match.group(1).split()
        if len(parts) >= 4:
            return float(parts[2]), float(parts[3])  # width, height
    return None, None


def remove_placeholder_elements(svg_content: str) -> str:
    """Remove widget placeholder elements (circles, rects with placeholder IDs)."""

    # Use regex to find and remove multi-line circle/rect elements with placeholder IDs
    # Pattern matches <circle or <rect with id="placeholder_id" ... /> or </circle></rect>

    def remove_element(tag_name: str, content: str) -> str:
        # Pattern for self-closing elements: <circle ... id="xxx" ... />
        # This handles multi-line by using DOTALL flag
        pattern = rf'<{tag_name}\s+[^>]*?id\s*=\s*"([^"]+)"[^>]*?/>'

        def replace_if_placeholder(match):
            element_id = match.group(1)
            if is_placeholder_id(element_id):
                return ''
            return match.group(0)

        content = re.sub(pattern, replace_if_placeholder, content, flags=re.DOTALL)

        # Also handle elements with closing tags: <circle ...></circle>
        pattern2 = rf'<{tag_name}\s+[^>]*?id\s*=\s*"([^"]+)"[^>]*>.*?</{tag_name}>'

        content = re.sub(pattern2, replace_if_placeholder, content, flags=re.DOTALL)

        return content

    svg_content = remove_element('circle', svg_content)
    svg_content = remove_element('rect', svg_content)
    svg_content = remove_element('ellipse', svg_content)

    return svg_content


def remove_gradient_definitions(svg_content: str) -> str:
    """Remove linearGradient definitions used for light backgrounds."""
    # Remove linearGradient elements
    svg_content = re.sub(
        r'<linearGradient[^>]*>.*?</linearGradient>',
        '',
        svg_content,
        flags=re.DOTALL
    )
    # Remove single-line linearGradient
    svg_content = re.sub(
        r'<linearGradient[^>]*/>\s*',
        '',
        svg_content
    )
    return svg_content


def replace_gradient_fills(svg_content: str) -> str:
    """Replace gradient fills with solid dark color."""
    # Replace url(#linearGradient...) with solid dark color
    svg_content = re.sub(
        r'fill\s*:\s*url\([^)]+\)',
        'fill:#18181a',
        svg_content
    )
    svg_content = re.sub(
        r'fill\s*=\s*"url\([^)]+\)"',
        'fill="#18181a"',
        svg_content
    )
    return svg_content


def transform_colors(svg_content: str) -> str:
    """Transform all colors from light theme to dark theme."""

    # Transform fill colors in style attributes
    def replace_fill_style(match):
        color = match.group(1)
        new_color = map_color(color)
        return f'fill:{new_color}'

    svg_content = re.sub(
        r'fill\s*:\s*(#[0-9a-fA-F]{3,6})',
        replace_fill_style,
        svg_content
    )

    # Transform fill colors as attributes
    def replace_fill_attr(match):
        color = match.group(1)
        new_color = map_color(color)
        return f'fill="{new_color}"'

    svg_content = re.sub(
        r'fill\s*=\s*"(#[0-9a-fA-F]{3,6})"',
        replace_fill_attr,
        svg_content
    )

    # Transform stroke colors in style attributes
    def replace_stroke_style(match):
        color = match.group(1)
        new_color = map_color(color)
        return f'stroke:{new_color}'

    svg_content = re.sub(
        r'stroke\s*:\s*(#[0-9a-fA-F]{3,6})',
        replace_stroke_style,
        svg_content
    )

    # Transform stroke colors as attributes
    def replace_stroke_attr(match):
        color = match.group(1)
        new_color = map_color(color)
        return f'stroke="{new_color}"'

    svg_content = re.sub(
        r'stroke\s*=\s*"(#[0-9a-fA-F]{3,6})"',
        replace_stroke_attr,
        svg_content
    )

    # Transform stop-color in gradients (though we remove most gradients)
    def replace_stop_color(match):
        color = match.group(1)
        new_color = map_color(color)
        return f'stop-color:{new_color}'

    svg_content = re.sub(
        r'stop-color\s*:\s*(#[0-9a-fA-F]{3,6})',
        replace_stop_color,
        svg_content
    )

    return svg_content


def add_dark_background(svg_content: str) -> str:
    """Add a dark background rect at the beginning of the SVG content."""
    width, height = extract_viewbox_dimensions(svg_content)

    if width is None or height is None:
        # Try to extract from width/height attributes
        width_match = re.search(r'width\s*=\s*"([0-9.]+)', svg_content)
        height_match = re.search(r'height\s*=\s*"([0-9.]+)', svg_content)
        if width_match and height_match:
            # These might be in mm, need to handle viewBox units
            pass

    if width is None:
        print("Warning: Could not determine SVG dimensions for background rect")
        return svg_content

    # Create background rect element
    bg_rect = f'''<rect
   x="0"
   y="0"
   width="{width}"
   height="{height}"
   style="fill:#18181a;fill-opacity:1;opacity:1;stroke:none"
   inkscape:groupmode="layer"
   id="dark_background" />'''

    # Insert after opening <svg> tag and any namedview/defs at the start
    # Find position after </sodipodi:namedview> or after <svg...>
    namedview_end = svg_content.find('</sodipodi:namedview>')
    if namedview_end != -1:
        insert_pos = namedview_end + len('</sodipodi:namedview>')
    else:
        # Find end of <svg> opening tag
        svg_tag_end = svg_content.find('>', svg_content.find('<svg'))
        if svg_tag_end != -1:
            insert_pos = svg_tag_end + 1
        else:
            return svg_content

    return svg_content[:insert_pos] + bg_rect + svg_content[insert_pos:]


def update_docname(svg_content: str, dark_filename: str) -> str:
    """Update the sodipodi:docname attribute to the dark filename."""
    return re.sub(
        r'sodipodi:docname\s*=\s*"[^"]+"',
        f'sodipodi:docname="{dark_filename}"',
        svg_content
    )


def generate_dark_panel(light_svg_path: str, output_path: str = None) -> str:
    """Generate a dark theme panel from a light theme panel."""

    light_path = Path(light_svg_path)

    if output_path is None:
        # Generate output filename: panel.svg → panel-dark.svg
        stem = light_path.stem
        if stem.endswith('-dark'):
            print(f"Warning: Input file appears to already be a dark panel: {light_path}")
        output_path = light_path.parent / f"{stem}-dark.svg"
    else:
        output_path = Path(output_path)

    # Read input file
    with open(light_path, 'r', encoding='utf-8') as f:
        svg_content = f.read()

    # Apply transformations in order
    print(f"Processing: {light_path}")

    # 1. Remove widget placeholder elements
    svg_content = remove_placeholder_elements(svg_content)
    print("  - Removed widget placeholders")

    # 2. Replace gradient fills with solid color
    svg_content = replace_gradient_fills(svg_content)
    print("  - Replaced gradient fills")

    # 3. Remove gradient definitions
    svg_content = remove_gradient_definitions(svg_content)
    print("  - Removed gradient definitions")

    # 4. Transform colors
    svg_content = transform_colors(svg_content)
    print("  - Transformed colors")

    # 5. Add dark background rect
    svg_content = add_dark_background(svg_content)
    print("  - Added dark background")

    # 6. Update document name
    svg_content = update_docname(svg_content, output_path.name)
    print("  - Updated document name")

    # Write output file
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(svg_content)

    print(f"Generated: {output_path}")
    return str(output_path)


def main():
    parser = argparse.ArgumentParser(
        description='Generate a dark theme panel SVG from a light theme panel SVG.'
    )
    parser.add_argument(
        'input',
        help='Path to the light theme panel SVG'
    )
    parser.add_argument(
        'output',
        nargs='?',
        help='Path for the output dark theme panel SVG (default: <input>-dark.svg)'
    )

    args = parser.parse_args()

    if not Path(args.input).exists():
        print(f"Error: Input file not found: {args.input}")
        sys.exit(1)

    generate_dark_panel(args.input, args.output)


if __name__ == '__main__':
    main()
