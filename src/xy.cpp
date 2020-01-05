//
// Voxglitch "xy" module for VCV Rack
//
// TODO:
// * add reset input
// * Take zoom into account

#include "plugin.hpp"
#include "osdialog.h"
#include "dr_wav.h"
#include <vector>
#include "cmath"

using namespace std;

#define DRAW_AREA_WIDTH 80
#define DRAW_AREA_HEIGHT 80

#define MODE_PLAYBACK 0
#define MODE_RECORDING 1

struct XY : Module
{
    Vec drag_position;
    vector<Vec> recording_memory;
    unsigned int mode = MODE_PLAYBACK;
    unsigned int playback_index = 0;
    dsp::SchmittTrigger clkTrigger;
    dsp::SchmittTrigger reset_trigger;

	enum ParamIds {
        RETRIGGER_SWITCH,
        NUM_PARAMS
	};
	enum InputIds {
        CLK_INPUT,
        RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		X_OUTPUT,
        Y_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	XY()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(RETRIGGER_SWITCH, 0.f, 1.f, 1.f, "Retrigger");
	}

	json_t *dataToJson() override
	{
        json_t *root = json_object();

        json_t *recording_memory_json_array = json_array();

        for(Vec position : recording_memory)
        {
            json_t *xy_vec = json_array();
            json_array_append_new(xy_vec, json_real(position.x));
            json_array_append_new(xy_vec, json_real(position.y));

            json_array_append_new(recording_memory_json_array, xy_vec);
        }

        json_object_set(root, "recording_memory_data", recording_memory_json_array);
        json_decref(recording_memory_json_array);

		return root;
	}

	void dataFromJson(json_t *root) override
	{
        json_t *recording_memory_data = json_object_get(root, "recording_memory_data");

        if(recording_memory_data)
        {
			recording_memory.clear();
			size_t i;
			json_t *json_array_pair_xy;

			json_array_foreach(recording_memory_data, i, json_array_pair_xy)
            {
                float x = json_real_value(json_array_get(json_array_pair_xy, 0));
                float y = json_real_value(json_array_get(json_array_pair_xy, 1));

				recording_memory.push_back(Vec(x,y));
			}
		}
	}

	void process(const ProcessArgs &args) override
	{
        if (reset_trigger.process(inputs[RESET_INPUT].getVoltage()))
        {
            playback_index = 0;
        }

        if (inputs[CLK_INPUT].isConnected())
		{
            if (clkTrigger.process(inputs[CLK_INPUT].getVoltage()))
            {
                if(mode == MODE_RECORDING)
                {
                    // Output the voltages
                    outputs[X_OUTPUT].setVoltage((drag_position.x / 235.0f) * 10.0f);
                    outputs[Y_OUTPUT].setVoltage(((235.0f - drag_position.y) / 235.0f) * 10.0f);

                    // Store the mouse x,y position
                    recording_memory.push_back(drag_position);
                }

                if(mode == MODE_PLAYBACK)
                {
                    if(recording_memory.size() > 0)
                    {
                        if(params[RETRIGGER_SWITCH].getValue())
                        {
                            // Restart playback if we've reached the end
                            if(playback_index >= recording_memory.size()) playback_index = 0;
                        }

                        if(playback_index < recording_memory.size())
                        {
                            // This will cause the XYDisplay to animate
                            this->drag_position = recording_memory[playback_index];

                            // Output the voltages
                            outputs[X_OUTPUT].setVoltage((drag_position.x / 235.0f) * 10.0f);
                            outputs[Y_OUTPUT].setVoltage(((235.0f - drag_position.y) / 235.0f) * 10.0f);

                            // Step to the next recorded x,y position
                            playback_index += 1;
                        }
                    }
                }
            }
        }
        else // CLK input is not connected
        {
            outputs[X_OUTPUT].setVoltage((drag_position.x / 235.0f) * 10.0f);
            outputs[Y_OUTPUT].setVoltage(((235.0f - drag_position.y) / 235.0f) * 10.0f);
        }
	}

    void start_recording()
    {
        recording_memory.clear();
        mode = MODE_RECORDING;
    }

    void start_playback()
    {
        playback_index = 0;
        mode = MODE_PLAYBACK;
    }
};

struct XYDisplay : OpaqueWidget
{
	XY *module;
	bool dragging = false;
	vector<Vec> fading_rectangles;

	XYDisplay(XY *module): OpaqueWidget()
    {
		this->module = module;
		box.size = mm2px(Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT));
	}

	void draw(const DrawArgs &args) override
    {
		OpaqueWidget::draw(args);
		const auto vg = args.vg;
        float color = 40;

        // Blank out the control
        /*
        nvgStrokeColor(vg, nvgRGB(color, color, color));
        nvgStrokeWidth(vg, 0.f);
        nvgFillColor(vg, nvgRGBA(color,color,color,255));
        nvgRect(vg, 0, 235, 235, 235);
        nvgStroke(vg);
        nvgFill(vg);
        */

		if(module)
        {
            float now_x = clamp(this->module->drag_position.x, 0.0f, 233.0f);
            float now_y = clamp(this->module->drag_position.y, 0.0f, 233.0f) - 233;
            float drag_y = clamp(this->module->drag_position.y, 0.0f, 233.0f);

            // draw x,y lines, just because I think they look cool
            nvgBeginPath(vg);
            nvgStrokeWidth(vg, 0.5f);
            nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
            nvgMoveTo(vg, now_x, 0);
            nvgLineTo(vg, now_x, 233);
            nvgStroke(vg);

            nvgBeginPath(vg);
            nvgStrokeWidth(vg, 0.5f);
            nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
            nvgMoveTo(vg, 0, drag_y);
            nvgLineTo(vg, 233, drag_y);
            nvgStroke(vg);

            fading_rectangles.push_back(Vec(now_x, now_y));

            if(fading_rectangles.size() > 30)
            {
                fading_rectangles.erase(fading_rectangles.begin());
            }

            for(auto position: fading_rectangles)
            {
                float x = position.x;
                float y = position.y;

                color += (150.0f / 30.0f);

                // draw thing
        		nvgBeginPath(vg);
        		nvgStrokeColor(vg, nvgRGB(color, color, color));
        		nvgStrokeWidth(vg, 0.f);
                nvgFillColor(vg, nvgRGBA(color,color,color,255));
        		nvgRect(vg, 0, 235, x, y);
        		nvgStroke(vg);
                nvgFill(vg);
            }



            /*
            nvgBeginPath(vg);
            nvgStrokeWidth(vg, 0.5f);
            nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
            nvgMoveTo(vg, 0, now_y);
            nvgLineTo(vg, 235, now_y);
            nvgStroke(vg);
            */
		}
	}

	void onButton(const event::Button &e) override
    {
        e.consume(this);
        this->module->drag_position = e.pos;

        if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            this->module->start_recording();
        }

        if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
        {
            this->module->start_playback();
        }
	}

	void onDragStart(const event::DragStart &e) override
    {
		OpaqueWidget::onDragStart(e);
		dragging = true;
	}

	void onDragEnd(const event::DragEnd &e) override
    {
		OpaqueWidget::onDragEnd(e);
		dragging = false;
	}

	void onDragMove(const event::DragMove &e) override
    {
		OpaqueWidget::onDragMove(e);

        this->module->drag_position = this->module->drag_position.plus(e.mouseDelta);



        /*
        Vec dragPosition_old = dragPosition;
		float zoom = std::pow(2.f, settings::zoom);
		dragPosition = dragPosition.plus(e.mouseDelta.div(zoom)); // take zoom into account
        */
		// int() rounds down, so the upper limit of rescale is buffer.size() without -1.
        /*
		int s = module->buffer.size();
		math::Vec box_size = box.size;
		int i1 = clamp(int(rescale(dragPosition_old.x, 0, box_size.x, 0, s)), 0, s - 1);
		int i2 = clamp(int(rescale(dragPosition.x,     0, box_size.x, 0, s)), 0, s - 1);
        */

	}

	void step() override {
		OpaqueWidget::step();
	}
};

struct XYWidget : ModuleWidget
{
	XYWidget(XY* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/xy_front_panel.svg")));

		// Cosmetic rack screws
		// addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));

		// X,Y outputs
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(59.664, 128.500 - 13.891)), module, XY::X_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(76.465, 128.500 - 13.891)), module, XY::Y_OUTPUT));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.477, 128.500 - 13.891)), module, XY::CLK_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25, 128.500 - 13.891)), module, XY::RESET_INPUT));
        addParam(createParamCentered<CKSS>(mm2px(Vec(39.5, 128.500 - 13.891)), module, XY::RETRIGGER_SWITCH));

        // xy mouse entry box
        XYDisplay *xy_display;
        xy_display = new XYDisplay(module);
        xy_display->box.pos = mm2px(Vec(3.45, 15.82));
        addChild(xy_display);
	}

	void appendContextMenu(Menu *menu) override
	{
		XY *module = dynamic_cast<XY*>(this->module);
		assert(module);

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		menu->addChild(new MenuEntry); // For spacing only


		//
		// Add the "select bank folder" menu item
		//
        /*
		MenuItemLoadBank *menu_item_load_bank = new MenuItemLoadBank();
		menu_item_load_bank->text = "Select Directory Containing WAV Files";
		menu_item_load_bank->wav_bank_module = module;
		menu->addChild(menu_item_load_bank);
        */
	}

};



Model* modelXY = createModel<XY, XYWidget>("xy");
