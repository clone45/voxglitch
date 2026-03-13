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
        float bottomBar = 48.0f;
        canvas = new RolzerCanvas();
        canvas->module = module;
        canvas->box.pos = Vec(2.0f, 2.0f);
        canvas->box.size = Vec(box.size.x - 4.0f, box.size.y - bottomBar);
        addChild(canvas);

        // Bottom strip layout
        float spacing = box.size.x / (Rolzer::NUM_ROLLS + 1);
        float knobY = box.size.y - 36.0f;
        float portY = box.size.y - 14.0f;

        // Tempo knobs (one per roll)
        for (int r = 0; r < Rolzer::NUM_ROLLS; r++)
        {
            float x = spacing * (r + 1);
            addParam(createParamCentered<RoundSmallBlackKnob>(
                Vec(x, knobY), module, Rolzer::TEMPO_PARAM + r));
        }

        // Gate outputs (assignable to any node)
        for (int g = 0; g < Rolzer::NUM_GATE_OUTPUTS; g++)
        {
            float x = spacing * (g + 1);
            addOutput(createOutputCentered<PJ301MPort>(
                Vec(x, portY), module, Rolzer::GATE_OUTPUT + g));
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
