//
// Voxglitch "Ghosts" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "sample.hpp"
#include "GrainSilo.hpp"

#define MAX_GRAVEYARD_CAPACITY 128.0f
#define MAX_GHOST_SPAWN_RATE 30000.0f

struct Ghosts : Module
{
	float spawn_rate_counter = 0;
	float step_amount = 0;
	float smooth_rate = 0;

	int step = 0;
	std::string root_dir;
	std::string path;

	GrainSilo graveyard;
	Sample sample;
	dsp::SchmittTrigger purge_trigger;
	dsp::SchmittTrigger purge_button_trigger;

	float jitter_divisor = 1;

	// The filename of the loaded sample.  This is used to display the currently
	// loaded sample in the right-click context menu.
	std::string loaded_filename = "[ EMPTY ]";

	enum ParamIds {
		GHOST_PLAYBACK_LENGTH_KNOB,
		GHOST_PLAYBACK_LENGTH_ATTN_KNOB,
		GRAVEYARD_CAPACITY_KNOB,
		GRAVEYARD_CAPACITY_ATTN_KNOB,
		GHOST_SPAWN_RATE_KNOB,
		GHOST_SPAWN_RATE_ATTN_KNOB,
		SAMPLE_PLAYBACK_POSITION_KNOB,
		SAMPLE_PLAYBACK_POSITION_ATTN_KNOB,
		PITCH_KNOB,
		PITCH_ATTN_KNOB,
		PURGE_BUTTON_PARAM,
		TRIM_KNOB,
		JITTER_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		PURGE_TRIGGER_INPUT,
		JITTER_CV_INPUT,
		GHOST_PLAYBACK_LENGTH_INPUT,
		GRAVEYARD_CAPACITY_INPUT,
		GHOST_SPAWN_RATE_INPUT,
		SAMPLE_PLAYBACK_POSITION_INPUT,
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT_LEFT,
		AUDIO_OUTPUT_RIGHT,
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
	Ghosts()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(GHOST_PLAYBACK_LENGTH_KNOB, 0.0f, 1.0f, 0.5f, "GhostLengthKnob");
		configParam(GHOST_PLAYBACK_LENGTH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "GhostLengthAttnKnob");
		configParam(GRAVEYARD_CAPACITY_KNOB, 0.0f, 1.0f, 0.2f, "GraveyardCapacityKnob");
		configParam(GRAVEYARD_CAPACITY_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "GraveyardCapacityAttnKnob");
		configParam(GHOST_SPAWN_RATE_KNOB, 0.01f, 1.0f, 0.2f, "GhostSpawnRateKnob");  // max 24000
		configParam(GHOST_SPAWN_RATE_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "GhostSpawnRateAttnKnob");
		configParam(SAMPLE_PLAYBACK_POSITION_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionKnob");
		configParam(SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionAttnKnob");
		configParam(PITCH_KNOB, -0.3f, 1.0f, 0.0f, "PitchKnob");
		configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "PitchAttnKnob");
		configParam(PURGE_BUTTON_PARAM, 0.f, 1.f, 0.f, "PurgeButtonParam");
		configParam(TRIM_KNOB, 0.0f, 2.0f, 1.0f, "TrimKnob");
		configParam(JITTER_SWITCH, 0.f, 1.f, 1.f, "Jitter");

		jitter_divisor = static_cast <float> (RAND_MAX / 1024.0);
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "path", json_string(sample.path.c_str()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		json_t *loaded_path_json = json_object_get(rootJ, ("path"));

		if(loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			sample.load(path, false);
			loaded_filename = sample.filename;
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
		float spawn_rate = calculate_inputs(GHOST_SPAWN_RATE_INPUT, GHOST_SPAWN_RATE_KNOB, GHOST_SPAWN_RATE_ATTN_KNOB, 4) + 1;
		float playback_length = calculate_inputs(GHOST_PLAYBACK_LENGTH_INPUT, GHOST_PLAYBACK_LENGTH_KNOB, GHOST_PLAYBACK_LENGTH_ATTN_KNOB, (args.sampleRate / 16));
		float start_position = calculate_inputs(SAMPLE_PLAYBACK_POSITION_INPUT, SAMPLE_PLAYBACK_POSITION_KNOB, SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, sample.total_sample_count);

		// Ensure that the inputs are within range
		spawn_rate = pow(10.0, spawn_rate);
		spawn_rate = clamp(spawn_rate, 0.0f, MAX_GHOST_SPAWN_RATE);
		if(start_position >= (sample.total_sample_count - playback_length)) start_position = sample.total_sample_count - playback_length;

		// Shorten the playback length if it would result in playback passing the end of the sample data.
		if(playback_length > (sample.total_sample_count - start_position)) playback_length = sample.total_sample_count - start_position;

		//
		// This next conditional is a little tricky, so let me break it down...
		//   If there's a cable in the Jitter CV Input, then apply jitter if the signal on that cable is greater than 0
		//   .. otherwise, use position of the jitter switch to determine if jitter should be applied.
		//
		// Additional Notes
		// * The jitter switch is ignored if a cable is connected to the jitter CV input.
		// * I'm not sure if (inputs[JITTER_CV_INPUT].getVoltage() > 0) is the proper way to
		//   check if the voltage is "on", or if I should us 0.5 or some other number?

		if(inputs[JITTER_CV_INPUT].isConnected() ? (inputs[JITTER_CV_INPUT].getVoltage() > 0) : params[JITTER_SWITCH].getValue())
		{
			float r = (static_cast <float> (rand()) / jitter_divisor) - 1024.0;
			start_position = start_position + r;
		}

		// Handle puge trigger input
		bool purge_is_triggered = purge_trigger.process(inputs[PURGE_TRIGGER_INPUT].getVoltage()) || purge_button_trigger.process(params[PURGE_BUTTON_PARAM].getValue());
		if(purge_is_triggered)
		{
			graveyard.markAllForRemoval();
		}
		lights[PURGE_LIGHT].setSmoothBrightness(purge_is_triggered, args.sampleTime);

		// Remove any completely dead ghosts from the graveyard
		// graveyard.cleanup();

		// Add more ghosts!
		if(spawn_rate_counter >= spawn_rate)
		{
			//
			// I ain't afraid of no ghosts! ♫ ♪
			//
			graveyard.add(start_position, playback_length, &sample);
			spawn_rate_counter = 0;
		}

		// Start killing off older ghosts.  This doesn't remove them immediately.
		// Instead, it marks them for removal.  Once marked for removal, the audio
		// smoothing process will be giving time to complete before the ghost has
		// been completely removed.

		int graveyard_capacity = calculate_inputs(GRAVEYARD_CAPACITY_INPUT, GRAVEYARD_CAPACITY_KNOB, GRAVEYARD_CAPACITY_ATTN_KNOB, MAX_GRAVEYARD_CAPACITY);

		if(graveyard.active() > graveyard_capacity)
		{
			// Mark the oldest 'nth' ghosts for removal
			graveyard.markOldestForRemoval(graveyard.active() - graveyard_capacity);
		}

		if (sample.loaded)
		{

			//
			// Iterate over all the ghosts in the graveyard and listen to what
			// they have to say...  What's that?  You want nachos?  Can you
			// even eat nachos?
			//

			if(graveyard.isEmpty() == false)
			{
				// pre-calculate step amount and smooth rate. This is to reduce the amount of math needed
				// within each Ghost's getStereoOutput() and age() functions.

				if(inputs[PITCH_INPUT].isConnected())
				{
					step_amount = (sample.sample_rate / args.sampleRate) + (((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 0.5f) * params[PITCH_ATTN_KNOB].getValue()) + params[PITCH_KNOB].getValue();
				}
				else
				{
					step_amount = (sample.sample_rate / args.sampleRate) + params[PITCH_KNOB].getValue();
				}

				smooth_rate = 128.0f / args.sampleRate;

				// Get the output from the graveyard and increase the age of each ghost
				std::pair<float, float> stereo_output = graveyard.process(smooth_rate, step_amount);
				float left_mix_output = stereo_output.first * params[TRIM_KNOB].getValue();
				float right_mix_output = stereo_output.second  * params[TRIM_KNOB].getValue();

				// Send audio to outputs
				outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_mix_output);
				outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_mix_output);
			}

			// TODO: spawn_rate_counter should probably take into consideration the selected sample rate.
			spawn_rate_counter = spawn_rate_counter + 1.0f;
		}
	}
};

struct GhostsLoadSample : MenuItem
{
	Ghosts *module;

	void onAction(const event::Action &e) override
	{
		const std::string dir = "";
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

		if(path)
		{
			module->sample.load(path, false);
			module->root_dir = std::string(path);
			module->loaded_filename = module->sample.filename;
			free(path);
		}
	}
};

struct GhostsWidget : ModuleWidget
{
	GhostsWidget(Ghosts* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ghosts_front_panel.svg")));

		// Purge
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366, 25.974)), module, Ghosts::PURGE_TRIGGER_INPUT));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(75.595, 25.974)), module, Ghosts::PURGE_BUTTON_PARAM));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(75.595, 25.974)), module, Ghosts::PURGE_LIGHT));

		// Jitter
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366, 45.713)), module, Ghosts::JITTER_CV_INPUT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(75.595, 45.713)), module, Ghosts::JITTER_SWITCH));

		//
		// Main Left-side Knobs
		//

		float y_offset = 1.8;
		float x_offset = -1.8;

		// Position
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 28.526 - y_offset)), module, Ghosts::SAMPLE_PLAYBACK_POSITION_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 28.526 - y_offset)), module, Ghosts::SAMPLE_PLAYBACK_POSITION_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 28.526 - y_offset)), module, Ghosts::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));

		// Pitch
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 50.489 - y_offset)), module, Ghosts::PITCH_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 50.489 - y_offset)), module, Ghosts::PITCH_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 50.489 - y_offset)), module, Ghosts::PITCH_ATTN_KNOB));

		// Length
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 72.452 - y_offset)), module, Ghosts::GHOST_PLAYBACK_LENGTH_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 72.452 - y_offset)), module, Ghosts::GHOST_PLAYBACK_LENGTH_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 72.452 - y_offset)), module, Ghosts::GHOST_PLAYBACK_LENGTH_ATTN_KNOB));

		// Graveyard Capacity
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 94.416 - y_offset)), module, Ghosts::GRAVEYARD_CAPACITY_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 94.416 - y_offset)), module, Ghosts::GRAVEYARD_CAPACITY_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 94.416 - y_offset)), module, Ghosts::GRAVEYARD_CAPACITY_ATTN_KNOB));

		// Spawn rate
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 116.634 - y_offset)), module, Ghosts::GHOST_SPAWN_RATE_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 116.634 - y_offset)), module, Ghosts::GHOST_SPAWN_RATE_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 116.634 - y_offset)), module, Ghosts::GHOST_SPAWN_RATE_ATTN_KNOB));

		// Trim
		addParam(createParamCentered<Trimpot>(mm2px(Vec(75.470, 103.043)), module, Ghosts::TRIM_KNOB));

		// WAV output

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.746, 114.702)), module, Ghosts::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(75.470, 114.702)), module, Ghosts::AUDIO_OUTPUT_RIGHT));
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 124.893)), module, Ghosts::DEBUG_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		Ghosts *module = dynamic_cast<Ghosts*>(this->module);
		assert(module);

		menu->addChild(new MenuEntry); // For spacing only
		menu->addChild(createMenuLabel("Sample"));

		GhostsLoadSample *menu_item_load_sample = new GhostsLoadSample();
		menu_item_load_sample->text = module->loaded_filename;
		menu_item_load_sample->module = module;
		menu->addChild(menu_item_load_sample);
	}

};



Model* modelGhosts = createModel<Ghosts, GhostsWidget>("ghosts");
