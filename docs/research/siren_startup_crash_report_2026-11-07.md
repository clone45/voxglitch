# SIREN: Rack crashes on startup while building a waveform cache

Plugin: stoermelder PackOne 2.7.0, module SIREN
Rack: VCV Rack Free 2.6.6, Windows 10, x64
Date: 2026-11-07

## What happens

When I start Rack, it loads my autosaved patch and then crashes about a
quarter of a second later. The crash happens every time, before I can click
anything. The log shows "Fatal signal 22" (an abort) on one of SIREN's
background worker threads, inside `buildWaveformCache`.

## What I think triggers it

The patch has one SIREN module. Its saved data looks like this:

```
"data": {
  "lastFile": "/kicks/7_SKIES_KICK_47_G_.wav",
  "lastPlayheadPos": 0.0,
  "activeRootIdx": 1,
  "trimIn": 0.0,
  "trimOut": 1.0
}
```

My SIREN root containers are:

```
0: C:\Users\Bret\Music\AI Generated\set1\death of power
1: E:\collection\drums
```

So on startup SIREN reloads `E:\collection\drums\kicks\7_SKIES_KICK_47_G_.wav`
for preview and starts building its waveform cache on a worker thread. The
abort happens inside that work. I think an exception is thrown on the worker
thread and nothing catches it, so the whole process aborts.

If I blank out `lastFile` in the autosave file, Rack starts normally. So the
preview of this one file on load is the trigger.

## The file

The file exists and plays in other software. It is a normal 24-bit stereo
WAV:

```
size: 158,844 bytes
format: PCM (tag 1), 2 channels, 44100 Hz, 24 bits per sample, block align 6
RIFF header (first 48 bytes):
52 49 46 46 74 6c 02 00 57 41 56 45 66 6d 74 20 10 00 00 00 01 00 02 00
44 ac 00 00 98 09 04 00 06 00 18 00 64 61 74 61 28 6c 02 00 5c 34 39 eb
```

I can send the file if that helps.

## The log

This is the end of `log.txt`. The patch's modules all finish loading first.
SIREN's panel SVGs load at 2.319 s and 2.556 s, and the abort is at 2.887 s.

```
[2.311 info src/engine/Engine.cpp:1301 fromJson] Creating module stoermelder SIREN
[2.319 info src/app/RackWidget.cpp:372 fromJson] Creating module widget stoermelder SIREN
[2.319 info src/window/Svg.cpp:28 loadFile] Loaded SVG .../Stoermelder-P1/res/Siren.svg
[2.556 info src/window/Svg.cpp:28 loadFile] Loaded SVG .../Stoermelder-P1/res/dark/Siren.svg
[2.887 fatal adapters/standalone.cpp:49 fatalSignalHandler] Fatal signal 22. Stack trace:
16:  0x0
15: raise 0x7ffb42f1e9f0
14: abort 0x7ffb42f23040
13: ZN7rigtorp4mpmc5QueueIN18StoermelderPackOne14MpmcTaskWorker8WorkItemENS0_16AlignedAllocatorINS0_4SlotIS4_EEEEED1Ev 0x7ffa85664150
12: Z8snprintfPcyPKcz 0x7ffa85424c90
11: ZN10soundtouch9TDStretchnwEy 0x7ffa857ca9b0
10: ZZZZN18StoermelderPackOne7Transit13TransitWidgetILi12EE17appendContextMenuEPN4rack2ui4MenuEENKUlS6_E2_clES6_ENKUlNS0_7OUTMODEEbE_clES8_bEN11OutModeItemD1Ev 0x7ffa857b8940
9: ZZZZN18StoermelderPackOne7Transit13TransitWidgetILi12EE17appendContextMenuEPN4rack2ui4MenuEENKUlS6_E2_clES6_ENKUlNS0_7OUTMODEEbE_clES8_bEN11OutModeItemD1Ev 0x7ffa857b8940
8: ZN18StoermelderPackOne5Siren18buildWaveformCacheExRNS0_11AudioStreamEiRNS0_18AudioWaveformCacheE 0x7ffa8558bb80
7: ZNSt17_Function_handlerIFvvEZN18StoermelderPackOne5Siren16SirenPreviewPane8loadItemERKNS2_14DataSourceNodeESt10shared_ptrINS2_10DataSourceEEbbbEUlvE_E9_M_invokeERKSt9_Any_data 0x7ffa856dc930
6: ZN18StoermelderPackOne14MpmcTaskWorker13processWorkerEv 0x7ffa85439c50
5: ZZZZN18StoermelderPackOne7Transit13TransitWidgetILi12EE17appendContextMenuEPN4rack2ui4MenuEENKUlS6_E2_clES6_ENKUlNS0_7OUTMODEEbE_clES8_bEN11OutModeItemD1Ev 0x7ffa857b8940
4: pthread_create_wrapper 0x7ffb3c1466a0
3: beginthreadex 0x7ffb42f2ef80
2: endthreadex 0x7ffb42f2f0d0
1: BaseThreadInitThunk 0x7ffb445dee20
0: RtlUserThreadStart 0x7ffb445dee20
```

The frames that name `Transit`, `snprintf` and `TDStretch` are probably wrong
labels. The Windows build has no symbols for those addresses, so the trace
shows the nearest exported name. The frames I trust are 6, 7 and 8: the task
worker, the `loadItem` task from the preview pane, and `buildWaveformCache`.

## Other details

- SIREN settings: `resampleOnPlayback: true`, `resampleOnDrop: true`,
  `resampleQuality: 6`, `convertToWavOnDrop: false`, `convertTarget: 2`.
- The `siren-cache` folder held 15 cache files, all with `sampleCount: 2248`
  and 2 channels, from October 30. I do not know whether one of them is for
  this file.
- The patch also has VCV Audio 2, Impromptu Clkd, and four Voxglitch modules
  (Wyrm, Sample Tracker, VX Drums, VX Drum Sequencer). All of them finish
  loading before the crash.
- I had previewed this same file in SIREN in an earlier session without a
  crash, as far as I remember. The crash started when Rack reloaded the
  patch.

## What I did to recover

I made a backup of `autosave/patch.json`, then set SIREN's `lastFile` to an
empty string. Rack starts normally after that. I have not changed anything
else.

## Suggestions

Two things might make this easier for the next person:

1. Catch exceptions in `MpmcTaskWorker::processWorker` and log them. Then a
   failed preview would show up as a log line instead of taking down Rack.
2. On startup, if reloading `lastFile` fails, give up on the preview rather
   than retrying.

I am happy to run a debug build, send the WAV file, or try anything else
that would help.

Bret Truchan (Voxglitch)
