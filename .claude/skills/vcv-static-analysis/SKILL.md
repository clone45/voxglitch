---
name: vcv-static-analysis
description: Run the VCV Rack library's static-analysis check on voxglitch locally, before submitting a release. Use when preparing a plugin submission, when the library files a "Static analysis issues" ticket, when asked to check for new cppcheck warnings, or when asked to fix uninitialized member variables.
---

# VCV Rack static analysis

The VCV library runs `rack-integration-tools` at integration time and files a
"Static analysis issues" ticket when it finds problems (first seen on voxglitch
as issue #281, August 2026). This reproduces that run locally so a release can
be checked before submitting.

## Run it

```bash
python3 scripts/static_analysis/analyze.py
```

About 5 seconds. Exits 1 if anything appears that isn't in the committed
baseline; prints only what's new.

The very first run on a machine builds cppcheck 2.21.1 into
`~/.cache/voxglitch-static-analysis/` (one time, 2-4 minutes, needs `git`,
`cmake`, a C++ compiler). Every run after that reuses it. Say so before starting
if it's going to build, so the wait isn't a surprise.

Other flags:

| | |
|---|---|
| `--all` | list every current finding, not just new ones |
| `--update-baseline` | accept the current state (use after deliberately fixing or accepting findings) |
| `--xml out.xml` | keep the raw cppcheck XML |

## Fixing uninitialized members

The single largest category. Rack widgets are configured after construction, so
cppcheck can't see the assignment and warns.

```bash
python3 scripts/static_analysis/fix_uninitialized.py           # preview
python3 scripts/static_analysis/fix_uninitialized.py --apply   # write
```

Then **build**, then run `analyze.py` again — findings are reported at the
*constructor* line, so one reported location can hide several members and a
second pass often finds more. Repeat until clean.

Always verify a pointer is genuinely assigned before use before accepting a
`= nullptr`. It's almost always right (it turns a garbage-pointer dereference
into an immediate null crash), but confirm the wiring exists rather than
assuming.

## Checking a build

There's no working `make` in WSL here (the checked-in `build/` holds Windows
paths and breaks the dependency files). To syntax-check what the Makefile
actually compiles:

```bash
S=../Rack-SDK
for f in src/*.cpp src/modules/*.cpp; do
  g++ -fsyntax-only -std=c++11 -DARCH_LIN -Isrc -I$S/include -I$S/dep/include "$f" || echo "FAILED: $f"
done
```

35 translation units. Note `-Isrc` only — adding `-Isrc/vgLib-2.0` shadows Rack's
own `common.hpp` and produces a flood of bogus errors.

## Reading the results

Findings split into voxglitch code and vendored third-party — currently just
`dr_wav.h`/`dr_mp3.h`, which are dr_libs. The maintainer has said he ignores
those as too noisy. Vendored findings are not voxglitch's to fix — say so rather
than "fixing" vendored code.

Note the library's invocation passes no `dep/` suppression, so any vendored code
kept under `src/` lands in its report.

Known-benign, currently in the baseline:

- **`duplInheritedMember`** (7) — `KaisekiSamplePlayer` methods that genuinely
  extend `SamplePlayer` (`trigger`, `step`, `stepReverse`, `stop`, `loadSample`,
  `releaseSample`, `initialize`). `SamplePlayer` has no vtable and is used by
  nine other modules, with `step`/`stepReverse` on the per-sample audio path, so
  these were left shadowing deliberately. Latent, not live: every call site holds
  the derived type and nothing casts to a `SamplePlayer` pointer or reference.
  Watch for that changing.
- **`dangerousTypeCast`** (2) — old-style C casts in `KaisekiReceiver.hpp`.

## Do not draft the issue reply

Bret writes the replies to cschol's tickets himself. Offer analysis and code
fixes; don't offer to write or post the comment.

## How this matches the library's run

The invocation in `analyze.py` is copied from the issue body verbatim:

```
cppcheck --std=c++11 --max-configs=1 --enable=warning -j 8 -q --xml <304 files>
```

The file list is *derived* (`src/**/*.cpp` sorted, then `src/**/*.hpp` sorted;
no `.h`), which reproduces the library's 304-file list exactly and stays correct
as modules are added. Verified against issue #281: all 189 reported findings
reproduce, none missing.

**The version matters.** `dangerousTypeCast` only exists from cppcheck 2.18.0;
Ubuntu's apt ships 2.7, which silently misses three of the checks entirely.
That's why it's pinned and built rather than installed.

Note the library's command has **no `dep/` suppression**, which is why vendored
oscpack and dr_libs appear at all. cschol's documented flags elsewhere do use
`-i.../dep -i.../tests`, so this may change.
