#pragma once
//
// Spectral Granulator — the RACK ADAPTER.
//
// Reads the knobs and jacks into spectral_granulator::Controls / Ext, calls
// spectral_granulator::Engine::process once per sample and scales what comes back.
// No DSP lives here; SpectralGranulatorEngine.hpp is the design record.
//
// Scaling: audio +/-5 V <-> the engine's +/-1. The analysis outputs are
// 0..10 V; ONSET is a 1 ms 10 V trigger.
//
// CV: every knob has a CV jack, and 10 V is the knob's full travel, except
// TUNE, which is 1 V/oct. Thirteen of them add straight to the knob (cvSum
// below, and the TUNE and PITCH lines). The other three, STRETCH,
// GRAIN and BLEND, go through an attenuverter, and when nothing is patched
// the jack receives an analysis signal instead: LEVEL, BRIGHT and TONAL in
// that order. So for those three the attenuverter alone sets how much the
// spectrum steers the knob. The engine does that substitution
// (Ext::*Patched); this file only reports the cables.
//
// TRIG pulls the grains' read position back to the present, with the chance
// RESYNC sets. Unpatched, it receives ONSET.
//
// The engine's grain buffer is megabytes and sized by the sample rate, so it
// is (re)allocated in onSampleRateChange, never in process().
//
// THREADS: the display reads engine.display and the three meter floats while
// the audio thread writes them. Plain floats; a torn read is one stale frame.
//

struct SpectralGranulator : VoxglitchModule
{
    enum ParamIds
    {
        FOCUS_PARAM,
        WIDTH_PARAM,
        SENS_PARAM,
        SMOOTH_PARAM,
        PITCH_PARAM,
        STEP_PARAM,
        STRETCH_PARAM,
        GRAIN_PARAM,
        BLEND_PARAM,
        RESYNC_PARAM,
        DENSITY_PARAM,
        IMPRINT_PARAM,
        TUNE_PARAM,
        JITTER_PARAM,
        SPREAD_PARAM,
        FEEDBACK_PARAM,
        STRETCH_ATTEN_PARAM,
        GRAIN_ATTEN_PARAM,
        BLEND_ATTEN_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        IN_L_INPUT,
        IN_R_INPUT,
        TRIG_INPUT,
        FOCUS_CV_INPUT,
        WIDTH_CV_INPUT,
        SENS_CV_INPUT,
        SMOOTH_CV_INPUT,
        PITCH_CV_INPUT,
        STEP_CV_INPUT,
        STRETCH_CV_INPUT,
        GRAIN_CV_INPUT,
        BLEND_CV_INPUT,
        RESYNC_CV_INPUT,
        DENSITY_CV_INPUT,
        IMPRINT_CV_INPUT,
        TUNE_CV_INPUT,
        JITTER_CV_INPUT,
        SPREAD_CV_INPUT,
        FEEDBACK_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        OUT_L_OUTPUT,
        OUT_R_OUTPUT,
        LEVEL_OUTPUT,
        BRIGHT_OUTPUT,
        TONAL_OUTPUT,
        ONSET_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    // STEP reads "Off" at the bottom of its travel, then milliseconds.
    struct StepQuantity : ParamQuantity
    {
        std::string getDisplayValueString() override
        {
            const float sec = spectral_granulator::stepSecFor(getValue());
            if (sec <= 0.f) return "Off";
            return sec < 0.1f ? string::f("%.1f ms", sec * 1000.f) : string::f("%.0f ms", sec * 1000.f);
        }
    };

    // JITTER reads the furthest a grain may reach back, in milliseconds.
    struct JitterQuantity : ParamQuantity
    {
        std::string getDisplayValueString() override
        {
            const float sec = spectral_granulator::jitterSecFor(getValue());
            if (sec <= 0.f) return "Off";
            return sec < 0.1f ? string::f("%.1f ms", sec * 1000.f) : string::f("%.0f ms", sec * 1000.f);
        }
    };

    // WIDTH reads "Full" at the top of its travel, then octaves.
    struct WidthQuantity : ParamQuantity
    {
        std::string getDisplayValueString() override
        {
            if (spectral_granulator::widthIsFull(getValue())) return "Full spectrum";
            return string::f("%.2f oct", spectral_granulator::widthOctFor(getValue()));
        }
    };

    spectral_granulator::Engine engine;
    dsp::SchmittTrigger trig_trigger;
    dsp::PulseGenerator onset_pulse;

    // What the display's meters read (0..1).
    float meter_level = 0.f;
    float meter_bright = 0.f;
    float meter_tonal = 0.f;

    SpectralGranulator()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // The knobs hold 0..1 (TUNE holds semitones); the engine's curves
        // (SpectralGranulatorEngine.hpp) turn them into units. Where a curve is a
        // plain exponential, Rack's own display maths shows the unit and
        // takes typed values; the defaults come from the engine's inverses.
        configParam(FOCUS_PARAM, 0.f, 1.f, 0.5f, "Focus (centre of the band the analysis listens to)", " Hz", 1000.f, 20.f);
        configParam<WidthQuantity>(WIDTH_PARAM, 0.f, 1.f, 1.f, "Width of the band (fully clockwise: the whole spectrum)");
        configParam(SMOOTH_PARAM, 0.f, 1.f, spectral_granulator::smoothNorm(0.050f), "Smoothing of Level / Bright / Tonal (and how long Imprint holds a shape)", " ms", 5000.f, 2.f);
        configParam(SENS_PARAM, 0.f, 1.f, 0.5f, "Onset sensitivity", "%", 0.f, 100.f);

        std::vector<std::string> set_names;
        for (int i = 0; i < spectral_granulator::PITCH_SET_COUNT; i++) set_names.push_back(spectral_granulator::pitchSets()[i].name);
        configSwitch(PITCH_PARAM, 0.f, (float)(spectral_granulator::PITCH_SET_COUNT - 1), 0.f, "Pitch set (the grains cycle through it in order)", set_names);
        configParam(TUNE_PARAM, -24.f, 24.f, 0.f, "Tune (transposes every grain)", " semitones");
        configParam<StepQuantity>(STEP_PARAM, 0.f, 1.f, 0.f, "Step (the grains re-read one spot for this long)");
        configParam(RESYNC_PARAM, 0.f, 1.f, 0.f, "Resync (chance that a trigger pulls the read position back to the present)", "%", 0.f, 100.f);

        configParam(STRETCH_PARAM, 0.f, 1.f, spectral_granulator::stretchNorm(4.f), "Stretch", "x", 128.f, 1.f);
        configParam(GRAIN_PARAM, 0.f, 1.f, spectral_granulator::grainNorm(0.095f), "Grain length", " ms", 1000.f, 1.f);
        configParam(BLEND_PARAM, 0.f, 1.f, 1.f, "Blend (how much of the input is sent to the grains; the rest passes straight through)", "%", 0.f, 100.f);
        configParam(FEEDBACK_PARAM, 0.f, 1.f, 0.f, "Feedback (the grains are fed back into themselves; past 100 % they build)", "%", 0.f, 110.f);
        configParam<JitterQuantity>(JITTER_PARAM, 0.f, 1.f, 0.f, "Jitter (how far back grains may stray; also loosens their timing)");
        configParam(DENSITY_PARAM, 0.f, 1.f, spectral_granulator::densityNorm(6.f), "Density", " grains", 64.f, 0.25f);
        configParam(SPREAD_PARAM, 0.f, 1.f, 0.5f, "Spread (stereo width of the grains)", "%", 0.f, 100.f);
        configParam(IMPRINT_PARAM, 0.f, 1.f, 0.f, "Imprint (the grains take on the live input's spectral shape)", "%", 0.f, 100.f);

        // Closed by default (the Scatter / Euclid rule): the module is a plain
        // smear until a steering amount is turned up.
        configParam(STRETCH_ATTEN_PARAM, -1.f, 1.f, 0.f, "Stretch steering (Level, or the Stretch CV jack)", "%", 0.f, 100.f);
        configParam(GRAIN_ATTEN_PARAM, -1.f, 1.f, 0.f, "Grain steering (Bright, or the Grain CV jack)", "%", 0.f, 100.f);
        configParam(BLEND_ATTEN_PARAM, -1.f, 1.f, 0.f, "Blend steering (Tonal, or the Blend CV jack)", "%", 0.f, 100.f);

        configInput(IN_L_INPUT, "Left / mono audio");
        configInput(IN_R_INPUT, "Right audio (normalled to left)");
        configInput(TRIG_INPUT, "Resync trigger (normalled to Onset)");

        configInput(FOCUS_CV_INPUT, "Focus CV (10 V = full travel)");
        configInput(WIDTH_CV_INPUT, "Width CV (10 V = full travel)");
        configInput(SENS_CV_INPUT, "Sensitivity CV (10 V = full travel)");
        configInput(SMOOTH_CV_INPUT, "Smooth CV (10 V = full travel)");
        configInput(PITCH_CV_INPUT, "Pitch set CV (10 V = all eight sets)");
        configInput(STEP_CV_INPUT, "Step CV (10 V = full travel)");
        configInput(STRETCH_CV_INPUT, "Stretch CV (normalled to Level; 10 V = full travel, through the attenuverter)");
        configInput(GRAIN_CV_INPUT, "Grain CV (normalled to Bright; 10 V = full travel, through the attenuverter)");
        configInput(BLEND_CV_INPUT, "Blend CV (normalled to Tonal; 10 V = full travel, through the attenuverter)");
        configInput(RESYNC_CV_INPUT, "Resync CV (10 V = full travel)");
        configInput(DENSITY_CV_INPUT, "Density CV (10 V = full travel)");
        configInput(IMPRINT_CV_INPUT, "Imprint CV (10 V = full travel)");
        configInput(TUNE_CV_INPUT, "Tune CV (1 V/oct)");
        configInput(JITTER_CV_INPUT, "Jitter CV (10 V = full travel)");
        configInput(SPREAD_CV_INPUT, "Spread CV (10 V = full travel)");
        configInput(FEEDBACK_CV_INPUT, "Feedback CV (10 V = full travel)");

        configOutput(OUT_L_OUTPUT, "Left audio");
        configOutput(OUT_R_OUTPUT, "Right audio");
        configOutput(LEVEL_OUTPUT, "Level (0 to 10 V: loudness inside the band)");
        configOutput(BRIGHT_OUTPUT, "Bright (0 to 10 V: where the energy sits, low to high)");
        configOutput(TONAL_OUTPUT, "Tonal (0 to 10 V: 0 is noisy, 10 is pitched)");
        configOutput(ONSET_OUTPUT, "Onset (1 ms trigger when the spectrum changes suddenly)");

        configBypass(IN_L_INPUT, OUT_L_OUTPUT);
        configBypass(IN_R_INPUT, OUT_R_OUTPUT);

        engine.seed(random::u32());
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override
    {
        engine.setSampleRate(e.sampleRate);
    }

    void onReset(const ResetEvent& e) override
    {
        engine.reset();
        Module::onReset(e);
    }

    // A knob plus its CV jack, 10 V = full travel, clamped to the knob's 0..1.
    float cvSum(int param_id, int input_id)
    {
        return rack::math::clamp(params[param_id].getValue() + inputs[input_id].getVoltage() * 0.1f, 0.f, 1.f);
    }

    void process(const ProcessArgs& args) override
    {
        spectral_granulator::Controls c;
        c.focus   = cvSum(FOCUS_PARAM, FOCUS_CV_INPUT);
        c.width   = cvSum(WIDTH_PARAM, WIDTH_CV_INPUT);
        c.sens    = cvSum(SENS_PARAM, SENS_CV_INPUT);
        c.smooth  = cvSum(SMOOTH_PARAM, SMOOTH_CV_INPUT);
        c.step    = cvSum(STEP_PARAM, STEP_CV_INPUT);
        c.resync  = cvSum(RESYNC_PARAM, RESYNC_CV_INPUT);
        c.density = cvSum(DENSITY_PARAM, DENSITY_CV_INPUT);
        c.imprint = cvSum(IMPRINT_PARAM, IMPRINT_CV_INPUT);
        c.jitter   = cvSum(JITTER_PARAM, JITTER_CV_INPUT);
        c.spread   = cvSum(SPREAD_PARAM, SPREAD_CV_INPUT);
        c.feedback = cvSum(FEEDBACK_PARAM, FEEDBACK_CV_INPUT);

        // TUNE: semitones on the knob, 1 V/oct on the jack. The engine clamps
        // the sum to +/-4 octaves.
        c.tune = params[TUNE_PARAM].getValue() + 12.f * inputs[TUNE_CV_INPUT].getVoltage();

        // PITCH: 10 V sweeps all the sets, added to the knob and rounded.
        const float top_set = (float)(spectral_granulator::PITCH_SET_COUNT - 1);
        c.pitchSet = (int)std::round(rack::math::clamp(
            params[PITCH_PARAM].getValue() + inputs[PITCH_CV_INPUT].getVoltage() * 0.1f * top_set, 0.f, top_set));

        // The three steered knobs: the engine adds (amount x source), and
        // picks the source: the cable when there is one, the analysis signal
        // when there is not.
        c.stretch = params[STRETCH_PARAM].getValue();
        c.grain   = params[GRAIN_PARAM].getValue();
        c.blend   = params[BLEND_PARAM].getValue();
        c.stretchAmt = params[STRETCH_ATTEN_PARAM].getValue();
        c.grainAmt   = params[GRAIN_ATTEN_PARAM].getValue();
        c.blendAmt   = params[BLEND_ATTEN_PARAM].getValue();

        spectral_granulator::Ext ext;
        ext.stretchPatched = inputs[STRETCH_CV_INPUT].isConnected();
        ext.grainPatched   = inputs[GRAIN_CV_INPUT].isConnected();
        ext.blendPatched   = inputs[BLEND_CV_INPUT].isConnected();
        ext.stretchCv = inputs[STRETCH_CV_INPUT].getVoltage() * 0.1f;
        ext.grainCv   = inputs[GRAIN_CV_INPUT].getVoltage() * 0.1f;
        ext.blendCv   = inputs[BLEND_CV_INPUT].getVoltage() * 0.1f;
        ext.resyncPatched = inputs[TRIG_INPUT].isConnected();
        ext.resyncEdge = trig_trigger.process(inputs[TRIG_INPUT].getVoltage(),
                                              constants::gate_low_trigger, constants::gate_high_trigger);

        const float in_l = inputs[IN_L_INPUT].getVoltageSum() * 0.2f;
        const float in_r = inputs[IN_R_INPUT].isConnected() ? inputs[IN_R_INPUT].getVoltageSum() * 0.2f : in_l;

        spectral_granulator::Out out = engine.process(in_l, in_r, c, ext);

        // A non-finite sample must not reach the cable or stay in the grain
        // buffer: start clean and say nothing this sample.
        if (!std::isfinite(out.left) || !std::isfinite(out.right))
        {
            engine.reset();
            out = spectral_granulator::Out();
        }

        if (out.onset) onset_pulse.trigger(1e-3f);

        outputs[OUT_L_OUTPUT].setVoltage(out.left * 5.f);
        outputs[OUT_R_OUTPUT].setVoltage(out.right * 5.f);
        outputs[LEVEL_OUTPUT].setVoltage(out.level * 10.f);
        outputs[BRIGHT_OUTPUT].setVoltage(out.bright * 10.f);
        outputs[TONAL_OUTPUT].setVoltage(out.tonal * 10.f);
        outputs[ONSET_OUTPUT].setVoltage(onset_pulse.process(args.sampleTime) ? 10.f : 0.f);

        meter_level = out.level;
        meter_bright = out.bright;
        meter_tonal = out.tonal;
    }
};
