#pragma once
#include "Kaiseki.hpp"
#include <osdialog.h>


// Port configuration menu item
struct OSCPortMenuItem : MenuItem {
    Kaiseki* module;
    
    Menu* createChildMenu() override {
        Menu* menu = new Menu;
        
        // Common OSC ports
        int ports[] = {7000, 7001, 8000, 8001, 9000, 9001, 57120, 57121};
        
        for (int port : ports) {
            menu->addChild(createCheckMenuItem(string::f("%d", port), "",
                [=]() { return module->oscPort() == port; },
                [=]() { 
                    // OSC is always enabled, so restart with new port
                    module->stopOSC();
                    module->setOscPort(port);
                    module->startOSC();
                }
            ));
        }
        
        return menu;
    }
};

struct KaisekiWidget : ModuleWidget
{

    KaisekiWidget(Kaiseki *module) {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/kaiseki/kaiseki_panel.svg")
        );

        // Add screws
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Add mix outputs
        addOutput(createOutputCentered<::VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, Kaiseki::MIX_LEFT_OUTPUT));
        addOutput(createOutputCentered<::VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, Kaiseki::MIX_RIGHT_OUTPUT));

        // Add polyphonic outputs
        addOutput(createOutputCentered<::VoxglitchOutputPort>(panelHelper.findNamed("left_poly_output"), module, Kaiseki::POLY_LEFT_OUTPUT));
        addOutput(createOutputCentered<::VoxglitchOutputPort>(panelHelper.findNamed("right_poly_output"), module, Kaiseki::POLY_RIGHT_OUTPUT));

        // Add clock output
        addOutput(createOutputCentered<::VoxglitchOutputPort>(panelHelper.findNamed("clock_output"), module, Kaiseki::CLOCK_OUTPUT));

        // Add control inputs
        addInput(createInputCentered<::VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, Kaiseki::RESET_INPUT));
        addInput(createInputCentered<::VoxglitchInputPort>(panelHelper.findNamed("start_input"), module, Kaiseki::START_INPUT));
        addInput(createInputCentered<::VoxglitchInputPort>(panelHelper.findNamed("stop_input"), module, Kaiseki::STOP_INPUT));
    }

    void appendContextMenu(Menu *menu) override {
        Kaiseki *module = dynamic_cast<Kaiseki *>(this->module);
        if (!module) return;

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Kaiseki Settings"));
        
        // Add port configuration menu
        OSCPortMenuItem* portItem = createMenuItem<OSCPortMenuItem>("OSC Port", string::f("%d", module->oscPort()));
        portItem->module = module;
        menu->addChild(portItem);
        
        // Add reset counters option
        menu->addChild(createMenuItem("Reset Counters", "",
            [=]() {
                module->resetCounters();
            }
        ));
        
        // Add OSC status info
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Dynamic Sample Loading"));
        menu->addChild(createMenuLabel(string::f("Messages Received: %d", module->messagesReceived())));
        menu->addChild(createMenuLabel(string::f("Messages Dropped: %d", module->messagesDropped())));

        // Info about OSC commands
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("OSC Commands:"));
        menu->addChild(createMenuLabel("/track/[1-8]/load path/file_120bpm.wav"));
        menu->addChild(createMenuLabel("/track/[1-8]/volume 0.8"));
        menu->addChild(createMenuLabel("/track/[1-8]/pitch 0.5"));
        menu->addChild(createMenuLabel("/track/[1-8]/trigger 1"));
        menu->addChild(createMenuLabel("/bpm 128.0"));
        menu->addChild(createMenuLabel("/master/stop 1"));
        menu->addChild(createMenuLabel("/master/start 1"));
        menu->addChild(createMenuLabel("/master/restart 1"));
        
        // Show current status (always listening)
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Status: LISTENING"));
    }
};