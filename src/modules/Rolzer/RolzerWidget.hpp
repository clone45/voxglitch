#pragma once

struct RolzerWidget : ModuleWidget
{
    RolzerCanvas *canvas = nullptr;

    RolzerWidget(Rolzer *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/rolzer/rolzer_panel.svg"));

        // Canvas takes upper portion, bottom strip has room for controls
        float bottomBar = 70.0f;
        canvas = new RolzerCanvas();
        canvas->module = module;
        canvas->box.pos = Vec(2.0f, 2.0f);
        canvas->box.size = Vec(box.size.x - 4.0f, box.size.y - bottomBar);
        addChild(canvas);

        // Bottom strip layout
        float portSpacing = box.size.x / (Rolzer::NUM_ROLLS + 1);
        float knobY = box.size.y - 56.0f;
        float wavePortY = box.size.y - 34.0f;
        float gatePortY = box.size.y - 12.0f;

        // Single tempo knob (E6 cap selector, shared by all rolls)
        addParam(createParamCentered<RoundSmallBlackKnob>(
            Vec(box.size.x * 0.5f, knobY), module, Rolzer::TEMPO_PARAM));

        // Waveform outputs (raw node capVoltage — the "brown jack" sandrode output)
        for (int r = 0; r < Rolzer::NUM_ROLLS; r++)
        {
            float x = portSpacing * (r + 1);
            addOutput(createOutputCentered<PJ301MPort>(
                Vec(x, wavePortY), module, Rolzer::WAVE_OUTPUT + r));
        }

        // Gate outputs (assignable to any node)
        for (int g = 0; g < Rolzer::NUM_GATE_OUTPUTS; g++)
        {
            float x = portSpacing * (g + 1);
            addOutput(createOutputCentered<PJ301MPort>(
                Vec(x, gatePortY), module, Rolzer::GATE_OUTPUT + g));
        }
    }

    void appendContextMenu(Menu *menu) override
    {
        Rolzer *module = dynamic_cast<Rolzer *>(this->module);
        if (!module) return;

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Rolzer"));

        menu->addChild(createMenuItem("Clear All Connections", "",
            [=]() { module->connections.clear(); }));

        menu->addChild(createMenuItem("Reset Gate Assignments", "",
            [=]() {
                for (int g = 0; g < Rolzer::NUM_GATE_OUTPUTS; g++)
                    module->gateSourceNode[g] = module->rollNodeOffset[g];
            }));
    }
};
