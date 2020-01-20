//
// Voxglitch "Goblins" module for VCV Rack
//


#include "plugin.hpp"
#include "osdialog.h"
#include "dr_wav.h"
#include <vector>
#include "cmath"
#include <memory>

using namespace std;
#define MAX_NUMBER_OF_GOBLINS 128.0f
#define MAX_SPAWN_RATE 8000.0f

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
		return(this->sample_ptr->playBuffer[sample_position]);
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
	float spawn_rate_counter = 0;
	float step_amount = 0;

	int step = 0;
	std::string root_dir;
	std::string path;

	vector<Goblin> countryside;
	Sample sample;
	dsp::SchmittTrigger purge_trigger;

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
		TRIM_KNOB,
		JITTER_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		PURGE_TRIGGER_INPUT,
		PITCH_INPUT,
		PLAYBACK_LENGTH_INPUT,
		COUNTRYSIDE_CAPACITY_INPUT,
		SPAWN_RATE_INPUT,
		SAMPLE_PLAYBACK_POSITION_INPUT,
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

		configParam(TRIM_KNOB, 0.0f, 2.0f, 1.0f, "TrimKnob");
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
		float spawn_rate = calculate_inputs(SPAWN_RATE_INPUT, SPAWN_RATE_KNOB, SPAWN_RATE_ATTN_KNOB, MAX_SPAWN_RATE);
		float playback_length = calculate_inputs(PLAYBACK_LENGTH_INPUT, PLAYBACK_LENGTH_KNOB, PLAYBACK_LENGTH_ATTN_KNOB, (args.sampleRate / 8));
		float start_position = calculate_inputs(SAMPLE_PLAYBACK_POSITION_INPUT, SAMPLE_PLAYBACK_POSITION_KNOB, SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, sample.total_sample_count);

		// Ensure that the inputs are within range
		spawn_rate = clamp(spawn_rate, 0.0f, MAX_SPAWN_RATE);

		// If the purge input it trittered, empty the goblins from the country side
		if (purge_trigger.process(inputs[PURGE_TRIGGER_INPUT].getVoltage()))
		{
            countryside.clear();
		}

		// Spawn new goblins
		if(spawn_rate_counter >= spawn_rate)
		{
			Goblin goblin;
			goblin.start_position = start_position;
			goblin.sample_ptr = &sample;
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

		if ((! sample.loading) && (sample.total_sample_count > 0))
		{
			// Iterate over all the goblins in the countryside and collect
			// their output.

			float mix_output = 0;

			if(countryside.empty() == false)
			{
				// old: step_amount = sample.sample_rate / args.sampleRate;

				// Increment sample offset (pitch)
				if (inputs[PITCH_INPUT].isConnected())
				{
					step_amount = (sample.sample_rate / args.sampleRate) + (((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 0.5f) * params[PITCH_ATTN_KNOB].getValue()) + params[PITCH_KNOB].getValue();
				}
				else
				{
					step_amount = (sample.sample_rate / args.sampleRate) + params[PITCH_KNOB].getValue();
				}

				for(Goblin& goblin : countryside)
				{
					mix_output += goblin.getOutput();
					goblin.age(step_amount, playback_length);
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

struct GoblinsLoadSample : MenuItem
{
	Goblins *module;

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

struct GoblinsWidget : ModuleWidget
{
	GoblinsWidget(Goblins* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/goblins_front_panel.svg")));

		// Purge
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 33)), module, Goblins::PURGE_TRIGGER_INPUT));

		// Position
		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(50, 33)), module, Goblins::SAMPLE_PLAYBACK_POSITION_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(50, 52)), module, Goblins::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50, 65)), module, Goblins::SAMPLE_PLAYBACK_POSITION_INPUT));

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

		// Trim / Wav Output
		addParam(createParamCentered<Trimpot>(mm2px(Vec(88.6, 37)), module, Goblins::TRIM_KNOB));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(88.6, 54.795)), module, Goblins::WAV_OUTPUT));

		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 124.893)), module, Ghosts::DEBUG_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		Goblins *module = dynamic_cast<Goblins*>(this->module);
		assert(module);

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		menu->addChild(new MenuEntry); // For spacing only

		GoblinsLoadSample *menu_item_load_sample = new GoblinsLoadSample();
		menu_item_load_sample->text = "Select .wav file";
		menu_item_load_sample->module = module;
		menu->addChild(menu_item_load_sample);


		//
		// Options
		// =====================================================================

		menu->addChild(new MenuEntry);
		menu->addChild(createMenuLabel("Options"));
	}

};

Model* modelGoblins = createModel<Goblins, GoblinsWidget>("goblins");
