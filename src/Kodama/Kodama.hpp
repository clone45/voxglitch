//
// Kodama - Performance sampler with sliders
#include <fstream>

struct Kodama : VoxglitchModule
{
    // bool dirty = false;
    // std::string text;
    dsp::BooleanTrigger step_trigger;
    dsp::PulseGenerator pulse_generator;

    std::vector<std::vector<bool>> sequences;
    unsigned int step = 0;

    // bool grid_data[COLS][ROWS];
    // GateSequencer gate_sequencers[ROWS];
    // std::array<GateSequencer, ROWS> gate_sequencers;

    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        STEP_INPUT,
        RESET_INPUT,
        NEXT_INPUT,
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
        if(! sequences.empty())
        {
            if (step_trigger.process(inputs[STEP_INPUT].getVoltage()))
            {
                step = ((step + 1) % 32);
                if(sequences[0][step]) pulse_generator.trigger(0.01f);
            }

            bool pulse = pulse_generator.process(1.0 / args.sampleRate);
            outputs[GATE_OUTPUT].setVoltage((pulse ? 10.0f : 0.0f));
        }
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

        // test by outputting the first sequence
        /*
        for(unsigned int i = 0; i < 32; i++)
        {
            DEBUG(std::to_string(sequences[0][i]).c_str());
        }
        */
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
