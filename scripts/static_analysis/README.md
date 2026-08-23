# VCV Rack static analysis

Reproduces the static-analysis check the VCV Rack library runs when a plugin is
submitted, so a release can be verified before submitting rather than finding
out via a filed ticket (issue #281, August 2026).

```bash
python3 scripts/static_analysis/analyze.py           # ~5s; exits 1 on new findings
python3 scripts/static_analysis/analyze.py --all     # list everything
python3 scripts/static_analysis/fix_uninitialized.py # preview initializer fixes
```

The first run on a machine builds cppcheck 2.21.1 into
`~/.cache/voxglitch-static-analysis/` — one time, 2-4 minutes, needs `git`,
`cmake` and a C++ compiler. Later runs reuse it and take about five seconds.
Set `VOXGLITCH_CPPCHECK=/path/to/cppcheck` to use an existing build instead.

## Files

| | |
|---|---|
| `analyze.py` | runs the check, diffs against the baseline, reports what's new |
| `fix_uninitialized.py` | adds explicit initializers to flagged members |
| `cppcheck_env.py` | finds or one-time-builds the pinned cppcheck |
| `baseline.json` | the accepted findings; regenerate with `--update-baseline` |

## Why cppcheck is pinned and built

`dangerousTypeCast` first ships in cppcheck 2.18.0. Ubuntu's apt still carries
2.7, which silently omits that check along with the current behaviour of
`duplInheritedMember` and `uninitMemberVarNoCtor` — you'd get a clean-looking
report that doesn't match what the library sees. 2.21.1 is the newest release
predating the August 2026 scan, and reproduces issue #281's 189 findings exactly.

## The baseline

`baseline.json` records findings that have been looked at and accepted, keyed by
`(file, check-id, message)` — deliberately **not** by line number, since those
shift whenever anything above them is edited and would make every unrelated
change look like a regression. Counts are tracked per key, so a second instance
of an already-known problem in the same file still surfaces.

After deliberately fixing something, or after deciding a new finding is
acceptable, re-run with `--update-baseline` and commit the result.

## Scope

Findings are split into voxglitch code and vendored third-party. `src/ip/` and
`src/osc/` are oscpack; `dr_wav.h` and `dr_mp3.h` are dr_libs. Those aren't ours
to fix, and the library maintainer has said he ignores the dr_libs output as too
noisy. Note the library's invocation passes no `dep/` suppression, which is the
only reason vendored code appears in the report at all.
