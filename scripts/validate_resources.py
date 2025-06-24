import os
import re
import sys

# Project root = parent of this script's folder
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RESOURCE_ROOT = os.path.join(BASE_DIR, "res")
SRC_DIR = os.path.join(BASE_DIR, "src")

# Collect all asset plugin references
svg_refs = []

for root, _, files in os.walk(SRC_DIR):
    for f in files:
        if f.endswith(".cpp") or f.endswith(".hpp"):
            full_file_path = os.path.join(root, f)
            with open(full_file_path, "r", encoding="utf-8") as file:
                for line_num, line in enumerate(file, start=1):
                    match = re.search(r'asset::plugin\(pluginInstance,\s*"(.*?)"\)', line)
                    if match:
                        svg_refs.append((match.group(1), full_file_path, line_num))

errors = False

for path, file_path, line_num in svg_refs:
    # Skip programmatically constructed paths
    if '+' in path or '{' in path or '}' in path:
        continue

    # Remove leading 'res/' if present
    if path.startswith("res/"):
        path = path[4:]

    full_path = os.path.join(RESOURCE_ROOT, path)

    if not os.path.exists(full_path):
        print(f"Missing: {full_path}  (in {file_path}, line {line_num})")
        errors = True
    else:
        actual_files = os.listdir(os.path.dirname(full_path))
        filename = os.path.basename(full_path)
        if filename not in actual_files:
            print(f"Case mismatch: {full_path}  (in {file_path}, line {line_num})")
            errors = True

if errors:
    sys.exit(1)
