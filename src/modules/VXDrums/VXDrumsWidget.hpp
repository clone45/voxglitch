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
// and the drop targets that appear over the six voice columns while a sample
// is being dragged in from stoermelder SIREN (VXDrumsDropTargets). Labels
// are paths in the art. `module` is NULL in the module browser; every path
// here tolerates that.

#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <typeinfo>
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
namespace vx_drums_ui
{
    // The kit list every kit menu shows: the factory kits, then, after a
    // separator, the user's saved kits (VXDrumKitStore.hpp), each checked by
    // uuid. Picking a factory kit keeps the knobs (the physical-knob rule);
    // picking a user kit loads its knobs too, one undo step either way.
    inline void addKitChoices(ui::Menu* menu, VXDrums* m)
    {
        for (int k = 0; k < vx_drums::NUM_KITS; k++)
        {
            const vx_drums::KitState kit = vx_drums::KitState::factory((vx_drums::KitId)k);
            menu->addChild(createMenuItem(kit.name, CHECKMARK(m->kit.sameKit(kit)),
                [m, kit]() { vxDrumsChangeKit(m, kit); }));
        }

        const vx_drums::KitLibrary& lib = vx_drums::kitStore().library;
        if (lib.kits.empty()) return;

        menu->addChild(new MenuSeparator);
        for (size_t i = 0; i < lib.kits.size(); i++)
        {
            const vx_drums::UserKit k = lib.kits[i];   // a copy: the library may change before the lambda runs
            menu->addChild(createMenuItem(k.kit.name, CHECKMARK(m->kit.sameKit(k.kit)),
                [m, k]() { vxDrumsLoadUserKit(m, k); }));
        }
    }

    // A name from the user, or "" if they cancelled or typed nothing.
    inline std::string promptKitName(const char* message, const std::string& initial)
    {
        char* text = osdialog_prompt(OSDIALOG_INFO, message, initial.c_str());
        if (!text) return "";
        std::string name = text;
        std::free(text);
        // Trim.
        const size_t a = name.find_first_not_of(" \t\r\n");
        const size_t b = name.find_last_not_of(" \t\r\n");
        if (a == std::string::npos) return "";
        return name.substr(a, b - a + 1);
    }

    // "Save kit as…": capture the module's current sound under a new name,
    // add it to the library, write the file, and make it the module's kit
    // (overrides are baked into the saved models, so they clear). The library
    // change is not undoable; the module's kit change is.
    inline void saveKitAs(VXDrums* m)
    {
        if (!m) return;
        const std::string name = promptKitName("Name for the new kit:", m->kit.custom ? m->kit.name : "My Kit");
        if (name.empty()) return;

        vx_drums::KitStore& store = vx_drums::kitStore();
        vx_drums::UserKit k = vxDrumsCaptureUserKit(m, vx_drums::mintUuid(), name);
        if (!store.library.add(k)) return;
        if (!store.save())
        {
            osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not write the kit library file.");
        }
        vxDrumsLoadUserKit(m, k);
    }

    // "Update <kit>": overwrite the loaded user kit with the module's current
    // sound; the name stays.
    inline void updateKit(VXDrums* m)
    {
        if (!m || !m->kit.custom) return;
        vx_drums::KitStore& store = vx_drums::kitStore();
        if (!store.library.find(m->kit.uuid)) return;

        vx_drums::UserKit k = vxDrumsCaptureUserKit(m, m->kit.uuid, m->kit.name);
        if (!store.library.update(k)) return;
        if (!store.save())
        {
            osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not write the kit library file.");
        }
        vxDrumsLoadUserKit(m, k);   // clears the overrides now baked in; a no-op otherwise
    }

    inline void renameKit(VXDrums* m, const std::string& uuid)
    {
        vx_drums::KitStore& store = vx_drums::kitStore();
        const vx_drums::UserKit* k = store.library.find(uuid);
        if (!k) return;
        const std::string name = promptKitName("New name for the kit:", k->kit.name);
        if (name.empty() || !store.library.rename(uuid, name)) return;
        store.save();

        // The module showing this kit follows the rename (a display change,
        // not a sound change: no undo step).
        if (m && m->kit.custom && m->kit.uuid == uuid) m->kit.name = name;
    }

    // Make the sample player with this file one column's override (one undo
    // step, like any other model choice), and say so if the file cannot be
    // read. Shared by "Load sample…" and the drop targets.
    inline void applySampleOverride(VXDrums* m, int column, const std::string& path)
    {
        if (!m || column < 0 || column >= vx_drums::COLUMNS || path.empty()) return;
        vxDrumsChangeOverride(m, column, vx_drums::ModelChoice::sample(path));
        if (m->sampleMissing(column))
        {
            osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, ("Could not read \"" + rack::system::getFilename(path) + "\".").c_str());
        }
    }

    // "Load sample…" for one column: pick an audio file. The dialog opens
    // where the column's current file lives, if it has one.
    inline void loadSampleOverride(VXDrums* m, int column)
    {
        if (!m || column < 0 || column >= vx_drums::COLUMNS) return;
        std::string dir;
        const vx_drums::ModelChoice& current = m->effectiveModel(column);
        if (current.isSample() && !current.sample_path.empty()) dir = rack::system::getDirectory(current.sample_path);

        osdialog_filters* filters = osdialog_filters_parse("Audio (WAV or MP3):wav,WAV,mp3,MP3");
        char* path_c = osdialog_file(OSDIALOG_OPEN, dir.empty() ? NULL : dir.c_str(), NULL, filters);
        osdialog_filters_free(filters);
        if (!path_c) return;
        const std::string path = path_c;
        std::free(path_c);

        applySampleOverride(m, column, path);
    }

    // A file dropped on one column. The file dialog filters by extension; a
    // drop cannot, so the check is here. SIREN hands over FLAC as-is unless
    // its "Convert to WAV on drop" option is on, which is the likely way to
    // get here, hence the hint.
    inline void dropSampleOverride(VXDrums* m, int column, const std::string& path)
    {
        const std::string ext = rack::string::lowercase(rack::system::getExtension(path));
        if (ext != ".wav" && ext != "wav" && ext != ".mp3" && ext != "mp3")
        {
            osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK,
                ("VX Drums plays WAV and MP3 files, and \"" + rack::system::getFilename(path) + "\" is neither.\n\n"
                 "If it came from SIREN, turn on \"Convert to WAV on drop\" in SIREN's source menu.").c_str());
            return;
        }
        applySampleOverride(m, column, path);
    }

    // Deleting a kit does not touch any module playing it: the module carries
    // its own copy, and the patch will still load and sound the same.
    inline void deleteKit(const std::string& uuid)
    {
        vx_drums::KitStore& store = vx_drums::kitStore();
        const vx_drums::UserKit* k = store.library.find(uuid);
        if (!k) return;
        const std::string question = "Delete the kit \"" + k->kit.name + "\"? Modules using it keep playing it.";
        if (!osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK_CANCEL, question.c_str())) return;
        if (store.library.remove(uuid)) store.save();
    }
}

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
        std::string s = module ? module->kit.name : std::string(vx_drums::kitSpec(vx_drums::KIT_HOUSE).name);
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
        vx_drums_ui::addKitChoices(menu, m);
    }
};

// ── Drop targets for samples dragged in from stoermelder SIREN ───────────────
//
// SIREN (stoermelder PackOne) is a sample browser. A file dragged out of it
// is an ordinary in-Rack mouse drag that ends in Rack's standard file-drop
// event at the release point (SirenDropHandler.hpp in PackOne), the same
// event a file dragged in from the OS produces. While such a drag is over
// this panel the six voice columns are painted as drop targets, the one under
// the cursor brighter; a release over a column makes the file that column's
// sample override, exactly like "Load sample…". An OS drag gives Rack no
// hover events, so it lands in the right column but without the highlight.
//
// The drag is watched from step(), not from the DragHover/DragEnter events:
// DragHover only arrives while the mouse moves, and every knob is an
// OpaqueWidget that would take DragEnter/DragLeave as the cursor crosses it.
// APP->event->getDraggedWidget() says what is being dragged every frame, and
// the rack's mouse position says where (the idiom SIREN itself uses).
//
// Column rectangles come from the panel anchors: from a column's strike lamp
// down past its LEVEL knob, centred on its TUNE knob, as wide as the pitch
// between two column rules. Added as the LAST child so it paints over the
// knobs; TransparentWidget, so it takes no events away from them.
//
struct VXDrumsDropTargets : TransparentWidget
{
    VXDrums* module = NULL;
    rack::Rect columns[vx_drums::COLUMNS];
    bool active = false;   // a SIREN drag is over the panel this frame
    int hovered = -1;      // the column under the cursor, or -1

    // The dragged widget last examined and the verdict, so the type lookup
    // runs once per drag rather than once per frame.
    widget::Widget* checked_origin = NULL;
    bool checked_is_siren = false;

    VXDrumsDropTargets(VXDrums* module, PanelHelper& panelHelper, Vec panel_size)
    {
        this->module = module;
        box.pos = Vec(0.f, 0.f);
        box.size = panel_size;

        float pitch = panelHelper.findNamed("column_rule_2").x - panelHelper.findNamed("column_rule_1").x;
        if (pitch <= 0.f) pitch = 59.f;   // the panel's column pitch, should the rules go missing
        const float half_width = pitch * 0.5f - 2.f;

        static const char* const PREFIX[vx_drums::COLUMNS] = { "bd", "sd", "cp", "perc", "ch", "oh" };
        for (int c = 0; c < vx_drums::COLUMNS; c++)
        {
            const std::string p = PREFIX[c];
            const float cx     = panelHelper.findNamed(p + "_tune_knob").x;
            const float top    = panelHelper.findNamed(p + "_light").y - 10.f;       // above the lamp and the title
            const float bottom = panelHelper.findNamed(p + "_level_knob").y + 30.f;  // below the LEVEL label
            columns[c] = rack::Rect(Vec(cx - half_width, top), Vec(half_width * 2.f, bottom - top));
        }
    }

    // The column under a panel-local point, or -1.
    int columnAt(Vec pos) const
    {
        for (int c = 0; c < vx_drums::COLUMNS; c++)
            if (columns[c].contains(pos)) return c;
        return -1;
    }

    // A drag from SIREN's file list or waveform pane starts inside its
    // ModuleWidget, so the plugin and module slugs identify it. SIREN's
    // "Ctrl+Shift-click anywhere" drag starts from an overlay it adds to the
    // rack itself, with no module above it, so that one is recognised by its
    // type name. Moving a module, or turning a knob, is never a sample drag.
    static bool isSirenDrag(widget::Widget* w)
    {
        if (dynamic_cast<ModuleWidget*>(w) || dynamic_cast<ParamWidget*>(w) || dynamic_cast<PortWidget*>(w)) return false;
        ModuleWidget* mw = w->getAncestorOfType<ModuleWidget>();
        if (mw)
            return mw->model && mw->model->plugin && mw->model->plugin->slug == "Stoermelder-P1" && mw->model->slug == "Siren";
        return std::strstr(typeid(*w).name(), "Siren") != NULL;
    }

    void step() override
    {
        TransparentWidget::step();
        active = false;
        hovered = -1;
        if (!module || !parent) return;

        widget::Widget* dragged = APP->event->getDraggedWidget();
        if (!dragged) { checked_origin = NULL; return; }
        if (dragged != checked_origin)
        {
            checked_origin = dragged;
            checked_is_siren = isSirenDrag(dragged);
        }
        if (!checked_is_siren) return;

        // Rack-local mouse position to panel-local (the module sits in a
        // container at the rack's origin; SIREN compares the same way).
        const Vec mouse = APP->scene->rack->getMousePos().minus(parent->box.pos);
        if (!rack::Rect(Vec(0.f, 0.f), box.size).contains(mouse)) return;
        active = true;
        hovered = columnAt(mouse);
    }

    // Layer 1, like the KIT text: on top of the knobs and lamps, and not
    // dimmed by the room brightness.
    void drawLayer(const DrawArgs& args, int layer) override
    {
        if (layer == 1 && active)
        {
            NVGcontext* vg = args.vg;
            nvgSave(vg);
            for (int c = 0; c < vx_drums::COLUMNS; c++)
            {
                const bool hot = (c == hovered);
                const rack::Rect& r = columns[c];
                nvgBeginPath(vg);
                nvgRoundedRect(vg, r.pos.x, r.pos.y, r.size.x, r.size.y, 6.f);
                nvgFillColor(vg, nvgRGBA(0xd8, 0xc9, 0x4a, hot ? 0x60 : 0x22));
                nvgFill(vg);
                nvgStrokeWidth(vg, hot ? 2.f : 1.2f);
                nvgStrokeColor(vg, nvgRGBA(0xd8, 0xc9, 0x4a, hot ? 0xff : 0x90));
                nvgStroke(vg);
            }
            nvgRestore(vg);
        }
        TransparentWidget::drawLayer(args, layer);
    }
};

// ── The panel ────────────────────────────────────────────────────────────────

struct VXDrumsWidget : ModuleWidget
{
    VXDrumsDropTargets* drop_targets = NULL;

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
        addInput(createInputCentered<VoxglitchInputPort>(
            panelHelper.findNamed("kit_cv_input"), module, VXDrums::KIT_CV_INPUT));

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

        // ── the SIREN drop targets, last so they paint over everything ──
        drop_targets = new VXDrumsDropTargets(module, panelHelper, box.size);
        addChild(drop_targets);
    }

    // A file dropped on the panel (from SIREN, or from the OS): the column
    // under the drop point takes it as its sample. Anywhere else, Rack's
    // default handling.
    void onPathDrop(const PathDropEvent& e) override
    {
        VXDrums* m = dynamic_cast<VXDrums*>(this->module);
        const int c = (m && drop_targets && !e.paths.empty()) ? drop_targets->columnAt(e.pos) : -1;
        if (c < 0)
        {
            ModuleWidget::onPathDrop(e);
            return;
        }
        e.consume(this);
        vx_drums_ui::dropSampleOverride(m, c, e.paths[0]);
    }

    // Double-click on a TUNE/DECAY/SHAPE knob must land on the EFFECTIVE
    // model's default, and the kit can change under the widget (menus, undo,
    // patch load), so the defaults are re-synced every frame. 18 lookups.
    void step() override
    {
        ModuleWidget::step();

        VXDrums* m = dynamic_cast<VXDrums*>(this->module);
        if (m)
        {
            m->serviceKitCv();       // KIT CV: follow the jack, or return to the manual kit (VXDrums.hpp)
            m->syncKnobDefaults();
            m->housekeepSamples();   // sample buffers replaced since last frame (VXDrums.hpp)
        }
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
            const vx_drums::ModelSpec& ms = vx_drums::modelSpec(m->effectiveModel(c).id);
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
    // the kit, the six per-column model overrides (grouped under "Kit
    // Customization"), the stereo panning switch, and the knob reset.
    void appendContextMenu(Menu* menu) override
    {
        VXDrums* m = dynamic_cast<VXDrums*>(this->module);
        if (!m) return;

        menu->addChild(new MenuSeparator);

        menu->addChild(createSubmenuItem("Kit", m->kit.name, [m](ui::Menu* sub) {
            vx_drums_ui::addKitChoices(sub, m);
        }));

        // The six per-column model overrides, one submenu each, under a
        // single "Kit Customization" entry so the top level stays short.
        menu->addChild(createSubmenuItem("Kit Customization", "", [m](ui::Menu* group) {
            static const char* const COLUMN_MENU[vx_drums::COLUMNS] = {
                "Bass drum model", "Snare model", "Clap model", "Percussion model", "Closed hat model", "Open hat model",
            };
            for (int c = 0; c < vx_drums::COLUMNS; c++)
            {
                group->addChild(createSubmenuItem(COLUMN_MENU[c], "", [m, c](ui::Menu* sub) {
                    const vx_drums::ModelChoice& ov = m->model_override[c];
                    const bool missing = m->sampleMissing(c);

                    std::string kit_model = m->kit.models[c].displayName();
                    if (ov.isNone() && missing) kit_model += " (missing)";
                    sub->addChild(createMenuItem("Kit default (" + kit_model + ")",
                        CHECKMARK(ov.isNone()),
                        [m, c]() { vxDrumsChangeOverride(m, c, vx_drums::ModelChoice::none()); }));
                    sub->addChild(new MenuSeparator);
                    for (int i = 0; i < vx_drums::NUM_MODELS; i++)
                    {
                        const vx_drums::ModelChoice choice((vx_drums::ModelId)i);
                        if (choice.isSample()) continue;   // the sample player is chosen through "Load sample…"
                        sub->addChild(createMenuItem(choice.displayName(),
                            CHECKMARK(ov == choice),
                            [m, c, choice]() { vxDrumsChangeOverride(m, c, choice); }));
                    }

                    // ── The sample player: the loaded file (checked) and the loader ──
                    sub->addChild(new MenuSeparator);
                    if (ov.isSample())
                    {
                        std::string label = ov.displayName();
                        if (missing) label += " (missing)";
                        sub->addChild(createMenuItem(label, CHECKMARK(true), []() {}));
                    }
                    sub->addChild(createMenuItem(ov.isSample() ? "Load another sample…" : "Load sample…", "",
                        [m, c]() { vx_drums_ui::loadSampleOverride(m, c); }));
                }));
            }

            // ── The user's kits (VXDrumKitStore.hpp; Bret, 2026-09-07) ──
            group->addChild(new MenuSeparator);
            group->addChild(createMenuItem("Save kit as…", "", [m]() { vx_drums_ui::saveKitAs(m); }));

            const bool loaded_user_kit = m->kit.custom && vx_drums::kitStore().library.find(m->kit.uuid) != nullptr;
            group->addChild(createMenuItem(loaded_user_kit ? "Update \"" + m->kit.name + "\"" : std::string("Update kit"), "",
                [m]() { vx_drums_ui::updateKit(m); }, !loaded_user_kit));

            const vx_drums::KitLibrary& lib = vx_drums::kitStore().library;
            group->addChild(createSubmenuItem("Manage kits", lib.kits.empty() ? "none saved" : std::to_string(lib.kits.size()) + " saved",
                [m](ui::Menu* kits) {
                    const vx_drums::KitLibrary& lib = vx_drums::kitStore().library;
                    if (lib.kits.empty())
                    {
                        kits->addChild(createMenuLabel("No saved kits"));
                        return;
                    }
                    for (size_t i = 0; i < lib.kits.size(); i++)
                    {
                        const std::string uuid = lib.kits[i].kit.uuid;
                        kits->addChild(createSubmenuItem(lib.kits[i].kit.name, "", [m, uuid](ui::Menu* one) {
                            one->addChild(createMenuItem("Rename…", "", [m, uuid]() { vx_drums_ui::renameKit(m, uuid); }));
                            one->addChild(createMenuItem("Delete…", "", [uuid]() { vx_drums_ui::deleteKit(uuid); }));
                        }));
                    }
                }));
        }));

        // The Machine's fixed per-voice pans on the L/R mix. A plain bool
        // checkbox (the ArpSeqWidget.hpp:602 idiom): no undo step, saved with
        // the patch.
        menu->addChild(createBoolPtrMenuItem("Stereo panning", "", &m->stereo_panning));

        VXDrumsWidget* self = this;
        menu->addChild(createMenuItem("Reset knobs to model defaults", "",
            [self, m]() { self->resetKnobsToModelDefaults(m); }));
    }
};
