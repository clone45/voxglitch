// Refresh icon curtesy of "Trendy" from the Noun Project

// ParamQuantity that snaps to 0.5 BPM increments.  A half-step lets the
// detector land on e.g. 87.5, which after a manual Double gives exactly 175.
struct HalfSnapBpmQuantity : ParamQuantity
{
  void setValue(float value) override
  {
    value = std::round(value * 2.0f) / 2.0f;
    ParamQuantity::setValue(value);
  }
};

struct Looper : VoxglitchSamplerModule
{
  std::string loaded_filename = "[ EMPTY ]";
  SamplePlayer sample_player;
  TrackModel track_model;
  dsp::SchmittTrigger resetTrigger;

  // Auto-analysis (BPM + beat-phase detection).  The analyzer runs on a worker
  // thread; the audio thread polls tryConsume() and applies the result once.
  beat_detect::BeatAnalyzer beat_analyzer;

  // TrackWidget::onDragMove() dereferences track_model.enable_vertical_drag_zoom
  // without a null check, so it needs a valid pointer.  We don't use vertical
  // drag-zoom in the Looper, so just back it with this flag.
  bool track_drag_zoom_enabled = false;
  float left_audio = 0;
  float right_audio = 0;
  std::string root_dir;

  // --- Grid / trigger state ---
  dsp::PulseGenerator bar_pulse;
  dsp::PulseGenerator beat_pulse;
  dsp::PulseGenerator subdiv_pulse;

  // Track the last subdivision index we were in so we can detect crossings.
  // Using a sentinel value to mark "not yet initialized" (first process tick).
  static constexpr long long INVALID_SUBDIV = std::numeric_limits<long long>::min();
  long long last_subdiv_index = INVALID_SUBDIV;

  // Voltage range settings
  unsigned int voltage_range_index = 3; // Default to -5.0 to 5.0

  std::string voltage_range_names[8] = {
    "0.0 to 10.0",
    "-10.0 to 10.0",
    "0.0 to 5.0",
    "-5.0 to 5.0",
    "0.0 to 3.0",
    "-3.0 to 3.0",
    "0.0 to 1.0",
    "-1.0 to 1.0"
  };

  double voltage_ranges[8][2] = {
    {0.0, 10.0},
    {-10.0, 10.0},
    {0.0, 5.0},
    {-5.0, 5.0},
    {0.0, 3.0},
    {-3.0, 3.0},
    {0.0, 1.0},
    {-1.0, 1.0}
  };

  enum ParamIds {
    VOLUME_KNOB,
    BPM_KNOB,
    NUDGE_KNOB,
    NUM_PARAMS
  };
  enum InputIds {
    RESET_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
    BAR_TRIG_OUTPUT,
    BEAT_TRIG_OUTPUT,
    SUBDIV_TRIG_OUTPUT,
    NUM_OUTPUTS
  };

  Looper()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    configParam(VOLUME_KNOB, 0.0f, 1.0f, 1.0f, "Volume");
    configParam<HalfSnapBpmQuantity>(BPM_KNOB, 40.0f, 300.0f, 120.0f, "Grid BPM", " bpm");
    configParam(NUDGE_KNOB, -2.0f, 2.0f, 0.0f, "Grid nudge", " beats");

    configInput(RESET_INPUT, "Reset");
    configOutput(AUDIO_OUTPUT_LEFT, "Audio left");
    configOutput(AUDIO_OUTPUT_RIGHT, "Audio right");
    configOutput(BAR_TRIG_OUTPUT, "Bar trigger");
    configOutput(BEAT_TRIG_OUTPUT, "Beat trigger");
    configOutput(SUBDIV_TRIG_OUTPUT, "16th-note trigger");

    // Wire the track model to the sample inside the sample_player.
    // TrackWidget::step() will call track_model.initialize() when a sample loads,
    // which sets visible_window_end = sample->size().
    track_model.sample = &sample_player.sample;
    track_model.playhead_position = 0;
    track_model.setVerticalDragZoomEnabled(&track_drag_zoom_enabled);
  }

  // Autosave module data.  VCV Rack decides when this should be called.
  json_t *dataToJson() override
  {
    json_t *root = json_object();
    json_object_set_new(root, "loaded_sample_path", json_string(sample_player.getPath().c_str()));
    json_object_set_new(root, "voltage_range_index", json_integer(voltage_range_index));

    // Call VoxglitchSamplerModule::saveSamplerData to save sampler data
    saveSamplerData(root);

    return root;
  }

  // Load module data
  void dataFromJson(json_t *root) override
  {
    json_t *loaded_sample_path = json_object_get(root, ("loaded_sample_path"));
    if (loaded_sample_path)
    {
      if(sample_player.loadSample(json_string_value(loaded_sample_path))) sample_player.trigger(0.0, true); // starting position=0.0, loop=true
      loaded_filename = sample_player.getFilename();
    }

    json_t *voltage_range_index_json = json_object_get(root, "voltage_range_index");
    if (voltage_range_index_json)
    {
      voltage_range_index = json_integer_value(voltage_range_index_json);
    }

    // Call VoxglitchSamplerModule::loadSamplerData to load sampler specific data
    loadSamplerData(root);
  }

  // Number of samples (in the sample's own sample-rate frames) between adjacent
  // 16th-note grid lines, based on the current BPM knob.
  double getSubdivSamples()
  {
    float bpm = params[BPM_KNOB].getValue();
    float sr  = (float)sample_player.getSampleRate();
    if (bpm <= 0.0f || sr <= 0.0f) return 0.0;
    // samples_per_beat = (60 / bpm) * sr, 4 subdivs per beat
    return (60.0 / (double)bpm) * (double)sr / 4.0;
  }

  // Grid nudge in sample-frames. Nudge knob is in units of beats
  // (±1 beat).  Returns the offset applied to grid line positions so that
  // grid_line(i) = i * subdiv_samples + nudge_samples.
  double getNudgeSamples()
  {
    float nudge_beats = params[NUDGE_KNOB].getValue();
    double subdiv = getSubdivSamples();
    return (double)nudge_beats * subdiv * 4.0;  // 4 subdivs per beat
  }

  // Safe positive modulo for long long
  static long long posmod(long long a, long long n)
  {
    long long r = a % n;
    if (r < 0) r += n;
    return r;
  }

  // Kick off a fresh beat-detection analysis on the currently-loaded sample.
  // Called from the UI thread after a user-initiated sample load.  Safe to
  // call while the sample is still in the module's buffers; the analyzer
  // copies the data into its worker thread.
  void startBeatAnalysis()
  {
    if (!sample_player.sample.loaded) return;
    const auto& L = sample_player.sample.sample_audio_buffer.left_buffer;
    const auto& R = sample_player.sample.sample_audio_buffer.right_buffer;
    if (L.empty() || R.empty()) return;
    beat_analyzer.analyze(L, R, (float)sample_player.sample.sample_rate);
  }

  // Apply an analysis result to the knobs.  BPM is snapped to the nearest
  // 0.5 (via the HalfSnapBpmQuantity); nudge is always reset to 0 so the
  // first grid line sits at t=0.  The user double/half-adjusts via the BPM
  // knob's context menu and nudges with the Nudge knob.
  void applyBeatResult(const beat_detect::BeatResult& r)
  {
    if (!r.valid) return;
    float bpm = std::max(40.0f, std::min(300.0f, r.bpm));
    params[BPM_KNOB].setValue(bpm);   // HalfSnapBpmQuantity snaps to 0.5
    params[NUDGE_KNOB].setValue(0.0f);
    last_subdiv_index = INVALID_SUBDIV;  // Re-arm grid crossing detection
  }

  void process(const ProcessArgs &args) override
  {
    // Apply a completed beat-detection result, if one became available.
    beat_detect::BeatResult beat_result;
    if (beat_analyzer.tryConsume(beat_result)) applyBeatResult(beat_result);

    if(resetTrigger.process(inputs[RESET_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
    {
      sample_player.trigger(); // starting position=0.0, loop=true
      last_subdiv_index = INVALID_SUBDIV;  // Re-arm grid crossing detection
    }

    sample_player.getStereoOutput(&left_audio, &right_audio, interpolation);

    // pitch = 0.0, sample_start = 0.0, sample_end = 1.0, loop = true
    sample_player.step(0.0, 0.0, 1.0, true);

    // Update the track model's playhead
    track_model.updatePlayheadPosition((unsigned int)sample_player.playback_position);

    // --- Grid crossing detection ---
    if (sample_player.sample.loaded)
    {
      double subdiv_samples = getSubdivSamples();
      if (subdiv_samples > 0.0)
      {
        double nudge_samples = getNudgeSamples();
        double pos = sample_player.playback_position;
        // Which subdivision index are we currently in?
        long long current_subdiv_index = (long long)std::floor((pos - nudge_samples) / subdiv_samples);

        if (last_subdiv_index == INVALID_SUBDIV)
        {
          // First tick after reset/load: don't fire, just initialize.
          last_subdiv_index = current_subdiv_index;
        }
        else if (current_subdiv_index != last_subdiv_index)
        {
          // We crossed into a new subdivision. Fire pulses for current_subdiv_index.
          // Only fire if the grid line is inside the sample (index >= 0).
          if (current_subdiv_index >= 0)
          {
            subdiv_pulse.trigger(0.001f);  // 1ms pulse
            if (posmod(current_subdiv_index, 4) == 0)  beat_pulse.trigger(0.001f);
            if (posmod(current_subdiv_index, 16) == 0) bar_pulse.trigger(0.001f);
          }
          last_subdiv_index = current_subdiv_index;
        }
      }
    }

    float volume = params[VOLUME_KNOB].getValue();

    // Scale audio from -1 to +1 range to selected voltage range
    double min_voltage = voltage_ranges[voltage_range_index][0];
    double max_voltage = voltage_ranges[voltage_range_index][1];

    // Rescale from -1..+1 to min_voltage..max_voltage
    float left_scaled  = rescale(left_audio  * volume, -1.0f, 1.0f, min_voltage, max_voltage);
    float right_scaled = rescale(right_audio * volume, -1.0f, 1.0f, min_voltage, max_voltage);

    outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_scaled);
    outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_scaled);

    outputs[BAR_TRIG_OUTPUT].setVoltage(bar_pulse.process(args.sampleTime) ? 10.0f : 0.0f);
    outputs[BEAT_TRIG_OUTPUT].setVoltage(beat_pulse.process(args.sampleTime) ? 10.0f : 0.0f);
    outputs[SUBDIV_TRIG_OUTPUT].setVoltage(subdiv_pulse.process(args.sampleTime) ? 10.0f : 0.0f);
  }

  void onSampleRateChange(const SampleRateChangeEvent& e) override
  {
    sample_player.updateSampleRate();
  }
};
