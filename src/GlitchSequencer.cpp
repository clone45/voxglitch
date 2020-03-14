#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include <vector>
#include "cmath"

#define MAX_SEQUENCER_STEPS 64
#define SEQUENCER_ROWS 16
#define SEQUENCER_COLUMNS 16

#define DRAW_AREA_WIDTH 277.4
#define DRAW_AREA_HEIGHT 277.4
#define DRAW_AREA_POSITION_X 3.800
#define DRAW_AREA_POSITION_Y 5.9

#define CELL_WIDTH 16.95
#define CELL_HEIGHT 16.95
#define CELL_PADDING 0.4

using namespace std;

struct CellularAutomatonSequencer
{
    bool pattern[SEQUENCER_ROWS][SEQUENCER_COLUMNS] = {
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,1,1, 0,1,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,1,1, 1,1,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,1, 1,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,1,1,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
    };

    bool state[SEQUENCER_ROWS][SEQUENCER_COLUMNS];

    // constructor
    CellularAutomatonSequencer()
    {
        for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
        {
            for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
            {
                state[row][column] = 0;
            }
        }
    }
};


struct GlitchSequencer : Module
{
    CellularAutomatonSequencer sequencer;

	enum ParamIds {
        LENGTH_KNOB,
        SEQUENCER_1_BUTTON,
        SEQUENCER_2_BUTTON,
        SEQUENCER_3_BUTTON,
        SEQUENCER_4_BUTTON,
        SEQUENCER_5_BUTTON,
        SEQUENCER_6_BUTTON,
        NUM_PARAMS
	};
	enum InputIds {
        STEP_INPUT,
        RESET_INPUT,
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

        configParam(LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "LengthKnob");
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

struct CellularAutomatonDisplay : TransparentWidget
{
    GlitchSequencer *module;
    Vec drag_position;
    bool mouse_lock = false;
    bool cell_edit_value = true;
    int old_row = -1;
    int old_column = -1;

	CellularAutomatonDisplay()
	{
		// box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
	}

	void draw(const DrawArgs &args) override
	{
        const auto vg = args.vg;

        // Save the drawing context to restore later
		nvgSave(vg);

		if(module)
		{
            // testing draw area
            /*
            nvgBeginPath(vg);
            nvgRect(vg, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
            nvgFillColor(vg, nvgRGBA(120, 20, 20, 100));
            nvgFill(vg);
            */

            for(unsigned int row=0; row < SEQUENCER_ROWS; row++)
            {
                for(unsigned int column=0; column < SEQUENCER_COLUMNS; column++)
                {
                    nvgBeginPath(vg);
                    nvgRect(vg, (column * CELL_WIDTH) + (column * CELL_PADDING), (row * CELL_HEIGHT) + (row * CELL_PADDING), CELL_WIDTH, CELL_HEIGHT);

                    if(module->sequencer.pattern[row][column])
                    {
                        nvgFillColor(vg, nvgRGB(255, 255, 255));
                    }
                    else
                    {
                        nvgFillColor(vg, nvgRGB(55, 55, 55));
                    }

                    nvgFill(vg);
                }
            }
        }

		nvgRestore(vg);
	}

    std::pair<int, int> getRowAndColumnFromVec(Vec position)
    {
        int row = position.y / (CELL_HEIGHT + CELL_PADDING);
        int column = position.x / (CELL_WIDTH + CELL_PADDING);

        return {row, column};
    }

    void onButton(const event::Button &e) override
    {
        e.consume(this);

		if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
		{
            if(this->mouse_lock == false)
            {
                this->mouse_lock = true;

                int row, column;
                tie(row, column)  = getRowAndColumnFromVec(e.pos);

                // Store the value that's being set for later in case the user
                // drags to set ("paints") additional triggers
                this->cell_edit_value = ! module->sequencer.pattern[row][column];

                // Set the trigger value in the sequencer
                module->sequencer.pattern[row][column] = this->cell_edit_value;

                // Store the initial drag position
                drag_position = e.pos;
            }
		}
        else if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
		{
            this->mouse_lock = false;
		}
	}

    void onDragMove(const event::DragMove &e) override
    {
		TransparentWidget::onDragMove(e);

        double zoom = std::pow(2.f, settings::zoom);
		drag_position = drag_position.plus(e.mouseDelta.div(zoom));

        int row, column;
        tie(row, column)  = getRowAndColumnFromVec(drag_position);

        if((row != old_row) || (column != old_column))
        {
            module->sequencer.pattern[row][column] = this->cell_edit_value;
            old_row = row;
            old_column = column;
        }
	}
};


struct GlitchSequencerWidget : ModuleWidget
{
	GlitchSequencerWidget(GlitchSequencer* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/glitch_sequencer_front_panel.svg")));

        float button_spacing = 9.6; // 9.1
        float button_group_x = 53.0;
        float button_group_y = 109.0;

        float inputs_y = 116.0;

        // Step
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, inputs_y)), module, GlitchSequencer::STEP_INPUT));

        // Reset
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 13.544, inputs_y)), module, GlitchSequencer::RESET_INPUT));

        // Length
        addParam(createParamCentered<Trimpot>(mm2px(Vec(10 + (13.544 * 2), inputs_y)), module, GlitchSequencer::LENGTH_KNOB));

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

        float y = button_group_y + 10;
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x, y)), module, GlitchSequencer::GATE_OUTPUT_1));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 1.0), y)), module, GlitchSequencer::GATE_OUTPUT_2));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 2.0), y)), module, GlitchSequencer::GATE_OUTPUT_3));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 3.0), y)), module, GlitchSequencer::GATE_OUTPUT_4));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 4.0), y)), module, GlitchSequencer::GATE_OUTPUT_5));

        CellularAutomatonDisplay *ca_display = new CellularAutomatonDisplay();
		ca_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
		ca_display->module = module;
		addChild(ca_display);
	}

	void appendContextMenu(Menu *menu) override
	{
		GlitchSequencer *module = dynamic_cast<GlitchSequencer*>(this->module);
		assert(module);
	}

};

Model* modelGlitchSequencer = createModel<GlitchSequencer, GlitchSequencerWidget>("glitchsequencer");
