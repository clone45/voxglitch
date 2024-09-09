// Text created using https://fsymbols.com/generators/carty/
// Link to design: https://docs.google.com/document/d/1wMAJvvd6BVBRalLTuQMHSaaxSqGrHbze5EtJAA4Q89c/edit

/* Next:

Business related:
 - Prepare json for release
 - Tooltips for start and end window

Features and functionality:
- Clear note buffer on reset, or when the note cable is disconnected
- light mode / dark mode switching
- Create presets for the ArpSeq

Later on:
 - Big code review / Look for memory leaks
 
Refactor ideas:
* move dice roll results into the page struct
* group all custom controls and variables into a "custom controls" class?  It could have the randomize and intitalize features.
* Create a "MutableSequencer" class that contains the chance sequencer, voltage sequencer, and cycle counters.
* Create a "Mod1Sequencer" and "Mod2Sequencer" class that contains the MutableSequencer, attenuator, and polarity.
* Consider creating an arpeggiator class

*/

#include "RateModel.hpp"
#include "ShapeModel.hpp"
#include "CycleCounters.hpp"
#include "Page.hpp"

struct ArpSeq : Module
{
    std::string version = "2.0.0";

    Page pages[NUMBER_OF_PAGES];
    std::function<std::string(int, float)> tooltip_callbacks[NUMBER_OF_PAGES];

    dsp::SchmittTrigger step_trigger;
    dsp::SchmittTrigger reset_trigger;
    dsp::SchmittTrigger page_button_triggers[NUMBER_OF_PAGES];
    dsp::PulseGenerator probability_pulse_generator;
    dsp::PulseGenerator cycle_pulse_generator;

    Quantizer output_quantizer;
    Quantizer transpose_quantizer;
    SampleAndHold sample_and_hold;
    ClockDivider clock_divider;
    ClockModifier clock_modifier;
    ArpSequencer arp_sequencer;
    SlewLimiter mod_1_slew_limiter;
    SlewLimiter mod_2_slew_limiter;


    // The purpose of the channel_step_counter is to determine
    // when the arppegiation of the notes is complete.  This is
    // non-trivial because of the random mode for the channel sequencer,
    // and requires this additional counter.
    unsigned int channel_step_counter = 0;

    unsigned int playback_mode = ArpSequencer::PlaybackMode::MANUAL;
    float current_voltage = 0.0f;
    unsigned int page = 0;
    bool apply_sequencer[NUMBER_OF_PAGES];
    bool trigger_cycle_output = false;
    std::vector<float> gate_voltages;
    std::map<int, bool> prevent_duplicate_notes;
    unsigned int number_of_gate_channels = 0;
    unsigned int number_of_note_channels = 0;
    float mod1_input = 0.0f;
    float mod2_input = 0.0f;
    float rate_attenuverter_range = 5.0;
    float shape_attenuverter_range = 5.0;

    // double gate_length_ms = 0.01;
    double time_before = 0.0;
    double time_now = 0.0;
    double time_of_last_clock = 0.0;
    double time_between_clocks_ms = 0.0;
    double gate_timer = 0.0;
    long clock_ignore_on_reset = 0;
    bool first_step = true;

    bool is_latch_accumulating = false;
    bool previous_any_gate = false;
    bool on_switch = false;
    unsigned int note_repeat = 2; // 2 == no repeat
    std::string shape_readout = "FWD";
    std::string rate_readout = " x1";

    // These variables will be sent down by reference into the digital
    // controls for modification.  They will also need to be saved and loaded
    // separately from the params.  They're also included in the initialize feature.
    float mod1_attenuation_low = 0.0f;
    float mod1_attenuation_high = 1.0f;
    float mod2_attenuation_low = 0.0f;
    float mod2_attenuation_high = 1.0f;
    float mod1_slew = 0.0f;
    float mod2_slew = 0.0f;
    bool mod1_polarity = false;
    bool mod2_polarity = false;
    bool quantize_transpose_cv = true;
    bool sample_and_hold_mode = true;
    bool legacy_reset_mode = false;    
    unsigned int probability_output_sequencer_attachment = GATE_SEQUENCER; // 10832fbc-d17b-460c-bafc-089a5b7c28db
    unsigned int cycle_output_sequencer_attachment = GATE_SEQUENCER; // ca5a92fc-7619-4f62-82a9-cd34f3d20324
    unsigned int output_quantization_scale_index = Quantizer::CHROMATIC_SCALE;
    unsigned int output_quantization_root_note_index = Quantizer::C;
    bool output_quantization = false;
    bool step_mode = STEP_EVERY_CLOCK;
    int probability_trigger_length_index = 0;
    int cycle_trigger_length_index = 0;
    //
    //

    std::vector<float> trigger_lengths = {0.001, 0.002, 0.005, 0.010, 0.020, 0.050, 0.100, 0.200};

    RateModel rate_model;
    ShapeModel shape_model;

    // Welcome to the gate_lock mini-tutorial.  Here I'll explain this magical and elusive variable.
    //
    // It's useful to think of this in three parts:
    //
    // 1. The current gate is fully ON (10.0V), and the next gate is also fully ON (10.0V)
    //    In this case, the gate output should remain HIGH until some subsequent gate drops to zero.
    // 2. The current gate is fully ON, and the next gate is some value between OFF (0.0V) and ON (10.0V)
    //    In this case, the gate output should remain HIGH until the next gate goes fully OFF.
    // 3. The current gate is fully ON, and the next gate is fully OFF (0.0V)
    //    In this case, the gate output should remain HIGH until the next step.
    //
    // This doesn't happen naturally because time_between_clocks_ms is often just shy of the time
    // between clock pulses, causing the first gate to drop to zero and the second gate to quickly rise to 10V.
    // This causes a very short gate to be output.  To fix this, we need to "lock" the gate output
    // to HIGH until the second gate is fully OFF.
    //

    bool gate_lock = false;

    enum ParamIds
    {
        LATCH_SWITCH,
        ON_SWITCH,
        RATE_KNOB,
        RATE_ATTENUATOR,
        SHAPE_KNOB,
        SHAPE_ATTENUATOR,
        ENUMS(CYCLE_KNOBS, MAX_SEQUENCER_STEPS *NUMBER_OF_PAGES),
        NUM_PARAMS
    };

    enum InputIds
    {
        POLY_NOTES_INPUT = 0,
        POLY_GATE_INPUT = 1,
        CLOCK_INPUT = 2,
        RESET_INPUT = 3,
        RATE_INPUT = 4,
        SHAPE_INPUT = 5,
        NUM_INPUTS // This will automatically be 6
    };

    enum OutputIds
    {
        PITCH_OUTPUT = 0,
        GATE_OUTPUT = 1,
        MOD1_OUTPUT = 2,
        MOD2_OUTPUT = 3,
        PROBABILITY_GATE_OUTPUT = 4,
        CYCLE_GATE_OUTPUT = 5,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        ON_LIGHT = 1,
        LATCH_LIGHT = 2,
        NUM_LIGHTS
    };

    //
    // Constructor
    //
    ArpSeq()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(POLY_NOTES_INPUT, "Poly Notes");
        configInput(POLY_GATE_INPUT, "Poly Gates");
        configInput(CLOCK_INPUT, "Clock");
        configInput(RESET_INPUT, "Reset");
        configInput(RATE_INPUT, "Rate");
        configInput(SHAPE_INPUT, "Shape");

        configOutput(PITCH_OUTPUT, "Pitch");
        configOutput(GATE_OUTPUT, "Gate");
        configOutput(MOD1_OUTPUT, "Mod1");
        configOutput(MOD2_OUTPUT, "Mod2");
        configOutput(PROBABILITY_GATE_OUTPUT, "Probability Gate");
        configOutput(CYCLE_GATE_OUTPUT, "Cycle Gate");

        // Configure the switches
        configParam(LATCH_SWITCH, 0.0f, 1.0f, 0.0f, "Latch");
        configParam(ON_SWITCH, 0.0f, 1.0f, 0.0f, "ON = Always Arpeggiate");

        paramQuantities[RATE_KNOB] = createClockDividerRateParamQuantity(this, RATE_KNOB, -7.0f, 7.0f, 0.0f, 1.0f, true, "Rate");
        paramQuantities[SHAPE_KNOB] = createShapeParamQuantity(this, SHAPE_KNOB, 0.0f, NUMBER_OF_PLAYBACK_MODES - 1, 0.0f, 1.0f, true, "Shape");

        configParam(RATE_ATTENUATOR, -1.0f, 1.0f, 0.0f, "Rate Attenuverter");
        configParam(SHAPE_ATTENUATOR, -1.0f, 1.0f, 0.0f, "Shape Attenuverter");

        for (unsigned int p = 0; p < NUMBER_OF_PAGES; p++)
        {
            apply_sequencer[p] = false;

            pages[p].chance_sequencer.snap_division = 0; // none
            pages[p].chance_sequencer.setDefaults(1.0, 1.0);
            pages[p].chance_sequencer.fill(1.0);

            // Configure cycle knobs
            for (int c = 0; c < MAX_SEQUENCER_STEPS; c++)
            {
                pages[p].cycle_counters->setCycle(c, params[CYCLE_KNOBS + c].getValue());

                unsigned int cycle_knob_index = getCycleParameterIndex(p, c);
                configParam(cycle_knob_index, CYCLE_MIN_VALUE, CYCLE_MAX_VALUE, CYCLE_DEFAULT_VALUE, "Cycle");
                paramQuantities[cycle_knob_index]->snapEnabled = true;
            }
        }

        //
        // Define tooltip callbacks for labels
        //

        tooltip_callbacks[0] = [this](int column, float value)
        { return getTooltipTextForGate(column, value); };
        tooltip_callbacks[1] = [this](int column, float value)
        { return getTooltipTextForTranspose(column, value); };
        tooltip_callbacks[2] = [this](int column, float value)
        { return getTooltipTextForMod1(column, value); };
        tooltip_callbacks[3] = [this](int column, float value)
        { return getTooltipTextForMod2(column, value); };

        //
        // Set TRANSPOSE_SEQUENCER specific defaults and default to 0.5
        //

        pages[TRANSPOSE_SEQUENCER].voltage_sequencer.setSnapDivision(24);
        pages[TRANSPOSE_SEQUENCER].voltage_sequencer.setPolarity(VoltageSequencer::BIPOLAR);
        pages[TRANSPOSE_SEQUENCER].voltage_sequencer.setDefaults(0.0, 0.5);
        pages[TRANSPOSE_SEQUENCER].voltage_sequencer.fill();

        //
        // Set MOD1 and MOD22 specific defaults
        //
        pages[MOD1_SEQUENCER].voltage_sequencer.setDefaults(0.0, 0.5);
        pages[MOD2_SEQUENCER].voltage_sequencer.setDefaults(0.0, 0.5);

        //
        // Set GATE_SEQUENCER specific defaults and default to 0.5
        //

        pages[GATE_SEQUENCER].voltage_sequencer.snap_division = 0; // None
        pages[GATE_SEQUENCER].voltage_sequencer.setDefaults(0.5, 0.5);
        pages[GATE_SEQUENCER].voltage_sequencer.fill();


        // Set quantizer defaults
        output_quantizer.setScale(Quantizer::CHROMATIC_SCALE);
        output_quantizer.setRoot(Quantizer::C);

        gate_voltages.resize(16);

        // On boot, I seem to be getting some weird gate signals.  This keeps those
        // from triggering an output pulse when the module first loads.
        clock_ignore_on_reset = (long)(rack::settings::sampleRate / 100);

        transpose_quantizer.setScale(Quantizer::CHROMATIC_SCALE);
    }

    //
    // Destructor
    //
    ~ArpSeq()
    {
    }

    // █▀ ▄▀█ █░█ █▀▀   ▄▀█ █▄░█ █▀▄   █░░ █▀█ ▄▀█ █▀▄
    // ▄█ █▀█ ▀▄▀ ██▄   █▀█ █░▀█ █▄▀   █▄▄ █▄█ █▀█ █▄▀

    // Save
    json_t *dataToJson() override
    {
        json_t *root_json = json_object();
        json_t *pages_json = json_array();

        // Save the version
        json_object_set_new(root_json, "version", json_string(version.c_str()));

        for (unsigned int page = 0; page < NUMBER_OF_PAGES; page++)
        {
            json_t *page_json = json_object();

            json_object_set_new(page_json, "voltage_sequencer", IO::saveSequencer(pages[page].voltage_sequencer));
            json_object_set_new(page_json, "chance_sequencer", IO::saveSequencer(pages[page].chance_sequencer));

            json_array_append_new(pages_json, page_json);
        }

        json_object_set_new(root_json, "pages", pages_json);

        json_object_set_new(root_json, "mod1_attenuation_high", json_real(mod1_attenuation_high));
        json_object_set_new(root_json, "mod1_attenuation_low", json_real(mod1_attenuation_low));
        json_object_set_new(root_json, "mod2_attenuation_high", json_real(mod2_attenuation_high));
        json_object_set_new(root_json, "mod2_attenuation_low", json_real(mod2_attenuation_low));
        json_object_set_new(root_json, "mod1_slew", json_real(mod1_slew));
        json_object_set_new(root_json, "mod2_slew", json_real(mod2_slew));
        json_object_set_new(root_json, "mod1_polarity", json_boolean(mod1_polarity));
        json_object_set_new(root_json, "mod2_polarity", json_boolean(mod2_polarity));
        json_object_set_new(root_json, "probability_trigger_length_index", json_real(probability_trigger_length_index));
        json_object_set_new(root_json, "cycle_trigger_length_index", json_real(cycle_trigger_length_index));

        json_object_set_new(root_json, "sample_and_hold_mode", json_boolean(sample_and_hold_mode));
        json_object_set_new(root_json, "legacy_reset_mode", json_boolean(legacy_reset_mode));
        json_object_set_new(root_json, "step_mode", json_integer(step_mode));
        json_object_set_new(root_json, "output_quantization", json_boolean(output_quantization));
        json_object_set_new(root_json, "output_quantization_scale_index", json_integer(output_quantization_scale_index));
        json_object_set_new(root_json, "output_quantization_root_note_index", json_integer(output_quantization_root_note_index));
        json_object_set_new(root_json, "probability_output_sequencer_attachment", json_integer(probability_output_sequencer_attachment));
        json_object_set_new(root_json, "cycle_output_sequencer_attachment", json_integer(cycle_output_sequencer_attachment));
        json_object_set_new(root_json, "rate_attenuverter_range", json_real(rate_attenuverter_range));
        json_object_set_new(root_json, "shape_attenuverter_range", json_real(shape_attenuverter_range));

        return root_json;
    }

    // Load
    void dataFromJson(json_t *root_json) override
    {
        json_t *pages_json = json_object_get(root_json, "pages");

        if (!pages_json)
            return;

        for (unsigned int page = 0; page < json_array_size(pages_json) && page < NUMBER_OF_PAGES; page++)
        {
            json_t *page_json = json_array_get(pages_json, page);
            if (page_json)
            {
                IO::loadSequencer(page_json, "voltage_sequencer", pages[page].voltage_sequencer);
                IO::loadSequencer(page_json, "chance_sequencer", pages[page].chance_sequencer);
            }
        }

        mod1_attenuation_high = JSON::getNumber(root_json, "mod1_attenuation_high");
        mod1_attenuation_low = JSON::getNumber(root_json, "mod1_attenuation_low");
        mod2_attenuation_high = JSON::getNumber(root_json, "mod2_attenuation_high");
        mod2_attenuation_low = JSON::getNumber(root_json, "mod2_attenuation_low");
        mod1_slew = JSON::getNumber(root_json, "mod1_slew");
        mod2_slew = JSON::getNumber(root_json, "mod2_slew");
        mod1_polarity = JSON::getBoolean(root_json, "mod1_polarity");
        mod2_polarity = JSON::getBoolean(root_json, "mod2_polarity");
        probability_trigger_length_index = JSON::getNumber(root_json, "probability_trigger_length_index");
        cycle_trigger_length_index = JSON::getNumber(root_json, "cycle_trigger_length_index");
        sample_and_hold_mode = JSON::getBoolean(root_json, "sample_and_hold_mode");
        legacy_reset_mode = JSON::getBoolean(root_json, "legacy_reset_mode");
        step_mode = JSON::getInteger(root_json, "step_mode");
        probability_output_sequencer_attachment = JSON::getInteger(root_json, "probability_output_sequencer_attachment");
        cycle_output_sequencer_attachment = JSON::getInteger(root_json, "cycle_output_sequencer_attachment");
        rate_attenuverter_range = JSON::getNumber(root_json, "rate_attenuverter_range");
        shape_attenuverter_range = JSON::getNumber(root_json, "shape_attenuverter_range");

        this->setQuantization(JSON::getBoolean(root_json, "output_quantization"));
        this->setQuantizationScale(JSON::getInteger(root_json, "output_quantization_scale_index"));
        this->setQuantizationRoot(JSON::getInteger(root_json, "output_quantization_root_note_index"));
    }
 

    // █▀█ █▀█ █▀█ █▀▀ █▀▀ █▀ █▀
    // █▀▀ █▀▄ █▄█ █▄▄ ██▄ ▄█ ▄█

    void process(const ProcessArgs &args) override
    {
        time_before = time_now;
        time_now += args.sampleTime;
        gate_timer -= args.sampleTime;

        // Read the latch switch
        bool latch = (params[LATCH_SWITCH].getValue() > 0.5f);

        processNoteAndGateInputs(latch);

        bool reset_triggered = reset_trigger.process(inputs[RESET_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger);

        // Process and store the rate information
        //
        // rate_model.update() reads the rate inputs  (knob, attenuator, and cv input),
        // then calculates and stores the rate_index.  .getRateIndex may be called to
        // retrieve the rate_index.

        rate_model.update(
            inputs[RATE_INPUT].getVoltage(), 
            params[RATE_KNOB].getValue(), 
            params[RATE_ATTENUATOR].getValue(),
            rate_attenuverter_range
        );
        
        rate_readout = rate_model.toString();

        // Read and process the shape inputs

        playback_mode = shape_model.update(
            inputs[SHAPE_INPUT].getVoltage(), 
            params[SHAPE_KNOB].getValue(), 
            params[SHAPE_ATTENUATOR].getValue(),
            shape_attenuverter_range
        );

        arp_sequencer.setPlaybackMode(playback_mode);

        shape_readout = shape_model.toString();

        // The polarity values may have been updated by the DigitalSwitch or by the save/load code,
        // so we need to ensure that the sequencers have the latest value
        pages[MOD1_SEQUENCER].voltage_sequencer.setPolarity(toPolarity(mod1_polarity));
        pages[MOD2_SEQUENCER].voltage_sequencer.setPolarity(toPolarity(mod2_polarity));

        //
        // Process reset input
        //
        if (reset_triggered)
        {
            reset();
            return;
        }

        //
        // Lower gate output after gate_timer expires
        //

        if ((gate_timer <= 0.0) && (gate_lock == false))
            outputs[GATE_OUTPUT].setVoltage(0.0);

        bool clock_triggered = isClockTriggered(time_now);

        //
        // Compute the time between clock input pulses
        // and store it in time_between_clocks_ms
        //
        if (clock_triggered)
        {
            time_between_clocks_ms = time_now - time_of_last_clock;
            time_of_last_clock = time_now;
        }

        //
        // Important: This is the part of the code which stops the sequencers
        // from running when the user doesn't hold down any notes.

        bool any_notes_to_play = prepareArpSequencer();

        if (any_notes_to_play == false)
        {
            return;
        }

        //
        // Handle note repeat
        //

        if (shape_model.isNoteRepeat() && clock_triggered)
        {
            note_repeat--;

            if (note_repeat > 0)
            {
                outputGate(pages[GATE_SEQUENCER].voltage_sequencer.getValue());

                // Don't allow normal processing to continue
                return;
            }
            else
            {
                note_repeat = 2;
                // Allow for normal processing to continue
            }
        }

        bool step_sequencers_forward = false;

        if (clock_triggered)
        {
            channel_step_counter++;

            arp_sequencer.step();

            if (step_mode == STEP_EVERY_CLOCK)
            {
                step_sequencers_forward = true;
            }

            if (channel_step_counter >= number_of_note_channels)
            {
                channel_step_counter = 0;
                step_sequencers_forward = true;
            }
        }

        if (step_sequencers_forward && (clock_ignore_on_reset == 0 || legacy_reset_mode == true))
        {
            // Two important things happen here.
            // 1. The sequencers are stepped

            if (legacy_reset_mode || first_step == false)
            {
                stepSequencers();
            }
            else
            {
                first_step = false;
            }

            // 2. The cycle counters are decremented.  However, as well as decrementing the
            //    cycle counters, this code also calculates the dice roll and sets the
            //    apply_sequencer array to true or false, depending on if a cycle counter
            //    reaches zero.

            decrementCycleCounters(); // This may modify the apply_sequencer array
        }

        float note_cv = arp_sequencer.getNote();

        // If the cycle is completed and dice roll is not "pass", then transpose the note
        if (apply_sequencer[TRANSPOSE_SEQUENCER] && pages[TRANSPOSE_SEQUENCER].dice_roll_outcome)
        {
            // getValue returns 0 to 1.0, so we need to map it to -1 to 1 for a range of 2 octaves
            // note_cv = note_cv + mapValueToVoltage(pages[TRANSPOSE_SEQUENCER].voltage_sequencer.getValue());
            note_cv = note_cv + (2.0f * clamp(pages[TRANSPOSE_SEQUENCER].voltage_sequencer.getValue(), 0.0f, 1.0f) - 1.0f);
        }

        // Quantize the note if the quantize switch is ON
        // if (params[QUANTIZE_SWITCH].getValue() > 0.5f)
        if (output_quantization)
        {
            note_cv = this->quantize(note_cv);
        }

        if (clock_triggered && step_mode == STEP_AFTER_ARP)
        {
            // In STEP_AFTER_ARP mode, when the clock is triggered,
            // then the gate sequencer should be applied to the gate output
            // on every clock pulse.  "Applied" means that the gate output
            // may or may not be toggled, depending on the gate sequencer.

            // Why does this exist?
            //
            // It is to ensure that that, when in STEP_AFTER_ARP mode, the
            // notes within the arpeggio are played with the gate and
            // percentage values applied.

            // The problem here is that if the cycle counters have note reached
            // zero, then gates will fire when they should not, therefore we'll
            // need to check the cycle counters here first

            unsigned int step_position = pages[GATE_SEQUENCER].voltage_sequencer.getPosition();

            // If the cycle count down is one or less, then apply the sequencer
            apply_sequencer[GATE_SEQUENCER] = pages[GATE_SEQUENCER].cycle_counters->getCountDown(step_position) <= 1;
        }

        // If both the apply_sequencer and dice roll are true, then output a gate.
        // When in sample and hold mode, the transposition sequencer is sampled.

        if (apply_sequencer[GATE_SEQUENCER] && pages[GATE_SEQUENCER].dice_roll_outcome && clock_triggered)
        {
            float gate_cv = pages[GATE_SEQUENCER].voltage_sequencer.getValue();

            outputGate(gate_cv);
            apply_sequencer[GATE_SEQUENCER] = false;

            // Activate the sample and hold feature
            if (gate_cv > 0.0f && sample_and_hold_mode)
            {
                sample_and_hold.trigger(note_cv);
            }
        }

        if (sample_and_hold_mode)
        {
            note_cv = sample_and_hold.getValue();
        }

        // Output the poly input voltage to the pitch output
        outputs[PITCH_OUTPUT].setVoltage(note_cv);

        // If the apply_sequencer is true for the mod1 sequencer, then output the mod1 sequencer value
        if (apply_sequencer[MOD1_SEQUENCER] && pages[MOD1_SEQUENCER].dice_roll_outcome)
        {
            mod1_input = applyRange(pages[MOD1_SEQUENCER].voltage_sequencer.getValue(), mod1_attenuation_low, mod1_attenuation_high, mod1_polarity);
        }
        mod_1_slew_limiter.setRiseFallTimes(mod1_slew / 4.0, mod1_slew / 4.0);
        outputs[MOD1_OUTPUT].setVoltage(mod_1_slew_limiter.process(mod1_input));


        // If the apply_sequencer is true for the mod2 sequencer, then output the mod2 sequencer value
        if (apply_sequencer[MOD2_SEQUENCER] && pages[MOD2_SEQUENCER].dice_roll_outcome)
        {
            mod2_input = applyRange(pages[MOD2_SEQUENCER].voltage_sequencer.getValue(), mod2_attenuation_low, mod2_attenuation_high, mod2_polarity);
        }
        mod_2_slew_limiter.setRiseFallTimes(mod2_slew / 4.0, mod2_slew / 4.0);
        outputs[MOD2_OUTPUT].setVoltage(mod_2_slew_limiter.process(mod2_input));

        outputs[PROBABILITY_GATE_OUTPUT].setVoltage(probability_pulse_generator.process(args.sampleTime) ? 10.0f : 0.0f);


        //
        // Handle the cycle output
        //

        if (trigger_cycle_output)
        {
            cycle_pulse_generator.trigger(trigger_lengths[cycle_trigger_length_index]);
            trigger_cycle_output = false;
        }

        outputs[CYCLE_GATE_OUTPUT].setVoltage(cycle_pulse_generator.process(args.sampleTime) ? 10.0f : 0.0f);


        // Decrement the clock_ignore_on_reset counter
        if (clock_ignore_on_reset > 0)
            clock_ignore_on_reset--;
    }

    // ======================================================================

    float applyRange(float value, float low, float high, bool polarity)
    {
        // Step 1: Calculate the actual range based on polarity
        float actualLow = polarity ? -5.0f : 0.0f;
        float actualHigh = polarity ? 5.0f : 10.0f;

        // Step 2: Map the value to the selected low and high within the actual range
        float scaledLow = actualLow + low * (actualHigh - actualLow);
        float scaledHigh = actualLow + high * (actualHigh - actualLow);

        float result = scaledLow + value * (scaledHigh - scaledLow);

        return result;
    }

    void outputGate(float gate_cv)
    {
        gate_lock = (gate_cv == 1.0);
        gate_timer = time_between_clocks_ms * gate_cv;

        if (gate_cv == 0.0)
        {
            outputs[GATE_OUTPUT].setVoltage(0.0);
            gate_lock = false;
        }
        else if (gate_timer > 0.0)
        {
            outputs[GATE_OUTPUT].setVoltage(10.0);
        }
    }

    void decrementCycleCounters()
    {
        for (unsigned int page = 0; page < NUMBER_OF_PAGES; page++)
        {
            unsigned int step_position = pages[page].voltage_sequencer.getPosition();
            int count_down = pages[page].cycle_counters->getCountDown(step_position);
            unsigned int max_cycle = params[getCycleParameterIndex(page, step_position)].getValue() - 1;

            if(max_cycle == 0)
            {
                bool dice_roll_outcome = pages[page].rollDice();
                onCycle(page, dice_roll_outcome);
            }
            else if(count_down <= 0)
            {
                // Reset the counter
                pages[page].cycle_counters->setCountDown(step_position, max_cycle);

                bool dice_roll_outcome = pages[page].rollDice();
                onCycle(page, dice_roll_outcome);

                // Set trigger_cycle_output to true if the cycle for the gate sequencer is set and the cycle fires
                // probability should be applied based on the probability of the gate sequencer
                if ((page == cycle_output_sequencer_attachment) && dice_roll_outcome && max_cycle > 1)
                {
                    trigger_cycle_output = true;
                }
            }
            // Otherwise, decrement the counter at the current step position
            else
            {
                pages[page].cycle_counters->decrement(step_position);
                apply_sequencer[page] = false;
            }
        }
    }

    void onCycle(unsigned int page, bool dice_roll_outcome)
    {
        pulseProbabilityOutput(page, dice_roll_outcome);
        apply_sequencer[page] = true; 
    }

    void pulseProbabilityOutput(unsigned int page, bool dice_roll_outcome)
    {
        if (page == probability_output_sequencer_attachment)
        {
            float chance = pages[page].chance_sequencer.getValue();

            // Only fire the probability output if there's chance involved
            // In other words, if the chance sequencer is set to 0 or 1, then
            // don't fire the probability output.

            if (chance > 0.0f && chance < 1.0f)
            {
                probability_pulse_generator.trigger(dice_roll_outcome ? trigger_lengths[probability_trigger_length_index] : 0.0f);
            }
        }
    }


    void resetCycleCounters()
    {
        for (unsigned int page = 0; page < NUMBER_OF_PAGES; page++)
        {
            pages[page].cycle_counters->reset();
        }
    }

    void stepSequencers()
    {
        for (unsigned int i = 0; i < NUMBER_OF_PAGES; i++)
        {
            pages[i].voltage_sequencer.step();
            pages[i].chance_sequencer.setPosition(pages[i].voltage_sequencer.getPosition());
        }
    }

    void reset()
    {
        for (unsigned int page = 0; page < NUMBER_OF_PAGES; page++)
        {
            pages[page].voltage_sequencer.reset();
            pages[page].chance_sequencer.reset();
        }

        arp_sequencer.reset();

        // Set up a (reverse) counter so that the clock input will ignore
        // incoming clock pulses for 1 millisecond after a reset input. This
        // is to comply with VCV Rack's standards.  See section "Timing" at
        // https://vcvrack.com/manual/VoltageStandards

        // Don't skip the first step after a reset
        clock_ignore_on_reset = (long)(rack::settings::sampleRate / 100);
        first_step = true;

        channel_step_counter = 0;
        gate_lock = false;
        note_repeat = 2;

        resetCycleCounters();
    }

    bool prepareArpSequencer()
    {
        std::vector<int> index_sequence;

        bool on_switch = (params[ON_SWITCH].getValue() > 0.5f);

        // The ON switch is a special case.  If it's on, then all notes are played
        // and the gate input is ignored.
        if (on_switch == true)
        {
            for (unsigned int channel = 0; channel < number_of_note_channels; channel++)
            {
                index_sequence.push_back(channel);
            }
        }
        // Otherwise, if the gate input is connected, then use the gate input to determine which notes to play
        else
        {
            if (inputs[POLY_GATE_INPUT].isPolyphonic() && (inputs[POLY_GATE_INPUT].isConnected()))
            {
                for (unsigned int channel = 0; channel < number_of_gate_channels; channel++)
                {
                    // If the gate input is high, then add the channel to the index sequence
                    if (gate_voltages[channel] > 0.0f)
                    {
                        index_sequence.push_back(channel);
                    }
                }
            }
            // Otherwise, if the gate input is not connected, use the number of note channels to
            // determine how many notes to play, and push their indexes on to the array.
            else if (gate_voltages[0] > 0.0f)
            {
                // For each number of note channels, push the channel index onto the index_sequence vector
                for (unsigned int channel = 0; channel < number_of_note_channels; channel++)
                {
                    index_sequence.push_back(channel);
                }
            }
        }

        // If there are no keys being held down, then don't play the arpeggio

        if (index_sequence.size() == 0)
        {
            arp_sequencer.reset();
            resetCycleCounters();
            return false;
        }

        arp_sequencer.updateIndicies(index_sequence);

        return true;
    }

    void processNoteAndGateInputs(bool latch)
    {

        //
        // Read the gate input values and store them in the gate_voltages vector
        //

        number_of_gate_channels = inputs[POLY_GATE_INPUT].getChannels();
        bool any_gate_high = false;

        if (latch == false)
        {
            for (unsigned int i = 0; i < number_of_gate_channels; i++)
            {
                gate_voltages[i] = inputs[POLY_GATE_INPUT].getVoltage(i);
            }
        }
        // Latch is TRUE
        else
        {
            for (unsigned int i = 0; i < 16; i++)
            {
                if (inputs[POLY_GATE_INPUT].getVoltage(i) > 0.0f)
                {
                    any_gate_high = true;
                    break;
                }
            }

            // gates went from some to none
            if (any_gate_high == false && previous_any_gate == true)
            {
                is_latch_accumulating = false;
            }

            if (any_gate_high)
            {
                is_latch_accumulating = true;
            }
        }

        number_of_note_channels = inputs[POLY_NOTES_INPUT].getChannels();

        arp_sequencer.setNumberOfNotes(number_of_note_channels);
        
        if (latch == false || is_latch_accumulating == true)
        {
            for (unsigned int i = 0; i < number_of_note_channels; i++)
            {
                arp_sequencer.setNoteAtIndex(i, inputs[POLY_NOTES_INPUT].getVoltage(i));
            }
        }

        // Deal with gates when latch is high
        if (latch == true)
        {
            if (previous_any_gate == false && any_gate_high == true)
            {
                gate_voltages.assign(16, 0.0f);
                prevent_duplicate_notes.clear();
            }

            if (is_latch_accumulating)
            {
                for (unsigned int i = 0; i < number_of_gate_channels; i++)
                {
                    float gate_voltage = inputs[POLY_GATE_INPUT].getVoltage(i);
                    float note_voltage = inputs[POLY_NOTES_INPUT].getVoltage(i);

                    int note_voltage_as_int = static_cast<int>(note_voltage * 10000);

                    // Only add the note if it's not already in the map
                    if (prevent_duplicate_notes.find(note_voltage_as_int) == prevent_duplicate_notes.end())
                    {
                        // Only add gates, don't remove them
                        if (gate_voltages[i] < gate_voltage)
                        {
                            gate_voltages[i] = gate_voltage;
                            prevent_duplicate_notes[note_voltage_as_int] = true;
                        }
                    }
                }
            }
        }

        previous_any_gate = any_gate_high;
    }

    void setQuantizationScale(int scale_index)
    {
        output_quantization_scale_index = scale_index;
        output_quantizer.setScale(scale_index);
    }

    void setQuantizationRoot(int root_index)
    {
        output_quantization_root_note_index = root_index;
        output_quantizer.setRoot(root_index);
    }

    void setQuantization(bool quantize)
    {
        output_quantization = quantize;
    }

    void setRateAttenuationRange(int rate_attenuator_range_index)
    {
        if(rate_attenuator_range_index == 0) rate_attenuverter_range = 5.0;
        if(rate_attenuator_range_index == 1) rate_attenuverter_range = 10.0;
    }

    void setShapeAttenuationRange(int shape_attenuator_range_index)
    {
        if(shape_attenuator_range_index == 0) shape_attenuverter_range = 5.0;
        if(shape_attenuator_range_index == 1) shape_attenuverter_range = 10.0;
    }

    void resetPageCycleCounters()
    {
        for (int step_position = pages[page].voltage_sequencer.getWindowStart(); step_position <= pages[page].voltage_sequencer.getWindowEnd(); step_position++)
        {
            unsigned int index = getCycleParameterIndex(page, step_position);
            params[index].setValue(1.0);
        }
    }

    void smartRandomizePageCycleCounters()
    {
        const double SCALE_FACTOR = 2.5;

        // This function will randomize the cycle counters for the current page
        // Lower numbers are more likely, but higher numbers still have a chance

        for (int step_position = pages[page].voltage_sequencer.getWindowStart(); step_position <= pages[page].voltage_sequencer.getWindowEnd(); step_position++)
        {
            // Generate a random number between 0 and 1
            float random_number = random::uniform();

            // Apply an exponential decay function to skew the distribution towards lower values
            // Adjust the exponent to control how quickly the probability decreases
            int value = static_cast<int>(-log(random_number) * SCALE_FACTOR);

            // Ensure the value is within the expected range
            value = clamp(value, 1, 16);

            params[getCycleParameterIndex(page, step_position)].setValue(static_cast<float>(value));
        }
    }


    std::vector<std::string> getTriggerLengthNames() 
    {
        std::vector<std::string> names;

        // iterate over trigger_lengths
        for (unsigned int i = 0; i < trigger_lengths.size(); i++)
        {
            // convert the trigger length to a string
            std::string label = std::to_string(trigger_lengths[i]);
            label.erase(label.find_last_not_of('0') + 1, std::string::npos);
            label.erase(label.find_last_not_of('.') + 1, std::string::npos);
            label += "s";

            // add the string to the vector
            names.push_back(label);
        }

        return names;           
    }

    std::string getTooltipTextForTranspose(int column, float unused_value)
    {
        // Fetch the value from the sequencer, which will range from 0.0 to 1.0
        float value = pages[TRANSPOSE_SEQUENCER].voltage_sequencer.getValue(column);

        // Check if quantize_transpose_cv is true or false
        if (quantize_transpose_cv)
        {
            // Convert the value to a range of -12 to +12 semitones (1V corresponds to 12 semitones)
            int semitones = static_cast<int>((value * 24.0f) - 12.0f);

            // If there is no change in pitch, return "None"
            if (semitones == 0)
            {
                return "None";
            }

            // Define the interval names
            std::vector<std::string> intervalNames = {
                "Unison", "Minor 2nd", "Major 2nd", "Minor 3rd", "Major 3rd",
                "Perfect 4th", "Tritone", "Perfect 5th", "Minor 6th", "Major 6th",
                "Minor 7th", "Major 7th", "Octave"};

            // Determine the direction (up or down) and the absolute number of semitones
            std::string direction = semitones >= 0 ? "Up" : "Down";
            semitones = std::abs(semitones);

            // Get the interval name, taking care to handle octave wraps
            std::string intervalName = intervalNames[semitones % 12];

            // Construct the result string
            std::stringstream result;
            result << intervalName << " " << direction;

            // If there are octave wraps, indicate the number of octaves
            int octaves = semitones / 12;
            if (octaves > 0)
            {
                result << " (" << octaves << (octaves > 1 ? " Octaves" : " Octave") << ")";
            }

            return result.str();
        }
        // In the current implementation, this code is never reached
        else
        {
            // If quantize_transpose_cv is false, display the voltage value (-1 to 1)
            float voltage = value * 2.0f - 1.0f;
            std::stringstream result;
            result << std::fixed << std::setprecision(4) << voltage << " V";
            return result.str();
        }
    }

    std::string getTooltipTextForGate(int column, float unused_value)
    {
        float value = pages[GATE_SEQUENCER].voltage_sequencer.getValue(column);
        int percentage = (value * 100);

        return "Gate Length: " + std::to_string(percentage) + "%";
    }

    std::string getTooltipTextForMod1(int column, float unused_value)
    {
        float value = applyRange(pages[MOD1_SEQUENCER].voltage_sequencer.getValue(column), mod1_attenuation_low, mod1_attenuation_high, mod1_polarity);
        return std::to_string(value);
    }

    std::string getTooltipTextForMod2(int column, float unused_value)
    {
        float value = applyRange(pages[MOD2_SEQUENCER].voltage_sequencer.getValue(column), mod2_attenuation_low, mod2_attenuation_high, mod2_polarity);
        return std::to_string(value);
    }

    std::string getTooltipTextForPercentage(int column, float value)
    {
        return "Chance to play: " + std::to_string((int)(value * 100)) + "%";
    }

    std::string getTooltipForMod1Slew()
    {
        float slewRate = mod1_slew * 4.0; // Replace 4.0 with the maximum V/s rate which we've calculated based on your code
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << slewRate;
        std::string slewRateStr = stream.str();

        return slewRateStr + " V/s";
    }

    std::string getTooltipForMod2Slew()
    {
        float slewRate = mod2_slew * 4.0; // Similarly for mod2_slew
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << slewRate;
        std::string slewRateStr = stream.str();

        return slewRateStr + " V/s";
    }

    VoltageSequencer::Polarity toPolarity(bool polarity_bool)
    {
        return (polarity_bool ? VoltageSequencer::BIPOLAR : VoltageSequencer::UNIPOLAR);
    }

    float quantize(float note_cv)
    {
        // Set the scale
        output_quantizer.setScale(output_quantization_scale_index);

        // Apply quantization
        note_cv = output_quantizer.quantize(note_cv);

        return note_cv;
    }

    unsigned int getCycleParameterIndex(unsigned int page, unsigned int step_position)
    {
        return CYCLE_KNOBS + ((page * MAX_SEQUENCER_STEPS) + step_position);
    }

    bool isClockTriggered(double time_now)
    {
        // Get the clock and rate control voltage values
        float clock_cv = inputs[CLOCK_INPUT].getVoltage();
        bool step_triggered = step_trigger.process(clock_cv, constants::gate_low_trigger, constants::gate_high_trigger);
        int rate_index = rate_model.getRateIndex();

        if (step_triggered)
        {
            return (clock_modifier.clock(time_now, rate_index));
        }

        // Handle clock multiplication
        if (rate_index > 0)
        {
            return (clock_modifier.process(time_now));
        }

        return false;
    }

    void onReset(const ResetEvent &e) override 
    {
        mod1_attenuation_high = 1.0f;
        mod1_attenuation_low = 0.0f;
        mod2_attenuation_high = 1.0f;
        mod2_attenuation_low = 0.0f;
        mod1_polarity = false;
        mod2_polarity = false;
        quantize_transpose_cv = true;
        sample_and_hold_mode = true;
        legacy_reset_mode = false;
        probability_output_sequencer_attachment = GATE_SEQUENCER;
        cycle_output_sequencer_attachment = GATE_SEQUENCER;
        output_quantization_scale_index = Quantizer::CHROMATIC_SCALE;
        output_quantization_root_note_index = Quantizer::C;
        output_quantization = false;
        step_mode = STEP_EVERY_CLOCK;
        probability_trigger_length_index = 0;
        cycle_trigger_length_index = 0;
        mod1_slew = 0.0f;
        mod2_slew = 0.0f;


        for (unsigned int page = 0; page < NUMBER_OF_PAGES; page++)
        {
            pages[page].voltage_sequencer.initialize();
            pages[page].chance_sequencer.initialize();
        }

        // Call the base class method (important!)
        rack::Module::onReset(e);
    }

};
