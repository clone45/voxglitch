//
// TODO:
// - support Cardinal

#include <fstream>

struct OneZero : VoxglitchModule
{
    dsp::SchmittTrigger step_trigger;
    dsp::SchmittTrigger reset_trigger;
    dsp::SchmittTrigger next_sequence_trigger;
    dsp::SchmittTrigger prev_sequence_trigger;
    dsp::SchmittTrigger zero_sequence_trigger;
    dsp::BooleanTrigger next_sequence_button_trigger;
    dsp::BooleanTrigger prev_sequence_button_trigger;
    dsp::BooleanTrigger zero_sequence_button_trigger;
    dsp::PulseGenerator output_pulse_generator;
    dsp::PulseGenerator eol_pulse_generator;

    // Pulses for illuminating the buttons
    dsp::PulseGenerator prevPulse;
    dsp::PulseGenerator nextPulse;
    dsp::PulseGenerator zeroPulse;

    std::vector<std::vector<bool>> sequences;
    unsigned int step = 0;
    unsigned int selected_sequence = 0;
    unsigned int real_selected_sequence = 0;
    std::string path = "";

    dsp::TTimer<double> reset_timer;
    bool first_step = true;
    bool wait_for_reset_timer = false;
    bool playback = true;

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
        GATE_OUTPUT,
        EOL_OUTPUT, // end of sequence
        NUM_OUTPUTS
    };
    enum LightIds
    {
        PREV_BUTTON_LIGHT,
        NEXT_BUTTON_LIGHT,
        ZERO_BUTTON_LIGHT,
        NUM_LIGHTS
    };

    OneZero()
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
        first_step = true;
        step = 0;
        wait_for_reset_timer = true;

        // Set up a counter so that the clock input will ignore
        // incoming clock pulses for 1 millisecond after a reset input. This
        // is to comply with VCV Rack's standards.  See section "Timing" at
        // https://vcvrack.com/manual/VoltageStandards

        reset_timer.reset();
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
        if (next_sequence_trigger.process(inputs[NEXT_SEQUENCE_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger) || next_sequence_button_trigger.process(params[NEXT_BUTTON_PARAM].getValue()))
        {
            selected_sequence = ((selected_sequence + 1) % sequences.size());
            nextPulse.trigger(1e-3f);
        }

        // Process PREV trigger and button
        if (prev_sequence_trigger.process(inputs[PREV_SEQUENCE_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger) || prev_sequence_button_trigger.process(params[PREV_BUTTON_PARAM].getValue()))
        {
            selected_sequence = (selected_sequence == 0) ? sequences.size() - 1 : selected_sequence - 1;
            prevPulse.trigger(1e-3f);
        }

        // Process ZERO trigger and button
        if (zero_sequence_trigger.process(inputs[ZERO_SEQUENCE_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger) || zero_sequence_button_trigger.process(params[ZERO_BUTTON_PARAM].getValue()))
        {
            zeroPulse.trigger(1e-3f);
            selected_sequence = 0;
        }

        // Process RESET input
        if (reset_trigger.process(inputs[RESET_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
        {
            reset();
        }

        // Adjust selected sequence based on CV input (if connected)
        if (inputs[CV_SEQUENCE_SELECT].isConnected())
        {
            unsigned int sequence_count = sequences.size() - 1;  // 20
            float sequence_select_cv = inputs[CV_SEQUENCE_SELECT].getVoltage() * params[CV_SEQUENCE_ATTN_KNOB].getValue();
            int cv_sequence_value = (int) rescale(sequence_select_cv, -5.0, 5.0, -20, 20);

            real_selected_sequence = wrap(selected_sequence + cv_sequence_value, 0, sequence_count);
        }
        else
        {
            real_selected_sequence = selected_sequence;
        }

        // Process STEP input
        if (!wait_for_reset_timer && step_trigger.process(inputs[STEP_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
        {
            // If there's a step input, but first_step is true, then don't
            // increment the step and output the value at step #1
            if (first_step)
            {
                first_step = false;
            }
            // Otherwise, step the sequencer
            else
            {
                step++;

                // If we're at the end of the sequencer, wrap to the beginning
                if (step == sequences[real_selected_sequence].size())
                {
                    step = 0;
                    eol_pulse_generator.trigger(0.01f);
                }
            }

            if (sequences[real_selected_sequence][step])
                output_pulse_generator.trigger(0.01f);
        }

        if (wait_for_reset_timer)
        {
            // Accumulate time in reset_timer
            if (reset_timer.process(args.sampleTime * 1000.0) > 1.0)
            {
                wait_for_reset_timer = false;
                reset_timer.reset();
            }
        }

        bool prevGate = prevPulse.process(args.sampleTime);
        bool nextGate = nextPulse.process(args.sampleTime);
        bool zeroGate = zeroPulse.process(args.sampleTime);

        // Lights
        lights[PREV_BUTTON_LIGHT].setSmoothBrightness(prevGate, args.sampleTime);
        lights[NEXT_BUTTON_LIGHT].setSmoothBrightness(nextGate, args.sampleTime);
        lights[ZERO_BUTTON_LIGHT].setSmoothBrightness(zeroGate, args.sampleTime);

        //
        // Outputs
        //

        bool output_pulse = output_pulse_generator.process(1.0 / args.sampleRate);
        outputs[GATE_OUTPUT].setVoltage((output_pulse ? 10.0f : 0.0f));

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
                std::vector<bool> sequence;

                for (auto &character : line)
                {
                    // Print current character
                    if(character == '1') sequence.push_back(true);
                    if(character == '0') sequence.push_back(false);                    
                }

                sequences.push_back(sequence);
            }

            /* 

            // I want to add this to the module, but I'm not sure how to include the library
            
            std::string dir = system::getDirectory(path);
            efsw = efsw_create(false);
            efsw_addwatch(efsw, dir.c_str(), watchCallback, false, this);
            efsw_watch(efsw);
            
            */

        }

        reset();
    }

    /*
	static void watchCallback(efsw_watcher watcher, efsw_watchid watchid, const char* dir, const char* filename, enum efsw_action action, const char* old_filename, void* param) {
		OneZero* that = (OneZero*) param;
		if (action == EFSW_ADD || action == EFSW_DELETE || action == EFSW_MODIFIED || action == EFSW_MOVED) {
			// Check filename
			std::string pathFilename = system::getFilename(that->path);
			if (pathFilename == filename) {
				that->loadData();
			}
		}
	}
    */

#ifndef USING_CARDINAL_NOT_RACK
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
#endif
};
