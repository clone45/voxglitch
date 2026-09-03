#pragma once
// VXDrums.hpp — the VX Drums kit: six trigger-driven, analogue-modelled drum
// voices (VXDrumVoices.hpp) behind ONE polyphonic TRIG jack and a SEPARATE
// ACC gate. No sequencer, no memories, no clock: the VX Drum Sequencer (or
// anything else) supplies the timing.
//
// Binding design record: DESIGN.md §2 (the cable protocol), §3 (this module),
// and DESIGN-KITS.md §4 (kits, models, the normalized knobs).
//
// KITS and MODELS. A kit is a named row of six voice models (one per column
// BD SD CP PERC CH OH); a per-column OVERRIDE replaces the kit's choice for
// that column. The effective model of column c is
//     override[c] if set, else kitSpec(kit).models[c]
// and process() keeps `voices` in step with it. Every column's three sound
// knobs are NORMALIZED 0..1 params (TUNE / DECAY / SHAPE); the effective
// model's KnobSpec maps them to natural units for display, typed entry and
// the double-click default. Knob POSITIONS survive a kit or model change
// (physical-knob metaphor): only the mapping changes.
//
// CV. Four POLYPHONIC CV jacks, one per knob row (TUNE / DECAY / SHAPE /
// LEVEL). Channel c (0..5) of a row's cable modulates column c's knob on that
// row; channel 6 modulates that row's MASTER knob (TUNE -> ACCENT, SHAPE ->
// DRIVE, LEVEL -> VOLUME; the DECAY row has no master knob, so its channel 6
// is ignored). Read with getPolyVoltage(), so a mono cable normals to every
// column; channels the cable does not carry are 0 V. Scaling: the CV adds
// cv / 10 of the knob's FULL range to the knob position, clamped to the
// param's range, so +5 V pushes a knob up by half its travel and -5 V pulls
// it down by half. The engine always sees the EFFECTIVE value (knob + CV);
// the knob widgets and tooltips keep showing the knob position. Knobs and CV
// are read at CONTROL RATE (every CONTROL_RATE_DIVISION samples) so an LFO on
// TUNE does not recompute filter coefficients every sample; the trigger and
// accent path stays per-sample.
//
// Threading: `kit` and `model_override[]` are single ints written on the UI
// thread (menus, undo, patch load) and read by process(). A torn read is not
// possible for an aligned int on any platform Rack ships on, and
// effectiveModel() range-checks what it reads, so the audio thread can at
// worst be one sample late — the accepted house pattern for small shared
// fields (PianoRoll.hpp:102-105). No mutex on the audio path.

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>

#include "VXDrumVoices.hpp"

struct VXDrums;

namespace vx_drums_ui
{
    // Column prefixes for knob labels ("BD Punch", "PERC Ratio"). Display only.
    const char* const COLUMN_PREFIX[vx_drums::COLUMNS] = { "BD", "SD", "CP", "PERC", "CH", "OH" };

    // The LEVEL defaults, unchanged from the first cut (vxdrumvoices.c:104-112).
    const float LEVEL_DEFAULT[vx_drums::COLUMNS] = { 0.9f, 0.8f, 0.75f, 0.7f, 0.7f, 0.65f };

    // Natural-unit formatting per DESIGN-KITS.md §4: "%" specs ×100 with one
    // decimal at most, Hz / ms as integers, s with two decimals; anything
    // else (the ratio spec, unit "") with two decimals.
    inline std::string formatKnob(const vx_drums::KnobSpec& k, double natural)
    {
        char buf[32] = {};
        if (k.display_multiplier_percent)
        {
            std::snprintf(buf, sizeof(buf), "%.1f", natural * 100.0);
            std::string s(buf);
            if (s.size() >= 2 && s.compare(s.size() - 2, 2, ".0") == 0) s.erase(s.size() - 2);
            return s;
        }
        std::string unit(k.unit);
        if (unit == " Hz" || unit == " ms") std::snprintf(buf, sizeof(buf), "%.0f", natural);
        else                                std::snprintf(buf, sizeof(buf), "%.2f", natural);
        return std::string(buf);
    }
}

// ── The TUNE / DECAY / SHAPE quantity ────────────────────────────────────────
// The param is 0..1; everything the user sees goes through the EFFECTIVE
// model's KnobSpec, looked up live so the tooltip, typed entry and label
// follow a kit or override change immediately. `module` is NULL in the
// browser and while a param is unattached, so every path has a fallback
// (the House kit's model for the column). Method bodies follow VXDrums.
struct VXDrumsKnobQuantity : ParamQuantity
{
    int column = 0;
    int which = 0;   // 0 tune, 1 decay, 2 shape

    const vx_drums::KnobSpec& spec();

    float getDisplayValue() override;
    void setDisplayValue(float displayValue) override;
    std::string getDisplayValueString() override;
    std::string getLabel() override;
    std::string getUnit() override;
};

struct VXDrums : VoxglitchModule
{
    // DESIGN-KITS.md §4 — column c: TUNE = c*4, DECAY = c*4+1, SHAPE = c*4+2,
    // LEVEL = c*4+3. Nothing is released, so the rename from the first cut
    // (PUNCH/SNAP/SPREAD/… → SHAPE, RS → PERC) is free; never reorder after release.
    enum ParamIds {
        BD_TUNE_PARAM,   BD_DECAY_PARAM,   BD_SHAPE_PARAM,   BD_LEVEL_PARAM,
        SD_TUNE_PARAM,   SD_DECAY_PARAM,   SD_SHAPE_PARAM,   SD_LEVEL_PARAM,
        CP_TUNE_PARAM,   CP_DECAY_PARAM,   CP_SHAPE_PARAM,   CP_LEVEL_PARAM,
        PERC_TUNE_PARAM, PERC_DECAY_PARAM, PERC_SHAPE_PARAM, PERC_LEVEL_PARAM,
        CH_TUNE_PARAM,   CH_DECAY_PARAM,   CH_SHAPE_PARAM,   CH_LEVEL_PARAM,
        OH_TUNE_PARAM,   OH_DECAY_PARAM,   OH_SHAPE_PARAM,   OH_LEVEL_PARAM,
        M_ACCENT_PARAM, M_DRIVE_PARAM, M_VOLUME_PARAM,
        M_VARY_PARAM,                                   // per-hit random spread of every voice knob (Bret, 2026-09-02)
        NUM_PARAMS
    };
    enum InputIds {
        TRIG_INPUT,          // poly: channel c = column c (0 BD 1 SD 2 CP 3 PERC 4 CH 5 OH)
        ACC_INPUT,           // accent GATE; mono normals to all six, poly = per voice
        // Poly CV, one per knob row, in ROW ORDER (cvInputForRow relies on
        // it): channel c = column c's knob, channel 6 = the row's master knob.
        TUNE_CV_INPUT,
        DECAY_CV_INPUT,
        SHAPE_CV_INPUT,
        LEVEL_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        LEFT_OUTPUT, RIGHT_OUTPUT,
        BD_OUTPUT, SD_OUTPUT, CP_OUTPUT, PERC_OUTPUT, CH_OUTPUT, OH_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(VOICE_LIGHTS, vx_drums::COLUMNS),
        NUM_LIGHTS
    };

    static const int KNOBS_PER_COLUMN = 4;
    static const int NO_OVERRIDE = -1;

    // CV conventions (header comment). 10 V spans a knob's full range; the
    // seventh channel addresses the row's master knob; knobs + CV are read
    // every CONTROL_RATE_DIVISION samples.
    static const int MASTER_CV_CHANNEL = vx_drums::COLUMNS;   // 6
    static const int CONTROL_RATE_DIVISION = 16;

    static int tuneParam(int c)  { return c * KNOBS_PER_COLUMN + 0; }
    static int decayParam(int c) { return c * KNOBS_PER_COLUMN + 1; }
    static int shapeParam(int c) { return c * KNOBS_PER_COLUMN + 2; }
    static int levelParam(int c) { return c * KNOBS_PER_COLUMN + 3; }

    // Row r (0 tune 1 decay 2 shape 3 level) -> its CV jack.
    static int cvInputForRow(int r) { return TUNE_CV_INPUT + r; }

    vx_drums::Voices voices;

    // ── Kit state (UI writes, audio reads; see the header comment) ──
    vx_drums::KitId kit = vx_drums::KIT_HOUSE;
    int model_override[vx_drums::COLUMNS] = { NO_OVERRIDE, NO_OVERRIDE, NO_OVERRIDE, NO_OVERRIDE, NO_OVERRIDE, NO_OVERRIDE };

    // One Schmitt per TRIG channel. Thresholds are the house pair
    // (constants::gate_low_trigger 0.1 V / gate_high_trigger 2 V, the idiom at
    // PianoRoll.hpp:611-613) rather than the source's 1 V / 0.1 V
    // (vxdrumvoices.c:372-381) — DESIGN.md §2.
    dsp::SchmittTrigger trig_triggers[vx_drums::COLUMNS];

    // The EFFECTIVE values (knob + CV, see effectiveValue) last pushed into
    // `voices`. A setter runs only when its effective value changed
    // (DESIGN.md §3.3): every coefficient-feeding setter does an exp() or
    // sin(). With the control-rate divider below, that is at most 27 compares
    // and 27 setter calls per 16 samples even under a fast LFO.
    float cached[NUM_PARAMS] = {};

    // VARY: every strike draws a fresh random offset, -1..1, for each of the
    // struck column's four knobs (a sample-and-hold "humanize" rather than a
    // continuous wobble, so a hit keeps one character for its whole ring).
    // Scaled by the VARY knob and a per-row depth in effectiveValue().
    float vary_offset[vx_drums::COLUMNS][KNOBS_PER_COLUMN] = {};

    // Control-rate gate for the knob + CV read (the house 16-sample idiom).
    dsp::ClockDivider control_divider;

    double out[vx_drums::OUTPUTS] = {};   // [mixL, mixR, c0..c5], volts

    VXDrums()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // Sound knobs: normalized, defaulting to the House kit's model for the
        // column (DESIGN-KITS.md §1 table). The label/unit strings here are
        // fallbacks only; VXDrumsKnobQuantity derives the live ones.
        const vx_drums::KitSpec& house = vx_drums::kitSpec(vx_drums::KIT_HOUSE);
        for (int c = 0; c < vx_drums::COLUMNS; c++)
        {
            const vx_drums::ModelSpec& ms = vx_drums::modelSpec(house.models[c]);
            const std::string prefix = vx_drums_ui::COLUMN_PREFIX[c];
            const vx_drums::KnobSpec* specs[3] = { &ms.tune, &ms.decay, &ms.shape };
            for (int which = 0; which < 3; which++)
            {
                VXDrumsKnobQuantity* q = configParam<VXDrumsKnobQuantity>(
                    c * KNOBS_PER_COLUMN + which, 0.f, 1.f,
                    (float)vx_drums::knobDefault01(*specs[which]),
                    prefix + " " + specs[which]->name);
                q->column = c;
                q->which = which;
            }
            configParam(levelParam(c), 0.f, 1.2f, vx_drums_ui::LEVEL_DEFAULT[c], prefix + " Level", "%", 0.f, 100.f);
        }

        configParam(M_ACCENT_PARAM, 0.f, 1.f, 0.5f,  "Accent", "%", 0.f, 100.f);   // vxdrumvoices.c:308-314
        configParam(M_DRIVE_PARAM,  0.f, 1.f, 0.15f, "Drive",  "%", 0.f, 100.f);
        configParam(M_VOLUME_PARAM, 0.f, 1.f, 0.8f,  "Volume", "%", 0.f, 100.f);
        configParam(M_VARY_PARAM,   0.f, 1.f, 0.f,   "Vary (per-hit random spread of TUNE, DECAY, SHAPE and LEVEL)", "%", 0.f, 100.f);

        configInput(TRIG_INPUT, "Triggers (poly: 1 BD, 2 SD, 3 CP, 4 PERC, 5 CH, 6 OH)");
        configInput(ACC_INPUT,  "Accent gate (mono normals to all voices; poly = per voice)");
        configInput(TUNE_CV_INPUT,  "Tune CV (poly: 1-6 = voice column, 7 = accent; 10 V = full knob range)");
        configInput(DECAY_CV_INPUT, "Decay CV (poly: 1-6 = voice column; 10 V = full knob range)");
        configInput(SHAPE_CV_INPUT, "Shape CV (poly: 1-6 = voice column, 7 = drive; 10 V = full knob range)");
        configInput(LEVEL_CV_INPUT, "Level CV (poly: 1-6 = voice column, 7 = volume; 10 V = full knob range)");

        configOutput(LEFT_OUTPUT,  "Mix left");
        configOutput(RIGHT_OUTPUT, "Mix right");
        configOutput(BD_OUTPUT,    "Bass drum (dry)");
        configOutput(SD_OUTPUT,    "Snare (dry)");
        configOutput(CP_OUTPUT,    "Clap (dry)");
        configOutput(PERC_OUTPUT,  "Percussion (dry)");
        configOutput(CH_OUTPUT,    "Closed hat (dry)");
        configOutput(OH_OUTPUT,    "Open hat (dry)");
        // No configBypass: there is no through path (DESIGN.md §3.1).

        // Rate first (so the setters below cache against the real rate — the
        // Timeline idiom, Timeline.hpp:140), then the seed: distinct instances
        // must decorrelate their noise (vxdrumvoices.c:301-303).
        voices.setSampleRate(APP->engine->getSampleRate());
        voices.setSeed(random::u32() | 1u);

        control_divider.setDivision(CONTROL_RATE_DIVISION);

        // Prime the kit from the param defaults config() just wrote (no cable
        // is connected yet, so effective == knob), so `voices` and `cached`
        // agree before the first process(). A patch load that changes a param
        // is then caught by the control-rate compare.
        for (int i = 0; i < NUM_PARAMS; i++)
        {
            cached[i] = effectiveValue(i);
            pushParam(i, cached[i]);
        }
    }

    // ── Knob + CV ────────────────────────────────────────────────────────────

    // The CV that applies to param `id`, in volts: channel `column` of the
    // row's jack for a voice knob, channel MASTER_CV_CHANNEL for a master
    // knob (the DECAY row has none). getPolyVoltage normals a mono cable to
    // every channel; the isConnected guard makes an unpatched jack 0 V
    // (its stale voltages[] are never read) and the DECAY master case 0 V.
    float cvForParam(int id)
    {
        int row = -1;
        int channel = 0;
        if (id >= 0 && id < vx_drums::COLUMNS * KNOBS_PER_COLUMN)
        {
            row = id % KNOBS_PER_COLUMN;
            channel = id / KNOBS_PER_COLUMN;
        }
        else
        {
            channel = MASTER_CV_CHANNEL;
            switch (id)
            {
                case M_ACCENT_PARAM: row = 0; break;   // TUNE row
                case M_DRIVE_PARAM:  row = 2; break;   // SHAPE row
                case M_VOLUME_PARAM: row = 3; break;   // LEVEL row
                default: break;
            }
        }
        if (row < 0) return 0.f;
        Input& in = inputs[cvInputForRow(row)];
        if (!in.isConnected()) return 0.f;
        return in.getPolyVoltage((uint8_t)channel);
    }

    // effective = clamp(knob + cv / 10 * range, min, max), range = max - min.
    // Every param here has min 0: the normalized TUNE/DECAY/SHAPE and the
    // master knobs span 0..1, LEVEL spans 0..1.2. The ParamQuantity is the
    // one source of truth for the range, so a future range change follows.
    float effectiveValue(int id)
    {
        if (id < 0 || id >= NUM_PARAMS) return 0.f;
        float knob = params[id].getValue();
        ParamQuantity* q = paramQuantities[id];
        float lo = q ? q->getMinValue() : 0.f;
        float hi = q ? q->getMaxValue() : 1.f;
        float range = hi - lo;
        float vary = 0.f;
        if (id < vx_drums::COLUMNS * KNOBS_PER_COLUMN)
        {
            const int column = id / KNOBS_PER_COLUMN;
            const int row = id % KNOBS_PER_COLUMN;
            vary = params[M_VARY_PARAM].getValue() * varyDepth(row) * vary_offset[column][row] * range;
        }
        return rack::math::clamp(knob + cvForParam(id) * 0.1f * range + vary, lo, hi);
    }

    // How far VARY at 100 % can push each row, as a fraction of the knob's
    // range, either way. TUNE is kept tight: a kick that wanders 8 % of its
    // range (about +-6 Hz on an 808) reads as a live drummer; more reads as a
    // pitch sequence. Decay, shape and level take bigger swings gracefully.
    static float varyDepth(int row)
    {
        switch (row)
        {
            case 0:  return 0.08f;   // TUNE
            case 1:  return 0.25f;   // DECAY
            case 2:  return 0.30f;   // SHAPE
            default: return 0.25f;   // LEVEL (0..1.2 range, so +-0.3)
        }
    }

    // A strike on column c: new random offsets, then push that column's four
    // effective values NOW (not at the next control tick) so the coefficients
    // the strike rings with are already the varied ones.
    void drawVary(int c)
    {
        if (params[M_VARY_PARAM].getValue() <= 0.f) return;
        for (int row = 0; row < KNOBS_PER_COLUMN; row++)
        {
            vary_offset[c][row] = random::uniform() * 2.f - 1.f;
            const int id = c * KNOBS_PER_COLUMN + row;
            const float v = effectiveValue(id);
            if (v != cached[id])
            {
                cached[id] = v;
                pushParam(id, v);
            }
        }
    }

    // ── Kit / model resolution ───────────────────────────────────────────────

    vx_drums::ModelId effectiveModel(int c) const
    {
        if (c < 0 || c >= vx_drums::COLUMNS) return vx_drums::MODEL_KICK_808;
        int ov = model_override[c];
        if (ov >= 0 && ov < vx_drums::NUM_MODELS) return (vx_drums::ModelId)ov;
        return vx_drums::kitSpec(kit).models[c];
    }

    bool anyOverride() const
    {
        for (int c = 0; c < vx_drums::COLUMNS; c++)
            if (model_override[c] != NO_OVERRIDE) return true;
        return false;
    }

    // UI thread. The single write path for kit + overrides (menus, undo/redo,
    // patch load), so the double-click defaults are never left stale.
    void setKitState(vx_drums::KitId new_kit, const int overrides[vx_drums::COLUMNS])
    {
        kit = ((int)new_kit >= 0 && (int)new_kit < vx_drums::NUM_KITS) ? new_kit : vx_drums::KIT_HOUSE;
        for (int c = 0; c < vx_drums::COLUMNS; c++)
        {
            int ov = overrides[c];
            model_override[c] = (ov >= 0 && ov < vx_drums::NUM_MODELS) ? ov : NO_OVERRIDE;
        }
        syncKnobDefaults();
    }

    // Keep each TUNE/DECAY/SHAPE quantity's defaultValue equal to the
    // effective model's default, so double-click (and Rack's reset) land on
    // the model's own sweet spot. Cheap: 18 lookups. The widget calls this
    // every frame (DESIGN-KITS.md §4, "widget-thread sync"); onReset calls it
    // itself so it does not depend on a widget existing.
    void syncKnobDefaults()
    {
        for (int c = 0; c < vx_drums::COLUMNS; c++)
        {
            const vx_drums::ModelSpec& ms = vx_drums::modelSpec(effectiveModel(c));
            const vx_drums::KnobSpec* specs[3] = { &ms.tune, &ms.decay, &ms.shape };
            for (int which = 0; which < 3; which++)
            {
                ParamQuantity* q = paramQuantities[c * KNOBS_PER_COLUMN + which];
                if (q) q->defaultValue = (float)vx_drums::knobDefault01(*specs[which]);
            }
        }
    }

    // Route one param into the setter that owns it (vxdrumvoices.c:333-365).
    void pushParam(int id, float v)
    {
        double d = (double)v;
        if (id >= 0 && id < vx_drums::COLUMNS * KNOBS_PER_COLUMN)
        {
            int column = id / KNOBS_PER_COLUMN;
            switch (id % KNOBS_PER_COLUMN)
            {
                case 0: voices.setTune(column, d);  break;
                case 1: voices.setDecay(column, d); break;
                case 2: voices.setShape(column, d); break;
                case 3: voices.setLevel(column, d); break;
                default: break;
            }
            return;
        }
        switch (id)
        {
            case M_ACCENT_PARAM: voices.setAccent(d); break;
            case M_DRIVE_PARAM:  voices.setDrive(d);  break;
            case M_VOLUME_PARAM: voices.setVolume(d); break;
            case M_VARY_PARAM:   break;   // consumed in effectiveValue(), nothing to push
            default: break;
        }
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override
    {
        voices.setSampleRate(e.sampleRate);
    }

    // Initialize: House kit, no overrides, then the base resets every param
    // to its default — which syncKnobDefaults() has just pointed at House's
    // models, so the knobs land on the House sound (DESIGN-KITS.md §2).
    void onReset(const ResetEvent& e) override
    {
        int none[vx_drums::COLUMNS] = { NO_OVERRIDE, NO_OVERRIDE, NO_OVERRIDE, NO_OVERRIDE, NO_OVERRIDE, NO_OVERRIDE };
        setKitState(vx_drums::KIT_HOUSE, none);
        Module::onReset(e);
    }

    void process(const ProcessArgs& args) override
    {
        // (0) Models → voices. setModel is allocation-free and re-pushes the
        // column's stored knobs into the new model, so nothing else is owed.
        for (int c = 0; c < vx_drums::COLUMNS; c++)
        {
            vx_drums::ModelId want = effectiveModel(c);
            if (voices.model(c) != want) voices.setModel(c, want);
        }

        // (1) Knobs + CV → voices, at control rate and on change only. The
        // EFFECTIVE value is what the engine sees; a moving CV therefore
        // re-runs a setter at most every CONTROL_RATE_DIVISION samples.
        if (control_divider.process())
        {
            for (int i = 0; i < NUM_PARAMS; i++)
            {
                float v = effectiveValue(i);
                if (v != cached[i])
                {
                    cached[i] = v;
                    pushParam(i, v);
                }
            }
        }

        // (2) Triggers (DESIGN.md §2). TRIG reads getVoltage(c), NOT
        // getPolyVoltage: a mono cable fires only BD. Channels beyond the
        // cable's count are 0 V, explicitly. ACC reads getPolyVoltage(c): a
        // mono accent gate normals to every voice, a poly one is per voice.
        // The accent is a GATE sampled at the trigger instant, never latched
        // (vxdrumvoices.c:369-372), and it rides the STRIKE amplitude into each
        // circuit — harder AND brighter, the Machine's behaviour. A retrigger
        // mid-tail restarts the voice (the source's flam, :27-28). The accent
        // amount is the EFFECTIVE one (knob + TUNE CV channel 7), as last
        // read at control rate.
        int channels = inputs[TRIG_INPUT].getChannels();
        double accent = vx_drums::clampd((double)cached[M_ACCENT_PARAM], 0.0, 1.0);
        for (int c = 0; c < vx_drums::COLUMNS; c++)
        {
            float v = (c < channels) ? inputs[TRIG_INPUT].getVoltage(c) : 0.f;
            if (trig_triggers[c].process(v, constants::gate_low_trigger, constants::gate_high_trigger))
            {
                bool accented = inputs[ACC_INPUT].getPolyVoltage(c) >= constants::gate_high_trigger;
                double amp = accented ? (1.0 + accent) : 1.0;   // :372
                drawVary(c);
                voices.strikeLane(c, amp);
                lights[VOICE_LIGHTS + c].setBrightness(1.f);
            }
        }

        // (3) One sample of every voice, then (4) the jacks. The enum order
        // LEFT, RIGHT, BD..OH mirrors the out[] slots [mixL, mixR, c0..c5].
        voices.process(out);
        static_assert(LEFT_OUTPUT == 0 && OH_OUTPUT == vx_drums::OUTPUTS - 1,
                      "OutputIds must mirror vx_drums::Voices::process() slots");
        for (int i = 0; i < vx_drums::OUTPUTS; i++)
            outputs[i].setVoltage((float)out[i]);

        // (5) Strike lamps: lit on the strike above, decaying every sample.
        for (int c = 0; c < vx_drums::COLUMNS; c++)
            lights[VOICE_LIGHTS + c].setBrightnessSmooth(0.f, args.sampleTime);
    }

    // ── Persistence ──────────────────────────────────────────────────────────
    // Params are Rack's to save. Ours: the kit and the six overrides, BY UUID
    // (frozen in VXDrumVoices.hpp; never the enum value or the name —
    // DESIGN-KITS.md §1). Shape:
    //   {"version":"1.1.0","kit":"<uuid>","model_overrides":["<uuid>"|"" ×6]}
    // Written with json_object_set_new / json_array_append_new so ownership
    // transfers and no decref is owed (PianoRoll.hpp:711-717).
    json_t* dataToJson() override
    {
        json_t* json_root = json_object();
        json_object_set_new(json_root, "version", json_string("1.1.0"));
        json_object_set_new(json_root, "kit", json_string(vx_drums::kitSpec(kit).uuid));

        json_t* overrides = json_array();
        for (int c = 0; c < vx_drums::COLUMNS; c++)
        {
            int ov = model_override[c];
            const char* uuid = (ov >= 0 && ov < vx_drums::NUM_MODELS)
                ? vx_drums::modelSpec((vx_drums::ModelId)ov).uuid : "";
            json_array_append_new(overrides, json_string(uuid));
        }
        json_object_set_new(json_root, "model_overrides", overrides);
        return json_root;
    }

    // Clear first (this also runs on paste / preset load into a populated
    // module), probe every key, and let an unknown uuid fall back: kit →
    // House, override → none. A 1.0.0 patch has neither key and loads as House.
    void dataFromJson(json_t* json_root) override
    {
        if (!json_root) return;

        vx_drums::KitId new_kit = vx_drums::KIT_HOUSE;
        int overrides[vx_drums::COLUMNS] = { NO_OVERRIDE, NO_OVERRIDE, NO_OVERRIDE, NO_OVERRIDE, NO_OVERRIDE, NO_OVERRIDE };

        json_t* kit_json = json_object_get(json_root, "kit");
        if (json_is_string(kit_json))
            new_kit = vx_drums::kitFromUuid(json_string_value(kit_json), vx_drums::KIT_HOUSE);

        json_t* overrides_json = json_object_get(json_root, "model_overrides");
        if (json_is_array(overrides_json))
        {
            size_t n = json_array_size(overrides_json);
            if (n > (size_t)vx_drums::COLUMNS) n = (size_t)vx_drums::COLUMNS;
            for (size_t c = 0; c < n; c++)
            {
                json_t* s = json_array_get(overrides_json, c);
                if (!json_is_string(s)) continue;
                const char* uuid = json_string_value(s);
                if (!uuid || uuid[0] == '\0') continue;   // "" = no override
                // NUM_MODELS as the fallback sentinel: an unknown uuid stays "none".
                vx_drums::ModelId id = vx_drums::modelFromUuid(uuid, vx_drums::NUM_MODELS);
                if (id != vx_drums::NUM_MODELS) overrides[c] = (int)id;
            }
        }

        setKitState(new_kit, overrides);
        // The control-rate compares in process() then switch the models and
        // re-push any param the patch changed.
    }
};

// ── VXDrumsKnobQuantity bodies (need the complete VXDrums) ───────────────────

inline const vx_drums::KnobSpec& VXDrumsKnobQuantity::spec()
{
    VXDrums* m = dynamic_cast<VXDrums*>(module);
    vx_drums::ModelId id = m ? m->effectiveModel(column)
                             : vx_drums::kitSpec(vx_drums::KIT_HOUSE).models[
                                   (column >= 0 && column < vx_drums::COLUMNS) ? column : 0];
    const vx_drums::ModelSpec& ms = vx_drums::modelSpec(id);
    if (which == 1) return ms.decay;
    if (which == 2) return ms.shape;
    return ms.tune;
}

// Natural units, ×100 for "%" specs — the same number the tooltip shows, so
// typed entry (setDisplayValueString → setDisplayValue) round-trips exactly.
inline float VXDrumsKnobQuantity::getDisplayValue()
{
    const vx_drums::KnobSpec& k = spec();
    double natural = vx_drums::knobNatural(k, (double)getValue());
    if (k.display_multiplier_percent) natural *= 100.0;
    return (float)natural;
}

inline void VXDrumsKnobQuantity::setDisplayValue(float displayValue)
{
    const vx_drums::KnobSpec& k = spec();
    double natural = (double)displayValue;
    if (k.display_multiplier_percent) natural /= 100.0;
    if (std::isnan(natural)) return;
    setImmediateValue((float)vx_drums::knobNormalized(k, natural));
}

inline std::string VXDrumsKnobQuantity::getDisplayValueString()
{
    const vx_drums::KnobSpec& k = spec();
    return vx_drums_ui::formatKnob(k, vx_drums::knobNatural(k, (double)getValue()));
}

inline std::string VXDrumsKnobQuantity::getLabel()
{
    int c = (column >= 0 && column < vx_drums::COLUMNS) ? column : 0;
    return std::string(vx_drums_ui::COLUMN_PREFIX[c]) + " " + spec().name;
}

inline std::string VXDrumsKnobQuantity::getUnit()
{
    return spec().unit;
}

// ── Undo: one action type for the kit menu and the override submenus ─────────
//
// Before/after copies of the whole kit state (7 ints), the module referenced
// BY ID (history.hpp: Rack may delete and restore the object). Usage (UI
// thread): captureBefore(m); mutate through m->setKitState(); captureAfter(m);
// skip the push when isNoop(). The helpers below do exactly that.
//
struct VXDrumsKitAction : history::ModuleAction
{
    vx_drums::KitId kit_before = vx_drums::KIT_HOUSE;
    vx_drums::KitId kit_after = vx_drums::KIT_HOUSE;
    int overrides_before[vx_drums::COLUMNS] = { -1, -1, -1, -1, -1, -1 };
    int overrides_after[vx_drums::COLUMNS] = { -1, -1, -1, -1, -1, -1 };

    VXDrumsKitAction()
    {
        name = "change kit";
    }

    void captureBefore(VXDrums* m)
    {
        moduleId = m->id;
        kit_before = m->kit;
        for (int c = 0; c < vx_drums::COLUMNS; c++) overrides_before[c] = m->model_override[c];
    }

    void captureAfter(VXDrums* m)
    {
        kit_after = m->kit;
        for (int c = 0; c < vx_drums::COLUMNS; c++) overrides_after[c] = m->model_override[c];
    }

    bool isNoop() const
    {
        if (kit_before != kit_after) return false;
        for (int c = 0; c < vx_drums::COLUMNS; c++)
            if (overrides_before[c] != overrides_after[c]) return false;
        return true;
    }

    void apply(vx_drums::KitId kit, const int overrides[vx_drums::COLUMNS])
    {
        VXDrums* m = dynamic_cast<VXDrums*>(APP->engine->getModule(moduleId));
        if (!m) return;
        m->setKitState(kit, overrides);
    }

    void undo() override { apply(kit_before, overrides_before); }
    void redo() override { apply(kit_after, overrides_after); }
};

// Pick a kit. Overrides are kept (DESIGN-KITS.md §5): a kit is the default
// row, an override is the user's explicit choice for that column.
inline void vxDrumsChangeKit(VXDrums* m, vx_drums::KitId kit)
{
    if (!m) return;
    VXDrumsKitAction* action = new VXDrumsKitAction;
    action->name = "change kit";
    action->captureBefore(m);
    // A kit change is a fresh start: every per-column override is cleared so the
    // kit plays as designed (Bret, 2026-09-02). Undo restores the overrides.
    int none[vx_drums::COLUMNS];
    for (int c = 0; c < vx_drums::COLUMNS; c++) none[c] = VXDrums::NO_OVERRIDE;
    m->setKitState(kit, none);
    action->captureAfter(m);
    if (action->isNoop()) { delete action; return; }
    APP->history->push(action);
}

// Set (model >= 0) or clear (model == VXDrums::NO_OVERRIDE) one column's override.
inline void vxDrumsChangeOverride(VXDrums* m, int column, int model)
{
    if (!m || column < 0 || column >= vx_drums::COLUMNS) return;
    VXDrumsKitAction* action = new VXDrumsKitAction;
    action->name = "change voice model";
    action->captureBefore(m);
    int overrides[vx_drums::COLUMNS] = {};
    for (int c = 0; c < vx_drums::COLUMNS; c++) overrides[c] = m->model_override[c];
    overrides[column] = model;
    m->setKitState(m->kit, overrides);
    action->captureAfter(m);
    if (action->isNoop()) { delete action; return; }
    APP->history->push(action);
}
