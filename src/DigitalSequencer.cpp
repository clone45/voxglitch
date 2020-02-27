//
// Voxglitch "DigitalSequencer" module for VCV Rack
//
// TODO: Consider per-sequence clock division

#include "plugin.hpp"
#include "osdialog.h"
#include <fstream>

#define GAIN 5.0

#define NUMBER_OF_SEQUENCERS 6
#define MAX_SEQUENCER_STEPS 32

// Constants for patterns
#define DRAW_AREA_WIDTH 486.0
#define DRAW_AREA_HEIGHT 214.0
#define BAR_HEIGHT 214.0
#define BAR_HORIZONTAL_PADDING .8
#define DRAW_AREA_POSITION_X 9
#define DRAW_AREA_POSITION_Y 9.5

// Constants for gate sequencer
#define GATES_DRAW_AREA_WIDTH 486.0
#define GATES_DRAW_AREA_HEIGHT 16
#define GATES_DRAW_AREA_POSITION_X 9
#define GATES_DRAW_AREA_POSITION_Y 86
#define GATE_BAR_HEIGHT 16


struct VoltageSequencer
{
    unsigned int sequence_length = 16;
    std::array<float, MAX_SEQUENCER_STEPS> sequence;
    unsigned int sequence_playback_position = 0;

    // constructor
    VoltageSequencer()
    {
        sequence.fill(0.0);
    }

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

    float getValue(int index)
    {
        return(sequence[index]);
    }

    float getValue()
    {
        return(sequence[sequence_playback_position]);
    }

    void setValue(int index, float value)
    {
        sequence[index] = value;
    }

    unsigned int getLength()
    {
        return(sequence_length);
    }

    void setLength(unsigned int sequence_length)
    {
        this->sequence_length = sequence_length;
    }

    void shiftLeft()
    {
        float temp = sequence[0];
        for(unsigned int i=0; i < this->sequence_length-1; i++)
        {
            sequence[i] = sequence[i+1];
        }
        sequence[this->sequence_length-1] = temp;
    }

    void shiftRight()
    {
        float temp = sequence[this->sequence_length - 1];

        for(unsigned int i=this->sequence_length-1; i>0; i--)
        {
            sequence[i] = sequence[i-1];
        }

        sequence[0] = temp;
    }
};

struct GateSequencer
{
    unsigned int sequence_length = 16;
    std::array<bool, MAX_SEQUENCER_STEPS> sequence;
    unsigned int sequence_playback_position = 0;

    // constructor
    GateSequencer()
    {
        sequence.fill(0.0);
        // std::fill(begin(sequence), end(sequence), 0.0);
    }

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

    unsigned int getLength()
    {
        return(sequence_length);
    }

    void setLength(unsigned int sequence_length)
    {
        this->sequence_length = sequence_length;
    }

    void shiftLeft()
    {
        bool temp = sequence[0];
        for(unsigned int i=0; i < this->sequence_length-1; i++)
        {
            sequence[i] = sequence[i+1];
        }
        sequence[this->sequence_length-1] = temp;
    }

    void shiftRight()
    {
        bool temp = sequence[this->sequence_length - 1];

        for(unsigned int i=this->sequence_length-1; i>0; i--)
        {
            sequence[i] = sequence[i-1];
        }

        sequence[0] = temp;
    }
};

struct DigitalSequencer : Module
{
	dsp::SchmittTrigger stepTrigger;
    dsp::SchmittTrigger resetTrigger;
	dsp::PulseGenerator clockOutputPulse;
	dsp::PulseGenerator endOutputPulse;
    long clock_ignore_on_reset = 0;

    VoltageSequencer voltage_sequencers[NUMBER_OF_SEQUENCERS];
    VoltageSequencer *selected_voltage_sequencer;

    GateSequencer gate_sequencers[NUMBER_OF_SEQUENCERS];
    GateSequencer *selected_gate_sequencer;

    int selected_sequencer_index = 0;
    int previously_selected_sequencer_index = -1;
    int voltage_outputs[NUMBER_OF_SEQUENCERS];
    int gate_outputs[NUMBER_OF_SEQUENCERS];

    dsp::PulseGenerator gateOutputPulseGenerators[NUMBER_OF_SEQUENCERS];

	enum ParamIds {
        SEQUENCE_LENGTH_KNOB,
        SEQUENCE_SELECTION_KNOB,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
        STEP_INPUT,
        RESET_INPUT,
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
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	DigitalSequencer()
	{
        voltage_outputs[0] = SEQ1_CV_OUTPUT;
        voltage_outputs[1] = SEQ2_CV_OUTPUT,
        voltage_outputs[2] = SEQ3_CV_OUTPUT,
        voltage_outputs[3] = SEQ4_CV_OUTPUT,
        voltage_outputs[4] = SEQ5_CV_OUTPUT,
        voltage_outputs[5] = SEQ6_CV_OUTPUT,

        gate_outputs[0] = SEQ1_GATE_OUTPUT;
        gate_outputs[1] = SEQ2_GATE_OUTPUT,
        gate_outputs[2] = SEQ3_GATE_OUTPUT,
        gate_outputs[3] = SEQ4_GATE_OUTPUT,
        gate_outputs[4] = SEQ5_GATE_OUTPUT,
        gate_outputs[5] = SEQ6_GATE_OUTPUT,

        selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(SEQUENCE_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "SequenceLengthKnob");
        configParam(SEQUENCE_SELECTION_KNOB, 0, NUMBER_OF_SEQUENCERS - 1, 0, "SequenceSelectionKnob");
	}

    /*
                                 ___                 _
                                / / |               | |
          ___  __ ___   _____  / /| | ___   __ _  __| |
        / __|/ _` \ \ / / _ \ / / | |/ _ \ / _` |/ _` |
        \__ \ (_| |\ V /  __// /  | | (_) | (_| | (_| |
        |___/\__,_| \_/ \___/_/   |_|\___/ \__,_|\__,_|

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
					// this->sequences[pattern_number][i] = json_integer_value(json_array_get(json_pattern_array, i));
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

	}

    /*
	float calculate_inputs(int input_index, int knob_index, int attenuator_index, float scale)
	{
		float input_value = inputs[input_index].getVoltage() / 10.0;
		float knob_value = params[knob_index].getValue();
		float attenuator_value = params[attenuator_index].getValue();

		return(((input_value * scale) * attenuator_value) + (knob_value * scale));
	}
    */

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

        selected_sequencer_index = params[SEQUENCE_SELECTION_KNOB].getValue();

        // Store the selected sequencers for convenience
        if(previously_selected_sequencer_index != selected_sequencer_index)
        {
            selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];
            selected_gate_sequencer = &gate_sequencers[selected_sequencer_index];

            params[SEQUENCE_LENGTH_KNOB].setValue(selected_voltage_sequencer->getLength());

            previously_selected_sequencer_index = selected_sequencer_index;
        }
        else
        {
            // Set the selected sequencers lengths
            selected_voltage_sequencer->setLength(params[SEQUENCE_LENGTH_KNOB].getValue());
            selected_gate_sequencer->setLength(params[SEQUENCE_LENGTH_KNOB].getValue());
        }

        // Reset ALL of the sequencers
        if(resetTrigger.process(inputs[RESET_INPUT].getVoltage()))
        {
            // Set up a (reverse) counter so that the clock input will ignore
            // incoming clock pulses for 1 millisecond after a reset input. This
            // is to comply with VCV Rack's standards.  See section "Timing" at
            // https://vcvrack.com/manual/VoltageStandards

            clock_ignore_on_reset = (long) (.001 * args.sampleRate);

            for(unsigned int i=0; i < (NUMBER_OF_SEQUENCERS - 1); i++)
            {
                voltage_sequencers[i].reset();
                gate_sequencers[i].reset();
            }
        }
        else if(clock_ignore_on_reset == 0)
        {
            // Step ALL of the sequencers
            if(stepTrigger.process(inputs[STEP_INPUT].getVoltage()))
            {
                for(unsigned int i=0; i < (NUMBER_OF_SEQUENCERS - 1); i++)
                {
                    voltage_sequencers[i].step();
                    gate_sequencers[i].step();

                    if(gate_sequencers[i].getValue())
                    {
                        gateOutputPulseGenerators[i].trigger(0.01f);
                    }
                }
            }
        }

        // output values
        for(unsigned int i=0; i<(NUMBER_OF_SEQUENCERS-1); i++)
        {
            outputs[voltage_outputs[i]].setVoltage((voltage_sequencers[i].getValue() / 214.0) * 10.0);
        }

        // process trigger outputs
        for(unsigned int i=0; i<(NUMBER_OF_SEQUENCERS-1); i++)
        {
            trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
		    outputs[gate_outputs[i]].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
        }

        if (clock_ignore_on_reset > 0) clock_ignore_on_reset--;
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

struct DigitalSequencerPatternDisplay : TransparentWidget
{
	DigitalSequencer *module;
	Vec drag_position;

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

		nvgSave(vg);

		if(module)
		{
			//
			// Display the pattern
			//

			int value;
			float value_height;
			NVGcolor bar_color;

            float bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;

			for(unsigned int i=0; i < MAX_SEQUENCER_STEPS; i++)
			{
				value = module->selected_voltage_sequencer->getValue(i);

                // Draw grey background bar
                bar_color = nvgRGBA(60, 60, 64, 255);
                nvgBeginPath(vg);
                nvgRect(vg, (i * bar_width) + (i * BAR_HORIZONTAL_PADDING), DRAW_AREA_HEIGHT - BAR_HEIGHT, bar_width, BAR_HEIGHT);
                nvgFillColor(vg, bar_color);
                nvgFill(vg);

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

				// value_height = (DRAW_AREA_HEIGHT * ((value + 1) / 16.0));

                value_height = value;

				if(value_height > 0)
				{
					nvgBeginPath(vg);
					nvgRect(vg, (i * bar_width) + (i * BAR_HORIZONTAL_PADDING), DRAW_AREA_HEIGHT - value_height, bar_width, value_height);
					nvgFillColor(vg, bar_color);
					nvgFill(vg);
				}

				if(i == module->selected_voltage_sequencer->getPlaybackPosition())
				{
					// Highlight entire column
					nvgBeginPath(vg);
					nvgRect(vg, (i * bar_width) + (i * BAR_HORIZONTAL_PADDING), 0, bar_width, DRAW_AREA_HEIGHT);
					nvgFillColor(vg, nvgRGBA(255, 255, 255, 20));
					nvgFill(vg);
				}
			}

            //
            // Draw vertical guides every 4 bars
            //

            for(unsigned int i=1; i < 8; i++)
            {
                nvgBeginPath(vg);
                int x = (i * 4 * bar_width) + (i * 4 * BAR_HORIZONTAL_PADDING);
                nvgRect(vg, x, 0, 1, DRAW_AREA_HEIGHT);
                nvgFillColor(vg, nvgRGBA(240, 240, 255, 40));
                nvgFill(vg);
            }

            // Draw blue overlay
			nvgBeginPath(vg);
			nvgRect(vg, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
			nvgFillColor(vg, nvgRGBA(0, 100, 255, 28));
			nvgFill(vg);

		}

		nvgRestore(vg);
	}

	void onButton(const event::Button &e) override
    {
        e.consume(this);

		if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
		{
			drag_position = e.pos;
			this->editBar(e.pos);
		}
		// DEBUG("%s %d,%d", "button press at: ", clicked_bar_x_index, clicked_bar_y_index);
	}

	void onDragStart(const event::DragStart &e) override
    {
		TransparentWidget::onDragStart(e);
	}

	void onDragEnd(const event::DragEnd &e) override
    {
		TransparentWidget::onDragEnd(e);
	}

	void onDragMove(const event::DragMove &e) override
    {
		TransparentWidget::onDragMove(e);
		drag_position = drag_position.plus(e.mouseDelta);
		editBar(drag_position);
	}

	void step() override {
		TransparentWidget::step();
	}

	void editBar(Vec mouse_position)
	{
        // VoltageSequencer *selected_voltage_sequencer = &module->voltage_sequencers[module->selected_sequencer_index];

        float bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;
		int clicked_bar_x_index = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
		int clicked_y = DRAW_AREA_HEIGHT - mouse_position.y;

		clicked_bar_x_index = clamp(clicked_bar_x_index, 0, MAX_SEQUENCER_STEPS - 1);
        clicked_y = clamp(clicked_y, 0, DRAW_AREA_HEIGHT);

        module->selected_voltage_sequencer->setValue(clicked_bar_x_index, clicked_y);
	}

	void onEnter(const event::Enter &e) override
    {
		TransparentWidget::onEnter(e);
	}

	void onLeave(const event::Leave &e) override
    {
		TransparentWidget::onLeave(e);
	}

    void onHoverKey(const event::HoverKey &e) override
    {
        if (e.key == GLFW_KEY_RIGHT)
        {
        	e.consume(this);

        	if(e.action == GLFW_PRESS)
        	{
        		module->selected_voltage_sequencer->shiftRight();
        	}
        }
        if (e.key == GLFW_KEY_LEFT)
        {
        	e.consume(this);

        	if(e.action == GLFW_PRESS)
        	{
        		module->selected_voltage_sequencer->shiftLeft();
        	}
        }
    }
};

struct DigitalSequencerGatesDisplay : TransparentWidget
{
	DigitalSequencer *module;
	Vec drag_position;
    bool mouse_lock = false;

	DigitalSequencerGatesDisplay()
	{
		box.size = Vec(GATES_DRAW_AREA_WIDTH, GATES_DRAW_AREA_HEIGHT);
	}

	void draw(const DrawArgs &args) override
	{
		const auto vg = args.vg;

		nvgSave(vg);

		if(module)
		{
			int value;
			float value_height;
			NVGcolor bar_color;

            float bar_width = (GATES_DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;

			for(unsigned int i=0; i < MAX_SEQUENCER_STEPS; i++)
			{
				value = module->selected_gate_sequencer->getValue(i);

                // Draw grey background bar
                bar_color = nvgRGBA(60, 60, 64, 255);
                nvgBeginPath(vg);
                nvgRect(vg, (i * bar_width) + (i * BAR_HORIZONTAL_PADDING), GATES_DRAW_AREA_HEIGHT - GATE_BAR_HEIGHT, bar_width, GATE_BAR_HEIGHT);
                nvgFillColor(vg, bar_color);
                nvgFill(vg);

                if(i == module->selected_gate_sequencer->getPlaybackPosition())
				{
					bar_color = nvgRGBA(255, 255, 255, 250);
				}
				else if(i < module->selected_gate_sequencer->getLength())
				{
					bar_color = nvgRGBA(255, 255, 255, 150);
				}
                else
                {
                    bar_color = nvgRGBA(255, 255, 255, 15);
                }

				value_height = (GATE_BAR_HEIGHT * value);

				if(value_height > 0)
				{
					nvgBeginPath(vg);
					nvgRect(vg, (i * bar_width) + (i * BAR_HORIZONTAL_PADDING), GATES_DRAW_AREA_HEIGHT - value_height, bar_width, value_height);
					nvgFillColor(vg, bar_color);
					nvgFill(vg);
				}

                // highlight active column
				if(i == module->selected_gate_sequencer->getPlaybackPosition())
				{
					nvgBeginPath(vg);
					nvgRect(vg, (i * bar_width) + (i * BAR_HORIZONTAL_PADDING), 0, bar_width, GATE_BAR_HEIGHT);
					nvgFillColor(vg, nvgRGBA(255, 255, 255, 20));
					nvgFill(vg);
				}
			}

            // Cool blue hue
            nvgBeginPath(vg);
			nvgRect(vg, 0, 0, GATES_DRAW_AREA_WIDTH, GATES_DRAW_AREA_HEIGHT);
			nvgFillColor(vg, nvgRGBA(0, 100, 255, 28));
			nvgFill(vg);
		}

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
    			this->editBar(e.pos);
            }
		}
        else if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
		{
            this->mouse_lock = false;
		}
		// DEBUG("%s %d,%d", "button press at: ", clicked_bar_x_index, clicked_bar_y_index);
	}

	void step() override {
		TransparentWidget::step();
	}

	void editBar(Vec mouse_position)
	{
        float bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;

		int clicked_bar_x_index = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
		clicked_bar_x_index = clamp(clicked_bar_x_index, 0, MAX_SEQUENCER_STEPS - 1);

        if(module->selected_gate_sequencer->getValue(clicked_bar_x_index))
        {
            module->selected_gate_sequencer->setValue(clicked_bar_x_index, 0);
        }
        else
        {
            module->selected_gate_sequencer->setValue(clicked_bar_x_index, 1);
        }
	}

	void onEnter(const event::Enter &e) override
    {
		TransparentWidget::onEnter(e);
	}

	void onLeave(const event::Leave &e) override
    {
		TransparentWidget::onLeave(e);
	}

    void onHoverKey(const event::HoverKey &e) override
    {
        if (e.key == GLFW_KEY_RIGHT)
        {
        	e.consume(this);

        	if(e.action == GLFW_PRESS)
        	{
        		module->selected_gate_sequencer->shiftRight();
        	}
        }
        if (e.key == GLFW_KEY_LEFT)
        {
        	e.consume(this);

        	if(e.action == GLFW_PRESS)
        	{
        		module->selected_gate_sequencer->shiftLeft();
        	}
        }
    }
};


struct DigitalSequencerLenDisplay : TransparentWidget
{
	DigitalSequencer *module;
	std::shared_ptr<Font> font;

    bool moused_over = false;

    // These shouldn't ever need to change
    float text_position_x = mm2px(6.4);  // position relative to widget position
    float text_position_y = mm2px(7.6); // position relative to widget position
    float box_size_width = 13;
    float box_size_height = 20;

	DigitalSequencerLenDisplay()
	{
        box.size = mm2px(Vec(box_size_width, box_size_height));
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		// Configure the font size, face, color, etc.
		nvgFontSize(args.vg, 13);
		nvgFontFaceId(args.vg, font->handle);
		nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgTextLetterSpacing(args.vg, -1);

		if(module)
		{
            VoltageSequencer *selected_voltage_sequencer = &module->voltage_sequencers[module->selected_sequencer_index];
            std::string len_string;

            if(moused_over)
            {
                len_string = std::to_string(selected_voltage_sequencer->getLength());
            }
            else
            {
                len_string = "LEN";
            }

			nvgText(args.vg, text_position_x, text_position_y, len_string.c_str(), NULL);
		}
		else
		{
			nvgText(args.vg, text_position_x, text_position_y, "LEN", NULL);
		}

        /*
        // For debugging
        nvgBeginPath(args.vg);
        nvgRect(args.vg, 0, 0, mm2px(box_size_width), mm2px(box_size_height));
        nvgFillColor(args.vg, nvgRGBA(0, 100, 255, 128));
        nvgFill(args.vg);
        */


		nvgRestore(args.vg);
	}

    void onHover(const event::Hover& e) override {
		TransparentWidget::onHover(e);
		e.consume(this);
	}

    void step() override {
		TransparentWidget::step();
	}

    void onEnter(const event::Enter &e) override
    {
		TransparentWidget::onEnter(e);
		this->moused_over = true;
	}

	void onLeave(const event::Leave &e) override
    {
		TransparentWidget::onLeave(e);
		this->moused_over = false;
	}
};

struct DigitalSequencerSeqDisplay : TransparentWidget
{
	DigitalSequencer *module;
	std::shared_ptr<Font> font;

    bool moused_over = false;

    // These shouldn't ever need to change
    float text_position_x = mm2px(6.4);  // position relative to widget position
    float text_position_y = mm2px(7.6); // position relative to widget position
    float box_size_width = 13;
    float box_size_height = 20;

	DigitalSequencerSeqDisplay()
	{
        box.size = mm2px(Vec(box_size_width, box_size_height));
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		// Configure the font size, face, color, etc.
		nvgFontSize(args.vg, 13);
		nvgFontFaceId(args.vg, font->handle);
		nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgTextLetterSpacing(args.vg, -1);

		if(module)
		{
            std::string len_string;

            if(moused_over)
            {
                len_string = std::to_string(module->selected_sequencer_index + 1);
            }
            else
            {
                len_string = "SEQ";
            }

			nvgText(args.vg, text_position_x, text_position_y, len_string.c_str(), NULL);
		}
		else
		{
			nvgText(args.vg, text_position_x, text_position_y, "1", NULL);
		}

        /*
        // For debugging
        nvgBeginPath(args.vg);
        nvgRect(args.vg, 0, 0, mm2px(box_size_width), mm2px(box_size_height));
        nvgFillColor(args.vg, nvgRGBA(0, 100, 255, 128));
        nvgFill(args.vg);
        */

		nvgRestore(args.vg);
	}

    void onHover(const event::Hover& e) override {
		TransparentWidget::onHover(e);
		e.consume(this);
	}

    void step() override {
		TransparentWidget::step();
	}

    void onEnter(const event::Enter &e) override
    {
		TransparentWidget::onEnter(e);
		this->moused_over = true;
	}

	void onLeave(const event::Leave &e) override
    {
		TransparentWidget::onLeave(e);
		this->moused_over = false;
	}
};

struct DigitalSequencerWidget : ModuleWidget
{
	DigitalSequencerWidget(DigitalSequencer* module)
	{
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

        addParam(createParamCentered<Trimpot>(mm2px(Vec(43.737, 114.893 + 1)), module, DigitalSequencer::SEQUENCE_SELECTION_KNOB));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(60.152, 114.893 + 1)), module, DigitalSequencer::SEQUENCE_LENGTH_KNOB));

        DigitalSequencerSeqDisplay *seq_display = new DigitalSequencerSeqDisplay();
        seq_display->box.pos = mm2px(Vec(37.440, 100 + 1));
		seq_display->module = module;
		addChild(seq_display);

        DigitalSequencerLenDisplay *len_display = new DigitalSequencerLenDisplay();
        len_display->box.pos = mm2px(Vec(37.440 + 16.404, 100 + 1));
		len_display->module = module;
		addChild(len_display);

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

	void appendContextMenu(Menu *menu) override
	{
	}
};

Model* modelDigitalSequencer = createModel<DigitalSequencer, DigitalSequencerWidget>("digitalsequencer");
