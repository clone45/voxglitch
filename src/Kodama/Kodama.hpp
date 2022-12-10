#include <fstream>

struct Kodama : VoxglitchModule
{
    dsp::BooleanTrigger step_trigger;
    dsp::BooleanTrigger reset_trigger;
    dsp::BooleanTrigger next_sequence_trigger;
    dsp::BooleanTrigger prev_sequence_trigger;
    dsp::PulseGenerator pulse_generator;

    std::vector<std::vector<bool>> sequences;
    unsigned int step = 0;
    unsigned int selected_sequence = 0;

    dsp::TTimer<double> reset_timer;
    bool first_step = true;
    bool wait_for_reset_timer = false;
    

    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        STEP_INPUT,
        RESET_INPUT,
        NEXT_SEQUENCE_INPUT,
        PREV_SEQUENCE_INPUT,
        CV_SEQUENCE_SELECT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        GATE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Kodama()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
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

        return json_root;
    }

    // Load module data
    void dataFromJson(json_t *json_root) override
    {
    }

    void process(const ProcessArgs &args) override
    {
        if(sequences.empty()) return;

        if (next_sequence_trigger.process(inputs[NEXT_SEQUENCE_INPUT].getVoltage()))
        {
            selected_sequence = ((selected_sequence + 1) % sequences.size());
        }

        if (prev_sequence_trigger.process(inputs[PREV_SEQUENCE_INPUT].getVoltage()))
        {
            selected_sequence = (selected_sequence == 0) ? sequences.size() - 1 : selected_sequence - 1;
        }

        if (reset_trigger.process(inputs[RESET_INPUT].getVoltage()))
        {
            first_step = true;
            step = 0;
            wait_for_reset_timer = true;

            // Set up a (reverse) counter so that the clock input will ignore
            // incoming clock pulses for 1 millisecond after a reset input. This
            // is to comply with VCV Rack's standards.  See section "Timing" at
            // https://vcvrack.com/manual/VoltageStandards

            reset_timer.reset();
        }

        if (!wait_for_reset_timer && step_trigger.process(inputs[STEP_INPUT].getVoltage()))
        // if (step_trigger.process(inputs[STEP_INPUT].getVoltage()))
        {
            if(first_step)
            {
                first_step = false;
            }
            else
            {
                step = ((step + 1) % sequences[selected_sequence].size());
            }
            
            if(sequences[selected_sequence][step]) pulse_generator.trigger(0.01f);
        }

        if(wait_for_reset_timer)
        {
            // Accumulate time in reset_timer
            if(reset_timer.process(args.sampleTime * 1000.0) > 1.0)
            {
                wait_for_reset_timer = false;
                reset_timer.reset();
            }
        }

        bool pulse = pulse_generator.process(1.0 / args.sampleRate);
        outputs[GATE_OUTPUT].setVoltage((pulse ? 10.0f : 0.0f));
    }

    void loadData(std::string path)
    {
        std::ifstream input_file(path);

        // test file open
        if (input_file)
        {
            std::string line = "";

            while (std::getline(input_file, line))
            {
                std::vector<bool> sequence;

                for (auto &character : line)
                {
                    // Print current character
                    sequence.push_back(character == '1');
                }

                sequences.push_back(sequence);
            }
        }
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
