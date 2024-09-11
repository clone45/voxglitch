import xml.etree.ElementTree as ET
import sys
import re

# Register the SVG namespace
ET.register_namespace('', "http://www.w3.org/2000/svg")
ET.register_namespace('sodipodi', "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd")

def remove_inkscape_attributes(element):
    for attr in list(element.attrib):
        if attr.startswith('{http://www.inkscape.org/namespaces/inkscape}'):
            del element.attrib[attr]

def optimize_style_attributes(element):
    style = element.get('style', '')
    if style:
        style = re.sub(r'display:\s*inline;?', '', style)
        style = re.sub(r'stroke:\s*none;?', '', style)
        
        if style:
            element.set('style', style.strip(';'))
        else:
            del element.attrib['style']
    
    if 'stroke' in element.attrib and element.attrib['stroke'] == 'none':
        if 'stroke-width' in element.attrib:
            del element.attrib['stroke-width']
        del element.attrib['stroke']

def remove_sodipodi_elements(root):
    for elem in root.findall(".//*"):
        if elem.tag.startswith("{http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd}"):
            root.remove(elem)

def round_float_values(match):
    return f"{float(match.group()):.4f}".rstrip('0').rstrip('.')

def optimize_svg(input_file, output_file):
    tree = ET.parse(input_file)
    root = tree.getroot()

    for elem in root.iter():
        remove_inkscape_attributes(elem)
        optimize_style_attributes(elem)

    remove_sodipodi_elements(root)

    tree.write(output_file, encoding='utf-8', xml_declaration=True)

    with open(output_file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    content = re.sub(r'\s*stroke:\s*none;?', '', content)
    content = re.sub(r'\s*display:\s*inline;?', '', content)
    content = re.sub(r'\s*xmlns:sodipodi="[^"]*"', '', content)
    
    # Round floating point numbers to 4 decimal places
    content = re.sub(r'-?\d+\.\d+', round_float_values, content)
    
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(content)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    optimize_svg(input_file, output_file)
    print(f"Optimized SVG saved to {output_file}")