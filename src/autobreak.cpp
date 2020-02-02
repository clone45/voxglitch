//
// Voxglitch "Autobreak" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"
#include "sample.hpp"
#include <fstream>

#define GAIN 5.0
#define NUMBER_OF_PATTERNS 16
#define NUMBER_OF_SAMPLES 5
#define DRAW_AREA_WIDTH 348.0
#define DRAW_AREA_HEIGHT 242.0
#define BAR_WIDTH 21.0
#define BAR_HORIZONTAL_PADDING .8
#define SAMPLE_OFFSET_INDICATION_LINE_WIDTH 356.0

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

	// Pattern lock is a flag which, when true, stops the pattern from changing
	// due to changes in the pattern knob or the CV input.  This flag is set
	// to true when the user is actively editing the current pattern.
 	bool pattern_lock = false;

	unsigned int bpm = 160;
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

		// No breaks
		{  0,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1 },

		// Jump backwards on beat 13 and variations
		{  0,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,  4,-1,-1,-1 },
		{  0,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, 10,-1,-1,-1 },
		{  0,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,  0,-1,-1,-1 },

		// Jump backwards more frenetic
		{  0,-1,-1,-1, -1, 0,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1 },
		{  0,-1,-1,-1, -1,-1, 4,-1,  8,-1,-1,-1,  4,-1,-1,-1 },
		{  0,-1, 0,-1,  4,-1,-1,-1, -1,-1,-1,-1, 10,-1,-1,-1 },
		{  0,-1,-1,-1,  0,-1,-1,-1, -1,-1,-1,-1,  0,-1, 3, 3 },

		// Jump back early on beat 5 with variations
		{  0,-1,-1,-1, 0,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1 },
		{  0,-1,-1,-1, 0,-1,-1,-1,  8,-1,-1,-1, -1,-1,-1,-1 },
		{  0,-1,-1,-1, 0,-1,-1,-1, -1,-1,-1,-1, 12,-1,-1,-1 },
		{  0,-1,-1,-1, 0,-1, 6,-1, -1,-1,-1,-1, -1,-1,-1,-1 },

		// Jump back early on beat 5 - busy
		{  0,-1,-1,-1, 0,-1, 0,-1,  4,-1,-1,-1,  2,-1,12,-1 },
		{  0,-1,-1,-1, 0,-1,-1,-1,  8,-1,-1,-1, -1,-1,-1,-1 },
		{  0,-1,-1,-1, 0,-1,-1,-1,  6,-1,-1,-1, 12,-1,12,-1 },
		{  0,-1, 0,-1, -1,-1, 6,-1, -1,-1,-1,-1, -1,-1,-1,-1 },
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

	// Autosave settings
	json_t *dataToJson() override
	{
		//
		// Save selected samples
		//

		json_t *json_root = json_object();
		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(json_root, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(samples[i].path.c_str()));
		}

		//
		// Save patterns
		//

		json_t *break_patterns_json_array = json_array();

		for(int pattern_number=0; pattern_number<NUMBER_OF_PATTERNS; pattern_number++)
		{
			json_t *pattern_json_array = json_array();

			for(int i=0; i<16; i++)
			{
				json_array_append_new(pattern_json_array, json_integer(this->break_patterns[pattern_number][i]));
			}

            json_array_append_new(break_patterns_json_array, pattern_json_array);
        }

        json_object_set(json_root, "patterns", break_patterns_json_array);
        json_decref(break_patterns_json_array);

		return json_root;
	}

	// Autoload settings
	void dataFromJson(json_t *json_root) override
	{
		//
		// Load samples
		//

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_sample_path = json_object_get(json_root, ("loaded_sample_path_" +  std::to_string(i+1)).c_str());
			if (loaded_sample_path)
			{
				samples[i].load(json_string_value(loaded_sample_path));
				loaded_filenames[i] = samples[i].filename;
			}
		}

		//
		// Load patterns
		//

		json_t *pattern_arrays_data = json_object_get(json_root, "patterns");

		if(pattern_arrays_data)
		{
			size_t pattern_number;
			json_t *json_pattern_array;

			json_array_foreach(pattern_arrays_data, pattern_number, json_pattern_array)
			{
				for(int i=0; i<NUMBER_OF_PATTERNS; i++)
				{
					this->break_patterns[pattern_number][i] = json_integer_value(json_array_get(json_pattern_array, i));
				}
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

			//
			// Handle smoothing
			//

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

			// Update a clock counter and provide a trigger output whenever 1/32nd of the sample has been reached.
			clock_counter = (incrementing_bpm_counter / samples_to_play_per_loop) * 32.0f;
			if(clock_counter != clock_counter_old) clockOutputPulse.trigger(0.01f);
			clock_counter_old = clock_counter;

			// Step the theoretical playback position
			theoretical_playback_position = theoretical_playback_position + 1;

			// Select the break pattern
			if(pattern_lock == false)
			{
				selected_break_pattern = calculate_inputs(PATTERN_INPUT, PATTERN_KNOB, PATTERN_ATTN_KNOB, NUMBER_OF_PATTERNS - 1);
				selected_break_pattern = clamp(selected_break_pattern, 0, NUMBER_OF_PATTERNS - 1);
			}

			//
			// Optionally jump to new breakbeat position
			//

			break_pattern_index = (incrementing_bpm_counter / samples_to_play_per_loop) * 16.0f;
			break_pattern_index = clamp(break_pattern_index, 0, 15);

			if(break_pattern_index != break_pattern_index_old)
			{
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

			// outputs[DEBUG_OUTPUT].setVoltage(theoretical_playback_position);

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
				nvgLineTo(args.vg, SAMPLE_OFFSET_INDICATION_LINE_WIDTH * (module->actual_playback_position/selected_sample->total_sample_count), 12);
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
	Vec drag_position;

	AutobreakPatternReadout()
	{
		// The bounding box needs to be a little deeper than the visual
		// controls to allow mouse drags to indicate '0' (off) column heights,
		// which is why 16 is being added to the draw height to define the
		// bounding box.
		box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT + 16);
	}

	void draw(const DrawArgs &args) override
	{
		const auto vg = args.vg;

		nvgSave(vg);

		if(module)
		{
			//
			// Display the pattern
			//

			int break_position;
			// float bar_width = 21;
			// float bar_horizontal_padding = .8;
			float break_position_bar_height;
			NVGcolor highlighted_bar_color;

			for(unsigned int i=0; i<16; i++)
			{
				break_position = module->break_patterns[module->selected_break_pattern][i];
				// if(item_display == "-1") item_display = ".";

				if(i == module->break_pattern_index)
				{
					highlighted_bar_color = nvgRGBA(255, 255, 255, 250);
				}
				else
				{
					highlighted_bar_color = nvgRGBA(255, 255, 255, 150);
				}

				break_position_bar_height = (DRAW_AREA_HEIGHT * ((break_position + 1) / 16.0));

				// Draw the break offset bar
				if(break_position_bar_height > 0)
				{
					nvgBeginPath(vg);
					// nvgRect(vg, (i * bar_width) + (i * bar_horizontal_padding), DRAW_AREA_HEIGHT, bar_width, -1 * DRAW_AREA_HEIGHT * ((item_display + 1) / 16.0));
					nvgRect(vg, (i * BAR_WIDTH) + (i * BAR_HORIZONTAL_PADDING), DRAW_AREA_HEIGHT - break_position_bar_height, BAR_WIDTH, break_position_bar_height);
					nvgFillColor(vg, highlighted_bar_color);
					nvgFill(vg);
				}

				if(i == module->break_pattern_index)
				{
					// Highlight entire column
					nvgBeginPath(vg);
					nvgRect(vg, (i * BAR_WIDTH) + (i * BAR_HORIZONTAL_PADDING), 0, BAR_WIDTH, DRAW_AREA_HEIGHT);
					nvgFillColor(vg, nvgRGBA(255, 255, 255, 20));
					nvgFill(vg);
				}
			}

			// Note to self: This is a nice orange for an overlay
			// and might be interesting to give an option to activate
			/*
			nvgBeginPath(vg);
			nvgRect(vg, 0,0,348,241);
			nvgFillColor(vg, nvgRGBA(255, 100, 0, 128));
			nvgFill(vg);
			*/
		}

		nvgRestore(vg);
	}

	Vec clampToDrawArea(Vec location)
    {
        float x = clamp(location.x, 0.0f, DRAW_AREA_WIDTH);
        float y = clamp(location.y, 0.0f, DRAW_AREA_HEIGHT);
        return(Vec(x,y));
    }

	void onButton(const event::Button &e) override
    {
        e.consume(this);

		if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
		{
			drag_position = e.pos;
			this->editBar(e.pos);
		}
		// DEBUG("%s %d,%d", "button press at: ", clicked_bar_x_index, clicked_bar_y_index);
	}

	void onDragStart(const event::DragStart &e) override
    {
		TransparentWidget::onDragStart(e);
	}

	void onDragEnd(const event::DragEnd &e) override
    {
		TransparentWidget::onDragEnd(e);
	}

	void onDragMove(const event::DragMove &e) override
    {
		TransparentWidget::onDragMove(e);
		drag_position = drag_position.plus(e.mouseDelta);
		editBar(drag_position);
	}

	void step() override {
		TransparentWidget::step();
	}

	void editBar(Vec mouse_position)
	{
		int clicked_bar_x_index = mouse_position.x / (BAR_WIDTH + BAR_HORIZONTAL_PADDING);
		int clicked_bar_y_index = 15 - (int) ((mouse_position.y / DRAW_AREA_HEIGHT) * 16.0);

		clicked_bar_x_index = clamp(clicked_bar_x_index, 0, 15);
		clicked_bar_y_index = clamp(clicked_bar_y_index, 0, 15);

		// DEBUG("%s %f,%f", "height vs ", DRAW_AREA_HEIGHT, drag_position.y);

		// If the mouse position is below the sequencer, then set the corresponding
		// row of the sequencer to "0" (meaning, don't jump to a new position in the beat)
		if(mouse_position.y > (DRAW_AREA_HEIGHT - 2))
		{
			// Special case: Set the break pattern to -1 for "don't jump to new position"
			module->break_patterns[module->selected_break_pattern][clicked_bar_x_index] = -1;
		}
		else
		{
			// Set the break pattern height
			module->break_patterns[module->selected_break_pattern][clicked_bar_x_index] = clicked_bar_y_index;
		}
	}

	void onEnter(const event::Enter &e) override
    {
		TransparentWidget::onEnter(e);
		this->module->pattern_lock = true;
		DEBUG("On enter called");
	}

	void onLeave(const event::Leave &e) override
    {
		TransparentWidget::onLeave(e);
		this->module->pattern_lock = false;
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
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/autobreak_front_panel_experimental.svg")));

		// Cosmetic rack screws
		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));

		// Pattern Select
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(13.848, 38)), module, Autobreak::PATTERN_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(13.848, 52)), module, Autobreak::PATTERN_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.848, 63.5)), module, Autobreak::PATTERN_INPUT));

		// Inputs for WAV selection
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(34.541, 38)), module, Autobreak::WAV_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(34.541, 52)), module, Autobreak::WAV_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.541, 63.5)), module, Autobreak::WAV_INPUT));

		AutobreakReadout *readout = new AutobreakReadout();
		readout->box.pos = mm2px(Vec(53.5, 22));
		readout->box.size = Vec(110, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		AutobreakPatternReadout *pattern_readout = new AutobreakPatternReadout();
		pattern_readout->box.pos = mm2px(Vec(55.141, 35.689));
		pattern_readout->module = module;
		addChild(pattern_readout);

		AutobreakBPMDislplay *bpm_display = new AutobreakBPMDislplay();
		bpm_display->box.pos = mm2px(Vec(11.5, 13.5));
		bpm_display->module = module;
		addChild(bpm_display);

		// BPM selection
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 12.2)), module, Autobreak::BPM_KNOB));

		// Reset
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.541, 88.685)), module, Autobreak::RESET_INPUT));

		// Outputs
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 100.893)), module, Autobreak::DEBUG_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(13.848, 114.893)), module, Autobreak::CLOCK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(13.848, 88.685)), module, Autobreak::END_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.541, 114.893)), module, Autobreak::WAV_OUTPUT));

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
