#pragma once
// VXDrumsWidget.hpp — the VX Drums panel. 33 HP (495 px).
//
// House panel convention (TimelineWidget.hpp:4-12): every control position
// comes from a named marker in the panel SVG, read through
// PanelHelper::findNamed. The ids below are the contract between this file
// and res/modules/vx_drums/vx_drums_panel.svg (DESIGN.md §3.4, DESIGN-KITS.md
// §5-6); the panel author works from the same list, so the two must never
// drift. Mirroring the ids in static tables here is the Timeline idiom
// (TimelineWidget.hpp:126-134).
//
// The jack column left of the knobs carries the four polyphonic CV inputs,
// one per knob row (tune_cv_input .. level_cv_input).
//
// Two things are drawn in code: the KIT display (a dark pill with the kit
// name, sized from the `kit_display` rect; a left click opens the kit menu)
// and nothing else. Labels are paths in the art. `module` is NULL in the
// module browser; every path here tolerates that.

#include <memory>
#include <string>
#include <vector>

#include "VXDrums.hpp"

// ── The kit display ──────────────────────────────────────────────────────────
//
// TransparentWidget: empty space around it must still drag the module
// (PianoRollControlBar.hpp:51-58). The plate is drawn in draw(); the amber
// text is emissive, so it is drawn in drawLayer(args, 1) — the browser has no
// layer-1 pass, so a NULL module draws it from draw() as well
// (VXDrumSequencerBpmDisplay.hpp:13-15). A " *" suffix says at least one
// column is overriding the kit's choice; the ▼ at the right is a path, since
// the font may not carry the glyph.
//
struct VXDrumsKitDisplay : TransparentWidget
{
    VXDrums* module = NULL;

    VXDrumsKitDisplay(VXDrums* module, rack::Rect bounds)
    {
        this->module = module;
        box.pos = bounds.pos;
        box.size = bounds.size;
    }

    // Null-safe: the browser shows the default kit.
    std::string label() const
    {
        vx_drums::KitId kit = module ? module->kit : vx_drums::KIT_HOUSE;
        std::string s(vx_drums::kitSpec(kit).name);
        for (size_t i = 0; i < s.size(); i++)
            if (s[i] >= 'a' && s[i] <= 'z') s[i] = (char)(s[i] - 'a' + 'A');
        if (module && module->anyOverride()) s += " *";
        return s;
    }

    void draw(const DrawArgs& args) override
    {
        NVGcontext* vg = args.vg;
        nvgSave(vg);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0.f, 0.f, box.size.x, box.size.y, 3.f);
        nvgFillColor(vg, nvgRGB(0x14, 0x15, 0x19));
        nvgFill(vg);

        if (!module) drawText(vg);

        nvgRestore(vg);
        TransparentWidget::draw(args);
    }

    void drawLayer(const DrawArgs& args, int layer) override
    {
        if (layer == 1 && module)
        {
            nvgSave(args.vg);
            drawText(args.vg);
            nvgRestore(args.vg);
        }
        TransparentWidget::drawLayer(args, layer);
    }

    void drawText(NVGcontext* vg)
    {
        const NVGcolor amber = nvgRGB(0xd8, 0xc9, 0x4a);

        // The ▼: 6 px wide, 4 px tall, 6 px in from the right edge, centred.
        const float tx = box.size.x - 6.f;
        const float ty = box.size.y * 0.5f;
        nvgBeginPath(vg);
        nvgMoveTo(vg, tx - 6.f, ty - 2.f);
        nvgLineTo(vg, tx,       ty - 2.f);
        nvgLineTo(vg, tx - 3.f, ty + 2.f);
        nvgClosePath(vg);
        nvgFillColor(vg, amber);
        nvgFill(vg);

        std::shared_ptr<Font> font = APP->window->loadFont(
            asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
        if (!font) return;

        nvgFontFaceId(vg, font->handle);
        nvgFontSize(vg, 11.f);
        nvgTextLetterSpacing(vg, 0.f);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFillColor(vg, amber);
        nvgText(vg, 7.f, ty, label().c_str(), NULL);
    }

    // Left press: consume (so the click does not start a module drag) and open
    // the kit menu. Right press is left alone so the module's own context
    // menu still opens over the display (PianoRollEditorWidget.hpp:1622-1642).
    void onButton(const ButtonEvent& e) override
    {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            e.consume(this);
            if (module) openKitMenu();
            return;
        }
        TransparentWidget::onButton(e);
    }

    void openKitMenu()
    {
        VXDrums* m = module;
        if (!m) return;

        ui::Menu* menu = createMenu();
        menu->addChild(createMenuLabel("Kit"));
        for (int k = 0; k < vx_drums::NUM_KITS; k++)
        {
            vx_drums::KitId kit = (vx_drums::KitId)k;
            menu->addChild(createMenuItem(vx_drums::kitSpec(kit).name, CHECKMARK(m->kit == kit),
                [m, kit]() { vxDrumsChangeKit(m, kit); }));
        }
    }
};

// ── The panel ────────────────────────────────────────────────────────────────

struct VXDrumsWidget : ModuleWidget
{
    VXDrumsWidget(VXDrums* module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/vx_drums/vx_drums_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/vx_drums/vx_drums_panel-dark.svg")
        );

        // Top screws only: this panel sits ABOVE VX Drum Sequencer, which carries the
        // bottom pair, so the stacked pair reads as one machine with four screws.
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));

        struct Anchor { const char* id; int pid; };

        // ── the kit strip: 6 voice columns × TUNE DECAY SHAPE LEVEL + the
        //    master column (ACCENT, gap, DRIVE, VOLUME), all stock RoundBlackKnob ──
        static const Anchor KNOBS[VXDrums::NUM_PARAMS] = {
            { "bd_tune_knob",    VXDrums::BD_TUNE_PARAM    },
            { "bd_decay_knob",   VXDrums::BD_DECAY_PARAM   },
            { "bd_shape_knob",   VXDrums::BD_SHAPE_PARAM   },
            { "bd_level_knob",   VXDrums::BD_LEVEL_PARAM   },

            { "sd_tune_knob",    VXDrums::SD_TUNE_PARAM    },
            { "sd_decay_knob",   VXDrums::SD_DECAY_PARAM   },
            { "sd_shape_knob",   VXDrums::SD_SHAPE_PARAM   },
            { "sd_level_knob",   VXDrums::SD_LEVEL_PARAM   },

            { "cp_tune_knob",    VXDrums::CP_TUNE_PARAM    },
            { "cp_decay_knob",   VXDrums::CP_DECAY_PARAM   },
            { "cp_shape_knob",   VXDrums::CP_SHAPE_PARAM   },
            { "cp_level_knob",   VXDrums::CP_LEVEL_PARAM   },

            { "perc_tune_knob",  VXDrums::PERC_TUNE_PARAM  },
            { "perc_decay_knob", VXDrums::PERC_DECAY_PARAM },
            { "perc_shape_knob", VXDrums::PERC_SHAPE_PARAM },
            { "perc_level_knob", VXDrums::PERC_LEVEL_PARAM },

            { "ch_tune_knob",    VXDrums::CH_TUNE_PARAM    },
            { "ch_decay_knob",   VXDrums::CH_DECAY_PARAM   },
            { "ch_shape_knob",   VXDrums::CH_SHAPE_PARAM   },
            { "ch_level_knob",   VXDrums::CH_LEVEL_PARAM   },

            { "oh_tune_knob",    VXDrums::OH_TUNE_PARAM    },
            { "oh_decay_knob",   VXDrums::OH_DECAY_PARAM   },
            { "oh_shape_knob",   VXDrums::OH_SHAPE_PARAM   },
            { "oh_level_knob",   VXDrums::OH_LEVEL_PARAM   },

            { "m_accent_knob",   VXDrums::M_ACCENT_PARAM   },
            { "m_drive_knob",    VXDrums::M_DRIVE_PARAM    },
            { "m_volume_knob",   VXDrums::M_VOLUME_PARAM   },
            { "m_vary_knob",     VXDrums::M_VARY_PARAM     },
        };
        for (int i = 0; i < VXDrums::NUM_PARAMS; i++)
            addParam(createParamCentered<RoundBlackKnob>(
                panelHelper.findNamed(KNOBS[i].id), module, KNOBS[i].pid));

        // ── strike lamps, one above each voice column ──
        static const char* const LIGHTS[vx_drums::COLUMNS] = {
            "bd_light", "sd_light", "cp_light", "perc_light", "ch_light", "oh_light",
        };
        for (int c = 0; c < vx_drums::COLUMNS; c++)
            addChild(createLightCentered<MediumLight<YellowLight>>(
                panelHelper.findNamed(LIGHTS[c]), module, VXDrums::VOICE_LIGHTS + c));

        // ── inputs: TRIG carries the six-channel bus, so it is a
        //    VoxglitchPolyPort (TimelineWidget.hpp:9-12); ACC is a plain gate ──
        addInput(createInputCentered<VoxglitchPolyPort>(
            panelHelper.findNamed("trig_input"), module, VXDrums::TRIG_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(
            panelHelper.findNamed("acc_input"), module, VXDrums::ACC_INPUT));

        // ── the CV column: one poly jack per knob row, left of the knobs.
        //    Channel c = column c's knob, channel 7 = the row's master knob ──
        static const Anchor CV_INPUTS[VXDrums::KNOBS_PER_COLUMN] = {
            { "tune_cv_input",  VXDrums::TUNE_CV_INPUT  },
            { "decay_cv_input", VXDrums::DECAY_CV_INPUT },
            { "shape_cv_input", VXDrums::SHAPE_CV_INPUT },
            { "level_cv_input", VXDrums::LEVEL_CV_INPUT },
        };
        for (int i = 0; i < VXDrums::KNOBS_PER_COLUMN; i++)
            addInput(createInputCentered<VoxglitchPolyPort>(
                panelHelper.findNamed(CV_INPUTS[i].id), module, CV_INPUTS[i].pid));

        // ── outputs, on the SVG's ink plate: the mix pair, then the dry outs ──
        static const Anchor OUTS[VXDrums::NUM_OUTPUTS] = {
            { "left_output",  VXDrums::LEFT_OUTPUT  },
            { "right_output", VXDrums::RIGHT_OUTPUT },
            { "bd_output",    VXDrums::BD_OUTPUT    },
            { "sd_output",    VXDrums::SD_OUTPUT    },
            { "cp_output",    VXDrums::CP_OUTPUT    },
            { "perc_output",  VXDrums::PERC_OUTPUT  },
            { "ch_output",    VXDrums::CH_OUTPUT    },
            { "oh_output",    VXDrums::OH_OUTPUT    },
        };
        for (int i = 0; i < VXDrums::NUM_OUTPUTS; i++)
            addOutput(createOutputCentered<VoxglitchOutputPort>(
                panelHelper.findNamed(OUTS[i].id), module, OUTS[i].pid));

        // ── the KIT display, top right. Added even without a module so the
        //    browser preview shows the plate (PianoRollWidget.hpp:101-102). ──
        rack::Rect kit_rect = panelHelper.findNamedRect("kit_display");
        if (kit_rect.size.x > 0.f && kit_rect.size.y > 0.f)
        {
            addChild(new VXDrumsKitDisplay(module, kit_rect));
        }
        else
        {
            WARN("VXDrums: panel is missing a 'kit_display' rect; the kit selector is menu-only");
        }
    }

    // Double-click on a TUNE/DECAY/SHAPE knob must land on the EFFECTIVE
    // model's default, and the kit can change under the widget (menus, undo,
    // patch load), so the defaults are re-synced every frame. 18 lookups.
    void step() override
    {
        ModuleWidget::step();

        VXDrums* m = dynamic_cast<VXDrums*>(this->module);
        if (m) m->syncKnobDefaults();
    }

    // Reset every column's TUNE/DECAY/SHAPE to its effective model's default,
    // as ONE undo step: a history::ComplexAction of Rack's own ParamChange
    // entries (history.hpp), applied through the engine the way ParamChange
    // itself replays them. LEVEL and the master knobs are left alone.
    void resetKnobsToModelDefaults(VXDrums* m)
    {
        if (!m) return;

        history::ComplexAction* complex = new history::ComplexAction;
        complex->name = "reset knobs to model defaults";

        for (int c = 0; c < vx_drums::COLUMNS; c++)
        {
            const vx_drums::ModelSpec& ms = vx_drums::modelSpec(m->effectiveModel(c));
            const vx_drums::KnobSpec* specs[3] = { &ms.tune, &ms.decay, &ms.shape };
            for (int which = 0; which < 3; which++)
            {
                int pid = c * VXDrums::KNOBS_PER_COLUMN + which;
                float old_value = m->params[pid].getValue();
                float new_value = (float)vx_drums::knobDefault01(*specs[which]);
                if (old_value == new_value) continue;

                APP->engine->setParamValue(m, pid, new_value);

                history::ParamChange* change = new history::ParamChange;
                change->moduleId = m->id;
                change->paramId = pid;
                change->oldValue = old_value;
                change->newValue = new_value;
                complex->push(change);
            }
        }

        if (complex->isEmpty()) { delete complex; return; }
        APP->history->push(complex);
    }

    // Module-wide settings live on the panel menu (PianoRollWidget.hpp:135-138):
    // the kit, the six per-column model overrides, and the knob reset.
    void appendContextMenu(Menu* menu) override
    {
        VXDrums* m = dynamic_cast<VXDrums*>(this->module);
        if (!m) return;

        menu->addChild(new MenuSeparator);

        std::vector<std::string> kit_names;
        for (int k = 0; k < vx_drums::NUM_KITS; k++)
            kit_names.push_back(vx_drums::kitSpec((vx_drums::KitId)k).name);
        menu->addChild(createIndexSubmenuItem("Kit", kit_names,
            [m]() { return (size_t)m->kit; },
            [m](size_t k) { vxDrumsChangeKit(m, (vx_drums::KitId)k); }));

        static const char* const COLUMN_MENU[vx_drums::COLUMNS] = {
            "Bass drum model", "Snare model", "Clap model", "Percussion model", "Closed hat model", "Open hat model",
        };
        for (int c = 0; c < vx_drums::COLUMNS; c++)
        {
            menu->addChild(createSubmenuItem(COLUMN_MENU[c], "", [m, c](ui::Menu* sub) {
                const char* kit_model = vx_drums::modelSpec(vx_drums::kitSpec(m->kit).models[c]).name;
                sub->addChild(createMenuItem(std::string("Kit default (") + kit_model + ")",
                    CHECKMARK(m->model_override[c] == VXDrums::NO_OVERRIDE),
                    [m, c]() { vxDrumsChangeOverride(m, c, VXDrums::NO_OVERRIDE); }));
                sub->addChild(new MenuSeparator);
                for (int i = 0; i < vx_drums::NUM_MODELS; i++)
                {
                    sub->addChild(createMenuItem(vx_drums::modelSpec((vx_drums::ModelId)i).name,
                        CHECKMARK(m->model_override[c] == i),
                        [m, c, i]() { vxDrumsChangeOverride(m, c, i); }));
                }
            }));
        }

        VXDrumsWidget* self = this;
        menu->addChild(createMenuItem("Reset knobs to model defaults", "",
            [self, m]() { self->resetKnobsToModelDefaults(m); }));
    }
};
