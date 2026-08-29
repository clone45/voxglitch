#!/usr/bin/env python3
"""
Pre-release checks for the Voxglitch VCV Rack plugin.

Ported from voxglitch_devices' release_check.py and adapted: this repo has no
DRM, builds are verified with a direct per-TU compile sweep (there is no
working `make` under WSL here -- the checked-in build/ holds Windows paths),
and instead of prepping an email it preps the VCV library issue posting.

    python3 scripts/release_check.py
    python3 scripts/release_check.py --module "Piano Roll" --kind update
    python3 scripts/release_check.py --skip-build          # fast re-run

Outputs JSON. Exit code 0 = every check passed.

The generated posting lands in docs/releases/library_issue_v<version>.md and
is exempt from the clean-tree check, so generating it never fails the run
that generated it.
"""

import argparse
import json
import os
import re
import subprocess
import sys
import urllib.error
import urllib.request
from concurrent.futures import ThreadPoolExecutor
from glob import glob
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = REPO_ROOT / "src"
PLUGIN_JSON = REPO_ROOT / "plugin.json"
MAKEFILE = REPO_ROOT / "Makefile"
DEBUGGING_HPP = SRC_DIR / "vgLib-2.0" / "helpers" / "Debugging.hpp"
RELEASES_DIR = REPO_ROOT / "docs" / "releases"

def _find_rack_dir():
    # An inherited RACK_DIR often holds an MSYS2-style path (/c/code/Rack-SDK)
    # meant for the Windows-side make; that is meaningless to this WSL-side
    # sweep, so only honour it when it actually resolves.
    env = os.environ.get("RACK_DIR")
    if env and (Path(env) / "include").exists():
        return Path(env)
    return REPO_ROOT.parent / "Rack-SDK"

RACK_DIR = _find_rack_dir()

# -DDEVELOPMENT in the Makefile, or an uncommented "#define DEVELOPMENT" in
# Debugging.hpp, turns VBUG into a live DEBUG() logger that writes to every
# user's Rack log at runtime. Both have shipped-adjacent near misses.
FORBIDDEN_BUILD_FLAGS = ["-DDEVELOPMENT"]

# Vendored code and the debug helper itself are exempt from the stray-print
# scan; docs are not source.
EXCLUDED_PATHS = [
    "src/vgLib-2.0/AudioFile.h",
    "src/vgLib-2.0/dr_wav.h",
    "src/vgLib-2.0/dr_mp3.h",
    "src/vgLib-2.0/helpers/Debugging.hpp",
]
EXCLUDED_DIRS = ["docs"]

DEBUG_PATTERNS = [
    r'(?<!sn)(?<!s)printf\(',   # printf( but not snprintf( / sprintf(
    r'std::cout',
    r'std::cerr',
    r'fprintf\(',
]

COMMENT_PATTERN = re.compile(r'^\s*//')


def is_excluded_path(filepath):
    rel = os.path.relpath(filepath, REPO_ROOT)
    for exc in EXCLUDED_PATHS:
        if os.path.normpath(rel) == os.path.normpath(exc):
            return True
    for exc_dir in EXCLUDED_DIRS:
        if rel.startswith(exc_dir + os.sep) or rel.startswith(exc_dir + "/"):
            return True
    return False


def check_on_master_branch():
    result = subprocess.run(["git", "branch", "--show-current"],
                            capture_output=True, text=True, cwd=REPO_ROOT)
    branch = result.stdout.strip()
    return {"name": "On master branch", "passed": branch == "master", "branch": branch}


def check_git_status():
    result = subprocess.run(["git", "status", "--porcelain"],
                            capture_output=True, text=True, cwd=REPO_ROOT)
    lines = result.stdout.rstrip().splitlines() if result.stdout.rstrip() else []

    issues = []
    for line in lines:
        filepath = line[3:].strip().strip('"')
        if filepath.startswith(".claude/"):
            continue
        if filepath.startswith("docs/releases/"):
            continue
        issues.append(line.strip())

    return {"name": "Code checked in", "passed": len(issues) == 0, "issues": issues}


def check_pushed():
    result = subprocess.run(["git", "status", "--branch", "--porcelain"],
                            capture_output=True, text=True, cwd=REPO_ROOT)
    first_line = result.stdout.splitlines()[0] if result.stdout.strip() else ""
    is_ahead = "ahead" in first_line
    return {"name": "Code pushed to remote", "passed": not is_ahead,
            "detail": first_line.strip() if is_ahead else ""}


def check_debug_statements():
    issues = []
    source_files = []
    for ext in ("*.hpp", "*.cpp", "*.h"):
        source_files.extend(SRC_DIR.rglob(ext))

    for filepath in sorted(source_files):
        if is_excluded_path(filepath):
            continue
        try:
            lines = filepath.read_text(encoding="utf-8", errors="replace").splitlines()
        except Exception:
            continue
        for line_num, line in enumerate(lines, start=1):
            if COMMENT_PATTERN.match(line):
                continue
            for pattern in DEBUG_PATTERNS:
                if re.search(pattern, line):
                    issues.append({
                        "file": str(os.path.relpath(filepath, REPO_ROOT)),
                        "line": line_num,
                        "content": line.strip(),
                    })
                    break

    return {"name": "Stray debug statements", "passed": len(issues) == 0, "issues": issues}


def check_development_define():
    """The VBUG DEVELOPMENT switch must be commented out in Debugging.hpp."""
    if not DEBUGGING_HPP.exists():
        return {"name": "DEVELOPMENT define commented out", "passed": False,
                "detail": "Debugging.hpp not found"}

    for lineno, line in enumerate(DEBUGGING_HPP.read_text(encoding="utf-8").splitlines(), 1):
        stripped = line.strip()
        if stripped.startswith("#define DEVELOPMENT"):
            return {"name": "DEVELOPMENT define commented out", "passed": False,
                    "detail": f"Debugging.hpp:{lineno}: {stripped}"}

    return {"name": "DEVELOPMENT define commented out", "passed": True, "detail": ""}


def check_build_flags():
    if not MAKEFILE.exists():
        return {"name": "No development build flags", "passed": False,
                "detail": "Makefile not found"}

    offenders = []
    for lineno, line in enumerate(MAKEFILE.read_text(encoding="utf-8").splitlines(), 1):
        if line.lstrip().startswith("#"):
            continue
        for flag in FORBIDDEN_BUILD_FLAGS:
            if flag in line:
                offenders.append({"line": lineno, "flag": flag, "text": line.strip()})

    return {"name": "No development build flags", "passed": len(offenders) == 0,
            "offenders": offenders}


def check_build():
    """Compile every translation unit the Makefile would, with full codegen.

    -fsyntax-only is NOT enough here: -Wunused-function and friends fire at
    code generation, which is how a batch of MinGW-visible warnings slipped
    past the old syntax-only sweep.
    """
    if not (RACK_DIR / "include").exists():
        return {"name": "Build succeeds", "passed": False,
                "detail": f"Rack SDK not found at {RACK_DIR} (set RACK_DIR)"}

    tus = sorted(glob(str(SRC_DIR / "*.cpp"))) + sorted(glob(str(SRC_DIR / "modules" / "*.cpp")))

    def compile_one(tu):
        cmd = ["g++", "-c", "-o", os.devnull, "-std=c++11", "-DARCH_LIN",
               "-Isrc", f"-I{RACK_DIR}/include", f"-I{RACK_DIR}/dep/include",
               "-Wall", "-Wextra", "-Wno-unused-parameter", tu]
        proc = subprocess.run(cmd, capture_output=True, text=True, cwd=REPO_ROOT)
        return tu, proc.returncode, proc.stderr

    failures = []
    warnings = []
    with ThreadPoolExecutor(max_workers=8) as pool:
        for tu, code, stderr in pool.map(compile_one, tus):
            rel = os.path.relpath(tu, REPO_ROOT)
            if code != 0:
                failures.append({"file": rel, "output": stderr.strip()[:2000]})
            elif stderr.strip():
                warnings.append({"file": rel, "output": stderr.strip()[:2000]})

    return {"name": "Build succeeds", "passed": len(failures) == 0 and len(warnings) == 0,
            "translation_units": len(tus), "failures": failures, "warnings": warnings}


def check_static_analysis():
    """Run the committed cppcheck gate: no findings beyond the baseline."""
    script = REPO_ROOT / "scripts" / "static_analysis" / "analyze.py"
    if not script.exists():
        return {"name": "Static analysis clean", "passed": False,
                "detail": "scripts/static_analysis/analyze.py not found"}

    proc = subprocess.run([sys.executable, str(script)],
                          capture_output=True, text=True, cwd=REPO_ROOT)
    tail = "\n".join(proc.stdout.strip().splitlines()[-6:])
    return {"name": "Static analysis clean", "passed": proc.returncode == 0, "detail": tail}


def check_version_bumped(confirmed=False):
    result = subprocess.run(["git", "log", "--oneline", "-1", "--", "plugin.json"],
                            capture_output=True, text=True, cwd=REPO_ROOT)
    version_commit = result.stdout.strip().split()[0] if result.stdout.strip() else None
    if not version_commit:
        return {"name": "Version bumped", "passed": False,
                "detail": "Could not find any commit that changed plugin.json"}

    result = subprocess.run(["git", "log", "--oneline", f"{version_commit}..HEAD", "--", "src/"],
                            capture_output=True, text=True, cwd=REPO_ROOT)
    src_commits = result.stdout.strip().splitlines() if result.stdout.strip() else []

    if src_commits and confirmed:
        return {"name": "Version bumped", "passed": True,
                "detail": (f"--version-ok: current version confirmed to cover "
                           f"{len(src_commits)} source commit(s) since {version_commit}")}
    if src_commits:
        return {"name": "Version bumped", "passed": False,
                "detail": (f"{len(src_commits)} source commit(s) since last plugin.json "
                           f"change ({version_commit}). Bump the version, or re-run with "
                           f"--version-ok to confirm the current one covers them.")}
    return {"name": "Version bumped", "passed": True, "detail": ""}


def check_module_manual_urls():
    data = json.loads(PLUGIN_JSON.read_text(encoding="utf-8"))
    modules = data.get("modules", [])

    missing = [m.get("slug", "?") for m in modules if not m.get("manualUrl")]
    invalid = [(m.get("slug", "?"), m["manualUrl"]) for m in modules
               if m.get("manualUrl") and not m["manualUrl"].startswith("http")]

    if missing or invalid:
        parts = []
        if missing:
            parts.append("missing manualUrl: " + ", ".join(missing))
        if invalid:
            parts.append("invalid manualUrl: " + ", ".join(f"{s}={u}" for s, u in invalid))
        return {"name": "Module manualUrl set", "passed": False, "detail": "; ".join(parts)}

    return {"name": "Module manualUrl set", "passed": True,
            "detail": f"all {len(modules)} module(s) have manualUrl"}


def check_module_manual_urls_reachable():
    data = json.loads(PLUGIN_JSON.read_text(encoding="utf-8"))
    modules = data.get("modules", [])

    def probe(m):
        slug = m.get("slug", "?")
        url = m.get("manualUrl")
        if not url:
            return None  # presence check reports it
        req = urllib.request.Request(url, method="HEAD",
                                     headers={"User-Agent": "voxglitch-release-check"})
        try:
            with urllib.request.urlopen(req, timeout=10) as resp:
                if resp.status >= 400:
                    return (slug, f"HTTP {resp.status}")
        except urllib.error.HTTPError as e:
            return (slug, f"HTTP {e.code}")
        except (urllib.error.URLError, TimeoutError, OSError) as e:
            return (slug, f"unreachable ({type(e).__name__})")
        return None

    with ThreadPoolExecutor(max_workers=8) as pool:
        failures = [f for f in pool.map(probe, modules) if f]

    if failures:
        return {"name": "Module manualUrl reachable", "passed": False,
                "detail": "; ".join(f"{s}: {err}" for s, err in failures)}
    return {"name": "Module manualUrl reachable", "passed": True,
            "detail": f"all {len(modules)} module(s) resolve"}


def get_commit_hash():
    full = subprocess.run(["git", "log", "-1", "--format=%H"],
                          capture_output=True, text=True, cwd=REPO_ROOT).stdout.strip()
    summary = subprocess.run(["git", "log", "-1", "--format=%h %s"],
                             capture_output=True, text=True, cwd=REPO_ROOT).stdout.strip()
    return {"name": "Latest commit hash", "hash": full, "summary": summary}


def check_commit_on_github(commit_hash):
    remote = subprocess.run(["git", "remote", "get-url", "origin"],
                            capture_output=True, text=True, cwd=REPO_ROOT).stdout.strip()
    match = re.search(r'github\.com[:/](.+?)(?:\.git)?$', remote)
    if not match:
        return {"name": "Commit exists on GitHub", "passed": False,
                "detail": "Could not parse GitHub repo from remote URL: " + remote}

    repo = match.group(1)
    result = subprocess.run(["gh", "api", f"repos/{repo}/commits/{commit_hash}", "--jq", ".sha"],
                            capture_output=True, text=True, cwd=REPO_ROOT)
    found = result.stdout.strip() == commit_hash
    return {"name": "Commit exists on GitHub", "passed": found,
            "detail": "" if found else result.stderr.strip()}


def get_plugin_version():
    return json.loads(PLUGIN_JSON.read_text(encoding="utf-8")).get("version", "unknown")


def write_library_issue(version, commit_hash, module, kind):
    """Prep the VCV library issue posting, matching the format of past ones:

        New module: Piano Roll (updated)

        https://github.com/clone45/voxglitch
        version: 2.43.0
        f81f6301a61b31b8148164af1a3b37872e562126
    """
    if module:
        title = f"New module: {module}" + (" (updated)" if kind == "update" else "")
    else:
        title = f"Voxglitch {version} (updated)"

    body = f"https://github.com/clone45/voxglitch\nversion: {version}\n{commit_hash}\n"

    RELEASES_DIR.mkdir(parents=True, exist_ok=True)
    out_path = RELEASES_DIR / f"library_issue_v{version}.md"
    out_path.write_text(f"# {title}\n\n{body}", encoding="utf-8")

    return {"name": "Library issue prepared",
            "file": str(out_path.relative_to(REPO_ROOT)),
            "title": title, "body": body}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--module", help='module name for the issue title, e.g. "Piano Roll"')
    parser.add_argument("--kind", choices=["new", "update"], default="update")
    parser.add_argument("--skip-build", action="store_true",
                        help="skip the compile sweep and static analysis (fast re-run)")
    parser.add_argument("--version-ok", action="store_true",
                        help="confirm the current plugin.json version is meant to cover "
                             "source commits made since it was last changed")
    args = parser.parse_args()

    commit_info = get_commit_hash()

    checks = [
        check_on_master_branch(),
        check_git_status(),
        check_pushed(),
        check_debug_statements(),
        check_development_define(),
        check_build_flags(),
        check_version_bumped(args.version_ok),
        check_module_manual_urls(),
        check_module_manual_urls_reachable(),
        commit_info,
        check_commit_on_github(commit_info["hash"]),
    ]

    if not args.skip_build:
        checks.insert(4, check_build())
        checks.insert(5, check_static_analysis())

    version = get_plugin_version()
    all_passed = all(c.get("passed", True) for c in checks)

    issue = None
    if all_passed:
        issue = write_library_issue(version, commit_info["hash"], args.module, args.kind)

    output = {
        "plugin_version": version,
        "all_passed": all_passed,
        "checks": checks,
        "library_issue": issue,
    }
    print(json.dumps(output, indent=2))
    sys.exit(0 if all_passed else 1)


if __name__ == "__main__":
    main()
