//
// Voxglitch "Autobreak" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"
#include "sample.hpp"

#define GAIN 5.0
#define NUMBER_OF_PATTERNS 6
#define NUMBER_OF_SAMPLES 5
#define NUMBER_OF_SAMPLES_FLOAT 5.0

struct Autobreak : Module
{
	unsigned int selected_sample_slot = 0;
	float old_actual_playback_position = 0;

	// Actual index into the sample's array for playback
	float actual_playback_position = 0;

	// A location in a theoretical sample that's 8 bars at the selected BPM.
	// This value is stepped and repositioned when jumping around in a breakbeat.
	// This value is then used to figure out the actual_playback_position based
	// on the sample's length.

	float theoretical_playback_position = 0;

	// incrementing_bpm_counter counts from 0 to the number of samples required
	// to play a theoretical sample for 8 bars at a specific bpm.
	unsigned int incrementing_bpm_counter = 0;

	unsigned int bpm = 160;
	int step = 0;
	unsigned int break_pattern_index = 0;
	unsigned int break_pattern_index_old = 0;
	int clock_counter = 0;
	int clock_counter_old = 0;
	unsigned int selected_break_pattern = 0;
	float smooth_ramp = 1;
	float last_wave_output_voltage = 0;
	std::string root_dir;
	std::string path;

	Sample samples[NUMBER_OF_SAMPLES];
	std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

	dsp::SchmittTrigger playTrigger;
	dsp::PulseGenerator clockOutputPulse;
	dsp::PulseGenerator endOutputPulse;

	int break_patterns[NUMBER_OF_PATTERNS][16] = {
		{  0,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1 },
		{  0,-1,-1,-1,  0,-1,-1,-1, -1,-1,-1,-1,  8,-1,-1,-1 },
		{  0,-1,-1,-1,  0,-1,-1,-1, -1,-1,-1,-1,  0,-1,-1,-1 },
		{  0,-1, 0,-1, -1,-1,-1, 4,  6,-1,-1,-1,  6,-1,-1,-1 },
		{  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 },
		{  0,14,12,12,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 12 }
	};

	enum ParamIds {
		WAV_KNOB,
		WAV_ATTN_KNOB,
		PATTERN_KNOB,
		PATTERN_ATTN_KNOB,
		BPM_KNOB,
		NUM_PARAMS
	};
	enum InputIds {
		RESET_INPUT,
		WAV_INPUT,
		PATTERN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		WAV_OUTPUT,
		DEBUG_OUTPUT,
		CLOCK_OUTPUT,
		END_OUTPUT,
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
		configParam(PATTERN_KNOB, 0.0f, 1.0f, 0.0f, "PatternSelectKnob");
		configParam(PATTERN_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "PatternSelectAttnKnob");
		configParam(BPM_KNOB, 50.0f, 200.0f, 120.0f, "PatternSelectKnob");

		std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();
		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(rootJ, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(samples[i].path.c_str()));
		}
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_sample_path = json_object_get(rootJ, ("loaded_sample_path_" +  std::to_string(i+1)).c_str());
			if (loaded_sample_path)
			{
				samples[i].load(json_string_value(loaded_sample_path));
				loaded_filenames[i] = samples[i].filename;
			}
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
		// unsigned int wav_input_value = (unsigned int) floor(NUMBER_OF_SAMPLES * (((inputs[WAV_INPUT].getVoltage() / 10.0) * params[WAV_ATTN_KNOB].getValue()) + params[WAV_KNOB].getValue()));
		unsigned int wav_input_value = calculate_inputs(WAV_INPUT, WAV_KNOB, WAV_ATTN_KNOB, NUMBER_OF_SAMPLES);
		wav_input_value = clamp(wav_input_value, 0, NUMBER_OF_SAMPLES - 1);

		bpm = params[BPM_KNOB].getValue();

		if(wav_input_value != selected_sample_slot)
		{
			// Reset the smooth ramp if the selected sample has changed
			smooth_ramp = 0;

			// Set the selected sample
			selected_sample_slot = wav_input_value;
		}

		Sample *selected_sample = &samples[selected_sample_slot];

		if (inputs[RESET_INPUT].isConnected())
		{
			//
			// playTrigger is a SchmittTrigger object.  Here, the SchmittTrigger
			// determines if a low-to-high transition happened at the RESET_INPUT
			//
			if (playTrigger.process(inputs[RESET_INPUT].getVoltage()))
			{
				actual_playback_position = 0;
				theoretical_playback_position = 0;
				incrementing_bpm_counter = 0;
				clock_counter = 0;
				clock_counter_old = -1;
				break_pattern_index = 0;
				break_pattern_index_old = 0;
				smooth_ramp = 0;
			}
		}

		if (selected_sample->loaded && (selected_sample->total_sample_count > 0))
		{
			// 60.0 is for conversion from minutes to seconds
			// 8.0 is for 8 beats (2 bars) of loops, which is a typical drum loop length
			float samples_to_play_per_loop = ((60.0 / (float) bpm) * args.sampleRate) * 8.0;

			actual_playback_position = clamp(actual_playback_position, 0.0, selected_sample->total_sample_count - 1);
			float wav_output_voltage = GAIN  * selected_sample->leftPlayBuffer[(int)actual_playback_position];

			if(smooth_ramp < 1) // temporarily removed
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

			// Update a clock counter and provide a trigger output whenever 1/32nd of the sample has been reached.
			clock_counter = (incrementing_bpm_counter / samples_to_play_per_loop) * 32.0f;
			if(clock_counter != clock_counter_old) clockOutputPulse.trigger(0.01f);
			clock_counter_old = clock_counter;

			// Step the theoretical playback position
			theoretical_playback_position = theoretical_playback_position + 1;

			//
			// Optionally jump to new breakbeat position
			//

			break_pattern_index = (incrementing_bpm_counter / samples_to_play_per_loop) * 16.0f;
			break_pattern_index = clamp(break_pattern_index, 0, 15);

			if(break_pattern_index != break_pattern_index_old)
			{
				selected_break_pattern = calculate_inputs(PATTERN_INPUT, PATTERN_KNOB, PATTERN_ATTN_KNOB, NUMBER_OF_PATTERNS - 1);
				selected_break_pattern = clamp(selected_break_pattern, 0, NUMBER_OF_PATTERNS - 1);

				float new_step = break_patterns[selected_break_pattern][break_pattern_index];

				if(new_step != -1)
				{
					theoretical_playback_position = new_step * (samples_to_play_per_loop / 16.0f);
				}

				break_pattern_index_old = break_pattern_index;
			}

			// Loop the theoretical_playback_position
			if(theoretical_playback_position >= samples_to_play_per_loop)
			{
				theoretical_playback_position = 0;
				smooth_ramp = 0;
			}

			// Map the theoretical playback position to the actual sample playback position
			actual_playback_position = ((float) theoretical_playback_position / samples_to_play_per_loop) * selected_sample->total_sample_count;

			//
			// Increment the bpm counter, which goes from 0 to the number of samples
			// needed to play an entire loop of a theoretical sample at the bpm specified.
			incrementing_bpm_counter = incrementing_bpm_counter + 1;
			if(incrementing_bpm_counter > samples_to_play_per_loop)
			{
				incrementing_bpm_counter = 0;
				endOutputPulse.trigger(0.01f);
			}

			outputs[DEBUG_OUTPUT].setVoltage(break_pattern_index);
		}
		else
		{
			outputs[WAV_OUTPUT].setVoltage(0);
		}

		//
		// Process pulse outputs
		//

		bool clock_output_pulse = clockOutputPulse.process(1.0 / args.sampleRate);
		outputs[CLOCK_OUTPUT].setVoltage((clock_output_pulse ? 10.0f : 0.0f));

		bool end_output_pulse = endOutputPulse.process(1.0 / args.sampleRate);
		outputs[END_OUTPUT].setVoltage((end_output_pulse ? 10.0f : 0.0f));
	}
};

struct AutobreakReadout : TransparentWidget
{
	Autobreak *module;

	std::shared_ptr<Font> font;

	AutobreakReadout()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		if(module)
		{
			Sample *selected_sample = &module->samples[module->selected_sample_slot];
			std::string text_to_display;

			if(selected_sample->loaded)
			{
				Sample *selected_sample = &module->samples[module->selected_sample_slot];
				std::string text_to_display = selected_sample->filename;

				// Display filename or "load sample" in the display area
				nvgFontSize(args.vg, 13);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
				nvgTextBox(args.vg, 5, 5, 700, text_to_display.c_str(), NULL);

				// Draw a line underneath the text to show where, in the sample, playback is happening
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, 5, 12);
				nvgLineTo(args.vg, 200.0 * (module->actual_playback_position/selected_sample->total_sample_count), 12);
				nvgStrokeColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
				nvgStroke(args.vg);
			}
			else
			{
				// If no file is loaded in the slot, then display "Right click to load sample" in the display area
				nvgFontSize(args.vg, 13);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
				nvgTextBox(args.vg, 5, 5, 700, "Right click to load sample", NULL);
			}
		}
		else
		{
			// Display a fake filename in the display area so people understand
			// what this module does when viewing the front panel in the library
			nvgFontSize(args.vg, 13);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);
			nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
			nvgTextBox(args.vg, 5, 5, 700, "drum_loop.wav", NULL);
		}

		nvgRestore(args.vg);
	}
};

struct AutobreakPatternReadout : TransparentWidget
{
	Autobreak *module;

	std::shared_ptr<Font> font;

	AutobreakPatternReadout()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		if(module)
		{
			std::string text_to_display = "";
			std::string item_display;

			// Configure the font size, face, color, etc.
			nvgFontSize(args.vg, 13);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);
			nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
			nvgTextLetterSpacing(args.vg, -2);

			//
			// Display the pattern
			//

			for(unsigned int i=0; i<16; i++)
			{
				item_display = std::to_string(module->break_patterns[module->selected_break_pattern][i]);
				if(item_display == "-1") item_display = ".";

				// Draw inverted text if it's the selected index
				if(i == module->break_pattern_index)
				{
					// Draw background while rectangle
					nvgBeginPath(args.vg);
					nvgRect(args.vg, (i * 13), -8, 10, 16);
					nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
					if(item_display == ".") nvgFillColor(args.vg, nvgRGBA(100, 100, 100, 0xff));
					nvgFill(args.vg);

					// Draw forground text in black
					nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 0xff));
				}
				// Otherwise just draw while text on the black background
				else
				{
					nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
				}

				nvgText(args.vg, 5 + (i * 13), 5, item_display.c_str(), NULL);
			}
		}

		nvgRestore(args.vg);
	}
};

struct AutobreakBPMDislplay : TransparentWidget
{
	Autobreak *module;
	std::shared_ptr<Font> font;

	AutobreakBPMDislplay()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		// Configure the font size, face, color, etc.
		nvgFontSize(args.vg, 13);
		nvgFontFaceId(args.vg, font->handle);
		nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgTextLetterSpacing(args.vg, -1);

		if(module)
		{
			std::string bpm_string = std::to_string(module->bpm);
			nvgText(args.vg, 5, 0, bpm_string.c_str(), NULL);
		}
		else
		{
			nvgText(args.vg, 5, 0, "120", NULL);
		}

		nvgRestore(args.vg);
	}
};

struct AutobreakLoadSample : MenuItem
{
	Autobreak *module;
	unsigned int sample_number = 0;

	void onAction(const event::Action &e) override
	{
		const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

		if (path)
		{
			module->samples[sample_number].load(path);
			module->root_dir = std::string(path);
			module->loaded_filenames[sample_number] = module->samples[sample_number].filename;
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

		// Input and label for the reset input
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, 114.893)), module, Autobreak::RESET_INPUT));

		// Inputs for WAV selection
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, 43)), module, Autobreak::WAV_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 43)), module, Autobreak::WAV_ATTN_KNOB));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(44, 43)), module, Autobreak::WAV_KNOB));

		// Pattern Select
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, 84.567)), module, Autobreak::PATTERN_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 84.567)), module, Autobreak::PATTERN_ATTN_KNOB));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(44, 84.567)), module, Autobreak::PATTERN_KNOB));

		// BPM selection
		addParam(createParamCentered<Trimpot>(mm2px(Vec(78, 43)), module, Autobreak::BPM_KNOB));

		AutobreakReadout *readout = new AutobreakReadout();
		readout->box.pos = mm2px(Vec(11.0, 26.0));
		readout->box.size = Vec(110, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		AutobreakPatternReadout *pattern_readout = new AutobreakPatternReadout();
		pattern_readout->box.pos = mm2px(Vec(11.0, 68.591));
		pattern_readout->box.size = Vec(110, 30); // bounding box of the widget
		pattern_readout->module = module;
		addChild(pattern_readout);

		AutobreakBPMDislplay *bpm_display = new AutobreakBPMDislplay();
		bpm_display->box.pos = mm2px(Vec(64.8, 44.2));
		// bpm_display->box.size = Vec(20, 30); // bounding box of the widget
		bpm_display->module = module;
		addChild(bpm_display);

		// Outputs
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 100.893)), module, Autobreak::DEBUG_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(40.264, 114.893)), module, Autobreak::CLOCK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(58.832, 114.893)), module, Autobreak::END_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(77.418, 114.893)), module, Autobreak::WAV_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		Autobreak *module = dynamic_cast<Autobreak*>(this->module);
		assert(module);

		menu->addChild(new MenuEntry); // For spacing only
		menu->addChild(createMenuLabel("Samples"));

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		std::string menu_text = "#";

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			AutobreakLoadSample *menu_item_load_sample = new AutobreakLoadSample;
			menu_item_load_sample->sample_number = i;
			menu_item_load_sample->text = std::to_string(i+1) + ": " + module->loaded_filenames[i];
			menu_item_load_sample->module = module;
			menu->addChild(menu_item_load_sample);
		}
	}

};



Model* modelAutobreak = createModel<Autobreak, AutobreakWidget>("autobreak");
