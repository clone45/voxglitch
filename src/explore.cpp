//
// Voxglitch "Ghosts" module for VCV Rack
//


#include "plugin.hpp"
#include "osdialog.h"
#include "dr_wav.h"
#include <vector>
#include "cmath"
#include <memory>

using namespace std;
#define GAIN 5.0
#define MAX_GRAVEYARD_CAPACITY 500.0f

struct Sample
{
	std::string path;
	std::string filename;
	drwav_uint64 total_sample_count;
	bool loading;
	vector<float> playBuffer;
	unsigned int sample_rate;
	unsigned int channels;
	bool run = false;

	Sample()
	{
		playBuffer.resize(0);
		total_sample_count = 0;
		loading = false;
		filename = "[ empty ]";
		path = "";
		sample_rate = 0;
		channels = 0;
	}

	void load(std::string path)
	{
		this->loading = true;

		unsigned int reported_channels;
		unsigned int reported_sample_rate;
		drwav_uint64 reported_total_sample_count;
		float *pSampleData;

		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &reported_channels, &reported_sample_rate, &reported_total_sample_count);

		if (pSampleData != NULL)
		{
			// I'm aware that the "this" pointer isn't necessary here, but I wanted to include
			// it just to make the code as clear as possible.

			this->channels = reported_channels;
			this->sample_rate = reported_sample_rate;
			this->playBuffer.clear();

			for (unsigned int i=0; i < reported_total_sample_count; i = i + this->channels)
			{
				this->playBuffer.push_back(pSampleData[i]);
			}

			drwav_free(pSampleData);

			this->total_sample_count = playBuffer.size();
			this->loading = false;
			this->filename = rack::string::filename(path);
			this->path = path;
		}
		else
		{
			this->loading = false;
		}
	};
};

struct Ghost
{
	// Start Position is the offset into the sample where playback should start.
	// It is set when the ghost is first created.
	float start_position;

	// Playback length for the ghost, measuring in .. er.. ticks?
	float playback_length;

	unsigned int pitch;

	// sample_ptr points to the loaded sample in memory
	Sample *sample_ptr;

	// playback_position is similar to samplePos used in for samples.  However,
	// it's relative to the Ghost's start_position rather than the sample
	// start position.
	float playback_position = 0.0f;

	// Once a Ghost dies, it's flagged using has_died to it can later be
	// removed from the graveyard (aka, the vector of Ghosts).
	bool has_died = false;

	// Used as a counter for smoothing transitions
	//float smooth_ramp = 1;
	//float last_wave_output_voltage = 0;

	float getOutput()
	{
		float samplePos = this->start_position + this->playback_position;

		if (floor(samplePos) < this->sample_ptr->total_sample_count && (samplePos >= 0))
		{
			return(this->sample_ptr->playBuffer[floor(samplePos)]);
		}
		else
		{
			// A decision that I've made (and I might change my mind later) is
			// that if a ghost's playback_position is past the end of a sample,
			// then it will output 0.  Another option could have been to reset
			// the ghost's playback_position if it reached the end of a sample,
			// but I worry that doing so may generate unexpected high tones.

			return(0);
		}
	}

	bool hasDied()
	{
		return this->has_died;
	}

	void age(float settings_sample_rate)
	{
		// Step the playback position forward.  TODO: Add pitch into this equation
		playback_position = playback_position + (this->sample_ptr->sample_rate / settings_sample_rate);

		// If the playback position is past the playback length, then wrap the playback position to the beginning
		if(playback_position >= playback_length)
		{
			playback_position = playback_position - playback_length;
		}

		if((playback_position >= playback_length) || (playback_position < 0)) playback_position = 0;
	}
};


struct Explore : Module
{
	float spawn_rate_counter = 0;
	int step = 0;
	std::string root_dir;
	std::string path;

	vector<Ghost> graveyard;
	Sample sample;
	dsp::SchmittTrigger playTrigger;

	enum ParamIds {
		GHOST_PLAYBACK_LENGTH_KNOB,
		GRAVEYARD_CAPACITY_KNOB,
		GRAVEYARD_CAPACITY_ATTN_KNOB,
		GHOST_SPAWN_RATE_KNOB,
		SAMPLE_PLAYBACK_POSITION_KNOB,
		TRIM_KNOB,
		SMOOTH_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		PITCH_INPUT,
		GRAVEYARD_CAPACITY_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		WAV_OUTPUT,
		DEBUG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	Explore()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(GHOST_PLAYBACK_LENGTH_KNOB, 0.0f, 1.0f, 0.5f, "GhostLengthKnob");
		configParam(GRAVEYARD_CAPACITY_KNOB, 0.0f, 1.0f, 1.0f, "GraveyardCapacityKnob");
		configParam(GHOST_SPAWN_RATE_KNOB, 3000.0f, 24000.0f, 6000.00f, "GhostSpawnRateKnob");
		configParam(SAMPLE_PLAYBACK_POSITION_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionKnob");
		configParam(TRIM_KNOB, 0.0f, 1.0f, 1.0f, "TrimKnob");
		configParam(SMOOTH_SWITCH, 0.f, 1.f, 1.f, "Smooth");
	}

	json_t *dataToJson() override
	{
		// TODO: Save and load ghosts

		json_t *rootJ = json_object();
		// json_object_set_new(rootJ, "path", json_string(this->path.c_str()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		// TODO: Save and load ghosts

        /*
		json_t *loaded_path_json = json_object_get(rootJ, ("path"));
		if (loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			this->load_samples_from_path(this->path.c_str());
		}
        */
	}

	void process(const ProcessArgs &args) override
	{

		outputs[DEBUG_OUTPUT].setVoltage(0);

		// if(spawn_rate_counter >= (params[GHOST_SPAWN_RATE_KNOB].getValue() * args.sampleRate))
		if(spawn_rate_counter >= params[GHOST_SPAWN_RATE_KNOB].getValue())
		{

			//
			// I ain't afraid of no ghosts!
			//

			Ghost ghost;
			ghost.start_position = params[SAMPLE_PLAYBACK_POSITION_KNOB].getValue() * sample.total_sample_count;
			ghost.playback_length = params[GHOST_PLAYBACK_LENGTH_KNOB].getValue() * args.sampleRate; // from 0 to 1 second
			ghost.sample_ptr = &sample;
			graveyard.push_back(ghost);

			outputs[DEBUG_OUTPUT].setVoltage(10);

			spawn_rate_counter = 0;
		}


		float graveyard_capacity_input = inputs[GRAVEYARD_CAPACITY_INPUT].getVoltage() / 10.0;
		float graveyard_capacity_attenuator = params[GRAVEYARD_CAPACITY_ATTN_KNOB].getValue();
		float graveyard_capacity_knob = params[GRAVEYARD_CAPACITY_KNOB].getValue();

		unsigned int graveyard_capacity = floor(((graveyard_capacity_input * MAX_GRAVEYARD_CAPACITY) * graveyard_capacity_attenuator) + (graveyard_capacity_knob * 500.0f));
		// unsigned int graveyard_capacity = graveyard_capacity_knob * 500.0f;
		if (graveyard_capacity > MAX_GRAVEYARD_CAPACITY) graveyard_capacity = MAX_GRAVEYARD_CAPACITY;

		// (((inputs[POSITION_INPUT].getVoltage() / 10.0) * params[POSITION_ATTN_KNOB].getValue()) + params[POSITION_KNOB].getValue());

		while(graveyard.size() > graveyard_capacity)
		{
			graveyard.erase(graveyard.begin());
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
				for(int i = graveyard.size() - 1; i >= 0; i--)
				{
					mix_output += graveyard[i].getOutput();
					graveyard[i].age(args.sampleRate);
					if(graveyard[i].hasDied()) graveyard.erase(graveyard.begin() + i);
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

struct MenuItemLoadSample : MenuItem
{
	Explore *module;

	void onAction(const event::Action &e) override
	{
		const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

		if (path)
		{
			module->sample.load(path);
			module->root_dir = std::string(path);
			free(path);
		}
	}
};

struct ExploreWidget : ModuleWidget
{
	ExploreWidget(Explore* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ghosts_front_panel.svg")));

		// Cosmetic rack screws
		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));

		// Input and label for the trigger input (which is labeled "CLK" on the front panel)
		// addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.185, 114.893)), module, Explore::PITCH_INPUT));

		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(43, 33)), module, Explore::SAMPLE_PLAYBACK_POSITION_KNOB));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(44, 68)), module, Explore::GHOST_PLAYBACK_LENGTH_KNOB));

		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(44, 112)), module, Explore::GHOST_SPAWN_RATE_KNOB));

		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(44, 90)), module, Explore::GRAVEYARD_CAPACITY_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 90)), module, Explore::GRAVEYARD_CAPACITY_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(23, 87)), module, Explore::GRAVEYARD_CAPACITY_ATTN_KNOB));

		// Trim
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.810, 93.915)), module, Explore::TRIM_KNOB));


		// WAV output
		addOutput(createOutput<PJ301MPort>(Vec(200, 324), module, Explore::WAV_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 124.893)), module, Explore::DEBUG_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		Explore *module = dynamic_cast<Explore*>(this->module);
		assert(module);

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		menu->addChild(new MenuEntry); // For spacing only


		//
		// Add the "select bank folder" menu item
		//
		MenuItemLoadSample *menu_item_load_sample = new MenuItemLoadSample();
		menu_item_load_sample->text = "Select .wav file";
		menu_item_load_sample->module = module;
		menu->addChild(menu_item_load_sample);
	}

};



Model* modelExplore = createModel<Explore, ExploreWidget>("explore");
