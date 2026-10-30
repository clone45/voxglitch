#!/usr/bin/env python3
"""
Module-browser smoke test: open every Voxglitch module the way Rack's browser
does, inside a real Rack, and fail if anything crashes or trips UBSan.

Why: the browser builds a preview of every module with module == NULL. A
widget that reaches through that pointer -- even `&module->page` before the
`if (module)` -- is undefined behaviour that some compilers turn into a crash
on opening the browser (ArpSeq, nightly of 2026-10-30, macOS) and others
quietly tolerate. No static tool we run reports it. This test runs the real
procedure in a UBSan-instrumented build, which names the line on any compiler.

How: the plugin is built with `make BROWSER_SMOKE=1`, which compiles in the
hidden BrowserSmoke module (src/modules/BrowserSmoke/BrowserSmoke.hpp) and
instruments the code. This script installs that build into a scratch Rack
user folder, with BrowserSmoke added to the scratch copy of plugin.json, puts
one BrowserSmoke in the autosave patch, and runs Rack. When Rack creates the
BrowserSmoke widget, it previews every other model (construct with NULL,
step, draw a few frames, destroy), writes a report and closes Rack.

Pass = the report's last line is "OK <n> models", Rack exited 0, and UBSan
wrote no log. Anything else fails, with the report, the UBSan log and the
tail of Rack's log printed.

    python3 scripts/browser_smoke.py --rack ~/Rack2Free
    python3 scripts/browser_smoke.py --rack ~/Rack2Free --plugin /path/to/build/tree

--rack is the folder holding the Rack binary and res/ (on macOS, the
Rack2Free.app bundle). --plugin is a folder holding plugin.so/.dylib,
plugin.json and res/ -- the repo root after `make BROWSER_SMOKE=1` by
default. Rack needs a display: on Linux without $DISPLAY the script runs it
under xvfb-run if available. Rack Free is a public download:
https://vcvrack.com/downloads/RackFree-<version>-<os>-<cpu>.zip (.dmg on macOS).
"""

import argparse
import glob
import json
import os
import platform
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
PLUGIN_SLUG = "voxglitch"
SMOKE_MODEL = {
    "slug": "BrowserSmoke",
    "name": "Browser Smoke Test",
    "description": "Test builds only: previews every Voxglitch module the way the module browser does.",
    "tags": ["Utility"],
    "hidden": True,
}


def plugin_binary_name():
    s = platform.system()
    return {"Linux": "plugin.so", "Darwin": "plugin.dylib", "Windows": "plugin.dll"}.get(s, "plugin.so")


def find_rack(rack_arg):
    """(executable, system dir) for a Rack install folder or .app bundle."""
    rack = Path(rack_arg).expanduser().resolve()
    if rack.suffix == ".app" or (rack / "Contents" / "MacOS" / "Rack").exists():
        return rack / "Contents" / "MacOS" / "Rack", rack / "Contents" / "Resources"
    for name in ("Rack", "Rack.exe"):
        if (rack / name).exists():
            return rack / name, rack
    # A folder holding the .app
    apps = list(rack.glob("*.app"))
    if apps:
        return find_rack(apps[0])
    sys.exit(f"browser_smoke: no Rack executable under {rack}")


def install_plugin(plugin_dir, user_dir):
    """Copy the test build into the scratch user folder, adding the
    BrowserSmoke manifest entry so Rack will load the model."""
    binary = plugin_dir / plugin_binary_name()
    manifest = plugin_dir / "plugin.json"
    res = plugin_dir / "res"
    for p in (binary, manifest, res):
        if not p.exists():
            sys.exit(f"browser_smoke: {p} not found -- build with `make BROWSER_SMOKE=1` first")

    # Rack in dev mode (-d) reads <user>/plugins; a release Rack reads
    # <user>/plugins-<os>-<cpu>. Both, so either works.
    targets = [user_dir / "plugins" / PLUGIN_SLUG]
    os_tag = {"Linux": "lin", "Darwin": "mac", "Windows": "win"}.get(platform.system(), "lin")
    cpu_tag = "arm64" if platform.machine().lower() in ("arm64", "aarch64") else "x64"
    targets.append(user_dir / f"plugins-{os_tag}-{cpu_tag}" / PLUGIN_SLUG)

    data = json.loads(manifest.read_text(encoding="utf-8"))
    if not any(m.get("slug") == SMOKE_MODEL["slug"] for m in data.get("modules", [])):
        data.setdefault("modules", []).append(SMOKE_MODEL)

    first = None
    for target in targets:
        target.parent.mkdir(parents=True, exist_ok=True)
        if first is None:
            target.mkdir()
            shutil.copy2(binary, target / binary.name)
            shutil.copytree(res, target / "res")
            (target / "plugin.json").write_text(json.dumps(data, indent=2), encoding="utf-8")
            first = target
        else:
            shutil.copytree(first, target)
    return data.get("version", "")


def write_patch(user_dir, plugin_version):
    autosave = user_dir / "autosave"
    autosave.mkdir(parents=True, exist_ok=True)
    patch = {
        "version": "2.0.0",
        "modules": [{
            "id": 1,
            "plugin": PLUGIN_SLUG,
            "model": SMOKE_MODEL["slug"],
            "version": plugin_version,
            "pos": [0, 0],
        }],
        "cables": [],
    }
    (autosave / "patch.json").write_text(json.dumps(patch, indent=2), encoding="utf-8")
    (user_dir / "settings.json").write_text(json.dumps({
        "autoCheckUpdates": False,
        "showTipsOnLaunch": False,
        "autosaveInterval": 0,
    }), encoding="utf-8")


def run_rack(rack_exe, system_dir, user_dir, report, ubsan_prefix, timeout, log_path):
    cmd = [str(rack_exe), "-d", "-s", str(system_dir), "-u", str(user_dir)]
    if platform.system() == "Linux" and not os.environ.get("DISPLAY") and not os.environ.get("WAYLAND_DISPLAY"):
        if shutil.which("xvfb-run"):
            cmd = ["xvfb-run", "-a", "-s", "-screen 0 1280x800x24"] + cmd
        else:
            sys.exit("browser_smoke: no $DISPLAY and no xvfb-run; install xvfb or run with a display")

    env = dict(os.environ)
    env["VOXGLITCH_BROWSER_SMOKE_REPORT"] = str(report)
    env["UBSAN_OPTIONS"] = f"log_path={ubsan_prefix}:print_stacktrace=1:halt_on_error=0"
    env.pop("RACK_DIR", None)

    with open(log_path, "wb") as log:
        try:
            proc = subprocess.run(cmd, cwd=str(system_dir), env=env, stdout=log, stderr=subprocess.STDOUT, timeout=timeout)
            return proc.returncode, False
        except subprocess.TimeoutExpired:
            return None, True


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--rack", required=True, help="Rack install folder (or .app) with the Rack binary and res/")
    ap.add_argument("--plugin", default=str(REPO_ROOT), help="folder with the BROWSER_SMOKE=1 build (default: repo root)")
    ap.add_argument("--timeout", type=int, default=300, help="seconds before Rack is killed (default 300)")
    ap.add_argument("--keep", action="store_true", help="keep the scratch user folder")
    ap.add_argument("--work", help="use this folder as the scratch user folder instead of a temp dir")
    args = ap.parse_args()

    rack_exe, system_dir = find_rack(args.rack)
    plugin_dir = Path(args.plugin).expanduser().resolve()

    work = Path(args.work).resolve() if args.work else Path(tempfile.mkdtemp(prefix="voxglitch-browser-smoke-"))
    if args.work:
        if work.exists():
            shutil.rmtree(work)
        work.mkdir(parents=True)
    user_dir = work / "user"
    user_dir.mkdir()
    report = work / "report.txt"
    ubsan_prefix = work / "ubsan"
    log_path = work / "rack.log"

    version = install_plugin(plugin_dir, user_dir)
    write_patch(user_dir, version)

    print(f"browser_smoke: Rack {rack_exe}")
    print(f"browser_smoke: plugin {plugin_dir} ({PLUGIN_SLUG} {version})")
    print(f"browser_smoke: scratch {work}")
    code, timed_out = run_rack(rack_exe, system_dir, user_dir, report, ubsan_prefix, args.timeout, log_path)

    report_lines = report.read_text(encoding="utf-8", errors="replace").splitlines() if report.exists() else []
    ubsan_logs = sorted(glob.glob(str(ubsan_prefix) + ".*"))
    last = report_lines[-1] if report_lines else ""
    ok = (last.startswith("OK ") and code == 0 and not timed_out and not ubsan_logs)

    print()
    if ok:
        print(f"browser_smoke: PASS -- {last}, Rack exited 0, no UBSan reports")
    else:
        print("browser_smoke: FAIL")
        if timed_out:
            print(f"  Rack did not finish within {args.timeout}s (the smoke module never ran, or hung)")
        elif code != 0:
            print(f"  Rack exited with code {code}" + (" (a crash)" if code and code < 0 or (code or 0) > 128 else ""))
        if not report_lines:
            print("  no report was written: Rack died before the smoke module ran, or the module was not loaded")
        elif not last.startswith("OK "):
            print(f"  report ends with: {last!r}")
            creating = [l for l in report_lines if l.startswith("creating ")]
            created = {l.split(" ", 1)[1] for l in report_lines if l.startswith("created ")}
            pending = [l.split(" ", 1)[1] for l in creating if l.split(" ", 1)[1] not in created]
            if pending:
                print(f"  died while creating: {pending[-1]}")
        for path in ubsan_logs:
            print(f"  UBSan report {path}:")
            for line in Path(path).read_text(encoding="utf-8", errors="replace").splitlines():
                print("    " + line)
        print("  report:")
        for line in report_lines[-15:]:
            print("    " + line)
        print(f"  Rack log tail ({log_path}):")
        tail = log_path.read_text(encoding="utf-8", errors="replace").splitlines()[-40:] if log_path.exists() else []
        for line in tail:
            print("    " + line)

    if not args.keep and not args.work and ok:
        shutil.rmtree(work, ignore_errors=True)
    elif not ok:
        print(f"browser_smoke: scratch folder kept at {work}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
