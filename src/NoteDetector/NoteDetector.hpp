//
// TODO:
//
// * Add tolerance setting in context menu
// * Save/load tolerance setting
// * Add GATE vs TRIGGER output to context menu [done]
// * Implement GATE vs TRIGGER output
// * Save/load output mode [done]
// * Add trigger duration setting in context menu  [done]
// * Implement trigger duration setting
// * Save/load trigger duration setting  [done]
// * Add "all" octave option
// * Implement light theme


struct NoteDetector : VoxglitchModule
{
    std::string version = "2.0.0";

    std::string note_readout = "";
    dsp::PulseGenerator output_pulse_generator;
    unsigned int output_mode = TRIGGER;
    float tolerance = 0.00f;
    int trigger_length_index = 3;
    float previous_target_voltage = -1234567.89f;
    bool trigger_lock = false;
    bool was_outside_tolerance = false;

    std::vector<float> trigger_lengths = {0.001, 0.002, 0.005, 0.010, 0.020, 0.050, 0.100, 0.200};

    enum ParamIds
    {
        NOTE_SELECTION_KNOB,
        OCTAVE_SELECTION_KNOB,
        NUM_PARAMS
    };
    enum InputIds
    {
        CV_INPUT,
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
        json_object_set_new(json_root, "tolerance", json_real(tolerance));
        json_object_set_new(json_root, "trigger_length_index", json_integer(trigger_length_index));

        return json_root;
    }

    // Load module data
    void dataFromJson(json_t *root) override
    {
        this->setOutputMode(JSON::getInteger(root, "output_mode"));
        this->setTolerance(JSON::getNumber(root, "tolerance"));
        this->setTriggerLengthIndex(JSON::getInteger(root, "trigger_length_index"));
    }        

    NoteDetector()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // Configure the shape knob to snap from 0 to NUMBER_OF_PLAYBACK_MODES
        configParam(NOTE_SELECTION_KNOB, 0.0f, 11.0, 10.0f, "Note");
        paramQuantities[NOTE_SELECTION_KNOB]->snapEnabled = true;

        configParam(OCTAVE_SELECTION_KNOB, 0.0f, 8.0, 4.0f, "Octave");
        paramQuantities[OCTAVE_SELECTION_KNOB]->snapEnabled = true;
    }

    // █▀█ █▀█ █▀█ █▀▀ █▀▀ █▀ █▀
    // █▀▀ █▀▄ █▄█ █▄▄ ██▄ ▄█ ▄█

    void process(const ProcessArgs &args) override
    {
        // Read the NOTE_SELECTION_KNOB and OCTAVE_SELECTION_KNOB parameters
        int note_selection = (int)roundf(params[NOTE_SELECTION_KNOB].getValue());
        int octave_selection = (int)roundf(params[OCTAVE_SELECTION_KNOB].getValue());
        
        // Update the note readout
        note_readout = getNoteName(note_selection, octave_selection);

        // Read the CV_INPUT and get target voltage
        float cv_input = inputs[CV_INPUT].getVoltage();
        float target_voltage = noteToVoltage(note_selection, octave_selection);

        bool has_target_voltage_changed = target_voltage != previous_target_voltage;
        bool is_within_tolerance = std::abs(cv_input - target_voltage) <= tolerance;

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

        if(! inputs[CV_INPUT].isConnected())
        {
            // Set output to 0V if no CV input is connected
            outputs[DETECTION_OUTPUT].setVoltage(0.0f);

            // Reset the pulse generator
            output_pulse_generator.reset();

            return;
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

        note_name += std::to_string(octave_selection);

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

    void setTolerance(float tolerance)
    {
        this->tolerance = tolerance;
    }

    void setTriggerLengthIndex(int trigger_length_index)
    {
        this->trigger_length_index = trigger_length_index;
    }
};
