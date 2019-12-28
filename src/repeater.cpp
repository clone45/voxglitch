//
// Voxglitch "repeater" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <vector>
#include "cmath"

using namespace std;

#define NUMBER_OF_SAMPLES 5
#define NUMBER_OF_SAMPLES_FLOAT 5.0

struct sample
{
	std::string path;
	std::string filename;
	drwav_uint64 total_sample_count;
	bool loading;
	vector<float> playBuffer;
	unsigned int sample_rate;
	unsigned int channels;
	bool run = false;

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
		}
		else
		{
			this->loading = false;
		}
	};
};


struct Repeater : Module
{
	unsigned int selected_sample_slot = 0;
	bool led_states[NUMBER_OF_SAMPLES];
	float samplePos = 0;
	int step = 0;

	sample samples[NUMBER_OF_SAMPLES];
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
		WAV_OUTPUT,
		TRG_OUTPUT,
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
		configParam(PITCH_KNOB, -1.0f, 1.0f, 0.0f, "PitchKnob");
		configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "PitchAttnKnob");
		configParam(CLOCK_DIVISION_KNOB, 0.0f, 10.0f, 0.0f, "ClockDivisionKnob");
		configParam(CLOCK_DIVISION_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "ClockDivisionAttnKnob");
		configParam(POSITION_KNOB, 0.0f, 1.0f, 0.0f, "PositionKnob");
		configParam(POSITION_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "PositionAttnKnob");
		configParam(SAMPLE_SELECT_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
		configParam(SAMPLE_SELECT_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");
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

	void process(const ProcessArgs &args) override
	{
		unsigned int sample_select_input_value = (unsigned int) floor(NUMBER_OF_SAMPLES_FLOAT * (((inputs[SAMPLE_SELECT_INPUT].getVoltage() / 10.0) * params[SAMPLE_SELECT_ATTN_KNOB].getValue()) + params[SAMPLE_SELECT_KNOB].getValue()));

		if(sample_select_input_value >= NUMBER_OF_SAMPLES) sample_select_input_value = NUMBER_OF_SAMPLES - 1;

		if(selected_sample_slot != sample_select_input_value)
		{
			selected_sample_slot = sample_select_input_value;
		}

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
					selected_sample->run = true;
					samplePos = selected_sample->total_sample_count * (((inputs[POSITION_INPUT].getVoltage() / 10.0) * params[POSITION_ATTN_KNOB].getValue()) + params[POSITION_KNOB].getValue());
				}
			}
		}

		if ((! selected_sample->loading) && (selected_sample->run) && (selected_sample->total_sample_count > 0) && ((abs(floor(samplePos)) < selected_sample->total_sample_count)))
		{
			if (samplePos >= 0)
			{
				outputs[WAV_OUTPUT].setVoltage(5 * selected_sample->playBuffer[floor(samplePos)]);
			}
			else
			{
				outputs[WAV_OUTPUT].setVoltage(5 * selected_sample->playBuffer[floor(selected_sample->total_sample_count - 1 + samplePos)]);
			}

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

struct Readout : TransparentWidget
{
	Repeater *module;

	int frame = 0;
	// unsigned int sample_number = 0;

	shared_ptr<Font> font;

	Readout()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		std::string to_display;

		if(! module)
		{
			to_display = "load sample";
		}
		else
		{
			std::string file_name = module->samples[module->selected_sample_slot].filename;
			to_display = "#" + std::to_string(module->selected_sample_slot + 1) + ":";

			// Create a short version of the filename.  Essentially truncates the filename if it is too long.
			for (int i=0; i<20; i++) to_display = to_display + file_name[i];
		}

		nvgFontSize(args.vg, 13);
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
			rm->samples[sample_number].load(path);
			rm->samplePos = 0;
			free(path);
		}
	}
};


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

		// Input and label for the trigger input (which is labeled "CLK" on the front panel)
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

		Readout *readout = new Readout();
		readout->box.pos = mm2px(Vec(22.0, 22.0));
		readout->box.size = Vec(110, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		addOutput(createOutput<PJ301MPort>(Vec(200, 324), module, Repeater::WAV_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(200, 259), module, Repeater::TRG_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		Repeater *module = dynamic_cast<Repeater*>(this->module);
		assert(module);

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
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
