//
// Voxglitch "Autobreak" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"
#include "dr_wav.h"
#include <vector>
#include "cmath"
#include <memory>

using namespace std;
#define GAIN 5.0
#define NUMBER_OF_PATTERNS 3

struct AutobreakSample
{
	std::string path;
	std::string filename;
	drwav_uint64 total_sample_count;
	bool loading;
	vector<float> playBuffer;
	unsigned int sample_rate;
	unsigned int channels;
	bool run = false;

	AutobreakSample()
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

            this->run = true;
		}
		else
		{
			this->loading = false;
		}
	};
};


struct Autobreak : Module
{
	unsigned int selected_sample_slot = 0;
	float old_sample_position = 0;
	float sample_position = 0;
	float break_sample_position = 0;
	int step = 0;
	unsigned int break_pattern_index = 0;
	unsigned int break_pattern_index_old = 0;
	float smooth_ramp = 1;
	float last_wave_output_voltage = 0;
	std::string rootDir;
	std::string path;

	vector<AutobreakSample> samples;
	dsp::SchmittTrigger playTrigger;

	int break_patterns[NUMBER_OF_PATTERNS][16] = {
		{  0,  1,  2,  3,   4,  5,  6,  7,   8,  8,  8,  8,  -1, -1, -1, -1 },
		{  0, -1, -1, -1,   0, -1, -1, -1,  -1, -1, -1, -1,   0, -1, -1, -1 },
		{ 0, -1, 0, -1,  -1, -1, -1, 4,  6, -1, -1, -1,  6, -1, -1, -1 }
	};

	enum ParamIds {
		WAV_KNOB,
		WAV_ATTN_KNOB,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		WAV_INPUT,
		PATTERN_INPUT,
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
	Autobreak()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(WAV_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
		configParam(WAV_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "path", json_string(this->path.c_str()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		json_t *loaded_path_json = json_object_get(rootJ, ("path"));
		if (loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			this->load_samples_from_path(this->path.c_str());
		}
	}

	void load_samples_from_path(const char *path)
	{
		// Clear out any old .wav files
		this->samples.clear();

		// Load all .wav files found in the folder specified by 'path'

		this->rootDir = std::string(path);
		std::list<std::string> dirList = system::getEntries(path);

		for (auto entry : dirList)
		{
			if (rack::string::lowercase(rack::string::filenameExtension(entry)) == "wav")
			{
				AutobreakSample new_sample;
				new_sample.load(entry);
				this->samples.push_back(new_sample);
			}
		}
	}

	void process(const ProcessArgs &args) override
	{
		unsigned int number_of_samples = samples.size();
		unsigned int wav_input_value = (unsigned int) floor(number_of_samples * (((inputs[WAV_INPUT].getVoltage() / 10.0) * params[WAV_ATTN_KNOB].getValue()) + params[WAV_KNOB].getValue()));

		if(wav_input_value >= number_of_samples) wav_input_value = number_of_samples - 1;

		if(wav_input_value != selected_sample_slot)
		{
			// Reset the smooth ramp if the selected sample has changed
			smooth_ramp = 0;

			// Set the selected sample
			selected_sample_slot = wav_input_value;
		}

		// Check to see if the selected sample slot refers to an existing sample.
		// If not, return.  This could happen before any samples have been loaded.
		if(! (samples.size() > selected_sample_slot)) return;

		AutobreakSample *selected_sample = &samples[selected_sample_slot];

		if (inputs[TRIG_INPUT].isConnected())
		{
			//
			// playTrigger is a SchmittTrigger object.  Here, the SchmittTrigger
			// determines if a low-to-high transition happened at the TRIG_INPUT
			//
			if (playTrigger.process(inputs[TRIG_INPUT].getVoltage()))
			{
				sample_position = 0;
				break_sample_position = 0;
				smooth_ramp = 0;
			}
		}



		if(sample_position >= selected_sample->total_sample_count)
		{
			sample_position = 0;
			smooth_ramp = 0;
		}

		if ((! selected_sample->loading) && (selected_sample->run) && (selected_sample->total_sample_count > 0))
		{
			float wav_output_voltage;

			wav_output_voltage = GAIN  * selected_sample->playBuffer[(int)sample_position];

			if(smooth_ramp < 1)
			{
				float smooth_rate = (128.0f / args.sampleRate);  // A smooth rate of 128 seems to work best
				smooth_ramp += smooth_rate;
				wav_output_voltage = (last_wave_output_voltage * (1 - smooth_ramp)) + (wav_output_voltage * smooth_ramp);
				outputs[WAV_OUTPUT].setVoltage(wav_output_voltage);
			}
			else
			{
				outputs[WAV_OUTPUT].setVoltage(wav_output_voltage);
			}

			last_wave_output_voltage = wav_output_voltage;

			sample_position = sample_position + (selected_sample->sample_rate / args.sampleRate);

			break_sample_position = break_sample_position + (selected_sample->sample_rate / args.sampleRate);
			if(break_sample_position >= selected_sample->total_sample_count) break_sample_position = 0;

			//
			// Optionally jump to new breakbeat position
			//

			break_pattern_index = (break_sample_position / selected_sample->total_sample_count) * 16.0f;

			if(break_pattern_index > 15) break_pattern_index = 15;

			if(break_pattern_index != break_pattern_index_old)
			{
				int selected_pattern = (inputs[PATTERN_INPUT].getVoltage() / 10.0f) * (NUMBER_OF_PATTERNS - 1);

				float new_step = break_patterns[selected_pattern][break_pattern_index];

				if(new_step != -1)
				{
					sample_position = new_step * (selected_sample->total_sample_count / 16.0f);
				}

				break_pattern_index_old = break_pattern_index;
			}
			// outputs[DEBUG_OUTPUT].setVoltage(break_pattern_index);
		}
		else
		{
			selected_sample->run = false;
			outputs[WAV_OUTPUT].setVoltage(0);
		}
	}
};

struct AutobreakReadout : TransparentWidget
{
	Autobreak *module;

	int frame = 0;
	shared_ptr<Font> font;

	AutobreakReadout()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{

		std::string text_to_display;

		text_to_display = "load sample";

		if(module)
		{
			if(module->samples.size() > module->selected_sample_slot)
			{
				text_to_display = module->samples[module->selected_sample_slot].filename;
			}
		}

		nvgFontSize(args.vg, 13);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0);
		nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));

		nvgRotate(args.vg, -M_PI / 2.0f);

		// void nvgTextBox(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end);
		nvgTextBox(args.vg, 5, 5, 700, text_to_display.c_str(), NULL);
	}
};

struct MenuItemLoadBank : MenuItem
{
	Autobreak *wav_bank_module;

	void onAction(const event::Action &e) override
	{
		const std::string dir = wav_bank_module->rootDir;
		char *path = osdialog_file(OSDIALOG_OPEN_DIR, dir.c_str(), NULL, NULL);

		if (path)
		{
			wav_bank_module->load_samples_from_path(path);
			wav_bank_module->path = path;
			free(path);
		}
	}
};

struct AutobreakWidget : ModuleWidget
{
	AutobreakWidget(Autobreak* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/autobreak_front_panel.svg")));

		// Cosmetic rack screws
		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));

		// Input and label for the trigger input (which is labeled "CLK" on the front panel)
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.185, 25.535)), module, Autobreak::TRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.185, 46)), module, Autobreak::WAV_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(13.185, 60)), module, Autobreak::WAV_ATTN_KNOB));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(13.185, 75)), module, Autobreak::WAV_KNOB));

		// Pattern Select
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.185, 100)), module, Autobreak::PATTERN_INPUT));

		AutobreakReadout *readout = new AutobreakReadout();
		readout->box.pos = mm2px(Vec(34.236, 92)); //22,22
		readout->box.size = Vec(110, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		// WAV output
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 100.893)), module, Autobreak::DEBUG_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 114.893)), module, Autobreak::WAV_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		Autobreak *module = dynamic_cast<Autobreak*>(this->module);
		assert(module);

		menu->addChild(new MenuEntry); // For spacing only

		//
		// Add the "select bank folder" menu item
		//
		MenuItemLoadBank *menu_item_load_bank = new MenuItemLoadBank();
		menu_item_load_bank->text = "Select Directory Containing WAV Files";
		menu_item_load_bank->wav_bank_module = module;
		menu->addChild(menu_item_load_bank);
	}

};



Model* modelAutobreak = createModel<Autobreak, AutobreakWidget>("autobreak");
