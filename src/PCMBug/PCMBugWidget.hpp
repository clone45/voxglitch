#pragma once

struct PCMBugWidget : ModuleWidget
{
    PCMBugWidget(PCMBug *module) {
        setModule(module);

        // Load panel using PanelHelper
        PanelHelper panelHelper(this);
        panelHelper.loadPanel(asset::plugin(pluginInstance, "res/modules/pcm_bug/pcm_bug_panel.svg"));

        // Create UI elements manually since we don't have named elements in the copied panel
        
        // Top section - Main corruption parameters (3 columns)
        float y_pos = 40.0f;
        float x_center = RACK_GRID_WIDTH * 6; // Assume 12HP module
        float knob_spacing = 40.0f;
        
        // Row 1: Bit Offset, Drift, Chunk Size
        addParam(createParamCentered<RoundBlackKnob>(
            Vec(x_center - knob_spacing, y_pos), module, PCMBug::BIT_OFFSET_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(
            Vec(x_center, y_pos), module, PCMBug::DRIFT_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(
            Vec(x_center + knob_spacing, y_pos), module, PCMBug::CHUNK_SIZE_PARAM));
        
        y_pos += 55.0f;
        
        // Row 2: Bit Depth, Endian Flip, Probability
        addParam(createParamCentered<RoundBlackKnob>(
            Vec(x_center - knob_spacing, y_pos), module, PCMBug::BIT_DEPTH_MISMATCH_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(
            Vec(x_center, y_pos), module, PCMBug::ENDIAN_FLIP_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(
            Vec(x_center + knob_spacing, y_pos), module, PCMBug::CORRUPTION_PROBABILITY_PARAM));
        
        y_pos += 55.0f;
        
        // Row 3: Sample Rate, Buffer Scramble, Filter
        addParam(createParamCentered<RoundBlackKnob>(
            Vec(x_center - knob_spacing, y_pos), module, PCMBug::SAMPLE_RATE_CORRUPTION_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(
            Vec(x_center, y_pos), module, PCMBug::BUFFER_SCRAMBLE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(
            Vec(x_center + knob_spacing, y_pos), module, PCMBug::FILTER_PARAM));
        
        y_pos += 55.0f;
        
        // Row 4: Mix (centered)
        addParam(createParamCentered<RoundBlackKnob>(
            Vec(x_center, y_pos), module, PCMBug::CORRUPTION_MIX_PARAM));
        
        y_pos += 70.0f;
        
        // CV inputs section - arranged to match knob layout
        float cv_y = y_pos;
        float port_spacing = knob_spacing; // Use same spacing as knobs
        
        // Row 1: CV inputs for top row knobs (Bit Offset, Drift, Chunk Size)
        addInput(createInputCentered<PJ301MPort>(
            Vec(x_center - port_spacing, cv_y), module, PCMBug::BIT_OFFSET_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(
            Vec(x_center, cv_y), module, PCMBug::DRIFT_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(
            Vec(x_center + port_spacing, cv_y), module, PCMBug::CHUNK_SIZE_CV_INPUT));
        
        cv_y += 35.0f;
        
        // Row 2: CV inputs for second row knobs (Probability, Sample Rate, Buffer Scramble)
        addInput(createInputCentered<PJ301MPort>(
            Vec(x_center - port_spacing, cv_y), module, PCMBug::CORRUPTION_PROBABILITY_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(
            Vec(x_center, cv_y), module, PCMBug::SAMPLE_RATE_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(
            Vec(x_center + port_spacing, cv_y), module, PCMBug::BUFFER_SCRAMBLE_CV_INPUT));
        
        cv_y += 35.0f;
        
        // Row 3: Filter CV input (centered)
        addInput(createInputCentered<PJ301MPort>(
            Vec(x_center, cv_y), module, PCMBug::FILTER_CV_INPUT));
        
        cv_y += 45.0f;
        
        // Trigger input (centered)
        addInput(createInputCentered<PJ301MPort>(
            Vec(x_center, cv_y), module, PCMBug::TRIGGER_INPUT));
        
        // Audio inputs/outputs - positioned to the right of knobs
        float audio_x = x_center + knob_spacing * 2.0f; // Right of the rightmost knobs
        float audio_y_start = 60.0f; // Start higher up, aligned with knobs
        
        // Audio inputs (vertical stack)
        addInput(createInputCentered<PJ301MPort>(
            Vec(audio_x, audio_y_start), module, PCMBug::AUDIO_L_INPUT));
        addInput(createInputCentered<PJ301MPort>(
            Vec(audio_x, audio_y_start + 40.0f), module, PCMBug::AUDIO_R_INPUT));
        
        // Audio outputs (below inputs)
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(audio_x, audio_y_start + 90.0f), module, PCMBug::AUDIO_L_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(audio_x, audio_y_start + 130.0f), module, PCMBug::AUDIO_R_OUTPUT));
        
        // Corruption light (below outputs)
        addChild(createLightCentered<MediumLight<RedLight>>(
            Vec(audio_x, audio_y_start + 170.0f), module, PCMBug::CORRUPTION_LIGHT));
    }

    void appendContextMenu(Menu *menu) override {
        PCMBug *pcm_module = dynamic_cast<PCMBug *>(this->module);
        if (!pcm_module) return;

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("PCM Bug - Digital Corruption Effects"));
    }
};