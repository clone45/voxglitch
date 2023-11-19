//
// TODO:
//

// * Subclass param stuff to get custom tooltips for octave selection knob
// * Write documentation
// * update light theme
// * get clock input working

struct CustomOctaveParamQuantity : ParamQuantity {
    std::string getDisplayValueString() override {
        if (getValue() == -1.0f) {
            return "All";
        }
        return ParamQuantity::getDisplayValueString();
    }
};

struct NoteDetector : VoxglitchModule
{
    std::string version = "2.0.0";

    std::string note_readout = "";
    dsp::PulseGenerator output_pulse_generator;
    unsigned int output_mode = TRIGGER;
    int trigger_length_index = 3;
    int tolerance_level_index = 0;
    float previous_target_voltage = -1234567.89f;
    bool was_outside_tolerance = false;

    dsp::SchmittTrigger clock_trigger;

    std::vector<float> trigger_lengths = {0.001, 0.002, 0.005, 0.010, 0.020, 0.050, 0.100, 0.200};
    std::vector<float> tolerance_presets = {0.0f, 0.01f, 0.025f, 0.05f, 0.075f, 0.1f, 0.15f, 0.2f, 0.35f, 0.5f};
    std::vector<std::string> tolerance_preset_labels = {"Exact (0)", "Ultra Fine (0.01)", "Extra Fine (0.025)", "Fine (0.05)", "Semi Fine (0.075)", "Medium (0.1)", "Semi Coarse (0.15)", "Coarse (0.2)", "Extra Coarse (0.35)", "Very Coarse (0.5)"};

    enum ParamIds
    {
        NOTE_SELECTION_KNOB,
        OCTAVE_SELECTION_KNOB,
        NUM_PARAMS
    };
    enum InputIds
    {
        CV_INPUT,
        CLOCK_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        DETECTION_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    enum OutputModes
    {
        TRIGGER,
        GATE,
        NUM_OUTPUT_MODES
    };

    // █▀ ▄▀█ █░█ █▀▀   ▄▀█ █▄░█ █▀▄   █░░ █▀█ ▄▀█ █▀▄
    // ▄█ █▀█ ▀▄▀ ██▄   █▀█ █░▀█ █▄▀   █▄▄ █▄█ █▀█ █▄▀

    // Save module data
    json_t *dataToJson() override
    {
        json_t *json_root = json_object();

        // Save the version
        json_object_set_new(json_root, "version", json_string(version.c_str()));

        json_object_set_new(json_root, "output_mode", json_integer(output_mode));
        json_object_set_new(json_root, "tolerance", json_integer(tolerance_level_index));
        json_object_set_new(json_root, "trigger_length_index", json_integer(trigger_length_index));

        return json_root;
    }

    // Load module data
    void dataFromJson(json_t *root) override
    {
        this->setOutputMode(JSON::getInteger(root, "output_mode"));
        this->setToleranceIndex(JSON::getInteger(root, "tolerance"));
        this->setTriggerLengthIndex(JSON::getInteger(root, "trigger_length_index"));
    }        

    NoteDetector()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // Configure the shape knob to snap from 0 to NUMBER_OF_PLAYBACK_MODES
        configParam(NOTE_SELECTION_KNOB, 0.0f, 11.0, 10.0f, "Note");
        paramQuantities[NOTE_SELECTION_KNOB]->snapEnabled = true;

        configParam(OCTAVE_SELECTION_KNOB, -1.0f, 8.0, 4.0f, "Octave");
        paramQuantities[OCTAVE_SELECTION_KNOB]->snapEnabled = true;
    }

    // █▀█ █▀█ █▀█ █▀▀ █▀▀ █▀ █▀
    // █▀▀ █▀▄ █▄█ █▄▄ ██▄ ▄█ ▄█

    void process(const ProcessArgs &args) override
    {
        // Read the NOTE_SELECTION_KNOB and OCTAVE_SELECTION_KNOB parameters
        int note_selection = (int)roundf(params[NOTE_SELECTION_KNOB].getValue());
        int octave_selection = (int)roundf(params[OCTAVE_SELECTION_KNOB].getValue());
        float target_voltage = noteToVoltage(note_selection, octave_selection);

        /*
        if (clock_trigger.process(inputs[CLOCK_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
        {
            // do something
        }
        */

        // Update the note readout
        note_readout = getNoteName(note_selection, octave_selection);

        // Update the param display string
        // This doesn't seem to work
        // if (octave_selection == -1) paramQuantities[OCTAVE_SELECTION_KNOB]->setDisplayValueString("All");

        // Read the CV_INPUT
        float cv_input = inputs[CV_INPUT].getVoltage();
        bool is_within_tolerance = isWithinTolerance(cv_input, note_selection, octave_selection);

        if(! inputs[CV_INPUT].isConnected())
        {
            // Set output to 0V if no CV input is connected
            outputs[DETECTION_OUTPUT].setVoltage(0.0f);

            // Reset the pulse generator
            output_pulse_generator.reset();

            return;
        }

        // Clocked operation
        if(inputs[CLOCK_INPUT].isConnected())
        {
            bool clocked = clock_trigger.process(inputs[CLOCK_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger);

            switch (output_mode)
            {
                case TRIGGER:
                    processTriggerModeClocked(is_within_tolerance, clocked, args);
                    break;

                case GATE:
                    processGateModeClocked(is_within_tolerance, clocked, args);
                    break;
            }
        }
        // Freeform without a clock attached
        else
        {
            switch (output_mode)
            {
                case TRIGGER:
                    processTriggerModeFreeform(target_voltage, cv_input, is_within_tolerance, args);
                    break;

                case GATE:
                    processGateModeFreeform(target_voltage, cv_input, is_within_tolerance, args);
                    break;
            }
        }
    }

    void processTriggerModeClocked(bool is_within_tolerance, bool clocked, const ProcessArgs &args)
    {
        // Trigger if within tolerance and 
        if (is_within_tolerance && clocked)
        {
            output_pulse_generator.trigger(trigger_lengths[trigger_length_index]);
        }

        // Check if the pulse generator is currently generating a pulse
        if (output_pulse_generator.process(args.sampleTime)) 
        {
            outputs[DETECTION_OUTPUT].setVoltage(10.0f); // High voltage for trigger
        } 
        else 
        {
            outputs[DETECTION_OUTPUT].setVoltage(0.0f); // No voltage
        }
    }

    void processGateModeClocked(bool is_within_tolerance, bool clocked, const ProcessArgs &args)
    {
        if (is_within_tolerance && clocked)
        {
            outputs[DETECTION_OUTPUT].setVoltage(10.0f); // High voltage for gate
        }
        else
        {
            outputs[DETECTION_OUTPUT].setVoltage(0.0f); // No voltage
        }
    }

    void processTriggerModeFreeform(float target_voltage, float cv_input, bool is_within_tolerance, const ProcessArgs &args)
    {
        bool has_target_voltage_changed = target_voltage != previous_target_voltage;

        // Trigger if within tolerance and either the voltage was previously outside tolerance 
        // or the target voltage has changed
        if (is_within_tolerance && (was_outside_tolerance || has_target_voltage_changed))
        {
            output_pulse_generator.trigger(trigger_lengths[trigger_length_index]);
            was_outside_tolerance = false;
        }
        else if (!is_within_tolerance)
        {
            was_outside_tolerance = true;
        }

        // Update the previous target voltage for the next cycle
        previous_target_voltage = target_voltage;

        // Check if the pulse generator is currently generating a pulse
        if (output_pulse_generator.process(args.sampleTime)) 
        {
            outputs[DETECTION_OUTPUT].setVoltage(10.0f); // High voltage for trigger
        } 
        else 
        {
            outputs[DETECTION_OUTPUT].setVoltage(0.0f); // No voltage
        }
    }

    void processGateModeFreeform(float target_voltage, float cv_input,  bool is_within_tolerance, const ProcessArgs &args)
    {
        if (is_within_tolerance)
        {
            outputs[DETECTION_OUTPUT].setVoltage(10.0f); // High voltage for gate
        }
        else
        {
            outputs[DETECTION_OUTPUT].setVoltage(0.0f); // No voltage
        }
    }

    bool isWithinTolerance(float cv_input, int note_selection, int octave_selection)
    {
        if (octave_selection == -1) // All Octaves
        {
            for (int octave = 0; octave <= 8; ++octave) // Adjust the range as per your module's specification
            {
                float octave_target_voltage = noteToVoltage(note_selection, octave);
                if (std::abs(cv_input - octave_target_voltage) <= tolerance_presets[tolerance_level_index])
                {
                    return true; // Input voltage is within tolerance for this octave
                }
            }
            return false; // No octave matched
        }
        else
        {
            // Standard single octave check
            float target_voltage = noteToVoltage(note_selection, octave_selection);
            return std::abs(cv_input - target_voltage) <= tolerance_presets[tolerance_level_index];
        }
    }


    std::string getNoteName(int note_selection, int octave_selection)
    {
        std::string note_name = "A4";

        switch (note_selection)
        {
            case 0:
                note_name = "C";
                break;
            case 1:
                note_name = "C#";
                break;
            case 2:
                note_name = "D";
                break;
            case 3:
                note_name = "D#";
                break;
            case 4:
                note_name = "E";
                break;
            case 5:
                note_name = "F";
                break;
            case 6:
                note_name = "F#";
                break;
            case 7:
                note_name = "G";
                break;
            case 8:
                note_name = "G#";
                break;
            case 9:
                note_name = "A";
                break;
            case 10:
                note_name = "A#";
                break;
            case 11:
                note_name = "B";
                break;
            default:
                note_name = "A4";
                break;
        }

        if(octave_selection >= 0) 
        {
            note_name += std::to_string(octave_selection);
        }

        return note_name;
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

    float noteToVoltage(int note, int octave) 
    {
        // C4 = 261.6256 Hz is considered 0V in 1V/Oct standard
        const int C4Note = 0; // C in the chromatic scale (0 = C, 1 = C#, ..., 10 = A#, 11 = B)
        const int C4Octave = 4;

        // Calculate the total number of semitones from C4
        int semitonesFromC4 = (octave - C4Octave) * 12 + (note - C4Note);

        // Convert semitones to voltage (1 semitone = 1/12 V)
        float voltage = semitonesFromC4 / 12.0f;
        return voltage;
    }

    void setOutputMode(int output_mode_index)
    {
        output_mode = output_mode_index;
    }

    void setToleranceIndex(int tolerance_index)
    {
        this->tolerance_level_index = tolerance_index;
    }

    void setTriggerLengthIndex(int trigger_length_index)
    {
        this->trigger_length_index = trigger_length_index;
    }
};
