#!/usr/bin/env python3
"""Reproduce the VCV Rack library's static-analysis run against this plugin.

The library files an issue when its integration bot finds problems (see
clone45/voxglitch#281). This runs the same cppcheck invocation locally so a
release can be checked before submitting, and compares against a committed
baseline so only NEW findings need attention.

  ./analyze.py                    # summary + anything new since the baseline
  ./analyze.py --all              # every current finding, grouped
  ./analyze.py --update-baseline  # accept the current state as the new baseline
  ./analyze.py --xml out.xml      # also keep the raw cppcheck XML

Exits 1 when findings appear that aren't in the baseline.
"""
from __future__ import annotations

import argparse
import collections
import json
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from cppcheck_env import CPPCHECK_VERSION, ensure_cppcheck  # noqa: E402

REPO = Path(__file__).resolve().parents[2]
BASELINE = Path(__file__).resolve().parent / "baseline.json"

# The library's exact invocation, from the issue body. Do not "improve" these:
# the point is to match what the library runs, not to run a better analysis.
CPPCHECK_FLAGS = ["--std=c++11", "--max-configs=1", "--enable=warning", "-q", "--xml"]

# Vendored third-party code. Findings here are reported separately -- they are
# real cppcheck output, but they aren't voxglitch's code to fix, and the library
# maintainer has said he ignores the dr_libs ones as too noisy.
VENDORED = ("src/vgLib-2.0/dr_wav.h", "src/vgLib-2.0/dr_mp3.h")


def source_files() -> list[str]:
    """The 304-file list the library passes to cppcheck: .cpp then .hpp, each sorted.

    Derived from the repo rather than hardcoded, so new modules are covered
    automatically. Verified to reproduce the issue's list exactly.
    """
    cpp = sorted(str(p.relative_to(REPO)) for p in REPO.glob("src/**/*.cpp"))
    hpp = sorted(str(p.relative_to(REPO)) for p in REPO.glob("src/**/*.hpp"))
    return [p.replace("\\", "/") for p in cpp + hpp]


def is_vendored(path: str) -> bool:
    return any(path.startswith(v) or path == v for v in VENDORED)


def run_cppcheck(binary: Path, files: list[str], jobs: int, keep_xml: Path | None) -> str:
    cmd = [str(binary), *CPPCHECK_FLAGS, "-j", str(jobs), *files]
    proc = subprocess.run(cmd, cwd=REPO, capture_output=True, text=True)
    xml = proc.stderr                      # cppcheck writes its XML report to stderr
    if "<results" not in xml:
        raise SystemExit(
            "cppcheck produced no XML report.\n"
            + (proc.stdout or "")[-2000:]
            + (xml or "")[-2000:]
        )
    if keep_xml:
        keep_xml.write_text(xml, encoding="utf-8")
    return xml


def parse(xml: str) -> list[dict]:
    findings = []
    for err in ET.fromstring(xml).findall(".//error"):
        loc = err.find("location")
        if loc is None:
            continue
        findings.append(
            {
                "file": (loc.get("file") or "").replace("\\", "/"),
                "line": int(loc.get("line") or 0),
                "id": err.get("id") or "",
                "msg": err.get("msg") or "",
            }
        )
    return findings


def key(f: dict) -> str:
    """Identity of a finding, deliberately excluding the line number.

    Line numbers shift whenever anything above them is edited, which would make
    every unrelated change look like a new finding. The message text carries the
    struct and member names, so it stays specific without being brittle.
    """
    return "%s\t%s\t%s" % (f["file"], f["id"], f["msg"])


def counts(findings: list[dict]) -> dict[str, int]:
    return dict(collections.Counter(key(f) for f in findings))


def load_baseline() -> dict:
    if not BASELINE.exists():
        return {"cppcheck_version": CPPCHECK_VERSION, "findings": {}}
    return json.loads(BASELINE.read_text(encoding="utf-8"))


def describe(k: str) -> tuple[str, str, str]:
    f, i, m = k.split("\t", 2)
    return f, i, m


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--all", action="store_true", help="list every current finding")
    ap.add_argument("--update-baseline", action="store_true",
                    help="record the current findings as accepted")
    ap.add_argument("--xml", type=Path, help="also write the raw cppcheck XML here")
    ap.add_argument("-j", "--jobs", type=int, default=8,
                    help="parallel cppcheck jobs (default 8, as the library uses)")
    args = ap.parse_args()

    binary = ensure_cppcheck()
    files = source_files()
    print("cppcheck %s  |  %d files" % (CPPCHECK_VERSION, len(files)), file=sys.stderr)

    findings = parse(run_cppcheck(binary, files, args.jobs, args.xml))
    now = counts(findings)

    mine = [f for f in findings if not is_vendored(f["file"])]
    vend = [f for f in findings if is_vendored(f["file"])]

    print("\n%d findings  (%d in voxglitch code, %d in vendored third-party)"
          % (len(findings), len(mine), len(vend)))
    for scope, group in (("voxglitch", mine), ("vendored", vend)):
        if not group:
            continue
        print("\n  %s:" % scope)
        for cid, n in collections.Counter(f["id"] for f in group).most_common():
            print("    %4d  %s" % (n, cid))

    if args.update_baseline:
        BASELINE.write_text(
            json.dumps({"cppcheck_version": CPPCHECK_VERSION,
                        "total": len(findings),
                        "findings": now},
                       indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        print("\nBaseline updated: %d findings accepted." % len(findings))
        return 0

    base = load_baseline()
    if base.get("cppcheck_version") != CPPCHECK_VERSION:
        print("\nWARNING: baseline was recorded with cppcheck %s, running %s."
              % (base.get("cppcheck_version"), CPPCHECK_VERSION), file=sys.stderr)
    known = base.get("findings", {})

    new = {k: n - known.get(k, 0) for k, n in now.items() if n > known.get(k, 0)}
    fixed = {k: known[k] - now.get(k, 0) for k in known if known[k] > now.get(k, 0)}

    if args.all:
        print("\nAll current findings:")
        for f in sorted(findings, key=lambda x: (x["file"], x["line"])):
            print("  %s:%d  [%s] %s" % (f["file"], f["line"], f["id"], f["msg"]))

    if fixed:
        print("\n%d finding(s) fixed since the baseline:" % sum(fixed.values()))
        for k, n in sorted(fixed.items()):
            f, i, m = describe(k)
            print("  -%d  %s  [%s] %s" % (n, f, i, m))

    if new:
        print("\nNEW -- %d finding(s) not in the baseline:" % sum(new.values()))
        by_key = collections.defaultdict(list)
        for f in findings:
            by_key[key(f)].append(f["line"])
        for k, n in sorted(new.items()):
            f, i, m = describe(k)
            lines = ", ".join(str(x) for x in sorted(by_key[k])[:6])
            print("  +%d  %s:%s\n        [%s] %s" % (n, f, lines, i, m))
        print("\nFix these, or accept them with --update-baseline.")
        return 1

    print("\nNo new findings. (%d known, unchanged.)" % sum(known.values()))
    return 0


if __name__ == "__main__":
    sys.exit(main())
