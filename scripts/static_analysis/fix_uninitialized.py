#!/usr/bin/env python3
"""Add explicit initializers to members cppcheck reports as uninitialized.

Rack widgets are built by the framework and configured afterwards, so members
get assigned post-construction:

    struct LengthDisplay : TransparentWidget
    {
        DigitalSequencer *module;      // garbage until the caller assigns it
        unsigned int sequencer_number = 0;
    };

cppcheck analyses the struct in isolation and can't see the later assignment, so
it warns. Giving each member an explicit default silences that honestly and
turns a would-be garbage-pointer dereference into an immediate null crash if the
wiring is ever missed.

  ./fix_uninitialized.py              # show what it would change
  ./fix_uninitialized.py --apply      # write the changes

Run analyze.py again afterwards: findings are reported at the CONSTRUCTOR line,
so several members can hide behind one reported location and a second pass may
still find more. Repeat until it reports nothing.
"""
from __future__ import annotations

import argparse
import collections
import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from analyze import (  # noqa: E402
    REPO, is_vendored, parse, run_cppcheck, source_files,
)
from cppcheck_env import ensure_cppcheck  # noqa: E402

UNINIT = ("uninitMemberVar", "uninitMemberVarNoCtor", "uninitDerivedMemberVar")

# A whole-line member declaration: everything before the ';' is kept verbatim so
# the original spacing and any trailing comment survive untouched.
DECL = re.compile(
    r"^(?P<head>\s*(?:const\s+)?[\w:<>,\s\*&]*?[\*&]?\s*"
    r"(?P<name>\w+)\s*(?P<arr>(?:\[[^\]]*\])*)\s*);(?P<tail>.*)$"
)


def read(path: Path) -> tuple[list[str], str]:
    raw = path.read_bytes().decode("utf-8")
    nl = "\r\n" if "\r\n" in raw else "\n"
    return raw.split(nl), nl


def default_for(head: str, name: str, arr: str) -> str | None:
    """The right initializer for a declaration, or None if there isn't a safe one."""
    decl = head.strip()
    typ = decl[: decl.rfind(name)].strip()
    if arr:
        return "{}"                    # value-initializes every element, pointers included
    if typ.endswith("*"):
        return "nullptr"
    if typ.endswith("&"):
        return None                    # a reference can't be defaulted here
    base = typ.replace("const", "").strip()
    if base == "bool":
        return "false"
    if base == "float":
        return "0.0f"
    if base in ("double", "long double"):
        return "0.0"
    if re.fullmatch(
        r"(unsigned|signed|long|short)(\s+(int|long|char|short))*"
        r"|int|char|size_t|u?int(8|16|32|64)_t|uint|ushort",
        base,
    ):
        return "0"
    return None                        # a class type: its own default ctor handles it


def index_definitions() -> dict[str, list[tuple[Path, int]]]:
    """Map every struct/class name to where it's defined, tree-wide.

    Needed because a derived class's finding names a member that belongs to a
    parent declared in a different file.
    """
    defs: dict[str, list[tuple[Path, int]]] = collections.defaultdict(list)
    for dirpath, _, names in os.walk(REPO / "src"):
        for n in names:
            if not n.endswith((".hpp", ".h", ".cpp")):
                continue
            p = Path(dirpath) / n
            lines, _ = read(p)
            for i, line in enumerate(lines):
                m = re.match(r"\s*(?:struct|class)\s+(\w+)\b", line)
                if m:
                    defs[m.group(1)].append((p, i))
    return defs


def body_range(lines: list[str], start: int) -> tuple[int, int]:
    depth, seen = 0, False
    for i in range(start, len(lines)):
        depth += lines[i].count("{") - lines[i].count("}")
        if lines[i].count("{"):
            seen = True
        if seen and depth <= 0:
            return start, i
    return start, len(lines) - 1


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--apply", action="store_true", help="write the changes")
    ap.add_argument("-j", "--jobs", type=int, default=8)
    args = ap.parse_args()

    binary = ensure_cppcheck()
    findings = parse(run_cppcheck(binary, source_files(), args.jobs, None))

    wanted = set()
    for f in findings:
        if f["id"] not in UNINIT or is_vendored(f["file"]):
            continue
        m = re.search(r"'(\w+)::(\w+)'", f["msg"])
        if m:
            wanted.add((f["file"], m.group(1), m.group(2)))

    if not wanted:
        print("No uninitialized-member findings in voxglitch code.")
        return 0

    defs = index_definitions()
    edits: dict[Path, dict[int, str]] = collections.defaultdict(dict)
    done, skipped = [], []

    for reported_file, struct, member in sorted(wanted):
        candidates = defs.get(struct, [])
        here = REPO / reported_file
        ordered = ([c for c in candidates if c[0] == here]
                   + [c for c in candidates if c[0] != here])
        if not ordered:
            skipped.append((reported_file, struct, member, "no definition of the struct"))
            continue

        placed = False
        for path, si in ordered:
            lines, _ = read(path)
            lo, hi = body_range(lines, si)
            for i in range(lo, hi + 1):
                m = DECL.match(lines[i])
                if not m or m.group("name") != member:
                    continue
                if "=" in m.group("head") or "{" in m.group("head"):
                    placed = True                      # already initialized
                    break
                d = default_for(m.group("head"), member, m.group("arr"))
                if d is None:
                    skipped.append((reported_file, struct, member,
                                    "no safe default for %r" % m.group("head").strip()))
                    placed = True
                    break
                new = "%s = %s;%s" % (m.group("head").rstrip(), d, m.group("tail"))
                edits[path][i] = new
                done.append((str(path.relative_to(REPO)).replace("\\", "/"),
                             i + 1, struct, member, new.strip()))
                placed = True
                break
            if placed:
                break
        if not placed:
            skipped.append((reported_file, struct, member, "declaration not found"))

    for path, changes in sorted(edits.items()):
        lines, nl = read(path)
        for i, new in changes.items():
            lines[i] = new
        if args.apply:
            path.write_bytes(nl.join(lines).encode("utf-8"))

    for rel, line, struct, member, text in sorted(done):
        print("%s:%d  %s::%s  ->  %s" % (rel, line, struct, member, text))

    n = sum(len(c) for c in edits.values())
    print("\n%d initializer(s) %s across %d file(s); %d skipped."
          % (n, "added" if args.apply else "would be added", len(edits), len(skipped)))
    for s in sorted(skipped):
        print("  SKIP %s  %s::%s  (%s)" % s)
    if not args.apply and n:
        print("\nRe-run with --apply to write these.")
    if args.apply and n:
        print("\nBuild before trusting this, then run analyze.py again "
              "(one reported location can hide several members).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
