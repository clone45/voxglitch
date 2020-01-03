//
// Voxglitch "wav bank" module for VCV Rack
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

            this->run = true;
		}
		else
		{
			this->loading = false;
		}
	};
};


struct WavBank : Module
{
	unsigned int selected_sample_slot = 0;
	float samplePos = 0;
	int step = 0;
	float smooth_ramp = 1;
	float last_wave_output_voltage = 0;
	std::string rootDir;
	std::string path;

	vector<Sample> samples;
	dsp::SchmittTrigger playTrigger;

	enum ParamIds {
		WAV_KNOB,
		WAV_ATTN_KNOB,
		PITCH_KNOB,
		PITCH_ATTN_KNOB,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		WAV_INPUT,
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		WAV_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	WavBank()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PITCH_KNOB, -1.0f, 1.0f, 0.0f, "PitchKnob");
		configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "PitchAttnKnob");
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
				Sample new_sample;
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

		Sample *selected_sample = &samples[selected_sample_slot];

		if (inputs[TRIG_INPUT].isConnected())
		{
			//
			// playTrigger is a SchmittTrigger object.  Here, the SchmittTrigger
			// determines if a low-to-high transition happened at the TRIG_INPUT
			//
			if (playTrigger.process(inputs[TRIG_INPUT].getVoltage()))
			{
				samplePos = 0;
				smooth_ramp = 0;
			}
		}

		if ((! selected_sample->loading) && (selected_sample->run) && (selected_sample->total_sample_count > 0) && ((abs(floor(samplePos)) < selected_sample->total_sample_count)))
		{
			float wav_output_voltage;

			if (samplePos >= 0)
			{
				wav_output_voltage = GAIN  * selected_sample->playBuffer[floor(samplePos)];
			}
			else
			{
				// What is this for?  Does it play the sample in reverse?  I think so.
				wav_output_voltage = GAIN * selected_sample->playBuffer[floor(selected_sample->total_sample_count - 1 + samplePos)];
			}

			if(smooth_ramp < 1)
			{
				float smooth_rate = 128.0f / args.sampleRate;  // A smooth rate of 128 seems to work best
				smooth_ramp += smooth_rate;
				wav_output_voltage = (last_wave_output_voltage * (1 - smooth_ramp)) + (wav_output_voltage * smooth_ramp);
				outputs[WAV_OUTPUT].setVoltage(wav_output_voltage);
			}
			else
			{
				outputs[WAV_OUTPUT].setVoltage(wav_output_voltage);
			}

			last_wave_output_voltage = wav_output_voltage;

			// Increment sample offset (pitch)
			if (inputs[PITCH_INPUT].isConnected())
			{
				samplePos = samplePos + (selected_sample->sample_rate / args.sampleRate) + (((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 0.5f) * params[PITCH_ATTN_KNOB].getValue()) + params[PITCH_KNOB].getValue();
			}
			else
			{
				samplePos = samplePos + (selected_sample->sample_rate / args.sampleRate) + params[PITCH_KNOB].getValue();
			}
		}
		else
		{
			selected_sample->run = false;
			outputs[WAV_OUTPUT].setVoltage(0);
		}
	}
};

struct WavBankReadout : TransparentWidget
{
	WavBank *module;

	int frame = 0;
	shared_ptr<Font> font;

	WavBankReadout()
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
	WavBank *wav_bank_module;

	void onAction(const event::Action &e) override
	{
		// const std::string dir = wav_bank_module->rootDir.empty() ? "" : wav_bank_module->rootDir;
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

struct WavBankWidget : ModuleWidget
{
	WavBankWidget(WavBank* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/wav_bank_front_panel.svg")));

		// Cosmetic rack screws
		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));

		// Input and label for the trigger input (which is labeled "CLK" on the front panel)
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.185, 25.535)), module, WavBank::TRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.185, 46)), module, WavBank::WAV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.185, 114.893)), module, WavBank::PITCH_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(13.185, 60)), module, WavBank::WAV_ATTN_KNOB));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(13.185, 75)), module, WavBank::WAV_KNOB));

		WavBankReadout *readout = new WavBankReadout();
		readout->box.pos = mm2px(Vec(34.236, 92)); //22,22
		readout->box.size = Vec(110, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		// WAV output
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 114.893)), module, WavBank::WAV_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		WavBank *module = dynamic_cast<WavBank*>(this->module);
		assert(module);

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

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



Model* modelWavBank = createModel<WavBank, WavBankWidget>("wavbank");
