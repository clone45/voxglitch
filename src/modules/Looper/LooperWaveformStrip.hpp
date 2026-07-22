#pragma once

// NOTE: this header intentionally does NOT #include "Looper.hpp",
// TrackWidget.hpp, or TrackModel.hpp -- those have no include guards and
// double-including them in the same translation unit causes redefinition
// errors. The convention in this module is that Looper.cpp pulls them in
// in the right order before including this file; all Looper sibling
// widgets (LooperWaveformGrid, LooperLoadSample, ...) follow the same
// pattern. <cmath> is fine because the standard library has its own
// guards.

#include <cmath>

// ============================================================================
// LooperWaveformStrip
// ============================================================================
//
// Vertical sibling of LooperWaveformGrid, designed for narrow side-column
// layouts (~1" wide). Sample axis runs top-to-bottom; amplitude is drawn
// horizontally from a center line. The BPM grid renders as horizontal
// lines crossing the strip at subdivision / beat / bar positions.
//
// Architecture:
//
//   Extends TrackWidget with TrackOrientation::VERTICAL. All waveform,
//   marker, playhead, and zoom/pan rendering comes for free from the base
//   class via its orientation-aware helpers (sampleToScreenMain returns
//   a Y coordinate in vertical mode, etc.). This file only adds the grid
//   overlay and the load-on-doubleclick behavior, mirroring the existing
//   LooperWaveformGrid.
//
//   Why a separate class instead of extending LooperWaveformGrid with an
//   orientation flag: the grid's line-drawing direction differs (lines
//   are horizontal here, vertical in Grid) and the visible-window /
//   density-adaption math is similar enough to share but simpler to
//   read as two focused implementations than one branchy one. If a
//   third orientation ever appears, that's the moment to extract.
//
// The horizontal LooperWaveformGrid is intentionally left in place so
// existing callers and future horizontal layouts keep working unchanged.
// ============================================================================

struct LooperWaveformStrip : TrackWidget
{
  Looper *looper_module = nullptr;
  bool load_requested = false;   // Deferred file-dialog request from onDoubleClick

  LooperWaveformStrip(float x, float y, float width, float height,
                      TrackModel *track_model, Looper *looper_module)
    : TrackWidget(x, y, width, height, track_model, TrackOrientation::VERTICAL),
      looper_module(looper_module)
  {
    // Distinctive playback indicator color, matches LooperWaveformGrid so
    // both orientations feel like the same "looper" widget family.
    playback_indicator_color = nvgRGBA(255, 255, 255, 220);
    playback_indicator_width = 2.0f;
  }

  // Run once per UI frame. Service a deferred load request here instead of
  // inside the double-click event handler itself -- opening a blocking
  // file dialog from inside a mouse event creates a re-entrant nested
  // event loop and, during that time, other Rack subsystems may try to
  // reach back into state we've already half-modified (e.g. buffers being
  // reallocated by Sample::load). Deferring to step() runs the dialog
  // from a clean call stack where no event is mid-dispatch.
  //
  // (Same rationale and flow as LooperWaveformGrid::step.)
  void step() override
  {
    TrackWidget::step();
    if (load_requested)
    {
      load_requested = false;
      performLoad();
    }
  }

  void performLoad()
  {
    if (!looper_module) return;

    #ifdef USING_CARDINAL_NOT_RACK
      Looper *m = looper_module;
      const std::string dir = m->samples_root_dir.empty() ? "" : m->samples_root_dir;
      async_dialog_filebrowser(false, NULL, dir.c_str(), "Load sample", [m](char* path) {
        if (path) {
          m->sample_player.stop();
          m->sample_player.loadSample(std::string(path));
          m->sample_player.trigger();
          m->loaded_filename = m->sample_player.getFilename();
          m->setRoot(std::string(path));
          m->startBeatAnalysis();
          free(path);
        }
      });
    #else
      std::string picked = looper_module->selectFileVCV();
      if (picked.empty()) return;

      // Stop playback before touching the sample buffers, so the audio
      // thread won't be reading freed memory while loadSample() calls
      // clear() on the vectors.
      looper_module->sample_player.stop();
      looper_module->sample_player.loadSample(picked);
      looper_module->sample_player.trigger();
      looper_module->loaded_filename = looper_module->sample_player.getFilename();
      looper_module->setRoot(picked);
      looper_module->startBeatAnalysis();
    #endif
  }

  void draw(const DrawArgs &args) override
  {
    // Base draws background, waveform, markers, and playhead in the
    // correct orientation already.
    TrackWidget::draw(args);

    if (!looper_module) return;
    if (!track_model || !track_model->sample || !track_model->sample->isLoaded()) return;

    drawGrid(args.vg);
  }

  // Grid overlay. Mirrors the algorithm in LooperWaveformGrid::drawGrid
  // but draws horizontal lines that span the cross axis (left-right) at
  // Y positions corresponding to sample positions. The density-adaption
  // logic uses the main-axis (vertical) drawable size since the strip is
  // tall but narrow.
  void drawGrid(NVGcontext *vg)
  {
    double subdiv_samples = looper_module->getSubdivSamples();
    if (subdiv_samples <= 0.0) return;

    double nudge_samples = looper_module->getNudgeSamples();

    double win_start = (double)track_model->visible_window_start;
    double win_end   = (double)track_model->visible_window_end;
    double win_span  = win_end - win_start;
    if (win_span <= 0.0) return;

    double sample_size = (double)track_model->sample->size();
    if (sample_size <= 0.0) return;

    // In vertical orientation, the main axis (sample) is Y; the cross
    // axis (perpendicular, used to span the line width) is X. Use the
    // orientation-aware helpers so the math reads cleanly.
    float drawable_main = drawableMain();
    float left  = crossAxisPaddingStart();
    float right = crossAxisSize() - crossAxisPaddingEnd();

    // Find the first subdivision index whose position is >= 0.
    long long first_i = 0;
    if (nudge_samples < 0.0)
      first_i = (long long)std::ceil(-nudge_samples / subdiv_samples);

    // Adaptive density: hide tiers whose lines would crowd the display.
    // The "pixels per X" measurement uses the main axis (height here);
    // thresholds are unchanged from the horizontal grid since they're
    // about visual spacing on the time axis, which is orientation-
    // independent.
    double samples_per_pixel = win_span / (double)drawable_main;
    double pixels_per_subdiv = subdiv_samples / samples_per_pixel;
    double pixels_per_beat   = pixels_per_subdiv * 4.0;
    double pixels_per_bar    = pixels_per_subdiv * 16.0;

    const double MIN_SUBDIV_SPACING = 8.0;
    const double MIN_BEAT_SPACING   = 8.0;
    const double MIN_BAR_SPACING    = 14.0;

    bool show_subdiv = pixels_per_subdiv >= MIN_SUBDIV_SPACING;
    bool show_beat   = pixels_per_beat   >= MIN_BEAT_SPACING;

    long long bar_stride = 1;
    while (pixels_per_bar * (double)bar_stride < MIN_BAR_SPACING && bar_stride < (1LL << 20))
      bar_stride *= 2;
    long long bar_mod = 16LL * bar_stride;

    // Same palette as LooperWaveformGrid for visual continuity between
    // the two looper waveform widgets.
    NVGcolor col_bar    = nvgRGBA(255, 170, 60, 230);
    NVGcolor col_beat   = nvgRGBA(120, 200, 255, 180);
    NVGcolor col_subdiv = nvgRGBA(180, 180, 180, 70);

    // Sample-position-to-Y. Local lambda keeps the double precision that
    // the visibility check relies on; sampleToScreenMain() exists on the
    // base class but clamps and uses float, so for grid math we recompute
    // here (matches LooperWaveformGrid's sampleToX).
    auto sampleToY = [&](double pos) -> float {
      double rel = (pos - win_start) / win_span;
      return mainAxisPaddingStart() + (float)rel * drawable_main;
    };

    auto isVisible = [&](double pos) {
      return pos >= win_start && pos <= win_end;
    };

    nvgSave(vg);

    // Pass 1: subdivisions (faint)
    if (show_subdiv)
    {
      nvgBeginPath(vg);
      for (long long i = first_i;; i++)
      {
        double pos = (double)i * subdiv_samples + nudge_samples;
        if (pos >= sample_size) break;
        if (i % 4 == 0) continue;
        if (!isVisible(pos)) continue;
        float y = sampleToY(pos);
        nvgMoveTo(vg, left,  y);
        nvgLineTo(vg, right, y);
      }
      nvgStrokeColor(vg, col_subdiv);
      nvgStrokeWidth(vg, 0.5f);
      nvgStroke(vg);
    }

    // Pass 2: beats (every 4th, not a bar)
    if (show_beat)
    {
      nvgBeginPath(vg);
      for (long long i = first_i;; i++)
      {
        double pos = (double)i * subdiv_samples + nudge_samples;
        if (pos >= sample_size) break;
        if (i % 4 != 0) continue;
        if (i % 16 == 0) continue;
        if (!isVisible(pos)) continue;
        float y = sampleToY(pos);
        nvgMoveTo(vg, left,  y);
        nvgLineTo(vg, right, y);
      }
      nvgStrokeColor(vg, col_beat);
      nvgStrokeWidth(vg, 0.75f);
      nvgStroke(vg);
    }

    // Pass 3: bars (with power-of-2 stride when very zoomed out)
    nvgBeginPath(vg);
    for (long long i = first_i;; i++)
    {
      double pos = (double)i * subdiv_samples + nudge_samples;
      if (pos >= sample_size) break;
      if (i % bar_mod != 0) continue;
      if (!isVisible(pos)) continue;
      float y = sampleToY(pos);
      nvgMoveTo(vg, left,  y);
      nvgLineTo(vg, right, y);
    }
    nvgStrokeColor(vg, col_bar);
    nvgStrokeWidth(vg, 1.0f);
    nvgStroke(vg);

    nvgRestore(vg);
  }

  // Override the base onButton to suppress the marker/context-menu
  // behavior we don't want in the Looper. Left presses still forward
  // to the base so click-drag pan keeps working; right-clicks pass
  // through so Rack's module context menu takes over.
  void onButton(const event::Button &e) override
  {
    if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
      return;
    }
    TrackWidget::onButton(e);
  }

  // Double-click loads a new sample. Defer the actual file dialog to
  // step() (see performLoad() comment).
  void onDoubleClick(const DoubleClickEvent &e) override
  {
    if (!looper_module) return;
    e.consume(this);
    load_requested = true;
  }
};
