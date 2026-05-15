#!/usr/bin/env python3
"""
Concatenates user_manual.md with all module reference pages into a single
master.md ready for pandoc.

Strategy:
- Walk user_manual.md line by line.
- Outside Chapter 6: pass through unchanged.
- Inside Chapter 6: detect category summary tables and, after each table,
  inject each referenced module's full content with headings shifted by 2
  (so `# Module Name` becomes `### Module Name` under `## Category` under
  `# Chapter 6`).

After concatenation, two textual passes:
- Rewrite Chapter 6 module links `(modules/foo.md)` → `(#foo)` so the
  summary tables become clickable cross-references to the embedded
  sections (pandoc derives the section id from the heading text).
- Italicize standalone `None.` paragraphs that some modules use in place
  of an empty Inputs/Outputs/Parameters table.

The list of modules and their categories is the one already encoded in
Chapter 6 of user_manual.md, so this stays in sync automatically.
"""

import re
import sys
from pathlib import Path

BASE = Path(__file__).resolve().parent.parent
USER_MANUAL = BASE / "user_manual.md"
MODULES_DIR = BASE / "modules"
OUTPUT = BASE / "master.md"

FRONT_MATTER = r"""---
title: "Voxglitch Groovebox Advanced"
subtitle: "User Manual and Module Reference"
author: "Bret Truchan"
date: \today
documentclass: extbook
fontsize: 8pt
classoption:
  - oneside
toc: true
toc-depth: 2
geometry:
  - margin=1in
colorlinks: true
linkcolor: NavyBlue
toccolor: black
urlcolor: NavyBlue
header-includes:
  - \usepackage{longtable}
  - \usepackage{booktabs}
  - \usepackage{array}
  - \usepackage{anyfontsize}
  - \usepackage{titlesec}
  - \usepackage{titling}
  - \usepackage{graphicx}
  - \usepackage{xcolor}
  - \AtBeginDocument{\fontsize{7pt}{8.4pt}\selectfont}
  - \AtBeginDocument{\setkeys{Gin}{width=0.65\linewidth,keepaspectratio}}
  - \setlength{\emergencystretch}{3em}
  - \setlength{\LTpre}{0.4em}
  - \setlength{\LTpost}{0.4em}
  - \renewcommand{\arraystretch}{1.15}
  - \titlespacing*{\section}{0pt}{2.6ex plus 0.5ex}{1.3ex plus 0.2ex}
  - \titlespacing*{\subsection}{0pt}{1.9ex plus 0.4ex}{0.8ex plus 0.2ex}
  - \titlespacing*{\subsubsection}{0pt}{1.3ex plus 0.3ex}{0.5ex plus 0.2ex}
  - \titlespacing*{\paragraph}{0pt}{0.9ex plus 0.2ex}{0.3em}
  - \pretitle{\begin{center}\vspace*{4em}\fontsize{30pt}{36pt}\selectfont\bfseries}
  - \posttitle{\par\vspace{1.2em}\rule{0.5\linewidth}{0.4pt}\par\vspace{2.5em}\end{center}}
  - \preauthor{\begin{center}\Large}
  - \postauthor{\par\end{center}}
  - \predate{\begin{center}\large}
  - \postdate{\par\vfill\null\end{center}}
  - \lstset{breaklines=true,breakatwhitespace=true,basicstyle=\ttfamily\footnotesize,columns=flexible,frame=single,framerule=0.3pt,rulecolor=\color{gray!40},xleftmargin=0.5em,xrightmargin=0.5em}
---

"""

MODULE_LINK_RE = re.compile(r"\(modules/([a-z0-9]+\.md)\)")
HEADING_RE = re.compile(r"^(#{1,6})(\s+.*)$")
FENCE_RE = re.compile(r"^\s*(```|~~~)")
CROSSREF_RE = re.compile(r"\(modules/([a-z0-9]+)\.md\)")
NONE_PARA_RE = re.compile(r"^None\.$", re.MULTILINE)


def shift_headings(text: str, shift_by: int) -> str:
    """Shift every markdown heading down by N levels, skipping code fences."""
    out = []
    in_fence = False
    for line in text.split("\n"):
        if FENCE_RE.match(line):
            in_fence = not in_fence
            out.append(line)
            continue
        if not in_fence:
            m = HEADING_RE.match(line)
            if m:
                new_level = "#" * min(6, len(m.group(1)) + shift_by)
                line = new_level + m.group(2)
        out.append(line)
    return "\n".join(out)


def load_module(filename: str) -> str:
    path = MODULES_DIR / filename
    if not path.exists():
        print(f"WARNING: missing module file {filename}", file=sys.stderr)
        return f"\n*Module reference for {filename} not found.*\n"
    return path.read_text()


def emit_modules(pending: list[str], out: list[str]) -> None:
    """Append shifted module content to the output, with a page break per module."""
    for filename in pending:
        content = load_module(filename).rstrip()
        content = shift_headings(content, 2)
        out.append("")
        out.append("\\newpage")
        out.append("")
        out.append(content)
        out.append("")


def postprocess(text: str) -> str:
    """Final textual passes on the assembled master document."""
    # Chapter 6 module summary tables: convert (modules/foo.md) -> (#foo)
    text = CROSSREF_RE.sub(r"(#\1)", text)
    # Replace standalone "None." paragraphs with italic placeholder for
    # visual consistency with the surrounding tables.
    text = NONE_PARA_RE.sub("*None.*", text)
    return text


def main() -> int:
    text = USER_MANUAL.read_text()
    lines = text.split("\n")

    out: list[str] = [FRONT_MATTER.rstrip()]

    in_chapter_6 = False
    pending: list[str] = []
    i = 0
    while i < len(lines):
        line = lines[i]

        if line.startswith("# Chapter 6:"):
            in_chapter_6 = True

        # Track module references found inside Chapter 6 tables.
        if in_chapter_6:
            m = MODULE_LINK_RE.search(line)
            if m:
                pending.append(m.group(1))

        out.append(line)

        # End-of-table detection: a blank line followed by a non-table line
        # (or EOF) means the current table is done. If we have pending modules
        # queued from this table, expand them right here.
        if in_chapter_6 and pending and line.strip() == "":
            j = i + 1
            while j < len(lines) and lines[j].strip() == "":
                j += 1
            ends_table = j >= len(lines) or not lines[j].lstrip().startswith("|")
            if ends_table:
                emit_modules(pending, out)
                pending = []

        i += 1

    if pending:
        emit_modules(pending, out)

    final_text = postprocess("\n".join(out)) + "\n"
    OUTPUT.write_text(final_text)
    print(f"Wrote {OUTPUT}")

    expected = sorted(p.name for p in MODULES_DIR.glob("ga*.md"))
    referenced = sorted(set(re.findall(r"modules/([a-z0-9]+\.md)", text)))
    missing = [f for f in expected if f not in referenced]
    if missing:
        print(
            f"NOTE: {len(missing)} module file(s) exist but are not referenced "
            f"in Chapter 6: {', '.join(missing)}",
            file=sys.stderr,
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())
