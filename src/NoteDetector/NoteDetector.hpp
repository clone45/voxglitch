//
// TODO:
//

// * Write documentation
// * update light theme

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
    bool use_flat_notation = false;

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

    /**
     * @brief Serializes the module's current state into JSON format.
     *
     * This function is responsible for saving the current state of the NoteDetector module.
     * It converts the module's settings into a JSON object that can be used for saving
     * the state of the module in a patch. The JSON object includes the version of the module,
     * the current output mode, tolerance level index, trigger length index, and notation 
     * preference (flat or sharp).
     *
     * @return json_t* A JSON object representing the current state of the module.
     *
     * The JSON object contains the following key-value pairs:
     * - "version": A string representing the version of the module.
     * - "output_mode": An integer indicating the current output mode of the module (e.g., Trigger or Gate).
     * - "tolerance": An integer representing the index of the currently selected tolerance level.
     * - "trigger_length_index": An integer indicating the index of the selected trigger length.
     * - "use_flat_notation": An integer (treated as a boolean) indicating whether flat notation is used (1) or not (0).
     *
     * Example JSON object:
     * {
     *   "version": "2.0.0",
     *   "output_mode": 0,
     *   "tolerance": 3,
     *   "trigger_length_index": 2,
     *   "use_flat_notation": 1
     * }
     */

    json_t *dataToJson() override
    {
        json_t *json_root = json_object();

        // Save the version
        json_object_set_new(json_root, "version", json_string(version.c_str()));

        json_object_set_new(json_root, "output_mode", json_integer(output_mode));
        json_object_set_new(json_root, "tolerance", json_integer(tolerance_level_index));
        json_object_set_new(json_root, "trigger_length_index", json_integer(trigger_length_index));
        json_object_set_new(json_root, "use_flat_notation", json_integer(use_flat_notation));

        return json_root;
    }

    /**
     * @brief Deserializes the module's state from a JSON object.
     *
     * This function is responsible for restoring the state of the NoteDetector module
     * from a given JSON object. It reads the module's settings from the JSON and applies
     * them to the module. This is typically used for loading the state of the module when
     * opening a saved patch. The function expects the JSON object to contain the module's
     * version, output mode, tolerance level index, trigger length index, and notation preference
     * (flat or sharp). Each of these settings is read from the JSON and applied to the module.
     *
     * @param root The JSON object containing the serialized state of the module.
     *
     * The JSON object should contain the following key-value pairs:
     * - "version": A string representing the version of the module.
     * - "output_mode": An integer indicating the output mode of the module (e.g., Trigger or Gate).
     * - "tolerance": An integer representing the index of the tolerance level.
     * - "trigger_length_index": An integer indicating the index of the trigger length.
     * - "use_flat_notation": An integer (treated as a boolean) indicating the use of flat notation (1) or not (0).
     *
     * Example JSON object:
     * {
     *   "version": "2.0.0",
     *   "output_mode": 0,
     *   "tolerance": 3,
     *   "trigger_length_index": 2,
     *   "use_flat_notation": 1
     * }
     *
     * @note If the JSON object does not contain any of these keys, the corresponding setting
     *       in the module should remain unchanged.
     */
    void dataFromJson(json_t *root) override
    {
        this->setOutputMode(JSON::getInteger(root, "output_mode"));
        this->setToleranceIndex(JSON::getInteger(root, "tolerance"));
        this->setTriggerLengthIndex(JSON::getInteger(root, "trigger_length_index"));
        this->setUseFlatNotation(JSON::getInteger(root, "use_flat_notation"));
    }        

    /**
     * @brief Constructor for the NoteDetector module.
     *
     * This function initializes the NoteDetector module, setting up its configuration and
     * initializing its parameters and quantizers. The constructor configures the module with
     * the defined number of parameters, inputs, outputs, and lights. It also sets up the note
     * and octave selection parameters, including their range and default values. The constructor
     * is responsible for setting the initial state of the module, ensuring that it is ready
     * for use immediately after instantiation.
     *
     * Configuration details:
     * - Note selection (NOTE_SELECTION_KNOB): Configured with a custom NoteParamQuantity.
     *   It allows selection of notes in the chromatic scale, with snapping enabled for precise selection.
     * - Octave selection (OCTAVE_SELECTION_KNOB): Configured with a custom OctaveParamQuantity.
     *   It provides a range of octaves for selection, with snapping enabled for precise selection.
     *
     * @param module A pointer to the NoteDetector module instance being constructed.
     *               If null, the widget is constructed without any linked module.
     *
     * Note:
     * The constructor does not set up any audio or CV processing. It only configures the
     * parameters and UI elements of the module. The processing logic is handled separately
     * in the module's `process` method.
     */

    NoteDetector()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // NoteParamQuantity is defined in src\Common\customParamQuantities.hpp
        NoteParamQuantity *note_param_quantity = new NoteParamQuantity();
        note_param_quantity->module = this;
        note_param_quantity->paramId = NOTE_SELECTION_KNOB;
        note_param_quantity->minValue = 0.0f;
        note_param_quantity->maxValue = 11.0f;
        note_param_quantity->defaultValue = 0.0f;
        note_param_quantity->displayMultiplier = 1.0f;
        note_param_quantity->snapEnabled = true;
        note_param_quantity->name = "Note";
        paramQuantities[NOTE_SELECTION_KNOB] = note_param_quantity;

        // OctaveParamQuantity is defined in src\Common\customParamQuantities.hpp
        OctaveParamQuantity *octave_param_quantity = new OctaveParamQuantity();
        octave_param_quantity->module = this;
        octave_param_quantity->paramId = OCTAVE_SELECTION_KNOB;
        octave_param_quantity->minValue = -1.0f;
        octave_param_quantity->maxValue = 8.0f;
        octave_param_quantity->defaultValue = 4.0f;
        octave_param_quantity->displayMultiplier = 1.0f;
        octave_param_quantity->snapEnabled = true;
        octave_param_quantity->name = "Octave";
        paramQuantities[OCTAVE_SELECTION_KNOB] = octave_param_quantity;
    }

    // █▀█ █▀█ █▀█ █▀▀ █▀▀ █▀ █▀
    // █▀▀ █▀▄ █▄█ █▄▄ ██▄ ▄█ ▄█
    /**
     * @brief Main processing function of the NoteDetector module.
     *
     * This function handles the core functionality of the NoteDetector module. It reads
     * the current state of the module's parameters, processes input signals, and sets the
     * module's outputs based on the logic defined within. The function performs several key
     * operations:
     *
     * 1. Reading Parameters:
     *    - Reads the NOTE_SELECTION_KNOB and OCTAVE_SELECTION_KNOB parameters to determine
     *      the selected note and octave.
     *    - Calculates the target voltage based on the note and octave selection.
     *    - Updates the note readout string based on the current selection and notation preference.
     *
     * 2. Processing CV Input:
     *    - Reads the voltage from the CV_INPUT.
     *    - Determines if the input voltage is within the defined tolerance of the target voltage.
     *
     * 3. Handling Outputs:
     *    - If no CV input is connected, sets the DETECTION_OUTPUT to 0V and resets the pulse generator.
     *    - If CLOCK_INPUT is connected (clocked operation), the function handles trigger/gate output based
     *      on the clock signal and the defined output mode (Trigger or Gate).
     *    - In freeform mode (no clock input), the function handles trigger/gate output based on the CV input
     *      and the selected output mode.
     *
     * @param args Structure containing information about the current step, such as the sample time.
     *
     * @note The function differentiates between clocked and freeform operation modes and delegates
     *       specific processing to the processTriggerModeClocked, processGateModeClocked,
     *       processTriggerModeFreeform, and processGateModeFreeform functions as appropriate.
     */

    void process(const ProcessArgs &args) override
    {
        // Read the NOTE_SELECTION_KNOB and OCTAVE_SELECTION_KNOB parameters
        int note_selection = (int)roundf(params[NOTE_SELECTION_KNOB].getValue());
        int octave_selection = (int)roundf(params[OCTAVE_SELECTION_KNOB].getValue());
        float target_voltage = noteToVoltage(note_selection, octave_selection);

        // Update the note readout
        note_readout = NOTES::getNoteName(note_selection, octave_selection, use_flat_notation);

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

    /**
     * @brief Processes the module in Trigger mode when a clock input is connected.
     *
     * This function is responsible for handling the trigger output of the NoteDetector module
     * in a clocked scenario. It generates a trigger signal based on the current state of the
     * module's parameters and the input clock signal. The function performs the following operations:
     *
     * 1. Trigger Signal Generation:
     *    - Checks if the current CV input is within the defined tolerance of the target voltage
     *      and if a clock signal is present.
     *    - If both conditions are met, a trigger pulse is generated using the output pulse generator.
     *      The length of the pulse is determined by the current setting of trigger_length_index.
     *
     * 2. Setting Output Voltage:
     *    - The output voltage at DETECTION_OUTPUT is set to 10V during the duration of the trigger pulse.
     *    - Once the pulse ends, the output voltage is set back to 0V.
     *
     * @param is_within_tolerance A boolean indicating whether the current CV input is within the
     *                            defined tolerance of the target voltage.
     * @param clocked A boolean indicating the presence of a clock signal.
     * @param args Structure containing information about the current step, such as the sample time.
     *
     * @note This function should be called only when a clock input is connected to the module. It is
     *       part of the module's processing logic that handles clocked trigger output.
     */
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
            outputs[DETECTION_OUTPUT].setVoltage(10.0f);
        } 
        else 
        {
            outputs[DETECTION_OUTPUT].setVoltage(0.0f);
        }
    }

    void processGateModeClocked(bool is_within_tolerance, bool clocked, const ProcessArgs &args)
    {
        if (is_within_tolerance && clocked)
        {
            outputs[DETECTION_OUTPUT].setVoltage(10.0f);
        }
        else
        {
            outputs[DETECTION_OUTPUT].setVoltage(0.0f);
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
            outputs[DETECTION_OUTPUT].setVoltage(10.0f);
        } 
        else 
        {
            outputs[DETECTION_OUTPUT].setVoltage(0.0f);
        }
    }

    /**
     * @brief Processes the module in Gate mode when a clock input is connected.
     *
     * This function is responsible for handling the gate output of the NoteDetector module
     * in a clocked scenario. It controls the gate signal based on the current CV input's 
     * proximity to the target voltage and the presence of a clock signal. The function 
     * operates as follows:
     *
     * 1. Gate Signal Control:
     *    - Checks if the current CV input is within the defined tolerance of the target voltage
     *      and if a clock signal is present.
     *    - If both conditions are satisfied, the function sets the output voltage at
     *      DETECTION_OUTPUT to 10V, indicating an active gate signal.
     *    - If either condition is not met, the output voltage is set to 0V, indicating an
     *      inactive gate signal.
     *
     * @param is_within_tolerance A boolean indicating whether the current CV input is within the
     *                            defined tolerance of the target voltage.
     * @param clocked A boolean indicating the presence of a clock signal.
     * @param args Structure containing information about the current step, such as the sample time.
     *
     * @note This function should be called only when a clock input is connected to the module. It forms
     *       a part of the module's processing logic specifically tailored for clocked gate output.
     */

    void processGateModeFreeform(float target_voltage, float cv_input,  bool is_within_tolerance, const ProcessArgs &args)
    {
        if (is_within_tolerance)
        {
            outputs[DETECTION_OUTPUT].setVoltage(10.0f);
        }
        else
        {
            outputs[DETECTION_OUTPUT].setVoltage(0.0f);
        }
    }

    /**
     * @brief Checks if the input CV voltage is within a specified tolerance of the target voltage.
     *
     * This function determines whether the input CV voltage matches the target voltage,
     * derived from the selected note and octave, within a certain tolerance level. It supports
     * checking against a single octave or across all octaves. The function operates as follows:
     *
     * 1. All Octaves Mode:
     *    - If octave_selection is set to -1, the function checks the input voltage against
     *      the target voltage for each octave (from 0 to 8).
     *    - If the input voltage is within the tolerance range for any octave, the function
     *      returns true.
     *    - If no octave matches, it returns false.
     *
     * 2. Single Octave Mode:
     *    - If a specific octave is selected, the function checks if the input voltage is within
     *      the tolerance range for the target voltage of that specific note and octave.
     *    - Returns true if the input voltage is within tolerance, otherwise false.
     *
     * @param cv_input The input CV voltage to be checked.
     * @param note_selection The selected note (based on which the target voltage is calculated).
     * @param octave_selection The selected octave. If set to -1, checks against all octaves.
     *
     * @return bool True if the input CV voltage is within tolerance, false otherwise.
     *
     * @note The tolerance level is determined by the current setting of tolerance_level_index.
     *       This function is used in the module's processing logic to determine whether the
     *       input voltage matches the expected voltage for the selected note and octave.
     */

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

    /**
     * @brief Retrieves a list of human-readable names for trigger lengths.
     *
     * This function generates a vector of strings, each representing the name of a trigger length
     * available in the module. It is used to provide a user-friendly way to display and select trigger
     * lengths in the module's UI, especially in context menus or similar settings. The function
     * operates as follows:
     *
     * 1. Iteration and Conversion:
     *    - Iterates over the trigger_lengths vector, which contains the trigger lengths in seconds.
     *    - Converts each trigger length (a float value) into a string.
     *    - Trims trailing zeros and the decimal point (if necessary) to make the display more concise.
     *    - Appends "s" to each string to denote seconds.
     *
     * 2. Building the Names List:
     *    - Each formatted string is added to a vector of strings.
     *    - The resulting vector contains all trigger length names, ready for display.
     *
     * @return std::vector<std::string> A vector containing the names of all trigger lengths.
     *
     * @note The trigger lengths are defined in the trigger_lengths vector and represent time durations.
     *       This function is primarily used for UI purposes, allowing users to see and select trigger
     *       lengths in a more intuitive format.
     */

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

    void setUseFlatNotation(int use_flat_notation_index)
    {
        this->use_flat_notation = use_flat_notation_index;
    }
};
