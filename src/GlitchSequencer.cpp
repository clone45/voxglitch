#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include "cmath"

#define MAX_SEQUENCE_LENGTH 64
#define SEQUENCER_ROWS 16
#define SEQUENCER_COLUMNS 16
#define NUMBER_OF_TRIGGER_GROUPS 5

#define DRAW_AREA_WIDTH 277.4
#define DRAW_AREA_HEIGHT 277.4
#define DRAW_AREA_POSITION_X 3.800
#define DRAW_AREA_POSITION_Y 5.9

#define CELL_WIDTH 16.95
#define CELL_HEIGHT 16.95
#define CELL_PADDING 0.4

#define PLAY_MODE 0
#define EDIT_SEED_MODE 1
#define EDIT_TRIGGERS_MODE 2
#define EDIT_LENGTH_MODE 3

using namespace std;

struct CellularAutomatonSequencer
{
    unsigned int position = 0;
    unsigned int length = 0;

    bool seed[SEQUENCER_ROWS][SEQUENCER_COLUMNS] = {
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,1,1, 1,1,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,1, 1,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
    };

    bool state[SEQUENCER_ROWS][SEQUENCER_COLUMNS];
    bool next[SEQUENCER_ROWS][SEQUENCER_COLUMNS];
    bool triggers[NUMBER_OF_TRIGGER_GROUPS][SEQUENCER_ROWS][SEQUENCER_COLUMNS];

    // constructor
    CellularAutomatonSequencer()
    {
        clearPattern(&state);
        clearPattern(&next);

        for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
        {
            clearPattern(&triggers[i]);
        }

        copyPattern(&state, &seed);
    }

    void step()
    {
        position ++;

        if(position >= length)
        {
            restart_sequence();
        }
        else
        {
            calculate_next_state();
        }
    }

    void restart_sequence()
    {
        position = 0;
        copyPattern(&state, &seed);  // dst < src
        clearPattern(&next);
    }

    void calculate_next_state()
    {
        for(unsigned int row = 1; row < SEQUENCER_ROWS - 1; row++)
        {
            for(unsigned int column = 1; column < SEQUENCER_COLUMNS - 1; column++)
            {
                unsigned int neighbors = 0;

                // Calculate number of neighbors
                if(state[row-1][column-1]) neighbors++;
                if(state[row-1][column]) neighbors++;
                if(state[row-1][column+1]) neighbors++;
                if(state[row][column-1]) neighbors++;
                if(state[row][column+1]) neighbors++;
                if(state[row+1][column-1]) neighbors++;
                if(state[row+1][column]) neighbors++;
                if(state[row+1][column+1]) neighbors++;

                // Apply game of life rules
                if(state[row][column] && neighbors < 2) next[row][column] = 0;
                else if(state[row][column] && neighbors > 3) next[row][column] = 0;
                else if(state[row][column] && (neighbors == 2 || neighbors == 3)) next[row][column] = 1;
                else if(state[row][column] == 0 && neighbors == 3) next[row][column] = 1;
            }
        }

        copyPattern(&state, &next); // dst < src
    }

    void copyPattern(bool (*dst)[SEQUENCER_ROWS][SEQUENCER_COLUMNS], bool (*src)[SEQUENCER_ROWS][SEQUENCER_COLUMNS])
    {
        for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
        {
            for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
            {
                (*dst)[row][column] = (*src)[row][column];
            }
        }
    }

    void clearPattern(bool (*pattern)[SEQUENCER_ROWS][SEQUENCER_COLUMNS])
    {
        for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
        {
            for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
            {
                (*pattern)[row][column] = 0;
            }
        }
    }

    void setLength(unsigned int length)
    {
        this->length = length;
    }
};


struct GlitchSequencer : Module
{
    CellularAutomatonSequencer sequencer;
    dsp::SchmittTrigger stepTrigger;
    unsigned int mode = PLAY_MODE;

    dsp::SchmittTrigger trigger_group_button_schmitt_trigger[5];
    bool trigger_button_is_triggered[5];
    unsigned int trigger_group_buttons[5];
    int selected_trigger_group_index = -1; // -1 means "none selected"

	enum ParamIds {
        LENGTH_KNOB,
        TRIGGER_GROUP_1_BUTTON,
        TRIGGER_GROUP_2_BUTTON,
        TRIGGER_GROUP_3_BUTTON,
        TRIGGER_GROUP_4_BUTTON,
        TRIGGER_GROUP_5_BUTTON,
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
		NUM_OUTPUTS
	};
	enum LightIds {
        TRIGGER_GROUP_1_LIGHT,
        TRIGGER_GROUP_2_LIGHT,
        TRIGGER_GROUP_3_LIGHT,
        TRIGGER_GROUP_4_LIGHT,
        TRIGGER_GROUP_5_LIGHT,
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	GlitchSequencer()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(LENGTH_KNOB, 1, MAX_SEQUENCE_LENGTH, 16, "LengthKnob");
        configParam(TRIGGER_GROUP_1_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup1Button");
        configParam(TRIGGER_GROUP_2_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup2Button");
        configParam(TRIGGER_GROUP_3_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup3Button");
        configParam(TRIGGER_GROUP_4_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup4Button");
        configParam(TRIGGER_GROUP_5_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup5Button");

        trigger_group_buttons[0] = TRIGGER_GROUP_1_BUTTON;
        trigger_group_buttons[1] = TRIGGER_GROUP_2_BUTTON;
        trigger_group_buttons[2] = TRIGGER_GROUP_3_BUTTON;
        trigger_group_buttons[3] = TRIGGER_GROUP_4_BUTTON;
        trigger_group_buttons[4] = TRIGGER_GROUP_5_BUTTON;
	}

    json_t *dataToJson() override
    {
        json_t *root = json_object();
    	return root;
    }

    void dataFromJson(json_t *root) override
    {
    }

    void toggleTriggerGroup(int index)
    {
        if(selected_trigger_group_index == index)
        {
            selected_trigger_group_index = -1;
            mode = PLAY_MODE;
        }
        else
        {
            selected_trigger_group_index = index;
            mode = EDIT_TRIGGERS_MODE;
        }
    }

	void process(const ProcessArgs &args) override
	{
        sequencer.setLength(params[LENGTH_KNOB].getValue());

        for(unsigned int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
        {
            trigger_button_is_triggered[i] = trigger_group_button_schmitt_trigger[i].process(params[trigger_group_buttons[i]].getValue());
            if(trigger_button_is_triggered[i]) toggleTriggerGroup(i);
        }

        if(stepTrigger.process(rescale(inputs[STEP_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
        {
            sequencer.step();
        }

        lights[TRIGGER_GROUP_1_LIGHT].setBrightness(selected_trigger_group_index == 0);
        lights[TRIGGER_GROUP_2_LIGHT].setBrightness(selected_trigger_group_index == 1);
        lights[TRIGGER_GROUP_3_LIGHT].setBrightness(selected_trigger_group_index == 2);
        lights[TRIGGER_GROUP_4_LIGHT].setBrightness(selected_trigger_group_index == 3);
        lights[TRIGGER_GROUP_5_LIGHT].setBrightness(selected_trigger_group_index == 4);
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
		box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
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

                    // bool cell_is_alive = (module->edit_mode) ? module->sequencer.pattern[row][column] : module->sequencer.state[row][column];

                    // Default color for inactive square
                    nvgFillColor(vg, nvgRGB(55, 55, 55));

                    // When in edit mode, the pattern that's being edited will be bright white
                    // and the underlying animation will continue to be shown but at a dim gray
                    switch(module->mode)
                    {
                        case PLAY_MODE:
                            if(module->sequencer.state[row][column]) nvgFillColor(vg, nvgRGB(255, 255, 255));
                            break;

                        case EDIT_SEED_MODE:
                            if(module->sequencer.state[row][column]) nvgFillColor(vg, nvgRGB(75, 75, 75));
                            if(module->sequencer.seed[row][column]) nvgFillColor(vg, nvgRGB(255, 255, 255));
                            break;

                        case EDIT_TRIGGERS_MODE:
                            if(module->selected_trigger_group_index >= 0)
                            {
                                if(module->sequencer.state[row][column]) nvgFillColor(vg, nvgRGB(75, 75, 75));
                                if(module->sequencer.triggers[module->selected_trigger_group_index][row][column]) nvgFillColor(vg, nvgRGB(255, 255, 255));
                            }
                            break;
                    }

                    nvgFill(vg);
                }
            }
        }
        // Paint static content for library display
        else
        {
            CellularAutomatonSequencer ca;

            for(unsigned int row=0; row < SEQUENCER_ROWS; row++)
            {
                for(unsigned int column=0; column < SEQUENCER_COLUMNS; column++)
                {
                    nvgBeginPath(vg);
                    nvgRect(vg, (column * CELL_WIDTH) + (column * CELL_PADDING), (row * CELL_HEIGHT) + (row * CELL_PADDING), CELL_WIDTH, CELL_HEIGHT);

                    nvgFillColor(vg, nvgRGB(55, 55, 55)); // Default color for inactive square
                    if(ca.seed[row][column]) nvgFillColor(vg, nvgRGB(255, 255, 255));

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

    void setSequencerCell(unsigned int row, unsigned int column, bool value)
    {
        module->sequencer.seed[row][column] = this->cell_edit_value;

        // If the sequencer is at the first step, also update the current "state"
        // The first "state" of the sequencer should always mirror the pattern
        if(module->sequencer.position == 0) module->sequencer.state[row][column] = this->cell_edit_value;
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
                tie(row, column) = getRowAndColumnFromVec(e.pos);

                if(module->mode == EDIT_SEED_MODE)
                {
                    // Store the value that's being set for later in case the user
                    // drags to set ("paints") additional triggers
                    this->cell_edit_value = ! module->sequencer.seed[row][column];

                    // Set the cell value in the sequencer
                    this->setSequencerCell(row, column, this->cell_edit_value);
                }

                if(module->mode == EDIT_TRIGGERS_MODE && module->selected_trigger_group_index >= 0)
                {
                    // Store the value that's being set for later in case the user
                    // drags to set ("paints") additional triggers
                    this->cell_edit_value = ! module->sequencer.triggers[module->selected_trigger_group_index][row][column];
                    module->sequencer.triggers[module->selected_trigger_group_index][row][column] = this->cell_edit_value;
                }

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
            if(module->mode == EDIT_SEED_MODE)
            {
                this->setSequencerCell(row, column, this->cell_edit_value);
            }

            if(module->mode == EDIT_TRIGGERS_MODE && module->selected_trigger_group_index >= 0)
            {
                module->sequencer.triggers[module->selected_trigger_group_index][row][column] = this->cell_edit_value;
            }

            old_row = row;
            old_column = column;
        }
	}

    void onEnter(const event::Enter &e) override
    {
        TransparentWidget::onEnter(e);
        if(this->module->mode == PLAY_MODE) this->module->mode = EDIT_SEED_MODE;
    }

    void onLeave(const event::Leave &e) override
    {
        TransparentWidget::onLeave(e);
        if(this->module->mode == EDIT_SEED_MODE) this->module->mode = PLAY_MODE;
    }

    void onHover(const event::Hover& e) override {
		TransparentWidget::onHover(e);
		e.consume(this);
	}
};

struct LengthReadoutDisplay : TransparentWidget
{
    GlitchSequencer *module;
    std::shared_ptr<Font> font;

    LengthReadoutDisplay()
	{
        font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
        const auto vg = args.vg;

        nvgSave(vg);

		std::string text_to_display = "16";

		if(module)
		{
			text_to_display = std::to_string(module->sequencer.length);
		}

		nvgFontSize(vg, 12);
		nvgFontFaceId(vg, font->handle);
        nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgTextLetterSpacing(vg, -1);
		nvgFillColor(vg, nvgRGB(3, 3, 3));
		nvgText(vg, 5, 5, text_to_display.c_str(), NULL);

		nvgRestore(vg);

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
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x, button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_1_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x, button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_1_LIGHT));
        // Sequence 2 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_2_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_2_LIGHT));
        // Sequence 3 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_3_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_3_LIGHT));
        // Sequence 4 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_4_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_4_LIGHT));
        // Sequence 5 button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_5_BUTTON));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_5_LIGHT));

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

        LengthReadoutDisplay *length_readout_display = new LengthReadoutDisplay();
        length_readout_display->box.pos = mm2px(Vec(30.0, 122.0));
        length_readout_display->module = module;
        addChild(length_readout_display);
	}

	void appendContextMenu(Menu *menu) override
	{
		GlitchSequencer *module = dynamic_cast<GlitchSequencer*>(this->module);
		assert(module);
	}

};

Model* modelGlitchSequencer = createModel<GlitchSequencer, GlitchSequencerWidget>("glitchsequencer");
