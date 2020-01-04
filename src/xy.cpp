//
// Voxglitch "xy" module for VCV Rack
//
// TODO: add manditory clock input to control recording and playback
// This is necessary to sync with the Repeater module

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

	enum ParamIds {
        NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		X_OUTPUT,
        Y_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
        RECORDING_INDICATOR,
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	XY()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
        /*
		json_t *loaded_path_json = json_object_get(rootJ, ("path"));
		if (loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			this->load_samples_from_path(this->path.c_str());
		}
        */
	}

	void process(const ProcessArgs &args) override
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
                lights[RECORDING_INDICATOR].value = 0;

                // Restart playback if we've reached the end
                if(playback_index >= recording_memory.size()) playback_index = 0;

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
            float now_x = this->module->drag_position.x;
            float now_y = this->module->drag_position.y - 235;

            // draw x,y lines, just because I think they look cool
            nvgBeginPath(vg);
            nvgStrokeWidth(vg, 0.5f);
            nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
            nvgMoveTo(vg, now_x, 0);
            nvgLineTo(vg, now_x, 235);
            nvgStroke(vg);

            nvgBeginPath(vg);
            nvgStrokeWidth(vg, 0.5f);
            nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
            nvgMoveTo(vg, 0, now_y);
            nvgLineTo(vg, 235, now_y);
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

        // xy mouse entry box
        XYDisplay *xy_display;
        xy_display = new XYDisplay(module);
        xy_display->box.pos = mm2px(Vec(2.993, 15.82));
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
