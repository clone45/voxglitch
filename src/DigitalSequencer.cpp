//
// Voxglitch "DigitalSequencer" module for VCV Rack
// by Bret Truchan
//
// Probably influenced by Nord Modular and Reaktor, but it's been too long for
// me to remember.
//
// Special thanks to Artem Leonov for his testing and suggestions.
// Special thanks to Marc Boulé for his help with reset trigger behavior.
// Special thanks to the entire VCV Rack community for their support.
//
// TODO: Move font loading into global area
// TODO: copy/paste between sequences

#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include <fstream>
#include <array>

#define NUMBER_OF_SEQUENCERS 6
#define MAX_SEQUENCER_STEPS 32
#define NUMBER_OF_VOLTAGE_RANGES 8
#define NUMBER_OF_SNAP_DIVISIONS 8

// Constants for patterns
#define DRAW_AREA_WIDTH 486.0
#define DRAW_AREA_HEIGHT 214.0
#define BAR_HEIGHT 214.0
#define BAR_HORIZONTAL_PADDING .8
#define DRAW_AREA_POSITION_X 9
#define DRAW_AREA_POSITION_Y 9.5

// Constants for gate sequencer
#define GATES_DRAW_AREA_WIDTH 486.0
#define GATES_DRAW_AREA_HEIGHT 16.0
#define GATES_DRAW_AREA_POSITION_X 9
#define GATES_DRAW_AREA_POSITION_Y 86
#define GATE_BAR_HEIGHT 16.0

#define TOOLTIP_WIDTH 33.0
#define TOOLTIP_HEIGHT 20.0

double voltage_ranges[NUMBER_OF_VOLTAGE_RANGES][2] = {
    { 0.0, 10.0 },
    { -10.0, 10.0 },
    { 0.0, 5.0 },
    { -5.0, 5.0 },
    { 0.0, 3.0 },
    { -3.0, 3.0 },
    { 0.0, 1.0},
    { -1.0, 1.0}
};

double snap_divisions[NUMBER_OF_SNAP_DIVISIONS] = { 0,8,10,12,16,24,32,26 };

struct Sequencer
{
    unsigned int sequence_length = 16;
    unsigned int sequence_playback_position = 0;

    void step()
    {
        sequence_playback_position = (sequence_playback_position + 1) % sequence_length;
    }

    void reset()
    {
        sequence_playback_position = 0;
    }

    unsigned int getPlaybackPosition()
    {
        return(sequence_playback_position);
    }

    unsigned int getLength()
    {
        return(sequence_length);
    }

    void setLength(unsigned int sequence_length)
    {
        this->sequence_length = sequence_length;
    }
};

struct VoltageSequencer : Sequencer
{
    std::array<double, MAX_SEQUENCER_STEPS> sequence;
    unsigned int voltage_range_index = 0; // see voltage_ranges in DigitalSequencer.h
    unsigned int snap_division_index = 0;
    bool sample_and_hold = false;

    // constructor
    VoltageSequencer()
    {
        sequence.fill(0.0);
    }

    // Returns the 'raw' output from the sequencer, which ranges from 0 to 214,
    // which is the height in pixels of the draw area.  To get the value with
    // the selected range applied, see getOutput()

    double getValue(int index)
    {
        return(sequence[index]);
    }

    // Same as GetValue, but if no index is provided, returns the value at the
    // current playback position.
    double getValue()
    {
        return(sequence[getPlaybackPosition()]);
    }

    // Returns the value of the sequencer at a specific index after the selected
    // voltage range has been applied.
    double getOutput(int index)
    {
        return(rescale(sequence[index], 0.0, DRAW_AREA_HEIGHT, voltage_ranges[voltage_range_index][0], voltage_ranges[voltage_range_index][1]));
    }

    double getOutput()
    {
        return(rescale(sequence[getPlaybackPosition()], 0.0, DRAW_AREA_HEIGHT, voltage_ranges[voltage_range_index][0], voltage_ranges[voltage_range_index][1]));
    }

    void setValue(int index, double value)
    {
        if(snap_division_index > 0)
        {
            double division = DRAW_AREA_HEIGHT / snap_divisions[snap_division_index];
            sequence[index] = division * roundf(value / division);
        }
        else
        {
            sequence[index] = value;
        }
    }

    void shiftLeft()
    {
        double temp = sequence[0];
        for(unsigned int i=0; i < this->sequence_length-1; i++)
        {
            sequence[i] = sequence[i+1];
        }
        sequence[this->sequence_length-1] = temp;
    }

    void shiftRight()
    {
        double temp = sequence[this->sequence_length - 1];

        for(unsigned int i=this->sequence_length-1; i>0; i--)
        {
            sequence[i] = sequence[i-1];
        }

        sequence[0] = temp;
    }

    void randomize()
    {
        for(unsigned int i=0; i < this->sequence_length; i++)
        {
            this->setValue(i, fmod(std::rand(), DRAW_AREA_HEIGHT));
        }
    }
};

struct GateSequencer : Sequencer
{
    std::array<bool, MAX_SEQUENCER_STEPS> sequence;

    // constructor
    GateSequencer()
    {
        sequence.fill(0.0);
    }

    bool getValue(int index)
    {
        return(sequence[index]);
    }

    bool getValue()
    {
        return(sequence[sequence_playback_position]);
    }

    void setValue(int index, bool value)
    {
        sequence[index] = value;
    }

    void shiftLeft()
    {
        double temp = sequence[0];
        for(unsigned int i=0; i < this->sequence_length-1; i++)
        {
            sequence[i] = sequence[i+1];
        }
        sequence[this->sequence_length-1] = temp;
    }

    void shiftRight()
    {
        double temp = sequence[this->sequence_length - 1];

        for(unsigned int i=this->sequence_length-1; i>0; i--)
        {
            sequence[i] = sequence[i-1];
        }

        sequence[0] = temp;
    }

    void randomize()
    {
        for(unsigned int i=0; i < this->sequence_length; i++)
        {
            this->setValue(i, fmod(std::rand(), 2));
        }
    }
};

struct DigitalSequencer : Module
{
	dsp::SchmittTrigger stepTrigger;
    dsp::SchmittTrigger sequencer_step_triggers[NUMBER_OF_SEQUENCERS];
    dsp::SchmittTrigger resetTrigger;

	dsp::SchmittTrigger sequencer_1_button_trigger;
    dsp::SchmittTrigger sequencer_2_button_trigger;
    dsp::SchmittTrigger sequencer_3_button_trigger;
    dsp::SchmittTrigger sequencer_4_button_trigger;
    dsp::SchmittTrigger sequencer_5_button_trigger;
    dsp::SchmittTrigger sequencer_6_button_trigger;

    long clock_ignore_on_reset = 0;
    unsigned int tooltip_timer = 0;

    VoltageSequencer voltage_sequencers[NUMBER_OF_SEQUENCERS];
    VoltageSequencer *selected_voltage_sequencer;

    GateSequencer gate_sequencers[NUMBER_OF_SEQUENCERS];
    GateSequencer *selected_gate_sequencer;

    int selected_sequencer_index = 0;
    // int previously_selected_sequencer_index = -1;
    int voltage_outputs[NUMBER_OF_SEQUENCERS];
    int gate_outputs[NUMBER_OF_SEQUENCERS];
    int sequencer_step_inputs[NUMBER_OF_SEQUENCERS];

    dsp::PulseGenerator gateOutputPulseGenerators[NUMBER_OF_SEQUENCERS];
    double sample_rate;

    bool sequencer_1_button_is_triggered;
    bool sequencer_2_button_is_triggered;
    bool sequencer_3_button_is_triggered;
    bool sequencer_4_button_is_triggered;
    bool sequencer_5_button_is_triggered;
    bool sequencer_6_button_is_triggered;

    std::string voltage_range_names[NUMBER_OF_VOLTAGE_RANGES] = {
        "0.0 to 10.0",
        "-10.0 to 10.0",
        "0.0 to 5.0",
        "-5.0 to 5.0",
        "0.0 to 3.0",
        "-3.0 to 3.0",
        "0.0 to 1.0",
        "-1.0 to 1.0"
    };

    std::string snap_division_names[NUMBER_OF_SNAP_DIVISIONS] = { "None", "8", "10", "12", "16", "24", "32", "36" };

	enum ParamIds {
        SEQUENCE_SELECTION_KNOB,
        SEQUENCER_1_LENGTH_KNOB,
        SEQUENCER_2_LENGTH_KNOB,
        SEQUENCER_3_LENGTH_KNOB,
        SEQUENCER_4_LENGTH_KNOB,
        SEQUENCER_5_LENGTH_KNOB,
        SEQUENCER_6_LENGTH_KNOB,

        SEQUENCE_START_KNOB,
        SEQUENCER_1_BUTTON,
        SEQUENCER_2_BUTTON,
        SEQUENCER_3_BUTTON,
        SEQUENCER_4_BUTTON,
        SEQUENCER_5_BUTTON,
        SEQUENCER_6_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
        STEP_INPUT,
        RESET_INPUT,
        SEQUENCER_1_STEP_INPUT,
        SEQUENCER_2_STEP_INPUT,
        SEQUENCER_3_STEP_INPUT,
        SEQUENCER_4_STEP_INPUT,
        SEQUENCER_5_STEP_INPUT,
        SEQUENCER_6_STEP_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		END_OUTPUT,

        SEQ1_CV_OUTPUT,
        SEQ2_CV_OUTPUT,
        SEQ3_CV_OUTPUT,
        SEQ4_CV_OUTPUT,
        SEQ5_CV_OUTPUT,
        SEQ6_CV_OUTPUT,

        SEQ1_GATE_OUTPUT,
        SEQ2_GATE_OUTPUT,
        SEQ3_GATE_OUTPUT,
        SEQ4_GATE_OUTPUT,
        SEQ5_GATE_OUTPUT,
        SEQ6_GATE_OUTPUT,

		NUM_OUTPUTS
	};
	enum LightIds {
        SEQUENCER_1_LIGHT,
        SEQUENCER_2_LIGHT,
        SEQUENCER_3_LIGHT,
        SEQUENCER_4_LIGHT,
        SEQUENCER_5_LIGHT,
        SEQUENCER_6_LIGHT,
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	DigitalSequencer()
	{
        voltage_outputs[0] = SEQ1_CV_OUTPUT;
        voltage_outputs[1] = SEQ2_CV_OUTPUT;
        voltage_outputs[2] = SEQ3_CV_OUTPUT;
        voltage_outputs[3] = SEQ4_CV_OUTPUT;
        voltage_outputs[4] = SEQ5_CV_OUTPUT;
        voltage_outputs[5] = SEQ6_CV_OUTPUT;

        gate_outputs[0] = SEQ1_GATE_OUTPUT;
        gate_outputs[1] = SEQ2_GATE_OUTPUT;
        gate_outputs[2] = SEQ3_GATE_OUTPUT;
        gate_outputs[3] = SEQ4_GATE_OUTPUT;
        gate_outputs[4] = SEQ5_GATE_OUTPUT;
        gate_outputs[5] = SEQ6_GATE_OUTPUT;

        sequencer_step_inputs[0] = SEQUENCER_1_STEP_INPUT;
        sequencer_step_inputs[1] = SEQUENCER_2_STEP_INPUT;
        sequencer_step_inputs[2] = SEQUENCER_3_STEP_INPUT;
        sequencer_step_inputs[3] = SEQUENCER_4_STEP_INPUT;
        sequencer_step_inputs[4] = SEQUENCER_5_STEP_INPUT;
        sequencer_step_inputs[5] = SEQUENCER_6_STEP_INPUT;

        selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];
        selected_gate_sequencer = &gate_sequencers[selected_sequencer_index];

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(SEQUENCER_1_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "SequenceLengthKnob");
        configParam(SEQUENCER_2_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "Sequencer2LengthKnob");
        configParam(SEQUENCER_3_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "Sequencer3LengthKnob");
        configParam(SEQUENCER_4_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "Sequencer4LengthKnob");
        configParam(SEQUENCER_5_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "Sequencer5LengthKnob");
        configParam(SEQUENCER_6_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "Sequencer6LengthKnob");

        configParam(SEQUENCER_1_BUTTON, 0.f, 1.f, 0.f, "Sequence1Button");
        configParam(SEQUENCER_2_BUTTON, 0.f, 1.f, 0.f, "Sequence2Button");
        configParam(SEQUENCER_3_BUTTON, 0.f, 1.f, 0.f, "Sequence3Button");
        configParam(SEQUENCER_4_BUTTON, 0.f, 1.f, 0.f, "Sequence4Button");
        configParam(SEQUENCER_5_BUTTON, 0.f, 1.f, 0.f, "Sequence5Button");
        configParam(SEQUENCER_6_BUTTON, 0.f, 1.f, 0.f, "Sequence6Button");
	}

    /*
        ==================================================================================================================================================
                                 ___                 _
                                / / |               | |
          ___  __ ___   _____  / /| | ___   __ _  __| |
        / __|/ _` \ \ / / _ \ / / | |/ _ \ / _` |/ _` |
        \__ \ (_| |\ V /  __// /  | | (_) | (_| | (_| |
        |___/\__,_| \_/ \___/_/   |_|\___/ \__,_|\__,_|

        ==================================================================================================================================================
    */

	json_t *dataToJson() override
	{
        json_t *json_root = json_object();

		//
		// Save patterns
		//

		json_t *sequences_json_array = json_array();

		for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
		{
			json_t *pattern_json_array = json_array();

			for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
			{
				json_array_append_new(pattern_json_array, json_integer(this->voltage_sequencers[sequencer_number].getValue(i)));
			}

            json_array_append_new(sequences_json_array, pattern_json_array);
        }

        json_object_set(json_root, "patterns", sequences_json_array);
        json_decref(sequences_json_array);

        //
		// Save gates
		//

		json_t *gates_json_array = json_array();

		for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
		{
			json_t *pattern_json_array = json_array();

			for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
			{
				json_array_append_new(pattern_json_array, json_integer(this->gate_sequencers[sequencer_number].getValue(i)));
			}

            json_array_append_new(gates_json_array, pattern_json_array);
        }

        json_object_set(json_root, "gates", gates_json_array);
        json_decref(gates_json_array);

        //
        // Save sequencer lengths
        //
        json_t *sequencer_lengths_json_array = json_array();
        for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            json_array_append_new(sequencer_lengths_json_array, json_integer(this->voltage_sequencers[sequencer_number].getLength()));
        }
        json_object_set(json_root, "lengths", sequencer_lengths_json_array);
        json_decref(sequencer_lengths_json_array);

        //
        // Save sequencer voltage range index selections
        //
        json_t *sequencer_voltage_range_json_array = json_array();
        for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            json_array_append_new(sequencer_voltage_range_json_array, json_integer(this->voltage_sequencers[sequencer_number].voltage_range_index));
        }
        json_object_set(json_root, "voltage_ranges", sequencer_voltage_range_json_array);
        json_decref(sequencer_voltage_range_json_array);

        //
        // Save sequencer snap index selections
        //
        json_t *sequencer_snap_json_array = json_array();
        for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            json_array_append_new(sequencer_snap_json_array, json_integer(this->voltage_sequencers[sequencer_number].snap_division_index));
        }
        json_object_set(json_root, "snap_divisions", sequencer_snap_json_array);
        json_decref(sequencer_snap_json_array);

        //
        // Save sequencer sample and hold selections
        //
        json_t *sequencer_sh_json_array = json_array();
        for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            json_array_append_new(sequencer_sh_json_array, json_integer(this->voltage_sequencers[sequencer_number].sample_and_hold));
        }
        json_object_set(json_root, "sample_and_hold", sequencer_sh_json_array);
        json_decref(sequencer_sh_json_array);

		return json_root;
	}

	// Autoload settings
	void dataFromJson(json_t *json_root) override
	{
		//
		// Load patterns
		//

		json_t *pattern_arrays_data = json_object_get(json_root, "patterns");

		if(pattern_arrays_data)
		{
			size_t pattern_number;
			json_t *json_pattern_array;

			json_array_foreach(pattern_arrays_data, pattern_number, json_pattern_array)
			{
				for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
				{
                    this->voltage_sequencers[pattern_number].setValue(i, json_integer_value(json_array_get(json_pattern_array, i)));
				}
			}
		}

        //
		// Load gates
		//

        json_t *gates_arrays_data = json_object_get(json_root, "gates");

        if(gates_arrays_data)
        {
            size_t pattern_number;
            json_t *json_pattern_array;

            json_array_foreach(gates_arrays_data, pattern_number, json_pattern_array)
            {
                for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
                {
                    // this->gates[pattern_number][i] = json_integer_value(json_array_get(json_pattern_array, i));
                    this->gate_sequencers[pattern_number].setValue(i, json_integer_value(json_array_get(json_pattern_array, i)));
                }
            }
        }

        //
        // Load lengths
        //
        json_t *lengths_json_array = json_object_get(json_root, "lengths");

        if(lengths_json_array)
        {
            size_t sequencer_number;
            json_t *length_json;

            json_array_foreach(lengths_json_array, sequencer_number, length_json)
            {
                this->voltage_sequencers[sequencer_number].setLength(json_integer_value(length_json));
                this->gate_sequencers[sequencer_number].setLength(json_integer_value(length_json));
            }
        }

        //
        // Load voltage ranges
        //
        json_t *voltage_ranges_json_array = json_object_get(json_root, "voltage_ranges");

        if(voltage_ranges_json_array)
        {
            size_t sequencer_number;
            json_t *voltage_range_json;

            json_array_foreach(voltage_ranges_json_array, sequencer_number, voltage_range_json)
            {
                this->voltage_sequencers[sequencer_number].voltage_range_index = json_integer_value(voltage_range_json);
            }
        }

        //
        // Load snap divisions
        //
        json_t *snap_divions_json_array = json_object_get(json_root, "snap_divisions");

        if(snap_divions_json_array)
        {
            size_t sequencer_number;
            json_t *snap_divion_json;

            json_array_foreach(snap_divions_json_array, sequencer_number, snap_divion_json)
            {
                this->voltage_sequencers[sequencer_number].snap_division_index = json_integer_value(snap_divion_json);
            }
        }

        //
        // Load Sample and Hold settings
        //
        json_t *sh_json_array = json_object_get(json_root, "sample_and_hold");

        if(sh_json_array)
        {
            size_t sequencer_number;
            json_t *sh_json;

            json_array_foreach(sh_json_array, sequencer_number, sh_json)
            {
                this->voltage_sequencers[sequencer_number].sample_and_hold = json_integer_value(sh_json);
            }
        }
	}


    /*

    ______
    | ___ \
    | |_/ / __ ___   ___ ___  ___ ___
    |  __/ '__/ _ \ / __/ _ \/ __/ __|
    | |  | | | (_) | (_|  __/\__ \__ \
    \_|  |_|  \___/ \___\___||___/___/


    */

	void process(const ProcessArgs &args) override
	{
        bool trigger_output_pulse = false;
        this->sample_rate = args.sampleRate;

        selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];
        selected_gate_sequencer = &gate_sequencers[selected_sequencer_index];

        sequencer_1_button_is_triggered = sequencer_1_button_trigger.process(params[SEQUENCER_1_BUTTON].getValue());
        sequencer_2_button_is_triggered = sequencer_2_button_trigger.process(params[SEQUENCER_2_BUTTON].getValue());
        sequencer_3_button_is_triggered = sequencer_3_button_trigger.process(params[SEQUENCER_3_BUTTON].getValue());
        sequencer_4_button_is_triggered = sequencer_4_button_trigger.process(params[SEQUENCER_4_BUTTON].getValue());
        sequencer_5_button_is_triggered = sequencer_5_button_trigger.process(params[SEQUENCER_5_BUTTON].getValue());
        sequencer_6_button_is_triggered = sequencer_6_button_trigger.process(params[SEQUENCER_6_BUTTON].getValue());

		if(sequencer_1_button_is_triggered) selected_sequencer_index = 0;
        if(sequencer_2_button_is_triggered) selected_sequencer_index = 1;
        if(sequencer_3_button_is_triggered) selected_sequencer_index = 2;
        if(sequencer_4_button_is_triggered) selected_sequencer_index = 3;
        if(sequencer_5_button_is_triggered) selected_sequencer_index = 4;
        if(sequencer_6_button_is_triggered) selected_sequencer_index = 5;

        voltage_sequencers[0].setLength(clamp((int) params[SEQUENCER_1_LENGTH_KNOB].getValue(), 1, 32));
        voltage_sequencers[1].setLength(clamp((int) params[SEQUENCER_2_LENGTH_KNOB].getValue(), 1, 32));
        voltage_sequencers[2].setLength(clamp((int) params[SEQUENCER_3_LENGTH_KNOB].getValue(), 1, 32));
        voltage_sequencers[3].setLength(clamp((int) params[SEQUENCER_4_LENGTH_KNOB].getValue(), 1, 32));
        voltage_sequencers[4].setLength(clamp((int) params[SEQUENCER_5_LENGTH_KNOB].getValue(), 1, 32));
        voltage_sequencers[5].setLength(clamp((int) params[SEQUENCER_6_LENGTH_KNOB].getValue(), 1, 32));

        gate_sequencers[0].setLength(clamp((int) params[SEQUENCER_1_LENGTH_KNOB].getValue(), 1, 32));
        gate_sequencers[1].setLength(clamp((int) params[SEQUENCER_2_LENGTH_KNOB].getValue(), 1, 32));
        gate_sequencers[2].setLength(clamp((int) params[SEQUENCER_3_LENGTH_KNOB].getValue(), 1, 32));
        gate_sequencers[3].setLength(clamp((int) params[SEQUENCER_4_LENGTH_KNOB].getValue(), 1, 32));
        gate_sequencers[4].setLength(clamp((int) params[SEQUENCER_5_LENGTH_KNOB].getValue(), 1, 32));
        gate_sequencers[5].setLength(clamp((int) params[SEQUENCER_6_LENGTH_KNOB].getValue(), 1, 32));

        // On incoming RESET, reset the sequencers
        if(resetTrigger.process(rescale(inputs[RESET_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
        {
            // Set up a (reverse) counter so that the clock input will ignore
            // incoming clock pulses for 1 millisecond after a reset input. This
            // is to comply with VCV Rack's standards.  See section "Timing" at
            // https://vcvrack.com/manual/VoltageStandards

            clock_ignore_on_reset = (long) (args.sampleRate / 100);

            stepTrigger.reset();

            for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
            {
                sequencer_step_triggers[i].reset();
                voltage_sequencers[i].reset();
                gate_sequencers[i].reset();
            }
        }
        else if(clock_ignore_on_reset == 0)
        {
            // Step ALL of the sequencers
            bool global_step_trigger = stepTrigger.process(rescale(inputs[STEP_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f));
            bool step;

            for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
            {
                step = false;

                if(inputs[sequencer_step_inputs[i]].isConnected() == false)
                {
                    if(global_step_trigger) step = true;
                }
                else if (sequencer_step_triggers[i].process(rescale(inputs[sequencer_step_inputs[i]].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
                {
                    step = true;
                }

                if(step)
                {
                    voltage_sequencers[i].step();
                    gate_sequencers[i].step();
                    if(gate_sequencers[i].getValue()) gateOutputPulseGenerators[i].trigger(0.01f);
                }
            }
        }

        // output values
        for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            if(voltage_sequencers[i].sample_and_hold)
            {
                if(gate_sequencers[i].getValue()) outputs[voltage_outputs[i]].setVoltage(voltage_sequencers[i].getOutput());
            }
            else
            {
                outputs[voltage_outputs[i]].setVoltage(voltage_sequencers[i].getOutput());
            }
        }

        // process trigger outputs
        for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
		    outputs[gate_outputs[i]].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
        }

        if (clock_ignore_on_reset > 0) clock_ignore_on_reset--;
        if (tooltip_timer > 0) tooltip_timer--;

        lights[SEQUENCER_1_LIGHT].setBrightness(selected_sequencer_index == 0);
        lights[SEQUENCER_2_LIGHT].setBrightness(selected_sequencer_index == 1);
        lights[SEQUENCER_3_LIGHT].setBrightness(selected_sequencer_index == 2);
        lights[SEQUENCER_4_LIGHT].setBrightness(selected_sequencer_index == 3);
        lights[SEQUENCER_5_LIGHT].setBrightness(selected_sequencer_index == 4);
        lights[SEQUENCER_6_LIGHT].setBrightness(selected_sequencer_index == 5);
	}

};

/*

 _    _ _     _            _
| |  | (_)   | |          | |
| |  | |_  __| | __ _  ___| |_ ___
| |/\| | |/ _` |/ _` |/ _ \ __/ __|
\  /\  / | (_| | (_| |  __/ |_\__ \
\/  \/|_|\__,_|\__, |\___|\__|___/
                __/ |
               |___/
*/

struct DigitalSequencerDisplay : TransparentWidget
{
    DigitalSequencer *module;
	Vec drag_position;
    double bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;

	void onDragStart(const event::DragStart &e) override
    {
		TransparentWidget::onDragStart(e);
	}

	void onDragEnd(const event::DragEnd &e) override
    {
		TransparentWidget::onDragEnd(e);
	}

    void step() override {
		TransparentWidget::step();
	}

    void onEnter(const event::Enter &e) override
    {
		TransparentWidget::onEnter(e);
	}

	void onLeave(const event::Leave &e) override
    {
		TransparentWidget::onLeave(e);
	}

    bool keypress(const event::HoverKey &e, int keycode)
    {
        if (e.key == keycode)
        {
            e.consume(this);
            if(e.action == GLFW_PRESS) return(true);
        }
        return(false);
    }

    bool keypressRight(const event::HoverKey &e)
    {
        return(keypress(e, GLFW_KEY_RIGHT));
    }

    bool keypressLeft(const event::HoverKey &e)
    {
        return(keypress(e, GLFW_KEY_LEFT));
    }

    bool keypressUp(const event::HoverKey &e)
    {
        return(keypress(e, GLFW_KEY_UP));
    }

    bool keypressDown(const event::HoverKey &e)
    {
        return(keypress(e, GLFW_KEY_DOWN));
    }

    void drawVerticalGuildes(NVGcontext *vg, double height)
    {
        for(unsigned int i=1; i < 8; i++)
        {
            nvgBeginPath(vg);
            int x = (i * 4 * bar_width) + (i * 4 * BAR_HORIZONTAL_PADDING);
            nvgRect(vg, x, 0, 1, height);
            nvgFillColor(vg, nvgRGBA(240, 240, 255, 40));
            nvgFill(vg);
        }
    }

    void drawBlueOverlay(NVGcontext *vg, double width, double height)
    {
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, width, height);
        nvgFillColor(vg, nvgRGBA(0, 100, 255, 28));
        nvgFill(vg);
    }

    void drawBar(NVGcontext *vg, double position, double height, double container_height, NVGcolor color)
    {
        nvgBeginPath(vg);
        nvgRect(vg, (position * bar_width) + (position * BAR_HORIZONTAL_PADDING), container_height - height, bar_width, height);
        nvgFillColor(vg, color);
        nvgFill(vg);
    }
};

struct DigitalSequencerPatternDisplay : DigitalSequencerDisplay
{
    bool draw_tooltip = false;
    double draw_tooltip_index = -1.0;
    double draw_tooltip_y = -1.0;
    double tooltip_value = 0.0;

	DigitalSequencerPatternDisplay()
	{
		// The bounding box needs to be a little deeper than the visual
		// controls to allow mouse drags to indicate '0' (off) column heights,
		// which is why 16 is being added to the draw height to define the
		// bounding box.
		box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT + 16);
	}

	void draw(const DrawArgs &args) override
	{
		const auto vg = args.vg;
        int value;
        NVGcolor bar_color;
        bool draw_from_center = false;

        // Save the drawing context to restore later
		nvgSave(vg);

		if(module)
		{
            double range_low = voltage_ranges[module->selected_voltage_sequencer->voltage_range_index][0];
            double range_high = voltage_ranges[module->selected_voltage_sequencer->voltage_range_index][1];

            if(range_low < 0 && range_high > 0) draw_from_center = true;

			//
			// Display the pattern
			//
			for(unsigned int i=0; i < MAX_SEQUENCER_STEPS; i++)
			{
				value = module->selected_voltage_sequencer->getValue(i);

                // Draw grey background bar
                if(i < module->selected_voltage_sequencer->getLength()) {
                    bar_color = nvgRGBA(60, 60, 64, 255);
                }
                else {
                    bar_color = nvgRGBA(45, 45, 45, 255);
                }

                drawBar(vg, i, BAR_HEIGHT, DRAW_AREA_HEIGHT, bar_color);

				if(i == module->selected_voltage_sequencer->getPlaybackPosition())
				{
					bar_color = nvgRGBA(255, 255, 255, 250);
				}
				else if(i < module->selected_voltage_sequencer->getLength())
				{
					bar_color = nvgRGBA(255, 255, 255, 150);
				}
                else
                {
                    bar_color = nvgRGBA(255, 255, 255, 10);
                }

                // Draw bars for the sequence values
				if(value > 0) drawBar(vg, i, value, DRAW_AREA_HEIGHT, bar_color);

                // Highlight the sequence playback column
				if(i == module->selected_voltage_sequencer->getPlaybackPosition())
				{
                    drawBar(vg, i, DRAW_AREA_HEIGHT, DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 20));
				}
			}

            // Draw a horizontal 0-indicator if the range is not symmetrical
            if(draw_from_center)
            {
                // This calculation for y takes advance of the fact that all
                // ranges that would have the 0-guide visible are symmetric, so
                // it will need updating if non-symmetric ranges are added.
                double y = DRAW_AREA_HEIGHT / 2.0;

                nvgBeginPath(vg);
                nvgRect(vg, 1, y, (DRAW_AREA_WIDTH - 2), 1.0);
                nvgFillColor(vg, nvgRGBA(240, 240, 255, 40));
                nvgFill(vg);
            }

		}
        else // Draw a demo sequence so that the sequencer looks nice in the library selector
        {
            double demo_sequence[32] = {100.0,100.0,93.0,80.0,67.0,55.0,47.0,44.0,43.0,44.0,50.0,69.0,117.0,137.0,166,170,170,164,148,120,105,77,65,41,28,23,22,22,28,48,69,94};

            for(unsigned int i=0; i < MAX_SEQUENCER_STEPS; i++)
			{
                // Draw blue background bars
                drawBar(vg, i, BAR_HEIGHT, DRAW_AREA_HEIGHT, nvgRGBA(60, 60, 64, 255));

                // Draw bar for value at i
                drawBar(vg, i, demo_sequence[i], DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 150));

                // Highlight active step
				if(i == 5) drawBar(vg, i, DRAW_AREA_HEIGHT, DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 20));
            }
        }

        drawVerticalGuildes(vg, DRAW_AREA_HEIGHT);
        drawBlueOverlay(vg, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);

        if(module)
        {
            if(module->tooltip_timer > 0) draw_tooltip = true;

            if(draw_tooltip)
            {
                drawTooltip(vg);
                draw_tooltip = false;
            }
        }

		nvgRestore(vg);
	}

    void drawTooltip(NVGcontext *vg)
    {
        nvgSave(vg);

        double x_offset = 3.0;
        double y = std::max(60.0, draw_tooltip_y);
        double x = ((draw_tooltip_index * bar_width) + (draw_tooltip_index * BAR_HORIZONTAL_PADDING)) + bar_width + x_offset;

        if(draw_tooltip_index > 26) x = x - bar_width - TOOLTIP_WIDTH - (x_offset * 2) - BAR_HORIZONTAL_PADDING;
        y = DRAW_AREA_HEIGHT - y + 30;

        // Draw box for containing text
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, TOOLTIP_WIDTH, TOOLTIP_HEIGHT, 2);
        nvgFillColor(vg, nvgRGBA(20, 20, 20, 250));
        nvgFill(vg);

        // Set up font style
        nvgFontSize(vg, 13);
		nvgFillColor(vg, nvgRGBA(255, 255, 255, 0xff));
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgTextLetterSpacing(vg, -1);

        // Display text
        std::string display_string = std::to_string(tooltip_value);
        display_string = display_string.substr(0,4);
        nvgText(vg, x + 16.5, y + 14, display_string.c_str(), NULL);

        nvgRestore(vg);
    }

    //
    // void editBar(Vec mouse_position)
    //
    // Called when the user clicks to edit one of the sequencer values.  Sets
    // the sequencer value that the user has selected, then sets some variables
    // for drawing the tooltip in this struct's draw(..) method.
    //
    void editBar(Vec mouse_position)
	{
        double bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;
		int clicked_bar_x_index = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
		int clicked_y = DRAW_AREA_HEIGHT - mouse_position.y;

		clicked_bar_x_index = clamp(clicked_bar_x_index, 0, MAX_SEQUENCER_STEPS - 1);
        clicked_y = clamp(clicked_y, 0, DRAW_AREA_HEIGHT);

        module->selected_voltage_sequencer->setValue(clicked_bar_x_index, clicked_y);

        // Tooltip drawing is done in the draw method
        draw_tooltip = true;
        draw_tooltip_index = clicked_bar_x_index;
        draw_tooltip_y = clicked_y;
        tooltip_value = module->selected_voltage_sequencer->getOutput(clicked_bar_x_index);
	}

    void onButton(const event::Button &e) override
    {
		if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
		{
            e.consume(this);
			drag_position = e.pos;
			this->editBar(e.pos);
		}
	}

	void onDragMove(const event::DragMove &e) override
    {
		TransparentWidget::onDragMove(e);
        double zoom = std::pow(2.f, settings::zoom);
		drag_position = drag_position.plus(e.mouseDelta.div(zoom));
		editBar(drag_position);
	}

    void onHoverKey(const event::HoverKey &e) override
    {
        if(keypressRight(e))
        {
            module->selected_voltage_sequencer->shiftRight();
            if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_gate_sequencer->shiftRight();
        }

        if(keypressLeft(e))
        {
            module->selected_voltage_sequencer->shiftLeft();
            if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_gate_sequencer->shiftLeft();
        }

        if(keypressUp(e))
        {
            int bar_x_index = e.pos.x / (bar_width + BAR_HORIZONTAL_PADDING);
            double value = module->selected_voltage_sequencer->getValue(bar_x_index);

            // (.01 * (214 / 10)), where 214 is the bar height and 10 is the max voltage
            value = value + (.01 * (214.0 / 10.0));
            // value = clamp(value, 0.0, DRAW_AREA_HEIGHT);
            if (value > DRAW_AREA_HEIGHT) value = DRAW_AREA_HEIGHT;

            module->selected_voltage_sequencer->setValue(bar_x_index, value);

            module->tooltip_timer = module->sample_rate * 2; // show tooltip for 2 seconds
            tooltip_value = roundf((value / DRAW_AREA_HEIGHT) * 1000) / 100;
            draw_tooltip_index = bar_x_index;
            draw_tooltip_y = value;
        }

        if(keypressDown(e))
        {
            int bar_x_index = e.pos.x / (bar_width + BAR_HORIZONTAL_PADDING);
            double value = module->selected_voltage_sequencer->getValue(bar_x_index);

            // (.01 * (214 / 10)), where 214 is the bar height and 10 is the max voltage
            value = value - (.01 * (214.0 / 10.0));
            // value = clamp(value, 0.0, DRAW_AREA_HEIGHT);
            if (value > DRAW_AREA_HEIGHT) value = DRAW_AREA_HEIGHT;

            module->selected_voltage_sequencer->setValue(bar_x_index, value);

            module->tooltip_timer = module->sample_rate * 2; // show tooltip for 2 seconds
            tooltip_value = roundf((value / DRAW_AREA_HEIGHT) * 1000) / 100;
            draw_tooltip_index = bar_x_index;
            draw_tooltip_y = value;
        }

        // Randomize sequence by hovering over and pressing 'r'
        if(keypress(e, GLFW_KEY_R))
        {
            module->selected_voltage_sequencer->randomize();
            if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_gate_sequencer->randomize();
        }
    }
};

struct DigitalSequencerGatesDisplay : DigitalSequencerDisplay
{
    bool mouse_lock = false;
    double bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;
    int old_drag_bar_x = -1;
    bool trigger_edit_value = false;

	DigitalSequencerGatesDisplay()
	{
		box.size = Vec(GATES_DRAW_AREA_WIDTH, GATES_DRAW_AREA_HEIGHT);
	}

	void draw(const DrawArgs &args) override
	{
		const auto vg = args.vg;
        int value;
        double value_height;
        NVGcolor bar_color;

		nvgSave(vg);

		if(module)
		{
			for(unsigned int i=0; i < MAX_SEQUENCER_STEPS; i++)
			{
				value = module->selected_gate_sequencer->getValue(i);

                // Draw grey background bar
                if(i < module->selected_gate_sequencer->getLength()) {
                    bar_color = nvgRGBA(60, 60, 64, 255);
                }
                else {
                    bar_color = nvgRGBA(45, 45, 45, 255);
                }
                drawBar(vg, i, GATE_BAR_HEIGHT, GATES_DRAW_AREA_HEIGHT, bar_color);

                unsigned int playback_position = module->selected_gate_sequencer->getPlaybackPosition();

                if(i == playback_position)
                {
					bar_color = nvgRGBA(255, 255, 255, 250);
				}
				else if(i < module->selected_gate_sequencer->getLength())
				{
					bar_color = nvgRGBA(255, 255, 255, 150);
				}
                else // dim bars past playback position
                {
                    bar_color = nvgRGBA(255, 255, 255, 15);
                }

				value_height = (GATE_BAR_HEIGHT * value);
				if(value_height > 0) drawBar(vg, i, value_height, GATES_DRAW_AREA_HEIGHT, bar_color);

                // highlight active column
				if(i == playback_position)
				{
                    drawBar(vg, i, GATE_BAR_HEIGHT, GATES_DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 20));
				}
            }
		}
        else // draw demo data for the module explorer
        {
            int demo_sequence[32] = {1,0,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0};

            for(unsigned int i=0; i < MAX_SEQUENCER_STEPS; i++)
			{
				value = demo_sequence[i];

                // Draw background grey bar
                drawBar(vg, i, GATE_BAR_HEIGHT, GATES_DRAW_AREA_HEIGHT, nvgRGBA(60, 60, 64, 255));

                // Draw value bar
                if (value > 0) drawBar(vg, i, (GATE_BAR_HEIGHT * value), GATES_DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 150));

                // highlight active column
				if(i == 5) drawBar(vg, i, GATE_BAR_HEIGHT, GATES_DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 20));
			}
        }

        drawVerticalGuildes(vg, GATES_DRAW_AREA_HEIGHT);
        drawBlueOverlay(vg, GATES_DRAW_AREA_WIDTH, GATES_DRAW_AREA_HEIGHT);

		nvgRestore(vg);

	}

	void onButton(const event::Button &e) override
    {
        e.consume(this);

		if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
		{
            if(this->mouse_lock == false)
            {
                this->mouse_lock = true;

                int index = getIndexFromX(e.pos.x);

                // Store the value that's being set for later in case the user
                // drags to set ("paints") additional triggers
                this->trigger_edit_value = ! module->selected_gate_sequencer->getValue(index);

                // Set the trigger value in the sequencer
                module->selected_gate_sequencer->setValue(index, this->trigger_edit_value);

                // Store the initial drag position
                drag_position = e.pos;
            }
		}
        else if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
		{
            this->mouse_lock = false;
		}
	}

    void onDragMove(const event::DragMove &e) override
    {
		TransparentWidget::onDragMove(e);

        double zoom = std::pow(2.f, settings::zoom);
		drag_position = drag_position.plus(e.mouseDelta.div(zoom));

        int drag_bar_x_index = getIndexFromX(drag_position.x);

        if(drag_bar_x_index != old_drag_bar_x)
        {
            // setTrigger(drag_bar_x_index, trigger_edit_value);
            module->selected_gate_sequencer->setValue(drag_bar_x_index, trigger_edit_value);
            old_drag_bar_x = drag_bar_x_index;
        }
	}

    void onHoverKey(const event::HoverKey &e) override
    {
        if(keypressRight(e))
        {
            module->selected_gate_sequencer->shiftRight();
            if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_voltage_sequencer->shiftRight();
        }

        if(keypressLeft(e))
        {
            module->selected_gate_sequencer->shiftLeft();
            if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_voltage_sequencer->shiftLeft();
        }

        // Randomize sequence by hovering over and pressing 'r'
        if(keypress(e, GLFW_KEY_R))
        {
            module->selected_gate_sequencer->randomize();
            if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_voltage_sequencer->randomize();
        }
    }

    int getIndexFromX(double x)
    {
        return(x / (barWidth() + BAR_HORIZONTAL_PADDING));
    }

    double barWidth()
    {
        return((DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING);
    }

};

struct DigitalSequencerWidget : ModuleWidget
{
    DigitalSequencer* module;

	DigitalSequencerWidget(DigitalSequencer* module)
	{
        this->module = module;
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/digital_sequencer_front_panel.svg")));

		// Cosmetic rack screws
		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(mm2px(Vec(171.5, 0))));

        // Main voltage sequencer display
		DigitalSequencerPatternDisplay *pattern_display = new DigitalSequencerPatternDisplay();
		pattern_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
		pattern_display->module = module;
		addChild(pattern_display);

        DigitalSequencerGatesDisplay *gates_display = new DigitalSequencerGatesDisplay();
		gates_display->box.pos = mm2px(Vec(GATES_DRAW_AREA_POSITION_X, GATES_DRAW_AREA_POSITION_Y));
		gates_display->module = module;
		addChild(gates_display);

        double button_spacing = 9.6; // 9.1
        double button_group_x = 48.0;
        double button_group_y = 103.0;
        // Sequence 1 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x, button_group_y)), module, DigitalSequencer::SEQUENCER_1_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x, button_group_y)), module, DigitalSequencer::SEQUENCER_1_LIGHT));
        // Sequence 2 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y)), module, DigitalSequencer::SEQUENCER_2_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y)), module, DigitalSequencer::SEQUENCER_2_LIGHT));
        // Sequence 3 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y)), module, DigitalSequencer::SEQUENCER_3_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y)), module, DigitalSequencer::SEQUENCER_3_LIGHT));
        // Sequence 4 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y)), module, DigitalSequencer::SEQUENCER_4_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y)), module, DigitalSequencer::SEQUENCER_4_LIGHT));
        // Sequence 5 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y)), module, DigitalSequencer::SEQUENCER_5_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y)), module, DigitalSequencer::SEQUENCER_5_LIGHT));
        // Sequence 6 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 5.0), button_group_y)), module, DigitalSequencer::SEQUENCER_6_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 5.0), button_group_y)), module, DigitalSequencer::SEQUENCER_6_LIGHT));

        addParam(createParamCentered<Trimpot>(mm2px(Vec(button_group_x, button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_1_LENGTH_KNOB));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_2_LENGTH_KNOB));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_3_LENGTH_KNOB));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_4_LENGTH_KNOB));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_5_LENGTH_KNOB));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(button_group_x + (button_spacing * 5.0), button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_6_LENGTH_KNOB));

        // 6 step inputs
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(button_group_x, button_group_y + 18.0)), module, DigitalSequencer::SEQUENCER_1_STEP_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y + 18.0)), module, DigitalSequencer::SEQUENCER_2_STEP_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y + 18.0)), module, DigitalSequencer::SEQUENCER_3_STEP_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y + 18.0)), module, DigitalSequencer::SEQUENCER_4_STEP_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y + 18.0)), module, DigitalSequencer::SEQUENCER_5_STEP_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 5.0), button_group_y + 18.0)), module, DigitalSequencer::SEQUENCER_6_STEP_INPUT));

        // Step
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 114.893)), module, DigitalSequencer::STEP_INPUT));

        // Reset
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 14.544, 114.893)), module, DigitalSequencer::RESET_INPUT));



        // 6 sequencer outputs
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(118, 108.224)), module, DigitalSequencer::SEQ1_CV_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(129, 108.224)), module, DigitalSequencer::SEQ2_CV_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(140, 108.224)), module, DigitalSequencer::SEQ3_CV_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(151, 108.224)), module, DigitalSequencer::SEQ4_CV_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(162, 108.224)), module, DigitalSequencer::SEQ5_CV_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(173, 108.224)), module, DigitalSequencer::SEQ6_CV_OUTPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(118, 119.309)), module, DigitalSequencer::SEQ1_GATE_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(129, 119.309)), module, DigitalSequencer::SEQ2_GATE_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(140, 119.309)), module, DigitalSequencer::SEQ3_GATE_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(151, 119.309)), module, DigitalSequencer::SEQ4_GATE_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(162, 119.309)), module, DigitalSequencer::SEQ5_GATE_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(173, 119.309)), module, DigitalSequencer::SEQ6_GATE_OUTPUT));
	}


    // Sample and Hold values
    struct SampleAndHoldItem : MenuItem {
        DigitalSequencer *module;
        int sequencer_number = 0;

        void onAction(const event::Action &e) override {
            module->voltage_sequencers[sequencer_number].sample_and_hold ^= true; // flip the value
        }
    };

    //
    // INPUT SNAP MENUS
    //

    struct InputSnapValueItem : MenuItem {
        DigitalSequencer *module;
        int snap_division_index = 0;
        int sequencer_number = 0;

        void onAction(const event::Action &e) override {
            module->voltage_sequencers[sequencer_number].snap_division_index = snap_division_index;
        }
    };

    struct InputSnapItem : MenuItem {
		DigitalSequencer *module;
        int sequencer_number = 0;

		Menu *createChildMenu() override {
			Menu *menu = new Menu;

            for (unsigned int i=0; i < NUMBER_OF_SNAP_DIVISIONS; i++)
            {
                InputSnapValueItem *input_snap_value_item = createMenuItem<InputSnapValueItem>(module->snap_division_names[i], CHECKMARK(module->voltage_sequencers[sequencer_number].snap_division_index == i));
    			input_snap_value_item->module = module;
    			input_snap_value_item->snap_division_index = i;
                input_snap_value_item->sequencer_number = this->sequencer_number;
    			menu->addChild(input_snap_value_item);
            }

			return menu;
		}
	};

    //
    // OUTPUT RANGE MENUS
    //

    struct OutputRangeValueItem : MenuItem {
        DigitalSequencer *module;
        int range_index = 0;
        int sequencer_number = 0;

        void onAction(const event::Action &e) override {
            module->voltage_sequencers[sequencer_number].voltage_range_index = range_index;
        }
    };

    struct OutputRangeItem : MenuItem {
		DigitalSequencer *module;
        int sequencer_number = 0;

		Menu *createChildMenu() override {
			Menu *menu = new Menu;

            for (unsigned int i=0; i < NUMBER_OF_VOLTAGE_RANGES; i++)
            {
                OutputRangeValueItem *output_range_value_menu_item = createMenuItem<OutputRangeValueItem>(module->voltage_range_names[i], CHECKMARK(module->voltage_sequencers[sequencer_number].voltage_range_index == i));
    			output_range_value_menu_item->module = module;
    			output_range_value_menu_item->range_index = i;
                output_range_value_menu_item->sequencer_number = this->sequencer_number;
    			menu->addChild(output_range_value_menu_item);
            }

			return menu;
		}
	};

    struct SequencerItem : MenuItem {
		DigitalSequencer *module;
        unsigned int sequencer_number = 0;

		Menu *createChildMenu() override {
			Menu *menu = new Menu;

            OutputRangeItem *output_range_item = createMenuItem<OutputRangeItem>("Output Range", RIGHT_ARROW);
            output_range_item->sequencer_number = this->sequencer_number;
			output_range_item->module = module;
			menu->addChild(output_range_item);

            InputSnapItem *input_snap_item = createMenuItem<InputSnapItem>("Snap", RIGHT_ARROW);
            input_snap_item->sequencer_number = this->sequencer_number;
			input_snap_item->module = module;
			menu->addChild(input_snap_item);

            SampleAndHoldItem *sample_and_hold_item = createMenuItem<SampleAndHoldItem>("Sample & Hold", CHECKMARK(module->voltage_sequencers[sequencer_number].sample_and_hold));
            sample_and_hold_item->sequencer_number = this->sequencer_number;
			sample_and_hold_item->module = module;
			menu->addChild(sample_and_hold_item);

			return menu;
		}
	};

    struct AllSequencersItem : MenuItem {

    };

	void appendContextMenu(Menu *menu) override
	{
        DigitalSequencer *module = dynamic_cast<DigitalSequencer*>(this->module);
		assert(module);

        // Menu in development
		menu->addChild(new MenuEntry); // For spacing only
        menu->addChild(createMenuLabel("Sequencer Settings"));

        // Add "all" sequencer settings
        /*
        AllSequencersItem *all_sequencer_items;
        all_sequencer_items = createMenuItem<AllSequencersItem>("All Sequencers", RIGHT_ARROW);
        menu->addChild(all_sequencer_items);
        */

        // Add individual sequencer settings
        SequencerItem *sequencer_items[6];

        for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            sequencer_items[i] = createMenuItem<SequencerItem>("Sequencer #" + std::to_string(i + 1), RIGHT_ARROW);
    		sequencer_items[i]->module = module;
            sequencer_items[i]->sequencer_number = i;
    		menu->addChild(sequencer_items[i]);
        }
	}

    void step() override {
        ModuleWidget::step();
    }

};

Model* modelDigitalSequencer = createModel<DigitalSequencer, DigitalSequencerWidget>("digitalsequencer");
