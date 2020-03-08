#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include "dr_wav.h"
#include <vector>
#include "cmath"

using namespace std;

struct GlitchSequencer : Module
{
	enum ParamIds {
        SEQUENCER_1_BUTTON,
        SEQUENCER_2_BUTTON,
        SEQUENCER_3_BUTTON,
        SEQUENCER_4_BUTTON,
        SEQUENCER_5_BUTTON,
        SEQUENCER_6_BUTTON,
        NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
        GATE_OUTPUT_1,
        GATE_OUTPUT_2,
        GATE_OUTPUT_3,
        GATE_OUTPUT_4,
        GATE_OUTPUT_5,
        GATE_OUTPUT_6,
		NUM_OUTPUTS
	};
	enum LightIds {
        SEQUENCER_1_LIGHT,
        SEQUENCER_2_LIGHT,
        SEQUENCER_3_LIGHT,
        SEQUENCER_4_LIGHT,
        SEQUENCER_5_LIGHT,
        SEQUENCER_6_LIGHT,
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	GlitchSequencer()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(SEQUENCER_1_BUTTON, 0.f, 1.f, 0.f, "Sequence1Button");
        configParam(SEQUENCER_2_BUTTON, 0.f, 1.f, 0.f, "Sequence2Button");
        configParam(SEQUENCER_3_BUTTON, 0.f, 1.f, 0.f, "Sequence3Button");
        configParam(SEQUENCER_4_BUTTON, 0.f, 1.f, 0.f, "Sequence4Button");
        configParam(SEQUENCER_5_BUTTON, 0.f, 1.f, 0.f, "Sequence5Button");
        configParam(SEQUENCER_6_BUTTON, 0.f, 1.f, 0.f, "Sequence6Button");
	}

    json_t *dataToJson() override
    {
        json_t *root = json_object();
    	return root;
    }

    void dataFromJson(json_t *root) override
    {
    }

	void process(const ProcessArgs &args) override
	{
	}
};


struct GlitchSequencerWidget : ModuleWidget
{
	GlitchSequencerWidget(GlitchSequencer* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/glitch_sequencer_front_panel.svg")));

        float button_spacing = 9.6; // 9.1
        float button_group_x = 48.0;
        float button_group_y = 103.0;

        // Sequence 1 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x, button_group_y)), module, GlitchSequencer::SEQUENCER_1_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x, button_group_y)), module, GlitchSequencer::SEQUENCER_1_LIGHT));
        // Sequence 2 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y)), module, GlitchSequencer::SEQUENCER_2_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y)), module, GlitchSequencer::SEQUENCER_2_LIGHT));
        // Sequence 3 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y)), module, GlitchSequencer::SEQUENCER_3_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y)), module, GlitchSequencer::SEQUENCER_3_LIGHT));
        // Sequence 4 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y)), module, GlitchSequencer::SEQUENCER_4_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y)), module, GlitchSequencer::SEQUENCER_4_LIGHT));
        // Sequence 5 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y)), module, GlitchSequencer::SEQUENCER_5_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y)), module, GlitchSequencer::SEQUENCER_5_LIGHT));
        // Sequence 6 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 5.0), button_group_y)), module, GlitchSequencer::SEQUENCER_6_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 5.0), button_group_y)), module, GlitchSequencer::SEQUENCER_6_LIGHT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x, 119.309)), module, GlitchSequencer::GATE_OUTPUT_1));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 1.0), 119.309)), module, GlitchSequencer::GATE_OUTPUT_2));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 2.0), 119.309)), module, GlitchSequencer::GATE_OUTPUT_3));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 3.0), 119.309)), module, GlitchSequencer::GATE_OUTPUT_4));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 4.0), 119.309)), module, GlitchSequencer::GATE_OUTPUT_5));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 5.0), 119.309)), module, GlitchSequencer::GATE_OUTPUT_6));
	}

	void appendContextMenu(Menu *menu) override
	{
		GlitchSequencer *module = dynamic_cast<GlitchSequencer*>(this->module);
		assert(module);
	}

};

Model* modelGlitchSequencer = createModel<GlitchSequencer, GlitchSequencerWidget>("glitchsequencer");
