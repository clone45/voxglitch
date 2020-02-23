//
// Voxglitch "DigitalSequencer" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include <fstream>

#define GAIN 5.0
#define DRAW_AREA_WIDTH 348.0
#define DRAW_AREA_HEIGHT 242.0
#define BAR_WIDTH 21.0
#define BAR_HORIZONTAL_PADDING .8
#define NUMBER_OF_SEQUENCES 16

struct DigitalSequencer : Module
{
	dsp::SchmittTrigger clockTrigger;
	dsp::PulseGenerator clockOutputPulse;
	dsp::PulseGenerator endOutputPulse;

    int sequences[NUMBER_OF_SEQUENCES][16];
    unsigned int sequence_playback_position = 0;
    int selected_sequence = 0;
    bool pattern_lock = false;

	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		END_OUTPUT,
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

        for(int sequence=0; sequence<NUMBER_OF_SEQUENCES; sequence++)
        {
            for(int i=0; i<16; i++)
            {
                sequences[sequence][i] = 0;
            }
        }

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	// Autosave settings
	json_t *dataToJson() override
	{
        json_t *json_root = json_object();

		//
		// Save patterns
		//

		json_t *sequences_json_array = json_array();

		for(int pattern_number=0; pattern_number<NUMBER_OF_SEQUENCES; pattern_number++)
		{
			json_t *pattern_json_array = json_array();

			for(int i=0; i<16; i++)
			{
				json_array_append_new(pattern_json_array, json_integer(this->sequences[pattern_number][i]));
			}

            json_array_append_new(sequences_json_array, pattern_json_array);
        }

        json_object_set(json_root, "patterns", sequences_json_array);
        json_decref(sequences_json_array);

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
				for(int i=0; i<NUMBER_OF_SEQUENCES; i++)
				{
					this->sequences[pattern_number][i] = json_integer_value(json_array_get(json_pattern_array, i));
				}
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

	void process(const ProcessArgs &args) override
	{

	}
};


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
			float bar_height;
			NVGcolor highlighted_bar_color;

			for(unsigned int i=0; i<16; i++)
			{
				value = module->sequences[module->selected_sequence][i];
				// if(item_display == "-1") item_display = ".";

				if(i == module->sequence_playback_position)
				{
					highlighted_bar_color = nvgRGBA(255, 255, 255, 250);
				}
				else
				{
					highlighted_bar_color = nvgRGBA(255, 255, 255, 150);
				}

				bar_height = (DRAW_AREA_HEIGHT * ((value + 1) / 16.0));

				// Draw the break offset bar
				if(bar_height > 0)
				{
					nvgBeginPath(vg);
					// nvgRect(vg, (i * bar_width) + (i * bar_horizontal_padding), DRAW_AREA_HEIGHT, bar_width, -1 * DRAW_AREA_HEIGHT * ((item_display + 1) / 16.0));
					nvgRect(vg, (i * BAR_WIDTH) + (i * BAR_HORIZONTAL_PADDING), DRAW_AREA_HEIGHT - bar_height, BAR_WIDTH, bar_height);
					nvgFillColor(vg, highlighted_bar_color);
					nvgFill(vg);
				}

				if(i == module->sequence_playback_position)
				{
					// Highlight entire column
					nvgBeginPath(vg);
					nvgRect(vg, (i * BAR_WIDTH) + (i * BAR_HORIZONTAL_PADDING), 0, BAR_WIDTH, DRAW_AREA_HEIGHT);
					nvgFillColor(vg, nvgRGBA(255, 255, 255, 20));
					nvgFill(vg);
				}
			}

			// Note to self: This is a nice orange for an overlay
			// and might be interesting to give an option to activate
			/*
			nvgBeginPath(vg);
			nvgRect(vg, 0,0,348,241);
			nvgFillColor(vg, nvgRGBA(255, 100, 0, 128));
			nvgFill(vg);
			*/
		}

		nvgRestore(vg);
	}

	Vec clampToDrawArea(Vec location)
    {
        float x = clamp(location.x, 0.0f, DRAW_AREA_WIDTH);
        float y = clamp(location.y, 0.0f, DRAW_AREA_HEIGHT);
        return(Vec(x,y));
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
		int clicked_bar_x_index = mouse_position.x / (BAR_WIDTH + BAR_HORIZONTAL_PADDING);
		int clicked_bar_y_index = 15 - (int) ((mouse_position.y / DRAW_AREA_HEIGHT) * 16.0);

		clicked_bar_x_index = clamp(clicked_bar_x_index, 0, 15);
		clicked_bar_y_index = clamp(clicked_bar_y_index, 0, 15);

		// DEBUG("%s %f,%f", "height vs ", DRAW_AREA_HEIGHT, drag_position.y);

		// If the mouse position is below the sequencer, then set the corresponding
		// row of the sequencer to "0" (meaning, don't jump to a new position in the beat)
        /*
		if(mouse_position.y > (DRAW_AREA_HEIGHT - 2))
		{
			// Special case: Set the break pattern to -1 for "don't jump to new position"
			module->sequences[module->selected_sequence][clicked_bar_x_index] = -1;
		}
		else
		{
			// Set the sequence value height
			module->sequences[module->selected_sequence][clicked_bar_x_index] = clicked_bar_y_index;
		}
        */

        module->sequences[module->selected_sequence][clicked_bar_x_index] = clicked_bar_y_index;
	}

	void onEnter(const event::Enter &e) override
    {
		TransparentWidget::onEnter(e);
		this->module->pattern_lock = true;
		// DEBUG("On enter called");
	}

	void onLeave(const event::Leave &e) override
    {
		TransparentWidget::onLeave(e);
		this->module->pattern_lock = false;
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

		// Sequence Select
        /*
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(13.848, 38)), module, DigitalSequencer::SEQUENCE_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(13.848, 52)), module, DigitalSequencer::SEQUENCE_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.848, 63.5)), module, DigitalSequencer::SEQUENCE_INPUT));
        */

		DigitalSequencerPatternDisplay *pattern_display = new DigitalSequencerPatternDisplay();
		pattern_display->box.pos = mm2px(Vec(55.141, 35.689));
		pattern_display->module = module;
		addChild(pattern_display);

        /*
		// BPM selection
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 12.2)), module, DigitalSequencer::BPM_KNOB));

		// Reset
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.541, 88.685)), module, DigitalSequencer::RESET_INPUT));

		// Outputs
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 100.893)), module, DigitalSequencer::DEBUG_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(13.848, 114.893)), module, DigitalSequencer::CLOCK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(13.848, 88.685)), module, DigitalSequencer::END_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.541, 114.893)), module, DigitalSequencer::WAV_OUTPUT));
        */

	}

	void appendContextMenu(Menu *menu) override
	{
	}
};

Model* modelDigitalSequencer = createModel<DigitalSequencer, DigitalSequencerWidget>("digitalsequencer");
