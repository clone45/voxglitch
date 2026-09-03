#pragma once
//
// VX Drum Sequencer — the panel. 30 HP.
//
// Control POSITIONS come from named markers in the panel SVG through
// PanelHelper::findNamed / findNamedRect (DESIGN §4.8; TimelineWidget.hpp:5-9):
// move a marker in Inkscape and the control follows. The ids are the contract
// between the art and this file; they are mirrored in the tables below
// (Timeline's Anchor idiom, TimelineWidget.hpp:126-134) so a missing one is
// visible in one place.
//
// Panel: res/modules/vx_drum_sequencer/vx_drum_sequencer_panel.svg (+ -dark).
//
// Jacks: VoxglitchInputPort for CLK / RST / MEM; the TRIG output is a
// VoxglitchPolyPort because it carries the six-voice trigger bus; ACC is a
// plain VoxglitchOutputPort (TimelineWidget.hpp:9-12 rule). Buttons are
// VCVButton (TimelineWidget.hpp:139-153), memory buttons
// VXDrumSequencerMemoryButton via createLightParamCentered
// (DigitalSequencerXPWidget.hpp:41-56). External clock only: there is no
// tempo, run or swing control.
//
// The grid is added even without a module, so the browser preview shows the
// beat rather than a blank plate (PianoRollWidget.hpp:101-107).
//

#include <cstdlib>
#include <string>
#include <vector>

struct VXDrumSequencerWidget : ModuleWidget
{
    VXDrumSequencerGridWidget* grid = NULL;

    VXDrumSequencerWidget(VXDrumSequencer* module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/vx_drum_sequencer/vx_drum_sequencer_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/vx_drum_sequencer/vx_drum_sequencer_panel-dark.svg"));

        // Bottom screws only: this panel sits BELOW VX Drums, which carries the top
        // pair, so the stacked pair reads as one machine with four screws.
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        struct Anchor { const char* id; int pid; };

        // ── inputs: CLK / RST / MEM (no plate: only outputs sit on one) ──
        static const Anchor INS[3] = {
            { "clk_input", VXDrumSequencer::CLOCK_INPUT  },
            { "rst_input", VXDrumSequencer::RESET_INPUT  },
            { "mem_input", VXDrumSequencer::MEM_CV_INPUT },
        };
        for (int i = 0; i < 3; i++)
        {
            addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed(INS[i].id), module, INS[i].pid));
        }

        // ── the buttons: RWD / RND ──
        addParam(createParamCentered<VCVButton>(panelHelper.findNamed("rewind_button"), module, VXDrumSequencer::REWIND_PARAM));
        addParam(createParamCentered<VCVButton>(panelHelper.findNamed("random_button"), module, VXDrumSequencer::RANDOM_PARAM));

        // ── the sixteen memory buttons: mem_1_button .. mem_16_button ──
        for (int i = 0; i < vx_drum_sequencer::SLOTS; i++)
        {
            const std::string id = "mem_" + std::to_string(i + 1) + "_button";

            VXDrumSequencerMemoryButton* button = createLightParamCentered<VXDrumSequencerMemoryButton>(
                panelHelper.findNamed(id), module,
                VXDrumSequencer::MEMORY_PARAMS + i, VXDrumSequencer::MEMORY_LIGHTS + i);
            button->slot = i;
            addParam(button);
        }

        // ── outputs, on the SVG's plate: TRIG (poly, six voices) and ACC ──
        addOutput(createOutputCentered<VoxglitchPolyPort>(panelHelper.findNamed("trig_output"), module, VXDrumSequencer::TRIG_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("acc_output"), module, VXDrumSequencer::ACC_OUTPUT));

        // ── the grid, sized from the panel's plate (PianoRollWidget.hpp:3-9) ──
        rack::Rect grid_rect = panelHelper.findNamedRect("grid_area");
        if (grid_rect.size.x > 0.0f && grid_rect.size.y > 0.0f)
        {
            grid = new VXDrumSequencerGridWidget(module, grid_rect);
            addChild(grid);
        }
        else
        {
            WARN("VXDrumSequencer: panel is missing a 'grid_area' rect; the grid will not be shown");
        }
    }

    // The RANDOM button is read on the audio thread, which only raises a
    // request; the randomize itself (a bank copy, rack::random, an undo push)
    // belongs on the UI thread, and step() is where the UI-side drain lives
    // (PianoRollEditorWidget.hpp:165-172).
    void step() override
    {
        ModuleWidget::step();

        VXDrumSequencer* m = dynamic_cast<VXDrumSequencer*>(this->module);
        if (m && m->random_request.exchange(false))
        {
            randomizeCurrentMemory(m);
        }
    }

    // A new beat into the CURRENT (effective) memory — the same slot the grid
    // shows and edits — as one undo step (vxdrums-random.js; DESIGN §4.6).
    void randomizeCurrentMemory(VXDrumSequencer* m)
    {
        if (!m) return;

        vx_drum_sequencer::Bank b = m->bankCopy();
        vx_drum_sequencer::randomizeMemory(b.memories[rack::math::clamp(m->current_slot, 0, vx_drum_sequencer::SLOTS - 1)]);
        vx_drum_sequencer_ui::commitBankEdit(m, "random pattern", b, m->mute);
    }

    // Module-wide settings only (PianoRollWidget.hpp:135-138): edits to the
    // pattern live on the grid's and the memory buttons' own right-click menus.
    void appendContextMenu(Menu* menu) override
    {
        VXDrumSequencer* m = dynamic_cast<VXDrumSequencer*>(this->module);
        if (!m) return;

        menu->addChild(new MenuSeparator);

        // ArpSeq's list (ArpSeqWidget.hpp:510-517), persisted as an index.
        static const std::vector<std::string> TRIGGER_LENGTH_LABELS = {
            "1 ms", "2 ms", "5 ms", "10 ms", "20 ms", "50 ms", "100 ms", "200 ms"
        };
        const int last = (int)VXDrumSequencer::triggerLengths().size() - 1;

        menu->addChild(createIndexSubmenuItem("Trigger length", TRIGGER_LENGTH_LABELS,
            [m, last]() -> size_t {
                return (size_t)rack::math::clamp(m->trigger_length_index, 0, last);
            },
            [m, last](size_t index) {
                m->trigger_length_index = rack::math::clamp((int)index, 0, last);
            }));

        // The playing memory as a file (Bret, 2026-09-02). The same JSON as the
        // clipboard (memoryToJson / memoryFromJson, format-tagged), so a file and
        // a paste are interchangeable. Import replaces the playing memory as one
        // undo step, like a paste.
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Export pattern…", "", [this]() { exportPattern(); }));
        menu->addChild(createMenuItem("Import pattern…", "", [this]() { importPattern(); }));
    }

    // ── pattern files (PianoRollEditorWidget.hpp:1797-1825 dialog idiom) ─────

    int effectiveSlot(VXDrumSequencer* m) const
    {
        return rack::math::clamp(m->current_slot, 0, vx_drum_sequencer::SLOTS - 1);
    }

    void exportPattern()
    {
        VXDrumSequencer* m = dynamic_cast<VXDrumSequencer*>(this->module);
        if (!m) return;

        osdialog_filters* filters = osdialog_filters_parse("VX Drum pattern (JSON):json");
        char* path = osdialog_file(OSDIALOG_SAVE, NULL, "vx-drum-pattern.json", filters);
        osdialog_filters_free(filters);
        if (!path) return;

        std::string filename = path;
        std::free(path);

        // Add the extension if the dialog did not.
        if (filename.find('.') == std::string::npos) filename += ".json";

        const int slot = effectiveSlot(m);
        json_t* root = vx_drum_sequencer::memoryToJson(m->liveBank().memories[slot]);
        const int result = json_dump_file(root, filename.c_str(), JSON_INDENT(2));
        json_decref(root);

        if (result != 0)
        {
            osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not write the pattern file.");
        }
    }

    void importPattern()
    {
        VXDrumSequencer* m = dynamic_cast<VXDrumSequencer*>(this->module);
        if (!m) return;

        osdialog_filters* filters = osdialog_filters_parse("VX Drum pattern (JSON):json");
        char* path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
        osdialog_filters_free(filters);
        if (!path) return;

        std::string filename = path;
        std::free(path);

        json_error_t error;
        json_t* root = json_load_file(filename.c_str(), 0, &error);
        vx_drum_sequencer::Memory imported;
        const bool ok = root && vx_drum_sequencer::memoryFromJson(root, imported);
        if (root) json_decref(root);

        if (!ok)
        {
            osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "That file is not a VX Drum pattern.");
            return;
        }

        vx_drum_sequencer::Bank b = m->bankCopy();
        b.memories[effectiveSlot(m)] = imported;
        vx_drum_sequencer_ui::commitBankEdit(m, "import pattern", b, m->mute);
    }
};
