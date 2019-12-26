// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <vector>
#include "cmath"
#include <dirent.h>
#include <algorithm> //----added by Joakim Lindbom

using namespace std;

#define NUMBER_OF_SAMPLES 5
#define NUMBER_OF_SAMPLES_FLOAT 5.0
#define WAVEFORM_DISPLAY_WIDTH 200

struct sample
{
	std::string path;
	std::string filename;
	drwav_uint64 total_sample_count;
	bool loading;
	vector<float> playBuffer;
	vector<double> display_buffer;
	unsigned int sample_rate;
	unsigned int channels;

	sample()
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

			// Create a vector that holds waveform information for dispay
			for (unsigned int i=0; i < this->total_sample_count; i = i + (this->total_sample_count / WAVEFORM_DISPLAY_WIDTH))
			{
				this->display_buffer.push_back(this->playBuffer[i]);
			}
		}
		else
		{
			this->loading = false;
		}
	};


};

struct Repeater : Module
{
	float phase = 0.f;
	float blinkPhase = 0.f;
	bool run = false;

	unsigned int channels;
	unsigned int selected_sample_slot = 0;

	bool led_states[NUMBER_OF_SAMPLES];
	float samplePos = 0;

	int sampnumber = 0;
	int step = 0;
	// vector <std::string> fichier;

	sample samples[NUMBER_OF_SAMPLES];

	// Input triggers
	dsp::SchmittTrigger loadsampleTrigger;
	dsp::SchmittTrigger playTrigger;

	enum ParamIds {
		CLOCK_DIVISION_KNOB,
		CLOCK_DIVISION_ATTN_KNOB,
		POSITION_KNOB,
		POSITION_ATTN_KNOB,
		SAMPLE_SELECT_KNOB,
		SAMPLE_SELECT_ATTN_KNOB,
		PITCH_KNOB,
		PITCH_ATTN_KNOB,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		POSITION_INPUT,
		CLOCK_DIVISION_INPUT,
		SAMPLE_SELECT_INPUT,
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		SAMPLE_SELECT_1_LIGHT,
		SAMPLE_SELECT_2_LIGHT,
		SAMPLE_SELECT_3_LIGHT,
		SAMPLE_SELECT_4_LIGHT,
		SAMPLE_SELECT_5_LIGHT,
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	Repeater()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PITCH_KNOB, -5.0f, 5.0f, -1.7f, "PitchKnob");
		configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "PitchAttnKnob");
		configParam(CLOCK_DIVISION_KNOB, 0.0f, 10.0f, 0.0f, "ClockDivisionKnob");
		configParam(CLOCK_DIVISION_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "ClockDivisionAttnKnob");
		configParam(POSITION_KNOB, 0.0f, 1.0f, 0.0f, "PositionKnob");
		configParam(POSITION_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "PositionAttnKnob");
		configParam(SAMPLE_SELECT_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
		configParam(SAMPLE_SELECT_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");

		for(int sample_slot = 0; sample_slot < NUMBER_OF_SAMPLES; sample_slot++ )
		{
			led_states[sample_slot] = false;
		}

		led_states[0] = true;
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(rootJ, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(samples[i].path.c_str()));
		}
		// json_object_set_new(rootJ, "loaded_sample_path_1", json_string(samples[0].path.c_str()));
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

	void process(const ProcessArgs &args) override {

		unsigned int sample_select_input_value = (unsigned int) floor(NUMBER_OF_SAMPLES_FLOAT * (((inputs[SAMPLE_SELECT_INPUT].getVoltage() / 10.0) * params[SAMPLE_SELECT_ATTN_KNOB].getValue()) + params[SAMPLE_SELECT_KNOB].getValue()));

		if(sample_select_input_value >= NUMBER_OF_SAMPLES) sample_select_input_value = NUMBER_OF_SAMPLES - 1;

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			led_states[i] = false;
		}

		selected_sample_slot = sample_select_input_value;
		led_states[selected_sample_slot] = true;

		// TODO: make this based on NUMBER_OF_SAMPLES
		lights[SAMPLE_SELECT_1_LIGHT].setBrightness(led_states[0] ? 0.9f : 0.f);
		lights[SAMPLE_SELECT_2_LIGHT].setBrightness(led_states[1] ? 0.9f : 0.f);
		lights[SAMPLE_SELECT_3_LIGHT].setBrightness(led_states[2] ? 0.9f : 0.f);
		lights[SAMPLE_SELECT_4_LIGHT].setBrightness(led_states[3] ? 0.9f : 0.f);
		lights[SAMPLE_SELECT_5_LIGHT].setBrightness(led_states[4] ? 0.9f : 0.f);

		sample *selected_sample = &samples[selected_sample_slot];

		if (inputs[TRIG_INPUT].isConnected())
		{
			//
			// playTrigger is a SchmittTrigger object.  Here, the SchmittTrigger
			// determines if a low-to-high transition happened at the TRIG_INPUT
			//
			if (playTrigger.process(inputs[TRIG_INPUT].getVoltage()))
			{
				float clock_division = floor((inputs[CLOCK_DIVISION_INPUT].getVoltage() * params[CLOCK_DIVISION_ATTN_KNOB].getValue()) + params[CLOCK_DIVISION_KNOB].getValue());

				if(clock_division > 10.0) clock_division = 10.0;

				step += 1;
				if(step > clock_division) step = 0;

				if(step == 0)
				{
					run = true;
					samplePos = selected_sample->total_sample_count * (((inputs[POSITION_INPUT].getVoltage() / 10.0) * params[POSITION_ATTN_KNOB].getValue()) + params[POSITION_KNOB].getValue());
				}
			}
		}

		if ((! selected_sample->loading) && (run) && (selected_sample->total_sample_count > 0) && ((abs(floor(samplePos)) < selected_sample->total_sample_count)))
		{
			if (samplePos >= 0)
			{
				outputs[OUT_OUTPUT].setVoltage(5 * selected_sample->playBuffer[floor(samplePos)]);
			}
			else
			{
				outputs[OUT_OUTPUT].setVoltage(5 * selected_sample->playBuffer[floor(selected_sample->total_sample_count - 1 + samplePos)]);
			}
			samplePos = samplePos + 1 + (params[PITCH_KNOB].getValue()) / 3;
		}
		else
		{
			run = false;
			outputs[OUT_OUTPUT].setVoltage(0);
		}
	}
};

//
// SampleLabel
//
// Defines a transparent widget for showing the labels next to the sample
// selection buttons.

struct SampleLabel : TransparentWidget
{
	Repeater *module;

	int frame = 0;
	unsigned int sample_number = 0;

	shared_ptr<Font> font;

	SampleLabel()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		std::string file_name = module ? module->samples[sample_number].filename : "load sample";
		std::string to_display = "";

		// Create a short version of the filename.  Essentially truncates the filename if it is too long.
		for (int i=0; i<14; i++) to_display = to_display + file_name[i];

		nvgFontSize(args.vg, 12);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0);
		nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));

		// void nvgTextBox(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end);
		nvgTextBox(args.vg, 5, 5, 700, to_display.c_str(), NULL);
	}
};


struct MenuItemLoadSample : MenuItem
{
	Repeater *rm;
	unsigned int sample_number = 0;

	void onAction(const event::Action &e) override
	{
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
		if (path)
		{
			rm->run = false;
			rm->samples[sample_number].load(path);
			rm->samplePos = 0;
			free(path);
		}
	}
};

//
// create_label_for_sample
//
// This function is simply to avoid a lot of duplicate code when creating the
// labels for the three samples on the front panel.
//

SampleLabel* create_label_for_sample(Repeater* module, int sample_number, Vec position)
{
	SampleLabel *sample_label = new SampleLabel();
	sample_label->box.pos = position;  // position of the widget on the panel
	sample_label->box.size = Vec(110, 30); // bounding box of the widget
	sample_label->module = module;
	sample_label->sample_number = sample_number; // sample number starting at 0

	return(sample_label);
}

struct RepeaterWidget : ModuleWidget
{
	RepeaterWidget(Repeater* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/repeater_front_panel.svg")));

		// Cosmetic rack screws
		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

		// Input and label for the sample speed input
		// addParam(createParam<Trimpot>(Vec(30, 339), module, Repeater::LSPEED_PARAM));

		// Input and label for the trigger input
		float trigger_input_x = 6.0;
		float trigger_input_y = 20.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(trigger_input_x, trigger_input_y)), module, Repeater::TRIG_INPUT));

		// Input, Knob, and Attenuator for the clock division
		float clock_division_x = 6.0;
		float clock_division_y = 42.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(clock_division_x, clock_division_y)), module, Repeater::CLOCK_DIVISION_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(clock_division_x + 16, clock_division_y + 1)), module, Repeater::CLOCK_DIVISION_ATTN_KNOB));
		addParam(createParam<RoundLargeBlackKnob>(mm2px(Vec(clock_division_x + 30, clock_division_y - 3)), module, Repeater::CLOCK_DIVISION_KNOB));

		// Input, Knob, and Attenuator for the position input
		float position_x = 6.0;
		float position_y = 64.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(position_x, position_y)), module, Repeater::POSITION_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(position_x + 16, position_y + 1)), module, Repeater::POSITION_ATTN_KNOB));
		addParam(createParam<RoundLargeBlackKnob>(mm2px(Vec(position_x + 30, position_y - 3)), module, Repeater::POSITION_KNOB));

		// Input, Knob, and Attenuator for the Sample Select
		float sample_select_x = 6.0;
		float sample_select_y = 86.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(sample_select_x, sample_select_y)), module, Repeater::SAMPLE_SELECT_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(sample_select_x + 16, sample_select_y + 1)), module, Repeater::SAMPLE_SELECT_ATTN_KNOB));
		addParam(createParam<RoundLargeBlackKnob>(mm2px(Vec(sample_select_x + 30, sample_select_y - 3)), module, Repeater::SAMPLE_SELECT_KNOB));

		// Input, Knob, and Attenuator for Pitch
		float pitch_x = 6.0;
		float pitch_y = 108.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(pitch_x, pitch_y)), module, Repeater::PITCH_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(pitch_x + 16, pitch_y + 1)), module, Repeater::PITCH_ATTN_KNOB));
		addParam(createParam<RoundLargeBlackKnob>(mm2px(Vec(pitch_x + 30, pitch_y - 3)), module, Repeater::PITCH_KNOB));


		// Sample select leds
		float sample_leds_x = 70;
		float sample_leds_y = 48.0;
		addChild(createLight<MediumLight<GreenLight>>(mm2px(Vec(sample_leds_x, sample_leds_y)), module, Repeater::SAMPLE_SELECT_1_LIGHT));
		addChild(createLight<MediumLight<GreenLight>>(mm2px(Vec(sample_leds_x, sample_leds_y + 8)), module, Repeater::SAMPLE_SELECT_2_LIGHT));
		addChild(createLight<MediumLight<GreenLight>>(mm2px(Vec(sample_leds_x, sample_leds_y + 16)), module, Repeater::SAMPLE_SELECT_3_LIGHT));
		addChild(createLight<MediumLight<GreenLight>>(mm2px(Vec(sample_leds_x, sample_leds_y + 24)), module, Repeater::SAMPLE_SELECT_4_LIGHT));
		addChild(createLight<MediumLight<GreenLight>>(mm2px(Vec(sample_leds_x, sample_leds_y + 32)), module, Repeater::SAMPLE_SELECT_5_LIGHT));

		// Add text labels to show the sample filenames next to the three sample selection buttons
		// These three calls to create_label_for_sample create and return instances of SampleLabel.
		// Note: create_label_for_sample(Repeater* module, int sample_number, int position_x, int position_y)

		int sample_label_x_offset = 6;
		int sample_label_y_offset = 1;
		addChild(create_label_for_sample(module, 0, mm2px(Vec(sample_leds_x + sample_label_x_offset, sample_leds_y + sample_label_y_offset))));
		addChild(create_label_for_sample(module, 1, mm2px(Vec(sample_leds_x + sample_label_x_offset, sample_leds_y + 8 + sample_label_y_offset))));
		addChild(create_label_for_sample(module, 2, mm2px(Vec(sample_leds_x + sample_label_x_offset, sample_leds_y + 16 + sample_label_y_offset))));
		addChild(create_label_for_sample(module, 3, mm2px(Vec(sample_leds_x + sample_label_x_offset, sample_leds_y + 24 + sample_label_y_offset))));
		addChild(create_label_for_sample(module, 4, mm2px(Vec(sample_leds_x + sample_label_x_offset, sample_leds_y + 32 + sample_label_y_offset))));

		addOutput(createOutput<PJ301MPort>(Vec(321, 324), module, Repeater::OUT_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		Repeater *module = dynamic_cast<Repeater*>(this->module);
		assert(module);

		//
		// Add the three "Load Sample.." menu options to the right-click context menu
		//

		menu->addChild(new MenuEntry);

		std::string menu_text = "Load Sample #";

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			MenuItemLoadSample *menu_item_load_sample = new MenuItemLoadSample;
			menu_item_load_sample->sample_number = i;
			menu_item_load_sample->text = menu_text + std::to_string(i+1);
			menu_item_load_sample->rm = module;
			menu->addChild(menu_item_load_sample);
		}
	}

};



Model* modelRepeater = createModel<Repeater, RepeaterWidget>("repeater");
