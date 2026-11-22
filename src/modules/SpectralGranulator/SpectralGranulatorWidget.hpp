#pragma once
//
// Spectral Granulator — the panel. 24 HP: knobs on top, a CV jack for every knob
// underneath in the same order, the audio and analysis jacks along the bottom.
//
// Control POSITIONS come from named markers in the panel SVG through
// PanelHelper::findNamed / findNamedRect; the ids are the contract with
// scripts/generate_spectral_granulator_panel.py, which writes the panel, and are
// mirrored in the tables below so a missing one is visible in one place
// (the Euclid / Timeline Anchor idiom).
//
// Panel: res/modules/spectral_granulator/spectral_granulator_panel.svg (+ -dark).
//

struct SpectralGranulatorWidget : ModuleWidget
{
    SpectralGranulatorWidget(SpectralGranulator* module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/spectral_granulator/spectral_granulator_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/spectral_granulator/spectral_granulator_panel-dark.svg"));

        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        struct Anchor { const char* id; int pid; };

        // Row A then row B, left to right.
        static const Anchor KNOBS[16] = {
            { "focus_knob",    SpectralGranulator::FOCUS_PARAM    },
            { "width_knob",    SpectralGranulator::WIDTH_PARAM    },
            { "smooth_knob",   SpectralGranulator::SMOOTH_PARAM   },
            { "sens_knob",     SpectralGranulator::SENS_PARAM     },
            { "pitch_knob",    SpectralGranulator::PITCH_PARAM    },
            { "tune_knob",     SpectralGranulator::TUNE_PARAM     },
            { "step_knob",     SpectralGranulator::STEP_PARAM     },
            { "resync_knob",   SpectralGranulator::RESYNC_PARAM   },
            { "stretch_knob",  SpectralGranulator::STRETCH_PARAM  },
            { "grain_knob",    SpectralGranulator::GRAIN_PARAM    },
            { "blend_knob",    SpectralGranulator::BLEND_PARAM    },
            { "feedback_knob", SpectralGranulator::FEEDBACK_PARAM },
            { "jitter_knob",   SpectralGranulator::JITTER_PARAM   },
            { "density_knob",  SpectralGranulator::DENSITY_PARAM  },
            { "spread_knob",   SpectralGranulator::SPREAD_PARAM   },
            { "imprint_knob",  SpectralGranulator::IMPRINT_PARAM  },
        };
        for (int i = 0; i < 16; i++)
        {
            addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed(KNOBS[i].id), module, KNOBS[i].pid));
        }

        static const Anchor TRIMPOTS[3] = {
            { "stretch_atten_knob", SpectralGranulator::STRETCH_ATTEN_PARAM },
            { "grain_atten_knob",   SpectralGranulator::GRAIN_ATTEN_PARAM   },
            { "blend_atten_knob",   SpectralGranulator::BLEND_ATTEN_PARAM   },
        };
        for (int i = 0; i < 3; i++)
        {
            addParam(createParamCentered<Trimpot>(panelHelper.findNamed(TRIMPOTS[i].id), module, TRIMPOTS[i].pid));
        }

        // One CV jack per knob, in the knob's own column, then the bottom row.
        static const Anchor INS[19] = {
            { "focus_cv_input",    SpectralGranulator::FOCUS_CV_INPUT    },
            { "width_cv_input",    SpectralGranulator::WIDTH_CV_INPUT    },
            { "smooth_cv_input",   SpectralGranulator::SMOOTH_CV_INPUT   },
            { "sens_cv_input",     SpectralGranulator::SENS_CV_INPUT     },
            { "pitch_cv_input",    SpectralGranulator::PITCH_CV_INPUT    },
            { "tune_cv_input",     SpectralGranulator::TUNE_CV_INPUT     },
            { "step_cv_input",     SpectralGranulator::STEP_CV_INPUT     },
            { "resync_cv_input",   SpectralGranulator::RESYNC_CV_INPUT   },
            { "stretch_cv_input",  SpectralGranulator::STRETCH_CV_INPUT  },
            { "grain_cv_input",    SpectralGranulator::GRAIN_CV_INPUT    },
            { "blend_cv_input",    SpectralGranulator::BLEND_CV_INPUT    },
            { "feedback_cv_input", SpectralGranulator::FEEDBACK_CV_INPUT },
            { "jitter_cv_input",   SpectralGranulator::JITTER_CV_INPUT   },
            { "density_cv_input",  SpectralGranulator::DENSITY_CV_INPUT  },
            { "spread_cv_input",   SpectralGranulator::SPREAD_CV_INPUT   },
            { "imprint_cv_input",  SpectralGranulator::IMPRINT_CV_INPUT  },
            { "in_l_input",        SpectralGranulator::IN_L_INPUT        },
            { "in_r_input",        SpectralGranulator::IN_R_INPUT        },
            { "trig_input",        SpectralGranulator::TRIG_INPUT        },
        };
        for (int i = 0; i < 19; i++)
        {
            addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed(INS[i].id), module, INS[i].pid));
        }

        static const Anchor OUTS[6] = {
            { "onset_output",  SpectralGranulator::ONSET_OUTPUT  },
            { "level_output",  SpectralGranulator::LEVEL_OUTPUT  },
            { "bright_output", SpectralGranulator::BRIGHT_OUTPUT },
            { "tonal_output",  SpectralGranulator::TONAL_OUTPUT  },
            { "out_l_output",  SpectralGranulator::OUT_L_OUTPUT  },
            { "out_r_output",  SpectralGranulator::OUT_R_OUTPUT  },
        };
        for (int i = 0; i < 6; i++)
        {
            addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed(OUTS[i].id), module, OUTS[i].pid));
        }

        // The display, sized from the panel's plate; the generator's rect if
        // the panel has lost it.
        rack::Rect display_rect = panelHelper.findNamedRect("spectrum_display");
        if (!(display_rect.size.x > 0.f && display_rect.size.y > 0.f))
        {
            WARN("Spectral Granulator: panel is missing a 'spectrum_display' rect; using the generator's 342 x 50 box at (9, 30)");
            display_rect.pos = Vec(9.f, 30.f);
            display_rect.size = Vec(342.f, 50.f);
        }
        addChild(new SpectralGranulatorDisplay(module, display_rect));
    }
};
