#pragma once
//
// Euclid — the panel. 9 HP (8 until 2026-09-18).
//
// Control POSITIONS come from named markers in the panel SVG through
// PanelHelper::findNamed / findNamedRect (VXDrumSequencerWidget.hpp:5-10):
// move a marker in Inkscape (or in scripts/generate_euclid_panel.py, which
// writes the panel) and the control follows. The ids are the contract
// between the art and this file; they are mirrored in the tables below
// (Timeline's Anchor idiom, TimelineWidget.hpp:126-134) so a missing one is
// visible in one place.
//
// Panel: res/modules/euclid/euclid_panel.svg (+ -dark).
//
// Knobs are RoundBlackKnob, the attenuverters Trimpot (ArpSeqWidget.hpp:
// 470-474); jacks are VoxglitchInputPort / VoxglitchOutputPort, all mono.
// Both screw pairs: a standalone module (PianoRollWidget.hpp:31-35).
//
// The ring is sized from the panel's `ring_display` rect (the VX Drums
// `kit_display` idiom, VXDrumsWidget.hpp:373-383) and added even without a
// module, so the browser preview shows the pattern rather than a blank plate
// (PianoRollWidget.hpp:101-107).
//
// The context menu carries Trigger length and nothing else of its own
// (design §5.2); the ring's edits are the ring's.
//

#include <string>
#include <vector>

struct EuclidWidget : ModuleWidget
{
    EuclidRingWidget* ring = NULL;

    EuclidWidget(Euclid* module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/euclid/euclid_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/euclid/euclid_panel-dark.svg"));

        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        struct Anchor { const char* id; int pid; };

        // ── the three knobs: HITS / STEPS / ROT ──
        static const Anchor KNOBS[3] = {
            { "hits_knob",  Euclid::HITS_PARAM  },
            { "steps_knob", Euclid::STEPS_PARAM },
            { "rot_knob",   Euclid::ROT_PARAM   },
        };
        for (int i = 0; i < 3; i++)
        {
            addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed(KNOBS[i].id), module, KNOBS[i].pid));
        }

        // ── the attenuverters, one under each knob ──
        static const Anchor TRIMPOTS[3] = {
            { "hits_atten_knob",  Euclid::HITS_ATTEN_PARAM  },
            { "steps_atten_knob", Euclid::STEPS_ATTEN_PARAM },
            { "rot_atten_knob",   Euclid::ROT_ATTEN_PARAM   },
        };
        for (int i = 0; i < 3; i++)
        {
            addParam(createParamCentered<Trimpot>(panelHelper.findNamed(TRIMPOTS[i].id), module, TRIMPOTS[i].pid));
        }

        // ── inputs: the CV row, then CLK / RST ──
        static const Anchor INS[5] = {
            { "hits_input",  Euclid::HITS_CV_INPUT  },
            { "steps_input", Euclid::STEPS_CV_INPUT },
            { "rot_input",   Euclid::ROT_CV_INPUT   },
            { "clk_input",   Euclid::CLOCK_INPUT    },
            { "rst_input",   Euclid::RESET_INPUT    },
        };
        for (int i = 0; i < 5; i++)
        {
            addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed(INS[i].id), module, INS[i].pid));
        }

        // ── the one output, on the SVG's plate: TRIG (GATE was dropped 2026-09-18) ──
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("trig_output"), module, Euclid::TRIG_OUTPUT));

        // ── the ring, sized from the panel's plate. A panel without the rect
        //    still gets a ring, at the place scripts/generate_euclid_panel.py
        //    writes the rect (x 9, y 30, 117 x 117: the SVG's ids are the
        //    contract), so the module is never left without its one control
        //    that is not a knob. ──
        rack::Rect ring_rect = panelHelper.findNamedRect("ring_display");
        if (!(ring_rect.size.x > 0.f && ring_rect.size.y > 0.f))
        {
            WARN("Euclid: panel is missing a 'ring_display' rect; using the generator's 117 x 117 box at (9, 30)");
            ring_rect.pos = Vec(9.f, 30.f);
            ring_rect.size = Vec(117.f, 117.f);
        }
        ring = new EuclidRingWidget(module, ring_rect);
        addChild(ring);
    }

    // Module-wide settings only (PianoRollWidget.hpp:135-138).
    void appendContextMenu(Menu* menu) override
    {
        Euclid* m = dynamic_cast<Euclid*>(this->module);
        if (!m) return;

        menu->addChild(new MenuSeparator);

        // ArpSeq's list (ArpSeqWidget.hpp:510-517), persisted as an index
        // (VXDrumSequencerWidget.hpp, the same submenu).
        static const std::vector<std::string> TRIGGER_LENGTH_LABELS = {
            "1 ms", "2 ms", "5 ms", "10 ms", "20 ms", "50 ms", "100 ms", "200 ms"
        };
        const int last = (int)Euclid::triggerLengths().size() - 1;

        menu->addChild(createIndexSubmenuItem("Trigger length", TRIGGER_LENGTH_LABELS,
            [m, last]() -> size_t {
                return (size_t)rack::math::clamp(m->trigger_length_index, 0, last);
            },
            [m, last](size_t index) {
                m->trigger_length_index = rack::math::clamp((int)index, 0, last);
            }));
    }
};
