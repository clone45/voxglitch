//
// Voxglitch "Goblins" module for VCV Rack
//


#include "plugin.hpp"
#include "osdialog.h"
#include "sample.hpp"

#define MAX_NUMBER_OF_GOBLINS 128.0f
#define MAX_SPAWN_RATE 12000.0f
#define NUMBER_OF_SAMPLES 5
#define NUMBER_OF_SAMPLES_FLOAT 5.0

struct Goblin
{
	// Start Position is the offset into the sample where playback should start.
	// It is set when the goblin is first created.
	float start_position;

	// sample_ptr points to the loaded sample in memory
	Sample *sample_ptr;

	// playback_position is similar to samplePos used in for samples.  However,
	// it's relative to the Goblin's start_position rather than the sample
	// start position.
	float playback_position = 0.0f;

	unsigned int sample_position = 0;

	float getOutput()
	{
		// Note that we're adding two floating point numbers, then casting
		// them to an int, which is much faster than using floor()
		sample_position = this->start_position + this->playback_position;

		// Wrap if the sample position is past the sample end point
		if (sample_position >= this->sample_ptr->total_sample_count) sample_position = sample_position % this->sample_ptr->total_sample_count;

        // Return the audio
		return(this->sample_ptr->leftPlayBuffer[sample_position]);
	}

	void age(float step_amount, float playback_length)
	{
        // Step the playback position forward.
		playback_position = playback_position + step_amount;

        // If the playback position is past the playback length, then wrap the playback position to the beginning
		if(playback_position >= playback_length) playback_position = playback_position - playback_length;

		// If the playback position is less than 0, it's possible that we're playing the sample backwards,
		// so wrap around the end of the sample.
		if (playback_position < 0) playback_position = playback_length - playback_position;
	}
};


struct Goblins : Module
{
	unsigned int selected_sample_slot = 0;
	float spawn_rate_counter = 0;
	float step_amount = 0;
	int step = 0;

	std::string root_dir;
	std::string path;

	std::vector<Goblin> countryside;
	Sample samples[NUMBER_OF_SAMPLES];
	dsp::SchmittTrigger purge_trigger;
	dsp::SchmittTrigger purge_button_trigger;

	enum ParamIds {
		SAMPLE_PLAYBACK_POSITION_KNOB,
		SAMPLE_PLAYBACK_POSITION_ATTN_KNOB,
		PLAYBACK_LENGTH_KNOB,
		PLAYBACK_LENGTH_ATTN_KNOB,
		SPAWN_RATE_KNOB,
		SPAWN_RATE_ATTN_KNOB,
		COUNTRYSIDE_CAPACITY_KNOB,
		COUNTRYSIDE_CAPACITY_ATTN_KNOB,
		PITCH_KNOB,
		PITCH_ATTN_KNOB,
		SAMPLE_SELECT_KNOB,
		SAMPLE_SELECT_ATTN_KNOB,
		PURGE_BUTTON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PURGE_TRIGGER_INPUT,
		PITCH_INPUT,
		PLAYBACK_LENGTH_INPUT,
		COUNTRYSIDE_CAPACITY_INPUT,
		SPAWN_RATE_INPUT,
		SAMPLE_PLAYBACK_POSITION_INPUT,
		SAMPLE_SELECT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		WAV_OUTPUT,
		DEBUG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PURGE_LIGHT,
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	Goblins()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SAMPLE_PLAYBACK_POSITION_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionKnob");
		configParam(SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionAttnKnob");
		configParam(PLAYBACK_LENGTH_KNOB, 0.0f, 1.0f, 0.5f, "LengthKnob");
		configParam(PLAYBACK_LENGTH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "LengthAttnKnob");
		configParam(SPAWN_RATE_KNOB, 0.00f, 1.0f, 0.2f, "SpawnRateKnob");
		configParam(SPAWN_RATE_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SpawnRateAttnKnob");
		configParam(COUNTRYSIDE_CAPACITY_KNOB, 0.0f, 1.0f, 1.0f, "CountrysideCapacityKnob");
		configParam(COUNTRYSIDE_CAPACITY_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "CountrysideCapacityAttnKnob");
		configParam(PITCH_KNOB, -1.0f, 1.0f, 0.0f, "PitchKnob");
		configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "PitchAttnKnob");
		configParam(SAMPLE_SELECT_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
		configParam(SAMPLE_SELECT_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");
		configParam(PURGE_BUTTON_PARAM, 0.f, 1.f, 0.f, "PurgeButtonParam");
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(rootJ, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(samples[i].path.c_str()));
		}

		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_sample_path = json_object_get(rootJ, ("loaded_sample_path_" +  std::to_string(i+1)).c_str());
			if (loaded_sample_path) samples[i].load(json_string_value(loaded_sample_path));
		}
	}

	float calculate_inputs(int input_index, int knob_index, int attenuator_index, float scale)
	{
		float input_value = inputs[input_index].getVoltage() / 10.0;
		float knob_value = params[knob_index].getValue();
		float attenuator_value = params[attenuator_index].getValue();

		return(((input_value * scale) * attenuator_value) + (knob_value * scale));
	}

	void process(const ProcessArgs &args) override
	{
		//
		//  Set selected sample based on inputs.
		//  This must happen before we calculate start_position

		// unsigned int sample_select_input_value = (unsigned int) (NUMBER_OF_SAMPLES_FLOAT * (((inputs[SAMPLE_SELECT_INPUT].getVoltage() / 10.0) * params[SAMPLE_SELECT_ATTN_KNOB].getValue()) + params[SAMPLE_SELECT_KNOB].getValue()));

		selected_sample_slot = (unsigned int) calculate_inputs(SAMPLE_SELECT_INPUT, SAMPLE_SELECT_KNOB, SAMPLE_SELECT_ATTN_KNOB, NUMBER_OF_SAMPLES_FLOAT);
		selected_sample_slot = clamp(selected_sample_slot, 0, NUMBER_OF_SAMPLES - 1);

		Sample *selected_sample = &samples[selected_sample_slot];

		//
		//  Calculate additional inputs

		float spawn_rate = calculate_inputs(SPAWN_RATE_INPUT, SPAWN_RATE_KNOB, SPAWN_RATE_ATTN_KNOB, MAX_SPAWN_RATE);
		float playback_length = calculate_inputs(PLAYBACK_LENGTH_INPUT, PLAYBACK_LENGTH_KNOB, PLAYBACK_LENGTH_ATTN_KNOB, (args.sampleRate / 8));
		float start_position = calculate_inputs(SAMPLE_PLAYBACK_POSITION_INPUT, SAMPLE_PLAYBACK_POSITION_KNOB, SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, selected_sample->total_sample_count);

		// Ensure that the inputs are within range
		spawn_rate = clamp(spawn_rate, 0.0f, MAX_SPAWN_RATE);

		// If the purge input it trittered, empty the goblins from the country side
		bool purge_is_triggered = purge_trigger.process(inputs[PURGE_TRIGGER_INPUT].getVoltage()) || purge_button_trigger.process(params[PURGE_BUTTON_PARAM].getValue());
		if (purge_is_triggered) countryside.clear();
		lights[PURGE_LIGHT].setSmoothBrightness(purge_is_triggered, args.sampleTime);

		// Spawn new goblins
		if((spawn_rate_counter >= spawn_rate) && (selected_sample->loaded))
		{
			Goblin goblin;
			goblin.start_position = start_position;
			goblin.sample_ptr = selected_sample;
			countryside.push_back(goblin);

			spawn_rate_counter = 0;
		}

		unsigned int countryside_capacity = calculate_inputs(COUNTRYSIDE_CAPACITY_INPUT, COUNTRYSIDE_CAPACITY_KNOB, COUNTRYSIDE_CAPACITY_ATTN_KNOB, MAX_NUMBER_OF_GOBLINS);
		if (countryside_capacity > MAX_NUMBER_OF_GOBLINS) countryside_capacity = MAX_NUMBER_OF_GOBLINS;

		// If there are too many goblins, kill off the oldest until the
		// population is under control.
		while(countryside.size() > countryside_capacity)
		{
            countryside.erase(countryside.begin());
		}

		if ((! selected_sample->loading) && (selected_sample->total_sample_count > 0))
		{
			// Iterate over all the goblins in the countryside and collect
			// their output.

			float mix_output = 0;

			if(countryside.empty() == false)
			{
				// TODO: selected_sample doesn't really have a meaning in this
				// part of the code.  The step_amount calculation will need to
				// be moved into the Goblin's age() code.

				// Increment sample offset (pitch)
				if (inputs[PITCH_INPUT].isConnected())
				{
					step_amount = (selected_sample->sample_rate / args.sampleRate) + (((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 0.5f) * params[PITCH_ATTN_KNOB].getValue()) + params[PITCH_KNOB].getValue();
				}
				else
				{
					step_amount = (selected_sample->sample_rate / args.sampleRate) + params[PITCH_KNOB].getValue();
				}

				for(Goblin& goblin : countryside)
				{
					mix_output += goblin.getOutput();
					goblin.age(step_amount, playback_length);
				}

				outputs[WAV_OUTPUT].setVoltage(mix_output);
			}

			spawn_rate_counter = spawn_rate_counter + 1.0f;
		}
		else
		{
			outputs[WAV_OUTPUT].setVoltage(0);
		}
	}
};

struct GoblinsSampleReadout : TransparentWidget
{
	Goblins *module;

	int frame = 0;
	std::shared_ptr<Font> font;

	GoblinsSampleReadout()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		std::string text_to_display;

		if(! module)
		{
			text_to_display = "load sample";
		}
		else
		{
			std::string file_name = module->samples[module->selected_sample_slot].filename;
			text_to_display = "#" + std::to_string(module->selected_sample_slot + 1) + ":";

			// std::string file_name = module->sample.filename;

			// Create a short version of the filename.  Essentially truncates the filename if it is too long.
			for (int i=0; i<40; i++) text_to_display = text_to_display + file_name[i];
		}

		nvgFontSize(args.vg, 13);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0);
		nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
		nvgTextBox(args.vg, 5, 5, 700, text_to_display.c_str(), NULL);

		nvgRestore(args.vg);
	}

};


struct GoblinsLoadSample : MenuItem
{
	Goblins *module;
	unsigned int sample_number = 0;

	void onAction(const event::Action &e) override
	{
		const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

		if (path)
		{
			module->samples[sample_number].load(path);
			module->root_dir = std::string(path);
			free(path);
		}
	}
};

struct GoblinsWidget : ModuleWidget
{
	GoblinsWidget(Goblins* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/goblins_front_panel.svg")));

		// Purge
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 45)), module, Goblins::PURGE_TRIGGER_INPUT));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(17, 52)), module, Goblins::PURGE_BUTTON_PARAM));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(17, 52)), module, Goblins::PURGE_LIGHT));

		// Position
		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(35, 33)), module, Goblins::SAMPLE_PLAYBACK_POSITION_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(54, 33)), module, Goblins::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(67, 33)), module, Goblins::SAMPLE_PLAYBACK_POSITION_INPUT));

		// Length
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(10, 90)), module, Goblins::PLAYBACK_LENGTH_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(10, 104)), module, Goblins::PLAYBACK_LENGTH_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 115.5)), module, Goblins::PLAYBACK_LENGTH_INPUT));

		// Rate
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(30, 90)), module, Goblins::SPAWN_RATE_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30, 104)), module, Goblins::SPAWN_RATE_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 115.5)), module, Goblins::SPAWN_RATE_INPUT));

		// Quantity (Countryside Capacity)
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(50, 90)), module, Goblins::COUNTRYSIDE_CAPACITY_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(50, 104)), module, Goblins::COUNTRYSIDE_CAPACITY_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50, 115.5)), module, Goblins::COUNTRYSIDE_CAPACITY_INPUT));

		// Pitch
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(70, 90)), module, Goblins::PITCH_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(70, 104)), module, Goblins::PITCH_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(70, 115.5)), module, Goblins::PITCH_INPUT));

		// Sample
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(90, 90)), module, Goblins::SAMPLE_SELECT_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(90, 104)), module, Goblins::SAMPLE_SELECT_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(90, 115.5)), module, Goblins::SAMPLE_SELECT_INPUT));

		// Trim / Wav Output
		// addParam(createParamCentered<Trimpot>(mm2px(Vec(88.6, 37)), module, Goblins::TRIM_KNOB));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(91.874, 44.611)), module, Goblins::WAV_OUTPUT));

		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 124.893)), module, Ghosts::DEBUG_OUTPUT));

		GoblinsSampleReadout *readout = new GoblinsSampleReadout();
		readout->box.pos = mm2px(Vec(8, 64.3));
		readout->box.size = Vec(200, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);
	}

	void appendContextMenu(Menu *menu) override
	{
		Goblins *module = dynamic_cast<Goblins*>(this->module);
		assert(module);

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		menu->addChild(new MenuEntry); // For spacing only

		std::string menu_text = "Load Sample #";

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			GoblinsLoadSample *menu_item_load_sample = new GoblinsLoadSample();
			menu_item_load_sample->sample_number = i;
			menu_item_load_sample->text = menu_text + std::to_string(i+1);
			menu_item_load_sample->module = module;
			menu->addChild(menu_item_load_sample);
		}
	}

};

Model* modelGoblins = createModel<Goblins, GoblinsWidget>("goblins");
