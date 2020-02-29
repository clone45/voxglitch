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

#include "plugin.hpp"
#include "osdialog.h"
#include <fstream>
#include <array>

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
#define GATES_DRAW_AREA_HEIGHT 16.0
#define GATES_DRAW_AREA_POSITION_X 9
#define GATES_DRAW_AREA_POSITION_Y 86
#define GATE_BAR_HEIGHT 16.0

#define TOOLTIP_WIDTH 33.0
#define TOOLTIP_HEIGHT 20.0

struct VoltageSequencer
{
    unsigned int sequence_length = 16;
    std::array<float, MAX_SEQUENCER_STEPS> sequence;
    unsigned int sequence_playback_position = 0;
    unsigned int clock_division = 1;
    unsigned int clock_division_counter = 0;

    // constructor
    VoltageSequencer()
    {
        sequence.fill(0.0);
    }

    void step()
    {
        clock_division_counter++;
        if(clock_division_counter >= clock_division)
        {
            clock_division_counter = 0;
            sequence_playback_position = (sequence_playback_position + 1) % sequence_length;
        }
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
        return(sequence[getPlaybackPosition()]);
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

    void setClockDivision(unsigned int clock_division)
    {
        this->clock_division = clock_division;
    }

    unsigned int getClockDivision()
    {
        return(this->clock_division);
    }
};

struct GateSequencer
{
    unsigned int sequence_length = 16;
    std::array<bool, MAX_SEQUENCER_STEPS> sequence;
    unsigned int sequence_playback_position = 0;
    unsigned int clock_division = 1;
    unsigned int clock_division_counter = 0;

    // constructor
    GateSequencer()
    {
        sequence.fill(0.0);
    }

    void step()
    {
        clock_division_counter++;
        if(clock_division_counter >= clock_division)
        {
            clock_division_counter = 0;
            sequence_playback_position = (sequence_playback_position + 1) % sequence_length;
        }
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

    void setClockDivision(unsigned int clock_division)
    {
        this->clock_division = clock_division;
    }

    unsigned int getClockDivision()
    {
        return(clock_division);
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
        SEQUENCE_SELECTION_KNOB,
        SEQUENCE_LENGTH_KNOB,
        SEQUENCE_CLOCK_DIVISION_KNOB,
        SEQUENCE_START_KNOB,
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
        configParam(SEQUENCE_CLOCK_DIVISION_KNOB, 1, 16, 1, "SequenceClockDivisionKnob");
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

        //
        // Save sequencer clock division settings
        //
        json_t *sequencer_clock_division_json_array = json_array();
        for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            json_array_append_new(sequencer_clock_division_json_array, json_integer(this->voltage_sequencers[sequencer_number].getClockDivision()));
        }
        json_object_set(json_root, "clock_divisions", sequencer_clock_division_json_array);
        json_decref(sequencer_clock_division_json_array);

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

        //
        // Load clock divisions
        //
        json_t *clock_division_json_array = json_object_get(json_root, "clock_divisions");

        if(clock_division_json_array)
        {
            size_t sequencer_number;
            json_t *division_json;

            json_array_foreach(clock_division_json_array, sequencer_number, division_json)
            {
                this->voltage_sequencers[sequencer_number].setClockDivision(json_integer_value(division_json));
                this->gate_sequencers[sequencer_number].setClockDivision(json_integer_value(division_json));
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

        selected_sequencer_index = params[SEQUENCE_SELECTION_KNOB].getValue();
        selected_sequencer_index = clamp(selected_sequencer_index, 0, NUMBER_OF_SEQUENCERS - 1);

        // Store the selected sequencers for convenience
        if(previously_selected_sequencer_index != selected_sequencer_index)
        {
            selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];
            selected_gate_sequencer = &gate_sequencers[selected_sequencer_index];

            params[SEQUENCE_LENGTH_KNOB].setValue(selected_voltage_sequencer->getLength());
            params[SEQUENCE_CLOCK_DIVISION_KNOB].setValue(selected_voltage_sequencer->getClockDivision());

            previously_selected_sequencer_index = selected_sequencer_index;
        }
        else
        {
            // Set the selected sequencer's length
            unsigned int sequence_length_knob_value = params[SEQUENCE_LENGTH_KNOB].getValue();
            sequence_length_knob_value = clamp(sequence_length_knob_value, 1, 32);
            selected_voltage_sequencer->setLength(sequence_length_knob_value);
            selected_gate_sequencer->setLength(sequence_length_knob_value);

            // Set the selected sequencer's clock division
            unsigned int sequence_clock_division_knob_value = params[SEQUENCE_CLOCK_DIVISION_KNOB].getValue();
            sequence_clock_division_knob_value = clamp(sequence_clock_division_knob_value, 1, 16);
            selected_voltage_sequencer->setClockDivision(sequence_clock_division_knob_value);
            selected_gate_sequencer->setClockDivision(sequence_clock_division_knob_value);
        }

        // Reset ALL of the sequencers
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
                voltage_sequencers[i].reset();
                gate_sequencers[i].reset();
            }
        }
        else if(clock_ignore_on_reset == 0)
        {
            // Step ALL of the sequencers
            if(stepTrigger.process(rescale(inputs[STEP_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
            {
                for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
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
            outputs[voltage_outputs[i]].setVoltage((voltage_sequencers[i].getValue() / 214.0) * 10.0);
        }

        // process trigger outputs
        for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
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

struct DigitalSequencerDisplay : TransparentWidget
{
    DigitalSequencer *module;
	Vec drag_position;
    float bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;

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

    bool keypressRight(const event::HoverKey &e)
    {
        if (e.key == GLFW_KEY_RIGHT)
        {
            e.consume(this);
            if(e.action == GLFW_PRESS) return(true);
        }
        return(false);
    }

    bool keypressLeft(const event::HoverKey &e)
    {
        if (e.key == GLFW_KEY_LEFT)
        {
            e.consume(this);
            if(e.action == GLFW_PRESS) return(true);
        }
        return(false);
    }

    void drawVerticalGuildes(NVGcontext *vg, float height)
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

    void drawBlueOverlay(NVGcontext *vg, float width, float height)
    {
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, width, height);
        nvgFillColor(vg, nvgRGBA(0, 100, 255, 28));
        nvgFill(vg);
    }

    void drawBar(NVGcontext *vg, float position, float height, float container_height, NVGcolor color)
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
    float draw_tooltip_index = -1.0;
    float draw_tooltip_y = -1.0;
    float tooltip_value = 0.0;

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

        // Save the drawing context to restore later
		nvgSave(vg);

		if(module)
		{
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

		}
        else // Draw a demo sequence so that the sequencer looks nice in the library selector
        {
            float demo_sequence[32] = {100.0,100.0,93.0,80.0,67.0,55.0,47.0,44.0,43.0,44.0,50.0,69.0,117.0,137.0,166,170,170,164,148,120,105,77,65,41,28,23,22,22,28,48,69,94};

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

        if(draw_tooltip)
        {
            drawTooltip(vg);
            draw_tooltip = false;
        }

		nvgRestore(vg);
	}

    void drawTooltip(NVGcontext *vg)
    {
        nvgSave(vg);

        float x_offset = 3.0;
        float y = std::max(60.0f, draw_tooltip_y);
        float x = ((draw_tooltip_index * bar_width) + (draw_tooltip_index * BAR_HORIZONTAL_PADDING)) + bar_width + x_offset;

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
        float bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;
		int clicked_bar_x_index = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
		int clicked_y = DRAW_AREA_HEIGHT - mouse_position.y;

		clicked_bar_x_index = clamp(clicked_bar_x_index, 0, MAX_SEQUENCER_STEPS - 1);
        clicked_y = clamp(clicked_y, 0, DRAW_AREA_HEIGHT);

        module->selected_voltage_sequencer->setValue(clicked_bar_x_index, clicked_y);

        // Tooltip drawing is done in the draw method
        draw_tooltip = true;
        draw_tooltip_index = clicked_bar_x_index;
        draw_tooltip_y = clicked_y;
        tooltip_value = floorf((clicked_y / 214.0) * 1000) / 100;
	}

    void onButton(const event::Button &e) override
    {
        e.consume(this);

		if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
		{
			drag_position = e.pos;
			this->editBar(e.pos);
		}
	}

	void onDragMove(const event::DragMove &e) override
    {
		TransparentWidget::onDragMove(e);
		drag_position = drag_position.plus(e.mouseDelta);
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
    }
};

struct DigitalSequencerGatesDisplay : DigitalSequencerDisplay
{
    bool mouse_lock = false;

	DigitalSequencerGatesDisplay()
	{
		box.size = Vec(GATES_DRAW_AREA_WIDTH, GATES_DRAW_AREA_HEIGHT);
	}

	void draw(const DrawArgs &args) override
	{
		const auto vg = args.vg;
        int value;
        float value_height;
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

                if(i == module->selected_gate_sequencer->getPlaybackPosition())
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
				if(i == module->selected_gate_sequencer->getPlaybackPosition())
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
    			this->editBar(e.pos);
            }
		}
        else if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
		{
            this->mouse_lock = false;
		}
		// DEBUG("%s %d,%d", "button press at: ", clicked_bar_x_index, clicked_bar_y_index);
	}

	void editBar(Vec mouse_position)
	{
        float bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;
		int clicked_bar_x_index = clamp(mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING), 0, MAX_SEQUENCER_STEPS - 1);

        module->selected_gate_sequencer->setValue(clicked_bar_x_index, ! module->selected_gate_sequencer->getValue(clicked_bar_x_index));
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
    }
};

struct DigitalSequencerCompactInputDisplay : TransparentWidget
{
    DigitalSequencer *module;
    std::shared_ptr<Font> font;

    bool moused_over = false;

    // These shouldn't ever need to change
    float text_position_x = mm2px(6.4);  // position relative to widget position
    float text_position_y = mm2px(7.6); // position relative to widget position
    float box_size_width = 13;
    float box_size_height = 20;

    DigitalSequencerCompactInputDisplay()
    {
        box.size = mm2px(Vec(box_size_width, box_size_height));
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
    }

    void setFontStyles(NVGcontext *vg)
    {
        nvgFontSize(vg, 13);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, nvgRGBA(255, 255, 255, 0xff));
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgTextLetterSpacing(vg, -1);
    }

    std::string getLabel()
    {
        return("");
    }

    std::string getValue()
    {
        return("0");
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

struct DigitalSequencerSeqDisplay : DigitalSequencerCompactInputDisplay
{
    void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		// Configure the font size, face, color, etc.
        setFontStyles(args.vg);

        // Draw the label or the value
        std::string display_string = (module && moused_over) ? std::to_string(module->selected_sequencer_index + 1) : "SEQ";
        nvgText(args.vg, text_position_x, text_position_y, display_string.c_str(), NULL);

		nvgRestore(args.vg);
	}
};

struct DigitalSequencerLenDisplay : DigitalSequencerCompactInputDisplay
{
	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		// Configure the font size, face, color, etc.
        setFontStyles(args.vg);

        // Draw the text
        std::string display_string = (module && moused_over) ? std::to_string(module->selected_voltage_sequencer->getLength()) : "LEN";
        nvgText(args.vg, text_position_x, text_position_y, display_string.c_str(), NULL);

		nvgRestore(args.vg);
	}
};

struct DigitalSequencerDivDisplay : DigitalSequencerCompactInputDisplay
{
	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		// Configure the font size, face, color, etc.
        setFontStyles(args.vg);

        // Render the text
        std::string display_string = (module && moused_over) ? std::to_string(module->selected_voltage_sequencer->getClockDivision()) : "DIV";
        nvgText(args.vg, text_position_x, text_position_y, display_string.c_str(), NULL);

		nvgRestore(args.vg);
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
        addParam(createParamCentered<Trimpot>(mm2px(Vec(72.152, 114.893 + 1)), module, DigitalSequencer::SEQUENCE_CLOCK_DIVISION_KNOB));
        // next is 84.152

        DigitalSequencerSeqDisplay *seq_display = new DigitalSequencerSeqDisplay();
        seq_display->box.pos = mm2px(Vec(37.440, 101));
		seq_display->module = module;
		addChild(seq_display);

        DigitalSequencerLenDisplay *len_display = new DigitalSequencerLenDisplay();
        len_display->box.pos = mm2px(Vec(53.844, 101));
		len_display->module = module;
		addChild(len_display);

        DigitalSequencerDivDisplay *div_display = new DigitalSequencerDivDisplay();
        div_display->box.pos = mm2px(Vec(65.844, 101)); // 77.844
		div_display->module = module;
		addChild(div_display);

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
