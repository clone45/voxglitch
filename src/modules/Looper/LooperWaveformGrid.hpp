// LooperWaveformGrid
//
// Extends CueResearch's TrackWidget to draw a BPM-driven grid overlaid on the
// waveform.  Subdivisions are 16th notes.  Every 4th subdivision is a beat
// (quarter note), and every 16th is a bar (assumed 4/4).  Grid lines are
// anchored by: grid_line(i) = i * subdiv_samples + nudge_samples.
//
// Zoom (scroll wheel) and pan (drag) behavior is inherited from TrackWidget;
// the grid re-renders correctly at any zoom/pan because all positions are
// computed relative to the visible_window_start/end in track_model.

struct LooperWaveformGrid : TrackWidget
{
  Looper *looper_module = nullptr;
  bool load_requested = false;   // Deferred file-dialog request from onDoubleClick

  LooperWaveformGrid(float x, float y, float width, float height, TrackModel *track_model, Looper *looper_module)
    : TrackWidget(x, y, width, height, track_model), looper_module(looper_module)
  {
    // Distinctive playback indicator color for the Looper
    playback_indicator_color = nvgRGBA(255, 255, 255, 220);
    playback_indicator_width = 2.0f;
  }

  // Run once per UI frame.  Service a deferred load request here instead of
  // inside the double-click event handler itself — opening a blocking file
  // dialog from inside a mouse event creates a re-entrant nested event loop
  // and, during that time, other Rack subsystems may try to reach back into
  // state we've already half-modified (e.g. buffers being reallocated by
  // Sample::load).  Deferring to step() runs the dialog from a clean call
  // stack where no event is mid-dispatch.
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
    // Draw base: background, waveform, (markers), playback indicator
    TrackWidget::draw(args);

    if (!looper_module) return;
    if (!track_model || !track_model->sample || !track_model->sample->isLoaded()) return;

    drawGrid(args.vg);
  }

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

    float drawable_width = box.size.x - (container_padding_left + container_padding_right);
    float top    = container_padding_top;
    float bottom = box.size.y - container_padding_bottom;

    // Find the first subdivision index whose position is >= 0 (inside the sample).
    long long first_i = 0;
    if (nudge_samples < 0.0)
      first_i = (long long)std::ceil(-nudge_samples / subdiv_samples);

    // --- Adaptive density: hide tiers whose lines would crowd the display ---
    double samples_per_pixel = win_span / (double)drawable_width;
    double pixels_per_subdiv = subdiv_samples / samples_per_pixel;
    double pixels_per_beat   = pixels_per_subdiv * 4.0;
    double pixels_per_bar    = pixels_per_subdiv * 16.0;

    const double MIN_SUBDIV_SPACING = 8.0;
    const double MIN_BEAT_SPACING   = 8.0;
    const double MIN_BAR_SPACING    = 14.0;

    bool show_subdiv = pixels_per_subdiv >= MIN_SUBDIV_SPACING;
    bool show_beat   = pixels_per_beat   >= MIN_BEAT_SPACING;

    // Bars always show, but step by a power-of-2 stride so they don't crowd.
    long long bar_stride = 1;
    while (pixels_per_bar * (double)bar_stride < MIN_BAR_SPACING && bar_stride < (1LL << 20))
      bar_stride *= 2;
    long long bar_mod = 16LL * bar_stride;  // draw bar when i % bar_mod == 0

    // Colors - aim for high contrast over typical waveform color (near-white).
    NVGcolor col_bar    = nvgRGBA(255, 170, 60, 230);   // warm amber, strong
    NVGcolor col_beat   = nvgRGBA(120, 200, 255, 180);  // cyan-ish, medium
    NVGcolor col_subdiv = nvgRGBA(180, 180, 180, 70);   // pale grey, faint

    auto sampleToX = [&](double pos) -> float {
      double rel = (pos - win_start) / win_span;
      return container_padding_left + (float)rel * drawable_width;
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
        float x = sampleToX(pos);
        nvgMoveTo(vg, x, top);
        nvgLineTo(vg, x, bottom);
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
        float x = sampleToX(pos);
        nvgMoveTo(vg, x, top);
        nvgLineTo(vg, x, bottom);
      }
      nvgStrokeColor(vg, col_beat);
      nvgStrokeWidth(vg, 0.75f);
      nvgStroke(vg);
    }

    // Pass 3: bars, possibly at power-of-2 stride when zoomed far out
    nvgBeginPath(vg);
    for (long long i = first_i;; i++)
    {
      double pos = (double)i * subdiv_samples + nudge_samples;
      if (pos >= sample_size) break;
      if (i % bar_mod != 0) continue;
      if (!isVisible(pos)) continue;
      float x = sampleToX(pos);
      nvgMoveTo(vg, x, top);
      nvgLineTo(vg, x, bottom);
    }
    nvgStrokeColor(vg, col_bar);
    nvgStrokeWidth(vg, 1.0f);
    nvgStroke(vg);

    nvgRestore(vg);
  }

  // Override the base onButton to suppress the marker/context-menu behavior
  // we don't want in the Looper.  We still want left-click pan (dragging)
  // to work, so we forward left presses to the base class while dropping
  // right-clicks so that Rack's module context menu takes over.
  void onButton(const event::Button &e) override
  {
    if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
      // Let the event propagate to the module widget so the default
      // module context menu appears.
      return;
    }
    TrackWidget::onButton(e);
  }

  // Double-click the waveform to load a new sample.  We only *request* the
  // load here — the dialog is actually opened in step() on a fresh event
  // stack (see performLoad()).
  void onDoubleClick(const DoubleClickEvent &e) override
  {
    if (!looper_module) return;
    e.consume(this);
    load_requested = true;
  }
};
