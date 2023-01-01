//
// TODO:
// - support Cardinal

#include <fstream>

struct OnePoint : VoxglitchModule
{
    dsp::BooleanTrigger step_trigger;
    dsp::BooleanTrigger reset_trigger;
    dsp::BooleanTrigger next_sequence_trigger;
    dsp::BooleanTrigger prev_sequence_trigger;
    dsp::BooleanTrigger zero_sequence_trigger;
    dsp::BooleanTrigger next_sequence_button_trigger;
    dsp::BooleanTrigger prev_sequence_button_trigger;
    dsp::BooleanTrigger zero_sequence_button_trigger;
    dsp::PulseGenerator eol_pulse_generator;

    std::vector<std::vector<float>> sequences;
    unsigned int step = 0;
    unsigned int selected_sequence = 0;
    unsigned int real_selected_sequence = 0;
    std::string path = "";

    enum ParamIds
    {
        PREV_BUTTON_PARAM,
        NEXT_BUTTON_PARAM,
        ZERO_BUTTON_PARAM,
        CV_SEQUENCE_ATTN_KNOB,
        NUM_PARAMS
    };
    enum InputIds
    {
        STEP_INPUT,
        RESET_INPUT,
        NEXT_SEQUENCE_INPUT,
        PREV_SEQUENCE_INPUT,
        ZERO_SEQUENCE_INPUT,
        CV_SEQUENCE_SELECT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        CV_OUTPUT,
        EOL_OUTPUT, // end of sequence
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    OnePoint()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(CV_SEQUENCE_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "Attenuator");
    }

    //
    // Autosave module data.  VCV Rack decides when this should be called.
    //
    // My saving and loading code could be more compact by saving arrays of
    // "ball" data tidily packaged up.  I'll do that if this code ever goes
    // through another iteration.
    //

    json_t *dataToJson() override
    {
        json_t *json_root = json_object();
        json_object_set_new(json_root, "path", json_string(path.c_str()));
        return json_root;
    }

    // Load module data
    void dataFromJson(json_t *json_root) override
    {
        json_t *loaded_path_json = json_object_get(json_root, ("path"));
        if (loaded_path_json)
        {
            this->path = json_string_value(loaded_path_json);
            this->loadData(this->path);
        }
    }

    void reset()
    {
        step = 0;
    }

    int wrap(int kX, int const kLowerBound, int const kUpperBound)
    {
        int range_size = kUpperBound - kLowerBound + 1;

        if (kX < kLowerBound)
            kX += range_size * ((kLowerBound - kX) / range_size + 1);

        return kLowerBound + (kX - kLowerBound) % range_size;
    }

    void process(const ProcessArgs &args) override
    {
        if (sequences.empty())
            return;

        // Process NEXT trigger and button
        if (next_sequence_trigger.process(inputs[NEXT_SEQUENCE_INPUT].getVoltage()) || next_sequence_button_trigger.process(params[NEXT_BUTTON_PARAM].getValue()))
        {
            selected_sequence = ((selected_sequence + 1) % sequences.size());
        }

        // Process PREV trigger and button
        if (prev_sequence_trigger.process(inputs[PREV_SEQUENCE_INPUT].getVoltage()) || prev_sequence_button_trigger.process(params[PREV_BUTTON_PARAM].getValue()))
        {
            selected_sequence = (selected_sequence == 0) ? sequences.size() - 1 : selected_sequence - 1;
        }

        // Process ZERO trigger and button
        if (zero_sequence_trigger.process(inputs[ZERO_SEQUENCE_INPUT].getVoltage()) || zero_sequence_button_trigger.process(params[ZERO_BUTTON_PARAM].getValue()))
        {
            selected_sequence = 0;
        }

        // Process RESET input
        if (reset_trigger.process(inputs[RESET_INPUT].getVoltage()))
        {
            reset();
        }

        // Adjust selected sequence based on CV input (if connected)
        if (inputs[CV_SEQUENCE_SELECT].isConnected())
        {
            unsigned int sequence_count = sequences.size() - 1; // 20
            float sequence_select_cv = inputs[CV_SEQUENCE_SELECT].getVoltage() * params[CV_SEQUENCE_ATTN_KNOB].getValue();
            int cv_sequence_value = (int)rescale(sequence_select_cv, -5.0, 5.0, -20, 20);

            real_selected_sequence = wrap(selected_sequence + cv_sequence_value, 0, sequence_count);
        }
        else
        {
            real_selected_sequence = selected_sequence;
        }

        // Process STEP input
        if (step_trigger.process(inputs[STEP_INPUT].getVoltage()))
        {
            step++;

            // If we're at the end of the sequencer, wrap to the beginning
            if (step == sequences[real_selected_sequence].size())
            {
                step = 0;
                eol_pulse_generator.trigger(0.01f);
            }
        }

        //
        // Outputs
        //

        // bool output_pulse = output_pulse_generator.process(1.0 / args.sampleRate);
        outputs[CV_OUTPUT].setVoltage(sequences[real_selected_sequence][step]);

        bool eol_pulse = eol_pulse_generator.process(1.0 / args.sampleRate);
        outputs[EOL_OUTPUT].setVoltage((eol_pulse ? 10.0f : 0.0f));
    }

    void loadData(std::string path)
    {
        std::ifstream input_file(path);

        selected_sequence = 0;

        // Clear out existing data
        sequences.clear();

        // test file open
        if (input_file)
        {
            std::string line = "";

            while (std::getline(input_file, line))
            {
                std::vector<float> sequence;

                std::stringstream ss(line);

                while( ss.good() )
                {
                    std::string substr;
                    getline(ss, substr,',');
                    sequence.push_back( std::stod(substr) );
                }

                sequences.push_back(sequence);
            }

            input_file.close();
        }

        reset();
    }

    std::string selectFileVCV()
    {
        std::string filename_string = "";
        osdialog_filters *filters = osdialog_filters_parse("TXT:txt");
        char *filename = osdialog_file(OSDIALOG_OPEN, "", NULL, filters);

        if (filename != NULL)
        {
            filename_string.assign(filename);
            osdialog_filters_free(filters);
            std::free(filename);
        }

        return (filename_string);
    }
};
