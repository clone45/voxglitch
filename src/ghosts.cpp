//
// Voxglitch "Ghosts" module for VCV Rack
//


#include "plugin.hpp"
#include "osdialog.h"
#include "sample.hpp"

#define MAX_GRAVEYARD_CAPACITY 128.0f
#define MAX_GHOST_SPAWN_RATE 12000.0f

struct Ghost
{
	// Start Position is the offset into the sample where playback should start.
	// It is set when the ghost is first created.
	float start_position;

	// Playback length for the ghost, measuring in .. er.. ticks?
	float playback_length;

	// sample_ptr points to the loaded sample in memory
	Sample *sample_ptr;

	// playback_position is similar to samplePos used in for samples.  However,
	// it's relative to the Ghost's start_position rather than the sample
	// start position.
	float playback_position = 0.0f;

	unsigned int sample_position = 0;

	// Used as a counter for smoothing transitions

	//
	// smooth_ramp:
	//
	// Setting this to 0, when the smooth switch it ON, will cause smoothing
	// of transitions between playback at the end of the sample and beginning
	// of the sample.  Notice that it's set to 0 by default.  That's so when new
	// ghosts are added, they'll smooth into the sound when first created.
	//

	float loop_smoothing_ramp = 0;
	float removal_smoothing_ramp = 0;

	float last_wave_output_voltage = 0;
	float output_voltage = 0;

	bool dead = false;
	bool dying = false;

	float getOutput(float smooth_rate)
	{

		// Note that we're adding two floating point numbers, then casting
		// them to an int, which is much faster than using floor()
		sample_position = this->start_position + this->playback_position;

		// Wrap if the sample position is past the sample end point
		if (sample_position >= this->sample_ptr->total_sample_count)
		{
			sample_position = sample_position % this->sample_ptr->total_sample_count;
		}

		output_voltage = this->sample_ptr->leftPlayBuffer[sample_position];

		if(loop_smoothing_ramp < 1)
		{
			loop_smoothing_ramp += smooth_rate;
			output_voltage = (last_wave_output_voltage * (1.0f - loop_smoothing_ramp)) + (output_voltage * loop_smoothing_ramp);
		}

		last_wave_output_voltage = output_voltage;

		if(dying && (removal_smoothing_ramp < 1))
		{
			removal_smoothing_ramp += 0.001f;
			output_voltage = (output_voltage * (1.0f - removal_smoothing_ramp));
			if(removal_smoothing_ramp >= 1) dead = true;
		}

		return(output_voltage);
	}

	void age(float step_amount)
	{
		// Step the playback position forward.
		playback_position = playback_position + step_amount;

		// If the playback position is past the playback length, then wrap the playback position to the beginning
		if(playback_position >= playback_length)
		{
			playback_position = playback_position - playback_length;
			loop_smoothing_ramp = 0; // smooth back into it
		}

		if((playback_position >= playback_length) || (playback_position < 0)) playback_position = 0;
	}

	void startDying()
	{
		dying = true;
	}
};


struct Ghosts : Module
{
	float spawn_rate_counter = 0;
	float step_amount = 0;
	float smooth_rate = 0;

	int step = 0;
	std::string root_dir;
	std::string path;

	std::vector<Ghost> graveyard;
	Sample sample;
	dsp::SchmittTrigger purge_trigger;
	dsp::SchmittTrigger purge_button_trigger;

	float jitter_divisor = 1;

	enum ParamIds {
		GHOST_PLAYBACK_LENGTH_KNOB,
		GHOST_PLAYBACK_LENGTH_ATTN_KNOB,
		GRAVEYARD_CAPACITY_KNOB,
		GRAVEYARD_CAPACITY_ATTN_KNOB,
		GHOST_SPAWN_RATE_KNOB,
		GHOST_SPAWN_RATE_ATTN_KNOB,
		SAMPLE_PLAYBACK_POSITION_KNOB,
		SAMPLE_PLAYBACK_POSITION_ATTN_KNOB,
		PURGE_BUTTON_PARAM,
		TRIM_KNOB,
		JITTER_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		PURGE_TRIGGER_INPUT,
		JITTER_CV_INPUT,
		PITCH_INPUT,
		GHOST_PLAYBACK_LENGTH_INPUT,
		GRAVEYARD_CAPACITY_INPUT,
		GHOST_SPAWN_RATE_INPUT,
		SAMPLE_PLAYBACK_POSITION_INPUT,
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
	Ghosts()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(GHOST_PLAYBACK_LENGTH_KNOB, 0.0f, 1.0f, 0.5f, "GhostLengthKnob");
		configParam(GHOST_PLAYBACK_LENGTH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "GhostLengthAttnKnob");
		configParam(GRAVEYARD_CAPACITY_KNOB, 0.0f, 1.0f, 1.0f, "GraveyardCapacityKnob");
		configParam(GRAVEYARD_CAPACITY_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "GraveyardCapacityAttnKnob");
		configParam(GHOST_SPAWN_RATE_KNOB, 0.01f, 1.0f, 0.2f, "GhostSpawnRateKnob");  // max 24000
		configParam(GHOST_SPAWN_RATE_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "GhostSpawnRateAttnKnob");
		configParam(SAMPLE_PLAYBACK_POSITION_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionKnob");
		configParam(SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionAttnKnob");
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

		if (loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			sample.load(path);
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
		float spawn_rate = calculate_inputs(GHOST_SPAWN_RATE_INPUT, GHOST_SPAWN_RATE_KNOB, GHOST_SPAWN_RATE_ATTN_KNOB, MAX_GHOST_SPAWN_RATE);
		float playback_length = calculate_inputs(GHOST_PLAYBACK_LENGTH_INPUT, GHOST_PLAYBACK_LENGTH_KNOB, GHOST_PLAYBACK_LENGTH_ATTN_KNOB, (args.sampleRate / 16));
		float start_position = calculate_inputs(SAMPLE_PLAYBACK_POSITION_INPUT, SAMPLE_PLAYBACK_POSITION_KNOB, SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, sample.total_sample_count);

		// Ensure that the inputs are within range
		spawn_rate = clamp(spawn_rate, 0.0f, MAX_GHOST_SPAWN_RATE);
		if(start_position >= sample.total_sample_count) start_position = sample.total_sample_count - 1;

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

		bool purge_is_triggered = purge_trigger.process(inputs[PURGE_TRIGGER_INPUT].getVoltage()) || purge_button_trigger.process(params[PURGE_BUTTON_PARAM].getValue());

		if (purge_is_triggered)
		{
			for(Ghost& ghost : graveyard)
			{
				if(! ghost.dying) ghost.startDying();
			}
		}

		lights[PURGE_LIGHT].setSmoothBrightness(purge_is_triggered, args.sampleTime);

		if(spawn_rate_counter >= spawn_rate)
		{
			//
			// I ain't afraid of no ghosts! ♫ ♪
			//

			Ghost ghost;
			ghost.start_position = start_position;
			ghost.playback_length = playback_length;
			ghost.sample_ptr = &sample;
			graveyard.push_back(ghost);

			spawn_rate_counter = 0;
		}

		// Kill off any completely dead ghosts
		for(int i = graveyard.size() - 1; i >= 0; i--)
		{
			if(graveyard[i].dead) graveyard.erase(graveyard.begin() + i);
		}

		// Start killing off older ghosts.  This doesn't remove them immediately.
		// Instead, it marks them for removal.  Once marked for removal, the audio
		// smoothing process will be giving time to complete before the ghost has
		// been completely removed.

		unsigned int graveyard_capacity = calculate_inputs(GRAVEYARD_CAPACITY_INPUT, GRAVEYARD_CAPACITY_KNOB, GRAVEYARD_CAPACITY_ATTN_KNOB, MAX_GRAVEYARD_CAPACITY);
		if (graveyard_capacity > MAX_GRAVEYARD_CAPACITY) graveyard_capacity = MAX_GRAVEYARD_CAPACITY;

		if(graveyard.size() > graveyard_capacity)
		{
			for(unsigned int i=0; i < graveyard.size() - graveyard_capacity; i++)
			{
				if(! graveyard[i].dying) graveyard[i].startDying();
			}
		}

		if ((! sample.loading) && (sample.total_sample_count > 0))
		{

			//
			// Iterate over all the ghosts in the graveyard and listen to what
			// they have to say...  What's that?  You want nachos?  Can you
			// even eat nachos?
			//

			float mix_output = 0;

			if(graveyard.empty() == false)
			{
				// pre-calculate part of the math used to determine sample positions
				// in the Ghost's age() function
				step_amount = sample.sample_rate / args.sampleRate;
				smooth_rate = 128.0f / args.sampleRate;

				for(Ghost& ghost : graveyard)
				{
					mix_output += ghost.getOutput(smooth_rate);
					ghost.age(step_amount);
				}

				mix_output = mix_output * params[TRIM_KNOB].getValue();
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

struct GhostsLoadSample : MenuItem
{
	Ghosts *module;

	void onAction(const event::Action &e) override
	{

		// const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
		const std::string dir = "";
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

		if (path)
		{
			module->sample.load(path);
			module->root_dir = std::string(path);
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
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 33)), module, Ghosts::PURGE_TRIGGER_INPUT));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(10, 46)), module, Ghosts::PURGE_BUTTON_PARAM));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(10, 46)), module, Ghosts::PURGE_LIGHT));

		// Jitter
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(26, 33)), module, Ghosts::JITTER_CV_INPUT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(26, 46)), module, Ghosts::JITTER_SWITCH));

		// Position
		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(71, 33)), module, Ghosts::SAMPLE_PLAYBACK_POSITION_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71, 52)), module, Ghosts::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(71, 65)), module, Ghosts::SAMPLE_PLAYBACK_POSITION_INPUT));

		// Length
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(44, 68)), module, Ghosts::GHOST_PLAYBACK_LENGTH_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 68)), module, Ghosts::GHOST_PLAYBACK_LENGTH_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 68)), module, Ghosts::GHOST_PLAYBACK_LENGTH_ATTN_KNOB));

		// Spawn rate
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(44, 112)), module, Ghosts::GHOST_SPAWN_RATE_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 112)), module, Ghosts::GHOST_SPAWN_RATE_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 112)), module, Ghosts::GHOST_SPAWN_RATE_ATTN_KNOB));

		// Graveyard Capacity
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(44, 90)), module, Ghosts::GRAVEYARD_CAPACITY_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 90)), module, Ghosts::GRAVEYARD_CAPACITY_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 90)), module, Ghosts::GRAVEYARD_CAPACITY_ATTN_KNOB));

		// Trim
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.810, 93.915)), module, Ghosts::TRIM_KNOB));


		// WAV output
		addOutput(createOutput<PJ301MPort>(Vec(200, 324), module, Ghosts::WAV_OUTPUT));
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 124.893)), module, Ghosts::DEBUG_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		Ghosts *module = dynamic_cast<Ghosts*>(this->module);
		assert(module);

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		menu->addChild(new MenuEntry); // For spacing only

		GhostsLoadSample *menu_item_load_sample = new GhostsLoadSample();
		menu_item_load_sample->text = "Select .wav file";
		menu_item_load_sample->module = module;
		menu->addChild(menu_item_load_sample);
	}

};



Model* modelGhosts = createModel<Ghosts, GhostsWidget>("ghosts");
