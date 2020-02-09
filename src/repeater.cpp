//
// Voxglitch "repeater" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"
#include "sample.hpp"
#include "smooth.hpp"

#define NUMBER_OF_SAMPLES 5
#define NUMBER_OF_SAMPLES_FLOAT 5.0
#define GAIN 5.0

struct Repeater : Module
{
	unsigned int selected_sample_slot = 0;
	float samplePos = 0;
	int step = 0;
	bool isPlaying = false;
	Smooth smooth;
	int retrigger;
	std::string root_dir;

	// When this flag is flase, the display area on the front panel will
	// display something like, "Right-click to Load.."
	bool any_sample_has_been_loaded = false;

	// When this flag is flase, the display area will yell at the user to
	// provide a clock signal.
	bool trig_input_is_connected = false;

	Sample samples[NUMBER_OF_SAMPLES];
	std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

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
		configParam(CLOCK_DIVISION_KNOB, 0.0f, 1.0f, 0.0f, "ClockDivisionKnob");
		configParam(CLOCK_DIVISION_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "ClockDivisionAttnKnob");
		configParam(POSITION_KNOB, 0.0f, 1.0f, 0.0f, "PositionKnob");
		configParam(POSITION_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "PositionAttnKnob");
		configParam(SAMPLE_SELECT_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
		configParam(SAMPLE_SELECT_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");
		configParam(SMOOTH_SWITCH, 0.f, 1.f, 1.f, "Smooth");

		std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");
	}

	// Autosave module data.  VCV Rack decides when this should be called.
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

	// Load module data
	void dataFromJson(json_t *rootJ) override
	{
		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_sample_path = json_object_get(rootJ, ("loaded_sample_path_" +  std::to_string(i+1)).c_str());
			if (loaded_sample_path)
			{
				samples[i].load(json_string_value(loaded_sample_path));
				loaded_filenames[i] = samples[i].filename;
				this->any_sample_has_been_loaded = true;
			}

			json_t* retrigger_json = json_object_get(rootJ, "retrigger");
			if (retrigger_json) retrigger = json_integer_value(retrigger_json);
		}
	}

	// TODO: Eventually share this code instead of duplicating it
	float calculate_inputs(int input_index, int knob_index, int attenuator_index, float scale)
	{
		float input_value = inputs[input_index].getVoltage() / 10.0;
		float knob_value = params[knob_index].getValue();
		float attenuator_value = params[attenuator_index].getValue();

		return(((input_value * scale) * attenuator_value) + (knob_value * scale));
	}

	//
	// Main module processing loop.  This runs at whatever samplerate is selected within VCV Rack.
	//

	void process(const ProcessArgs &args) override
	{
		bool trigger_output_pulse = false;

		unsigned int sample_select_input_value = calculate_inputs(SAMPLE_SELECT_INPUT, SAMPLE_SELECT_KNOB, SAMPLE_SELECT_ATTN_KNOB, NUMBER_OF_SAMPLES_FLOAT);
		sample_select_input_value = clamp(sample_select_input_value, 0, NUMBER_OF_SAMPLES - 1);

		if(sample_select_input_value != selected_sample_slot)
		{
			// Start smoothing if the selected sample has changed
			smooth.trigger();

			// Set the selected sample
			selected_sample_slot = sample_select_input_value;
		}

		// This is just for convenience
		Sample *selected_sample = &samples[selected_sample_slot];

		if (inputs[TRIG_INPUT].isConnected())
		{
			trig_input_is_connected = true;

			//
			// playTrigger is a SchmittTrigger object.  Here, the SchmittTrigger
			// determines if a low-to-high transition happened at the TRIG_INPUT
			//
			if (playTrigger.process(inputs[TRIG_INPUT].getVoltage()))
			{
				float clock_division = calculate_inputs(CLOCK_DIVISION_INPUT, CLOCK_DIVISION_KNOB, CLOCK_DIVISION_ATTN_KNOB, 10);
				clock_division = clamp(clock_division, 0.0, 10.0);

				step += 1;
				if(step > clock_division) step = 0;

				if(step == 0)
				{
					isPlaying = true;
					samplePos = calculate_inputs(POSITION_INPUT, POSITION_KNOB, POSITION_ATTN_KNOB, selected_sample->total_sample_count);
					smooth.trigger();
					triggerOutputPulse.trigger(0.01f);
				}
			}
		}
		else
		{
			trig_input_is_connected = false;
		}

		// If the sample has completed playing and the retrigger option is true,
		// then restart sample playback.

		if(retrigger && abs(floor(samplePos)) >= selected_sample->total_sample_count)
		{
			samplePos = 0;
			smooth.trigger();
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

			// Apply smoothing
			if(params[SMOOTH_SWITCH].getValue()) wav_output_voltage = smooth.process(wav_output_voltage, (128.0f / args.sampleRate));

			// Output voltage
			outputs[WAV_OUTPUT].setVoltage(wav_output_voltage);

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
	std::shared_ptr<Font> font;

	Readout()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		std::string text_to_display = "load sample";

		if(module)
		{
			if(module->any_sample_has_been_loaded == true)
			{
				text_to_display = "#" + std::to_string(module->selected_sample_slot + 1) + ":" + module->samples[module->selected_sample_slot].filename;
				text_to_display.resize(30); // Truncate long text to fit in the display
			}
			else
			{
				text_to_display = "Right-click to load samples";
			}

			if(module->trig_input_is_connected == false)
			{
				text_to_display = "Connect a clock signal to CLK";
			}
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
	Repeater *module;
	unsigned int sample_number = 0;

	void onAction(const event::Action &e) override
	{
		const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

		if (path)
		{
			module->any_sample_has_been_loaded = true;
			module->samples[sample_number].load(path);
			module->root_dir = std::string(path);
			module->loaded_filenames[sample_number] = module->samples[sample_number].filename;
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

		menu->addChild(new MenuEntry); // For spacing only
		menu->addChild(createMenuLabel("Samples"));

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			MenuItemLoadSample *menu_item_load_sample = new MenuItemLoadSample;
			menu_item_load_sample->sample_number = i;
			menu_item_load_sample->text = std::to_string(i+1) + ": " + module->loaded_filenames[i];
			menu_item_load_sample->module = module;
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
