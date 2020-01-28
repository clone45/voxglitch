//
// Voxglitch "wav bank" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"
#include "sample.hpp"

#define GAIN 5.0

// There's not many times that you would want smoothing disabled, so to make
// room on the front panel for the loop switch, I'm simply going to set
// smoothing to true.  If there's demand, I'll add an option to the right-click
// context menu for enabling and disabling smoothing.

#define SMOOTH_ENABLED 1

struct WavBank : Module
{
	unsigned int selected_sample_slot = 0;
	float samplePos = 0;
	float smooth_ramp = 1;
	float last_wave_output_voltage[2] = {0};
	std::string rootDir;
	std::string path;

	std::vector<Sample> samples;
	dsp::SchmittTrigger playTrigger;

	bool triggered = false;

	enum ParamIds {
		WAV_KNOB,
		WAV_ATTN_KNOB,
		LOOP_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		WAV_INPUT,
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		WAV_LEFT_OUTPUT,
		WAV_RIGHT_OUTPUT,
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
		configParam(WAV_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
		configParam(WAV_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");
		configParam(LOOP_SWITCH, 0.0f, 1.0f, 0.0f, "LoopSwitch");
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
				DEBUG("LOAD OK");
				new_sample.load(entry, false);
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

			// Reset sample position so playback does not start at previous sample position
			samplePos = 0;

			// Set the selected sample
			selected_sample_slot = wav_input_value;

			triggered = false;
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
				triggered = true;
			}
		}
		else {
			triggered = true;
		}

		// Loop
		if(params[LOOP_SWITCH].getValue() == 1.f) {
			if(abs(floor(samplePos)) >= selected_sample->total_sample_count) {
				samplePos = 0;
			}
		}

		if (triggered && (! selected_sample->loading) && (selected_sample->loaded) && (selected_sample->total_sample_count > 0) && ((abs(floor(samplePos)) < selected_sample->total_sample_count)))
		{
			float left_wav_output_voltage;
			float right_wav_output_voltage;

			if (samplePos >= 0)
			{
				left_wav_output_voltage = GAIN  * selected_sample->leftPlayBuffer[floor(samplePos)];
				if(selected_sample->channels > 1) {
					right_wav_output_voltage = GAIN  * selected_sample->rightPlayBuffer[floor(samplePos)];
				}
				else {
					right_wav_output_voltage = left_wav_output_voltage;
				}
			}
			else
			{
				// What is this for?  Does it play the sample in reverse?  I think so.
				left_wav_output_voltage = GAIN * selected_sample->leftPlayBuffer[floor(selected_sample->total_sample_count - 1 + samplePos)];
				if(selected_sample->channels > 1) {
					right_wav_output_voltage = GAIN * selected_sample->rightPlayBuffer[floor(selected_sample->total_sample_count - 1 + samplePos)];
				}
				else {
					right_wav_output_voltage = left_wav_output_voltage;
				}
			}

			if(SMOOTH_ENABLED && (smooth_ramp < 1))
			{
				float smooth_rate = (128.0f / args.sampleRate);  // A smooth rate of 128 seems to work best
				smooth_ramp += smooth_rate;
				left_wav_output_voltage = (last_wave_output_voltage[0] * (1 - smooth_ramp)) + (left_wav_output_voltage * smooth_ramp);
				if(selected_sample->channels > 1) {
					right_wav_output_voltage = (last_wave_output_voltage[1] * (1 - smooth_ramp)) + (right_wav_output_voltage * smooth_ramp);
				}
				else {
					right_wav_output_voltage = left_wav_output_voltage;
				}
			}

			outputs[WAV_LEFT_OUTPUT].setVoltage(left_wav_output_voltage);
			outputs[WAV_RIGHT_OUTPUT].setVoltage(right_wav_output_voltage);

			last_wave_output_voltage[0] = left_wav_output_voltage;
			last_wave_output_voltage[1] = right_wav_output_voltage;

			// Increment sample offset (pitch)
			if (inputs[PITCH_INPUT].isConnected())
			{
				samplePos = samplePos + (selected_sample->sample_rate / args.sampleRate) + ((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 0.5f);
			}
			else
			{
				samplePos = samplePos + (selected_sample->sample_rate / args.sampleRate);
			}
		}
		else
		{
			triggered = false; // Cancel current trigger
			outputs[WAV_LEFT_OUTPUT].setVoltage(0);
			outputs[WAV_RIGHT_OUTPUT].setVoltage(0);
		}
	}
};

struct WavBankReadout : TransparentWidget
{
	WavBank *module;

	int frame = 0;
	std::shared_ptr<Font> font;

	WavBankReadout()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		std::string text_to_display = "Right-Click to Load Samples";

		if(module)
		{
			text_to_display = "";

			if(module->samples.size() > module->selected_sample_slot)
			{
				text_to_display = module->samples[module->selected_sample_slot].filename;
				text_to_display.resize(30); // truncate long text
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
		addParam(createParamCentered<CKSS>(mm2px(Vec(13.185, 97)), module, WavBank::LOOP_SWITCH));

		WavBankReadout *readout = new WavBankReadout();
		readout->box.pos = mm2px(Vec(34.236, 82));
		readout->box.size = Vec(110, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		// WAV output
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 104)), module, WavBank::WAV_LEFT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 114.9)), module, WavBank::WAV_RIGHT_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		WavBank *module = dynamic_cast<WavBank*>(this->module);
		assert(module);

		// For spacing only
		menu->addChild(new MenuEntry);

		// Add the "select bank folder" menu item
		MenuItemLoadBank *menu_item_load_bank = new MenuItemLoadBank();
		menu_item_load_bank->text = "Select Directory Containing WAV Files";
		menu_item_load_bank->wav_bank_module = module;
		menu->addChild(menu_item_load_bank);
	}

};



Model* modelWavBank = createModel<WavBank, WavBankWidget>("wavbank");
