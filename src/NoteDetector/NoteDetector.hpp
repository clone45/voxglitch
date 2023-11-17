struct NoteDetector : VoxglitchModule
{
    std::string note_readout = " A4";
    dsp::PulseGenerator output_pulse_generator;

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

    // Save module data
    json_t *dataToJson() override
    {
        json_t *json_root = json_object();
        return json_root;
    }

    // Load module data
    void dataFromJson(json_t *root) override
    {

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

    void process(const ProcessArgs &args) override
    {
        // Read the NOTE_SELECTION_KNOB and OCTAVE_SELECTION_KNOB parameters
        int note_selection = (int)roundf(params[NOTE_SELECTION_KNOB].getValue());
        int octave_selection = (int)roundf(params[OCTAVE_SELECTION_KNOB].getValue());

        // Read the CV_INPUT
        float cv_input = inputs[CV_INPUT].getVoltage();

        // Update the note readout
        note_readout = getNoteName(note_selection, octave_selection);

        float targetVoltage = noteToVoltage(note_selection, octave_selection);

        // Compare the target voltage with the input voltage
        float tolerance = 0.00f; // Adjust this value as needed
        if (std::abs(cv_input - targetVoltage) <= tolerance) 
        {
            // Trigger the pulse generator
            output_pulse_generator.trigger(1e-3); // Trigger duration of 1 millisecond
        }

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
};
