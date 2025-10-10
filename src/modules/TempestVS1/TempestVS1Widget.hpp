#pragma once
#include <rack.hpp>
#include <componentlibrary.hpp>
#include "../../vgLib-2.0/components/VoxglitchComponents.hpp"
using namespace rack;

// Forward declaration
struct TempestVS1Widget;

// Custom ring widget for hover indicators
struct OutputRingWidget : widget::TransparentWidget {
    NVGcolor ringColor = nvgRGB(0, 150, 255);
    float ringWidth = 2.0;

    void draw(const DrawArgs& args) override {
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, box.size.x / 2.0, box.size.y / 2.0, box.size.x / 2.0 - ringWidth / 2.0);
        nvgStrokeColor(args.vg, ringColor);
        nvgStrokeWidth(args.vg, ringWidth);
        nvgStroke(args.vg);
        widget::TransparentWidget::draw(args);
    }
};

// Hover indicator wrapper for knobs and params
template <typename TParam>
struct HoverIndicatorParam : TParam {
    TempestVS1Widget* parentWidget = nullptr;
    int paramId = -1;

    void onEnter(const widget::Widget::EnterEvent& e) override {
        TParam::onEnter(e);
        if (parentWidget && paramId >= 0) {
            showOutputRing();
        }
    }

    void onLeave(const widget::Widget::LeaveEvent& e) override {
        TParam::onLeave(e);
        if (parentWidget && paramId >= 0) {
            hideOutputRing();
        }
    }

    void showOutputRing();
    void hideOutputRing();
};

// Custom MIDI configuration button
struct MidiConfigButton : VCVButton {
    TempestVS1* module;

    void appendMidiMenu(ui::Menu* menu, midi::InputQueue* port) {
        // Force channel to "all channels"
        port->channel = -1;

        menu->addChild(createMenuLabel("MIDI Driver"));

        for (int driverId : midi::getDriverIds()) {
            menu->addChild(createCheckMenuItem(midi::getDriver(driverId)->getName(),
                "",
                [=]() { return port->getDriverId() == driverId; },
                [=]() { port->setDriverId(driverId); }
            ));
        }

        menu->addChild(new ui::MenuSeparator);
        menu->addChild(createMenuLabel("MIDI Device"));

        {
            menu->addChild(createCheckMenuItem("(No device)",
                "",
                [=]() { return port->getDeviceId() == -1; },
                [=]() { port->setDeviceId(-1); }
            ));
        }

        for (int deviceId : port->getDeviceIds()) {
            menu->addChild(createCheckMenuItem(port->getDeviceName(deviceId),
                "",
                [=]() { return port->getDeviceId() == deviceId; },
                [=]() { port->setDeviceId(deviceId); }
            ));
        }
    }

    void onButton(const event::Button& e) override {
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            e.consume(this);

            if (!module) return;

            ui::Menu* menu = createMenu();
            appendMidiMenu(menu, &module->midiInput);
        }
    }
};

struct TempestVS1Widget : ModuleWidget
{
    std::map<int, std::vector<Widget*>> outputRings; // paramId -> ring widgets (can be multiple)

    void showOutputRing(int paramId) {
        if (outputRings.count(paramId)) {
            for (Widget* ring : outputRings[paramId]) {
                ring->visible = true;
            }
        }
    }

    void hideOutputRing(int paramId) {
        if (outputRings.count(paramId)) {
            for (Widget* ring : outputRings[paramId]) {
                ring->visible = false;
            }
        }
    }

    TempestVS1Widget(TempestVS1 *module) {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(asset::plugin(pluginInstance, "res/modules/tempest_vs1/tempest_vs1_panel.svg"));

        // Add MIDI configuration button using custom button
        MidiConfigButton* midiButton = createParamCentered<MidiConfigButton>(panelHelper.findNamed("midi_button"), module, TempestVS1::MIDI_BUTTON_PARAM);
        midiButton->module = module;
        addParam(midiButton);

        // Add push buttons using positions from SVG
        HoverIndicatorParam<VS1PushButton>* modeButton = createParamCentered<HoverIndicatorParam<VS1PushButton>>(panelHelper.findNamed("mode_button"), module, TempestVS1::MODE_BUTTON_PARAM);
        modeButton->parentWidget = this;
        modeButton->paramId = TempestVS1::MODE_BUTTON_PARAM;
        addParam(modeButton);

        HoverIndicatorParam<VS1PushButton>* presetButton = createParamCentered<HoverIndicatorParam<VS1PushButton>>(panelHelper.findNamed("preset_button"), module, TempestVS1::PRESET_BUTTON_PARAM);
        presetButton->parentWidget = this;
        presetButton->paramId = TempestVS1::PRESET_BUTTON_PARAM;
        addParam(presetButton);

        HoverIndicatorParam<VS1PushButton>* clearButton = createParamCentered<HoverIndicatorParam<VS1PushButton>>(panelHelper.findNamed("clear_button"), module, TempestVS1::CLEAR_BUTTON_PARAM);
        clearButton->parentWidget = this;
        clearButton->paramId = TempestVS1::CLEAR_BUTTON_PARAM;
        addParam(clearButton);

        // Add switch using positions from SVG
        HoverIndicatorParam<NKKTwoPosition>* modeSwitch = createParamCentered<HoverIndicatorParam<NKKTwoPosition>>(panelHelper.findNamed("mode_switch"), module, TempestVS1::MODE_SWITCH_PARAM);
        modeSwitch->parentWidget = this;
        modeSwitch->paramId = TempestVS1::MODE_SWITCH_PARAM;
        addParam(modeSwitch);

        // Add knob parameters using positions from SVG
        // Main knobs (M1-M4) - using VS1KnobDI
        HoverIndicatorParam<VS1KnobDI>* m1Knob = createParamCentered<HoverIndicatorParam<VS1KnobDI>>(panelHelper.findNamed("m1_knob"), module, TempestVS1::M1_KNOB_PARAM);
        m1Knob->parentWidget = this;
        m1Knob->paramId = TempestVS1::M1_KNOB_PARAM;
        addParam(m1Knob);

        HoverIndicatorParam<VS1KnobDI>* m2Knob = createParamCentered<HoverIndicatorParam<VS1KnobDI>>(panelHelper.findNamed("m2_knob"), module, TempestVS1::M2_KNOB_PARAM);
        m2Knob->parentWidget = this;
        m2Knob->paramId = TempestVS1::M2_KNOB_PARAM;
        addParam(m2Knob);

        HoverIndicatorParam<VS1KnobDI>* m3Knob = createParamCentered<HoverIndicatorParam<VS1KnobDI>>(panelHelper.findNamed("m3_knob"), module, TempestVS1::M3_KNOB_PARAM);
        m3Knob->parentWidget = this;
        m3Knob->paramId = TempestVS1::M3_KNOB_PARAM;
        addParam(m3Knob);

        HoverIndicatorParam<VS1KnobDI>* m4Knob = createParamCentered<HoverIndicatorParam<VS1KnobDI>>(panelHelper.findNamed("m4_knob"), module, TempestVS1::M4_KNOB_PARAM);
        m4Knob->parentWidget = this;
        m4Knob->paramId = TempestVS1::M4_KNOB_PARAM;
        addParam(m4Knob);

        // Small knobs (SM1-SM4) - using SkirtedVenturaKnob
        HoverIndicatorParam<SkirtedVenturaKnob>* sm1Knob = createParamCentered<HoverIndicatorParam<SkirtedVenturaKnob>>(panelHelper.findNamed("sm1_knob"), module, TempestVS1::SM_KNOB1_PARAM);
        sm1Knob->parentWidget = this;
        sm1Knob->paramId = TempestVS1::SM_KNOB1_PARAM;
        addParam(sm1Knob);

        HoverIndicatorParam<SkirtedVenturaKnob>* sm2Knob = createParamCentered<HoverIndicatorParam<SkirtedVenturaKnob>>(panelHelper.findNamed("sm2_knob"), module, TempestVS1::SM2_KNOB_PARAM);
        sm2Knob->parentWidget = this;
        sm2Knob->paramId = TempestVS1::SM2_KNOB_PARAM;
        addParam(sm2Knob);

        HoverIndicatorParam<SkirtedVenturaKnob>* sm3Knob = createParamCentered<HoverIndicatorParam<SkirtedVenturaKnob>>(panelHelper.findNamed("sm3_knob"), module, TempestVS1::SM_3_KNOB_PARAM);
        sm3Knob->parentWidget = this;
        sm3Knob->paramId = TempestVS1::SM_3_KNOB_PARAM;
        addParam(sm3Knob);

        HoverIndicatorParam<SkirtedVenturaKnob>* sm4Knob = createParamCentered<HoverIndicatorParam<SkirtedVenturaKnob>>(panelHelper.findNamed("sm4_knob"), module, TempestVS1::SM4_KNOB_PARAM);
        sm4Knob->parentWidget = this;
        sm4Knob->paramId = TempestVS1::SM4_KNOB_PARAM;
        addParam(sm4Knob);

        // Central controls - epic knob uses double indicator, center uses regular Moog style
        HoverIndicatorParam<FlyingSaucerSkirtKnob>* epicKnob = createParamCentered<HoverIndicatorParam<FlyingSaucerSkirtKnob>>(panelHelper.findNamed("epic_knob"), module, TempestVS1::EPIC_KNOB_PARAM);
        epicKnob->parentWidget = this;
        epicKnob->paramId = TempestVS1::EPIC_KNOB_PARAM;
        addParam(epicKnob);

        HoverIndicatorParam<VS1CenterKnob>* centerKnob = createParamCentered<HoverIndicatorParam<VS1CenterKnob>>(panelHelper.findNamed("center_knob"), module, TempestVS1::CENTER_KNOB_PARAM);
        centerKnob->parentWidget = this;
        centerKnob->paramId = TempestVS1::CENTER_KNOB_PARAM;
        addParam(centerKnob);

        // Rotary encoder knob
        HoverIndicatorParam<VS1RotaryKnob>* rotaryKnob = createParamCentered<HoverIndicatorParam<VS1RotaryKnob>>(panelHelper.findNamed("rotary_knob"), module, TempestVS1::ROTARY_KNOB_PARAM);
        rotaryKnob->parentWidget = this;
        rotaryKnob->paramId = TempestVS1::ROTARY_KNOB_PARAM;
        addParam(rotaryKnob);

        // Add outputs using positions from SVG
        
        // Main knobs (M1-M4)
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("m1_knob_output"), module, TempestVS1::M1_KNOB_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("m2_knob_output"), module, TempestVS1::M2_KNOB_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("m3_knob_output"), module, TempestVS1::M3_KNOB_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("m4_knob_output"), module, TempestVS1::M4_KNOB_OUTPUT));
        
        // Small knobs (SM1-SM4)
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("sm_knob1_output"), module, TempestVS1::SM_KNOB1_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("sm2_knob_output"), module, TempestVS1::SM2_KNOB_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("sm3_knob_output"), module, TempestVS1::SM_3_KNOB_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("sm4_knob_output"), module, TempestVS1::SM4_KNOB_OUTPUT));
        
        // Central controls
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("epic_knob_output"), module, TempestVS1::EPIC_KNOB_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("center_knob_output"), module, TempestVS1::CENTER_KNOB_OUTPUT));

        // Encoder triggers
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("rotary_knob_output_left"), module, TempestVS1::ENCODER_LEFT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("rotary_knob_output_right"), module, TempestVS1::ENCODER_RIGHT_OUTPUT));
        
        // Buttons
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("mode_button_output"), module, TempestVS1::MODE_BUTTON_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("preset_button_output"), module, TempestVS1::PRESET_BUTTON_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("clear_button_output"), module, TempestVS1::CLEAR_BUTTON_OUTPUT));
        
        // Switches
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("mode_switch_output"), module, TempestVS1::MODE_SWITCH_OUTPUT));

        // Add activity light
        addChild(createLightCentered<MediumLight<GreenLight>>(panelHelper.findNamed("activity_light"), module, TempestVS1::MIDI_ACTIVITY_LIGHT));

        // Create hover indicator rings for each output
        // Main knobs (M1-M4)
        addOutputRing(TempestVS1::M1_KNOB_PARAM, panelHelper.findNamed("m1_knob_output"));
        addOutputRing(TempestVS1::M2_KNOB_PARAM, panelHelper.findNamed("m2_knob_output"));
        addOutputRing(TempestVS1::M3_KNOB_PARAM, panelHelper.findNamed("m3_knob_output"));
        addOutputRing(TempestVS1::M4_KNOB_PARAM, panelHelper.findNamed("m4_knob_output"));

        // Small knobs (SM1-SM4)
        addOutputRing(TempestVS1::SM_KNOB1_PARAM, panelHelper.findNamed("sm_knob1_output"));
        addOutputRing(TempestVS1::SM2_KNOB_PARAM, panelHelper.findNamed("sm2_knob_output"));
        addOutputRing(TempestVS1::SM_3_KNOB_PARAM, panelHelper.findNamed("sm3_knob_output"));
        addOutputRing(TempestVS1::SM4_KNOB_PARAM, panelHelper.findNamed("sm4_knob_output"));

        // Central controls
        addOutputRing(TempestVS1::EPIC_KNOB_PARAM, panelHelper.findNamed("epic_knob_output"));
        addOutputRing(TempestVS1::CENTER_KNOB_PARAM, panelHelper.findNamed("center_knob_output"));

        // Rotary encoder - uses same param for both left and right outputs
        addOutputRing(TempestVS1::ROTARY_KNOB_PARAM, panelHelper.findNamed("rotary_knob_output_left"));
        addOutputRing(TempestVS1::ROTARY_KNOB_PARAM, panelHelper.findNamed("rotary_knob_output_right"));

        // Buttons
        addOutputRing(TempestVS1::MODE_BUTTON_PARAM, panelHelper.findNamed("mode_button_output"));
        addOutputRing(TempestVS1::PRESET_BUTTON_PARAM, panelHelper.findNamed("preset_button_output"));
        addOutputRing(TempestVS1::CLEAR_BUTTON_PARAM, panelHelper.findNamed("clear_button_output"));

        // Switch
        addOutputRing(TempestVS1::MODE_SWITCH_PARAM, panelHelper.findNamed("mode_switch_output"));
    }

    void appendContextMenu(Menu *menu) override {
        TempestVS1 *module = dynamic_cast<TempestVS1 *>(this->module);
        if (!module) return;

        // Context menu can be used for other module-specific settings if needed
    }

    // Helper to create a ring widget around an output port
    void addOutputRing(int paramId, Vec position) {
        OutputRingWidget* ring = new OutputRingWidget();
        ring->box.size = Vec(38.0, 38.0); // Slightly larger than port
        ring->box.pos = position.minus(Vec(19.0, 19.0)); // Center the ring
        ring->visible = false; // Initially hidden
        addChild(ring);
        outputRings[paramId].push_back(ring);
    }
};

// Template method implementations
template <typename TParam>
void HoverIndicatorParam<TParam>::showOutputRing() {
    parentWidget->showOutputRing(paramId);
}

template <typename TParam>
void HoverIndicatorParam<TParam>::hideOutputRing() {
    parentWidget->hideOutputRing(paramId);
}