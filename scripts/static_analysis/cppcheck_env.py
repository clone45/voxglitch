"""Locate (or one-time build) the pinned cppcheck used by the VCV Rack library.

The VCV library's integration bot runs cppcheck at a specific version. Analysing
with an older build silently misses whole checks -- `dangerousTypeCast` only
exists from 2.18.0 on, and Ubuntu's apt still ships 2.7 -- so results wouldn't
match what the library reports.

The binary is built once into a cache directory outside the repo and reused by
every later run. Nothing large is installed per check.
"""
from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path

# Pinned to match the VCV library's integration run. Verified against
# clone45/voxglitch#281: reproduces all 189 reported findings exactly.
CPPCHECK_VERSION = "2.21.1"
CPPCHECK_REPO = "https://github.com/danmar/cppcheck.git"

CACHE_ROOT = Path(
    os.environ.get("XDG_CACHE_HOME", Path.home() / ".cache")
) / "voxglitch-static-analysis"


def _cached_binary() -> Path:
    return CACHE_ROOT / f"cppcheck-{CPPCHECK_VERSION}" / "bin" / "cppcheck"


def _version_of(binary: Path | str) -> str | None:
    try:
        out = subprocess.run(
            [str(binary), "--version"], capture_output=True, text=True, timeout=30
        )
    except (OSError, subprocess.SubprocessError):
        return None
    if out.returncode != 0:
        return None
    # "Cppcheck 2.21.1"
    parts = out.stdout.strip().split()
    return parts[-1] if parts else None


def _usable(binary: Path | str) -> bool:
    """True if the binary is the pinned version AND can load its std.cfg.

    A build whose FILESDIR no longer resolves still reports its version happily
    but fails on every real run, so version alone isn't enough of a check.
    """
    if _version_of(binary) != CPPCHECK_VERSION:
        return False
    try:
        probe = subprocess.run(
            [str(binary), "--enable=warning", "--std=c++11", "-q", "-"],
            input="int main(){return 0;}\n",
            capture_output=True, text=True, timeout=60,
        )
    except (OSError, subprocess.SubprocessError):
        return False
    return "Failed to load" not in (probe.stderr + probe.stdout)


def _build(dest: Path, jobs: int) -> None:
    src = CACHE_ROOT / f"src-{CPPCHECK_VERSION}"
    if src.exists():
        shutil.rmtree(src)
    src.parent.mkdir(parents=True, exist_ok=True)

    print(
        f"  Building cppcheck {CPPCHECK_VERSION} (one time, ~2-4 min).\n"
        f"  Cached at {dest.parent.parent} for every later run.",
        file=sys.stderr,
    )

    prefix = dest.parent.parent
    # cppcheck resolves its std.cfg through a FILESDIR baked in at compile time --
    # there is no runtime override in this version -- so it has to point into the
    # cache. Consequently the cache directory is not relocatable; moving it means
    # rebuilding, which ensure_cppcheck() detects and does automatically.
    filesdir = prefix / "share" / "Cppcheck"

    steps = [
        (
            "cloning",
            ["git", "clone", "--quiet", "--depth", "1", "--branch",
             CPPCHECK_VERSION, CPPCHECK_REPO, str(src)],
        ),
        (
            "configuring",
            ["cmake", "-S", str(src), "-B", str(src / "build"),
             "-DCMAKE_BUILD_TYPE=Release", "-DUSE_MATCHCOMPILER=ON",
             f"-DFILESDIR={filesdir}"],
        ),
        ("compiling", ["cmake", "--build", str(src / "build"), "-j", str(jobs)]),
    ]
    for label, cmd in steps:
        print(f"    {label}...", file=sys.stderr)
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode != 0:
            tail = (r.stderr or r.stdout).strip().splitlines()[-15:]
            raise SystemExit(
                f"cppcheck build failed while {label}:\n  "
                + "\n  ".join(tail)
                + "\n\nNeeded: git, cmake, a C++ compiler."
            )

    # Install by hand rather than via `cmake --install --prefix`, which is a
    # no-op on older cmake and would silently target /usr/local.
    print("    installing...", file=sys.stderr)
    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src / "build" / "bin" / "cppcheck", dest)
    if filesdir.exists():
        shutil.rmtree(filesdir)
    filesdir.mkdir(parents=True, exist_ok=True)
    shutil.copytree(src / "cfg", filesdir / "cfg")

    shutil.rmtree(src, ignore_errors=True)


def ensure_cppcheck(jobs: int | None = None) -> Path:
    """Return a path to cppcheck at the pinned version, building it if needed."""
    override = os.environ.get("VOXGLITCH_CPPCHECK")
    if override:
        got = _version_of(override)
        if got is None:
            raise SystemExit(f"VOXGLITCH_CPPCHECK={override!r} is not runnable.")
        if got != CPPCHECK_VERSION:
            print(
                f"  WARNING: VOXGLITCH_CPPCHECK is {got}, not the pinned "
                f"{CPPCHECK_VERSION}. Results may not match the VCV library's.",
                file=sys.stderr,
            )
        return Path(override)

    cached = _cached_binary()
    if _usable(cached):
        return cached

    # A system cppcheck at exactly the right version is just as good.
    system = shutil.which("cppcheck")
    if system and _usable(system):
        return Path(system)

    _build(cached, jobs or max(1, (os.cpu_count() or 4) - 2))
    if not _usable(cached):
        raise SystemExit(f"Built cppcheck but {cached} is not usable.")
    return cached
