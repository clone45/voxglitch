#pragma once

struct ResomatWidget : ModuleWidget
{
    ResomatWidget(Resomat *module)
    {
        setModule(module);

        // Load panel using PanelHelper
        PanelHelper panelHelper(this);
        panelHelper.loadPanel(asset::plugin(pluginInstance, "res/modules/resomat/resomat_panel.svg"));

        // Add parameters - using named elements from the SVG panel
        // Note: These names should match elements in your SVG file
        // For now, using generic positions - adjust based on actual panel layout

        // Pitch knob
        addParam(createParamCentered<RoundLargeBlackKnob>(
            panelHelper.findNamed("pitch_knob"),
            module,
            Resomat::PITCH_PARAM));

        // Damping knob
        addParam(createParamCentered<RoundBlackKnob>(
            panelHelper.findNamed("damping_knob"),
            module,
            Resomat::DAMPING_PARAM));

        // Decay knob
        addParam(createParamCentered<RoundBlackKnob>(
            panelHelper.findNamed("decay_knob"),
            module,
            Resomat::DECAY_PARAM));

        // Equation selector knob
        addParam(createParamCentered<RoundBlackKnob>(
            panelHelper.findNamed("equation_knob"),
            module,
            Resomat::EQUATION_SELECT_PARAM));

        // Equation offset knob
        addParam(createParamCentered<RoundBlackKnob>(
            panelHelper.findNamed("equation_offset_knob"),
            module,
            Resomat::EQUATION_OFFSET_PARAM));

        // Equation rate knob
        addParam(createParamCentered<RoundBlackKnob>(
            panelHelper.findNamed("equation_rate_knob"),
            module,
            Resomat::EQUATION_RATE_PARAM));

        // Rate sync button
        addParam(createParamCentered<LEDButton>(
            panelHelper.findNamed("rate_sync_button"),
            module,
            Resomat::RATE_SYNC_BUTTON));

        // Bleed knob
        addParam(createParamCentered<RoundBlackKnob>(
            panelHelper.findNamed("bleed_knob"),
            module,
            Resomat::BLEED_PARAM));

        // Injection knob
        addParam(createParamCentered<RoundBlackKnob>(
            panelHelper.findNamed("injection_knob"),
            module,
            Resomat::INJECTION_PARAM));

        // Source button
        addParam(createParamCentered<LEDButton>(
            panelHelper.findNamed("source_button"),
            module,
            Resomat::SOURCE_BUTTON));

        // P1 knob
        addParam(createParamCentered<RoundBlackKnob>(
            panelHelper.findNamed("p1_knob"),
            module,
            Resomat::P1_PARAM));

        // P2 knob
        addParam(createParamCentered<RoundBlackKnob>(
            panelHelper.findNamed("p2_knob"),
            module,
            Resomat::P2_PARAM));

        // P3 knob
        addParam(createParamCentered<RoundBlackKnob>(
            panelHelper.findNamed("p3_knob"),
            module,
            Resomat::P3_PARAM));

        // Trigger button
        addParam(createParamCentered<LEDButton>(
            panelHelper.findNamed("trigger_button"),
            module,
            Resomat::TRIGGER_BUTTON));

        // Attenuverter knobs
        addParam(createParamCentered<Trimpot>(
            panelHelper.findNamed("damping_atten_knob"),
            module,
            Resomat::DAMPING_ATTEN_PARAM));

        addParam(createParamCentered<Trimpot>(
            panelHelper.findNamed("decay_atten_knob"),
            module,
            Resomat::DECAY_ATTEN_PARAM));

        addParam(createParamCentered<Trimpot>(
            panelHelper.findNamed("equation_atten_knob"),
            module,
            Resomat::EQUATION_ATTEN_PARAM));

        addParam(createParamCentered<Trimpot>(
            panelHelper.findNamed("offset_atten_knob"),
            module,
            Resomat::OFFSET_ATTEN_PARAM));

        addParam(createParamCentered<Trimpot>(
            panelHelper.findNamed("rate_atten_knob"),
            module,
            Resomat::RATE_ATTEN_PARAM));

        addParam(createParamCentered<Trimpot>(
            panelHelper.findNamed("bleed_atten_knob"),
            module,
            Resomat::BLEED_ATTEN_PARAM));

        addParam(createParamCentered<Trimpot>(
            panelHelper.findNamed("injection_atten_knob"),
            module,
            Resomat::INJECTION_ATTEN_PARAM));

        addParam(createParamCentered<Trimpot>(
            panelHelper.findNamed("p1_atten_knob"),
            module,
            Resomat::P1_ATTEN_PARAM));

        addParam(createParamCentered<Trimpot>(
            panelHelper.findNamed("p2_atten_knob"),
            module,
            Resomat::P2_ATTEN_PARAM));

        addParam(createParamCentered<Trimpot>(
            panelHelper.findNamed("p3_atten_knob"),
            module,
            Resomat::P3_ATTEN_PARAM));

        // V/Oct input
        addInput(createInputCentered<PJ301MPort>(
            panelHelper.findNamed("voct_input"),
            module,
            Resomat::VOCT_INPUT));

        // Trigger input
        addInput(createInputCentered<PJ301MPort>(
            panelHelper.findNamed("trigger_input"),
            module,
            Resomat::TRIGGER_INPUT));

        // CV inputs
        addInput(createInputCentered<PJ301MPort>(
            panelHelper.findNamed("damping_input"),
            module,
            Resomat::DAMPING_INPUT));

        addInput(createInputCentered<PJ301MPort>(
            panelHelper.findNamed("decay_input"),
            module,
            Resomat::DECAY_INPUT));

        addInput(createInputCentered<PJ301MPort>(
            panelHelper.findNamed("equation_input"),
            module,
            Resomat::EQUATION_INPUT));

        addInput(createInputCentered<PJ301MPort>(
            panelHelper.findNamed("offset_input"),
            module,
            Resomat::OFFSET_INPUT));

        addInput(createInputCentered<PJ301MPort>(
            panelHelper.findNamed("rate_input"),
            module,
            Resomat::RATE_INPUT));

        addInput(createInputCentered<PJ301MPort>(
            panelHelper.findNamed("bleed_input"),
            module,
            Resomat::BLEED_INPUT));

        addInput(createInputCentered<PJ301MPort>(
            panelHelper.findNamed("injection_input"),
            module,
            Resomat::INJECTION_INPUT));

        addInput(createInputCentered<PJ301MPort>(
            panelHelper.findNamed("p1_input"),
            module,
            Resomat::P1_INPUT));

        addInput(createInputCentered<PJ301MPort>(
            panelHelper.findNamed("p2_input"),
            module,
            Resomat::P2_INPUT));

        addInput(createInputCentered<PJ301MPort>(
            panelHelper.findNamed("p3_input"),
            module,
            Resomat::P3_INPUT));

        // Audio output
        addOutput(createOutputCentered<PJ301MPort>(
            panelHelper.findNamed("audio_output"),
            module,
            Resomat::AUDIO_OUTPUT));

        // Trigger light
        addChild(createLightCentered<MediumLight<RedLight>>(
            panelHelper.findNamed("trigger_light"),
            module,
            Resomat::TRIGGER_LIGHT));

        // Source mode LEDs
        addChild(createLightCentered<MediumLight<GreenLight>>(
            panelHelper.findNamed("noise_light"),
            module,
            Resomat::NOISE_LIGHT));

        addChild(createLightCentered<MediumLight<GreenLight>>(
            panelHelper.findNamed("equation_light"),
            module,
            Resomat::EQUATION_LIGHT));

        addChild(createLightCentered<MediumLight<GreenLight>>(
            panelHelper.findNamed("equation_continuous_light"),
            module,
            Resomat::EQUATION_CONTINUOUS_LIGHT));

        // Rate sync LED
        addChild(createLightCentered<MediumLight<GreenLight>>(
            panelHelper.findNamed("rate_sync_light"),
            module,
            Resomat::RATE_SYNC_LIGHT));
    }

    void appendContextMenu(Menu *menu) override
    {
        Resomat *module = dynamic_cast<Resomat *>(this->module);
        if (!module) return;

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Resomat Settings"));
    }
};
