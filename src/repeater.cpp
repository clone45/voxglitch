//
// Voxglitch "repeater" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"
#include "sample.hpp"

#define NUMBER_OF_SAMPLES 5
#define NUMBER_OF_SAMPLES_FLOAT 5.0
#define GAIN 5.0

struct Repeater : Module
{
	unsigned int selected_sample_slot = 0;
	float samplePos = 0;
	int step = 0;
	bool isPlaying = false;
	float smooth_ramp = 1;
	float last_wave_output_voltage = 0;
	int retrigger;
	std::string root_dir;

	Sample samples[NUMBER_OF_SAMPLES];
	dsp::SchmittTrigger playTrigger;
	dsp::PulseGenerator triggerOutputPulse;

	enum ParamIds {
		CLOCK_DIVISION_KNOB,
		CLOCK_DIVISION_ATTN_KNOB,
		POSITION_KNOB,
		POSITION_ATTN_KNOB,
		SAMPLE_SELECT_KNOB,
		SAMPLE_SELECT_ATTN_KNOB,
		PITCH_KNOB,
		PITCH_ATTN_KNOB,
		SMOOTH_SWITCH,
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
		configParam(SMOOTH_SWITCH, 0.f, 1.f, 1.f, "Smooth");
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(rootJ, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(samples[i].path.c_str()));
		}

		json_object_set_new(rootJ, "retrigger", json_integer(retrigger));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_sample_path = json_object_get(rootJ, ("loaded_sample_path_" +  std::to_string(i+1)).c_str());
			if (loaded_sample_path) samples[i].load(json_string_value(loaded_sample_path));

			json_t* retrigger_json = json_object_get(rootJ, "retrigger");
			if (retrigger_json) retrigger = json_integer_value(retrigger_json);
		}
	}

	void process(const ProcessArgs &args) override
	{
		bool trigger_output_pulse = false;

		// TODO: Remove expensive floor() call
		unsigned int sample_select_input_value = (unsigned int) floor(NUMBER_OF_SAMPLES_FLOAT * (((inputs[SAMPLE_SELECT_INPUT].getVoltage() / 10.0) * params[SAMPLE_SELECT_ATTN_KNOB].getValue()) + params[SAMPLE_SELECT_KNOB].getValue()));

		if(sample_select_input_value >= NUMBER_OF_SAMPLES) sample_select_input_value = NUMBER_OF_SAMPLES - 1;

		if(sample_select_input_value != selected_sample_slot)
		{
			// Reset the smooth ramp if the selected sample has changed
			smooth_ramp = 0;

			// Set the selected sample
			selected_sample_slot = sample_select_input_value;
		}

		Sample *selected_sample = &samples[selected_sample_slot];

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
					isPlaying = true;
					samplePos = selected_sample->total_sample_count * (((inputs[POSITION_INPUT].getVoltage() / 10.0) * params[POSITION_ATTN_KNOB].getValue()) + params[POSITION_KNOB].getValue());
					smooth_ramp = 0;
					triggerOutputPulse.trigger(0.01f);
				}
			}
		}

		// If the sample has completed playing and the retrigger option is true,
		// then restart sample playback.

		if(retrigger && abs(floor(samplePos)) >= selected_sample->total_sample_count)
		{
			samplePos = 0;
			smooth_ramp = 0;
		}

		if (isPlaying && (! selected_sample->loading) && (selected_sample->loaded) && (selected_sample->total_sample_count > 0) && ((abs(floor(samplePos)) < selected_sample->total_sample_count)))
		{
			float wav_output_voltage;

			if (samplePos >= 0)
			{
				wav_output_voltage = GAIN  * selected_sample->leftPlayBuffer[floor(samplePos)];
			}
			else
			{
				// What is this for?  Does it play the sample in reverse?  I think so.
				wav_output_voltage = GAIN * selected_sample->leftPlayBuffer[floor(selected_sample->total_sample_count - 1 + samplePos)];
			}

			if(params[SMOOTH_SWITCH].getValue()  && (smooth_ramp < 1))
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
			isPlaying = false;
			outputs[WAV_OUTPUT].setVoltage(0);
		}

		trigger_output_pulse = triggerOutputPulse.process(1.0 / args.sampleRate);
		outputs[TRG_OUTPUT].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
	}
};

struct Readout : TransparentWidget
{
	Repeater *module;

	int frame = 0;
	std::shared_ptr<Font> font;

	Readout()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		std::string text_to_display = "load sample";;

		if(module)
		{
			text_to_display = "#" + std::to_string(module->selected_sample_slot + 1) + ":" + module->samples[module->selected_sample_slot].filename;
			text_to_display.resize(30); // Truncate long text to fit in the display
		}

		nvgFontSize(args.vg, 13);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0);
		nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));

		// void nvgTextBox(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end);
		nvgTextBox(args.vg, 5, 5, 700, text_to_display.c_str(), NULL);

		nvgRestore(args.vg);
	}

};


struct MenuItemLoadSample : MenuItem
{
	Repeater *repeater_module;
	unsigned int sample_number = 0;

	void onAction(const event::Action &e) override
	{

		const std::string dir = repeater_module->root_dir.empty() ? "" : repeater_module->root_dir;
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

		if (path)
		{
			repeater_module->samples[sample_number].load(path);
			repeater_module->root_dir = std::string(path);
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

		// Input and label for the trigger input (which is labeled "CLK" on the front panel)
		float trigger_input_x = 67.7;
		float trigger_input_y = 42.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(trigger_input_x, trigger_input_y)), module, Repeater::TRIG_INPUT));

		// Input, Knob, and Attenuator for the clock division
		float clock_division_x = 7.0;
		float clock_division_y = 42.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(clock_division_x, clock_division_y)), module, Repeater::CLOCK_DIVISION_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(clock_division_x + 16, clock_division_y + 1)), module, Repeater::CLOCK_DIVISION_ATTN_KNOB));
		addParam(createParam<RoundLargeBlackKnob>(mm2px(Vec(clock_division_x + 30, clock_division_y - 3)), module, Repeater::CLOCK_DIVISION_KNOB));

		// Input, Knob, and Attenuator for the position input
		float position_x = 7.0;
		float position_y = 64.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(position_x, position_y)), module, Repeater::POSITION_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(position_x + 16, position_y + 1)), module, Repeater::POSITION_ATTN_KNOB));
		addParam(createParam<RoundLargeBlackKnob>(mm2px(Vec(position_x + 30, position_y - 3)), module, Repeater::POSITION_KNOB));

		// Input, Knob, and Attenuator for the Sample Select
		float sample_select_x = 7.0;
		float sample_select_y = 86.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(sample_select_x, sample_select_y)), module, Repeater::SAMPLE_SELECT_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(sample_select_x + 16, sample_select_y + 1)), module, Repeater::SAMPLE_SELECT_ATTN_KNOB));
		addParam(createParam<RoundLargeBlackKnob>(mm2px(Vec(sample_select_x + 30, sample_select_y - 3)), module, Repeater::SAMPLE_SELECT_KNOB));

		// Input, Knob, and Attenuator for Pitch
		float pitch_x = 7.0;
		float pitch_y = 108.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(pitch_x, pitch_y)), module, Repeater::PITCH_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(pitch_x + 16, pitch_y + 1)), module, Repeater::PITCH_ATTN_KNOB));
		addParam(createParam<RoundLargeBlackKnob>(mm2px(Vec(pitch_x + 30, pitch_y - 3)), module, Repeater::PITCH_KNOB));

		// Smooth
		addParam(createParam<CKSS>(Vec(205, 190), module, Repeater::SMOOTH_SWITCH));

		Readout *readout = new Readout();
		readout->box.pos = mm2px(Vec(11.0, 22.0)); //22,22
		readout->box.size = Vec(110, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		// Outputs
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

		menu->addChild(new MenuEntry); // For spacing only

		std::string menu_text = "Load Sample #";

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			MenuItemLoadSample *menu_item_load_sample = new MenuItemLoadSample;
			menu_item_load_sample->sample_number = i;
			menu_item_load_sample->text = menu_text + std::to_string(i+1);
			menu_item_load_sample->repeater_module = module;
			menu->addChild(menu_item_load_sample);
		}

		//
		// Options
		// =====================================================================

		menu->addChild(new MenuEntry);
		menu->addChild(createMenuLabel("Options"));

		// Retrigger option

		struct RetriggerMenuItem : MenuItem {
			Repeater* module;
			void onAction(const event::Action& e) override {
				module->retrigger = !(module->retrigger);
			}
		};

		RetriggerMenuItem* retrigger_menu_item = createMenuItem<RetriggerMenuItem>("Retrigger");
		retrigger_menu_item->rightText = CHECKMARK(module->retrigger == 1);
		retrigger_menu_item->module = module;
		menu->addChild(retrigger_menu_item);
	}

};



Model* modelRepeater = createModel<Repeater, RepeaterWidget>("repeater");
