---
name: release-check
description: Pre-release readiness check for the voxglitch plugin, run before submitting a version to the VCV Rack library. Verifies the tree is clean and pushed, no debug switches or stray prints ship, every TU compiles, static analysis matches the baseline, manual links resolve, and the version was bumped — then preps the library issue posting. Use when preparing a release, checking release readiness, or when asked to run the prerelease check.
---

# Voxglitch release check

All checks live in one script — do not re-derive them by hand:

```bash
python3 scripts/release_check.py --module "Piano Roll" --kind update
```

- `--module` / `--kind new|update` shape the issue title
  (`New module: Piano Roll (updated)`); omit both for a plain plugin update.
- `--skip-build` skips the compile sweep and cppcheck for a fast re-run after
  fixing a non-code finding.

**Rack SDK version lives in FOUR places and must move together**: the
`rack-sdk-version` env in `.github/workflows/build-macos.yml`,
`build-linux.yml` and `build-windows.yml`, plus the local `../Rack-SDK` this
check compiles against. Skew between them is how the Linux workflow stayed
red for eight months (the CI image had a pre-2.6 SDK while local and macOS
had 2.6.4, so `isKeyCommand` built here and failed there). When bumping the
SDK, change all three workflows and refresh the local SDK in the same
sitting.
- `--version-ok` passes the version-bump check when Bret has confirmed the
  current version is meant to cover source commits made since it changed.
  Only pass it on his explicit say-so.
- The compile sweep needs the Rack SDK; it defaults to `../Rack-SDK`, override
  with `RACK_DIR=...`. Full run takes 2-4 minutes.

The script prints JSON and exits non-zero if anything failed. Report the
results as a short pass/fail table, quoting the `detail` of any failure.

When everything passes it writes the library posting to
`docs/releases/library_issue_v<version>.md` (title + repo URL + version +
full commit hash — the exact format of past submissions). Point Bret at the
file. **Never post the issue yourself** — library submissions, like all of
cschol's tickets, are written and posted by Bret personally.

What it checks, for context (the script is authoritative):

- on master, tree clean (`.claude/` and `docs/releases/` exempt), pushed
- no stray `printf`/`cout`/`cerr`/`fprintf` outside vendored code
- `#define DEVELOPMENT` commented out in `vgLib-2.0/helpers/Debugging.hpp`,
  and no `-DDEVELOPMENT` in the Makefile (either ships VBUG logging to every
  user's Rack log)
- every TU compiles with full codegen and `-Wall -Wextra` (warnings fail —
  `-fsyntax-only` is not used because it misses codegen-time warnings)
- `scripts/static_analysis/analyze.py` reports nothing beyond the baseline
- `plugin.json` version changed since the last `src/` commit
- every module's `manualUrl` resolves (HEAD request). A *missing* manualUrl
  is warning-only — fourteen legacy modules have never had one (Bret's call,
  2026-08-29) — but a set-and-broken or non-http URL still fails.
- HEAD's commit exists on GitHub
