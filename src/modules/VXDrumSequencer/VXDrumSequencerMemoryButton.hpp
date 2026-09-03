#pragma once
//
// VX Drum Sequencer — one of the sixteen memory buttons.
//
// The house selector button (VCVLightBezel<WhiteLight>, created with
// createLightParamCentered: DigitalSequencerXPWidget.hpp:41-56) with a
// Copy / Paste / Clear menu on its right-click, the vxsynth memory-row menu
// (panel-memory.js:95-103, memory-slots.js:89-98). Selection itself is the
// param: process() reads the press and ignores it while the MEM CV is patched
// (DESIGN §4.4), and the light follows the EFFECTIVE slot.
//
// The clipboard is the OS clipboard, as JSON with a format tag
// (DigitalSequencerXP.hpp:128-197, issue #220): a memory copied here pastes
// into another VX Drum Sequencer instance, and across Rack sessions. A
// DigitalSequencer clip ("voxglitch-sequence") is rejected by memoryFromJson.
//
// Paste and Clear write the whole memory in one undo step; Copy is not an
// edit. Copy/Paste/Clear act on the button's OWN slot, whether or not it is
// the one playing, and even while the CV owns the selection — the CV decides
// which memory PLAYS, these edit what a memory CONTAINS
// (GlitchSequencer.hpp:135-137).
//

#include <cstdlib>
#include <cstring>
#include <string>

namespace vx_drum_sequencer_ui
{
    // Memory -> OS clipboard (DigitalSequencerXP.hpp:147-155 shape).
    inline void memoryToClipboard(const vx_drum_sequencer::Memory& m)
    {
        json_t* root = vx_drum_sequencer::memoryToJson(m);

        char* clipboard_string = json_dumps(root, JSON_COMPACT);
        if (clipboard_string)
        {
            glfwSetClipboardString(APP->window->win, clipboard_string);
            std::free(clipboard_string);   // json_dumps allocates with malloc
        }

        json_decref(root);
    }

    // OS clipboard -> Memory. False (and `out` untouched) when the clipboard
    // is empty, not JSON, or not a VX drum memory.
    inline bool memoryFromClipboard(vx_drum_sequencer::Memory& out)
    {
        const char* clipboard_string = glfwGetClipboardString(APP->window->win);
        if (!clipboard_string) return false;

        json_error_t error;
        json_t* root = json_loads(clipboard_string, 0, &error);
        if (!root) return false;

        const bool ok = vx_drum_sequencer::memoryFromJson(root, out);
        json_decref(root);
        return ok;
    }
}

struct VXDrumSequencerMemoryButton : VCVLightBezel<WhiteLight>
{
    // Which memory this button is: set by the module widget right after
    // createLightParamCentered. The slot is a fixed index of the fixed row
    // (structure, not identity); the button number on the panel is display only.
    int slot = 0;

    // The module comes from ParamWidget::module (engine::Module*, NULL in the
    // browser). Deliberately NOT re-declared as a VXDrumSequencer* member: a
    // same-named member would shadow the base's and trip cppcheck's
    // duplInheritedMember (brief-rack-conventions §9.3, rule 3).
    VXDrumSequencer* sequencer()
    {
        return dynamic_cast<VXDrumSequencer*>(module);
    }

    void appendContextMenu(ui::Menu* menu) override
    {
        VXDrumSequencer* m = sequencer();
        if (!m) return;

        const int s = rack::math::clamp(slot, 0, vx_drum_sequencer::SLOTS - 1);

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Memory " + std::to_string(s + 1)));

        menu->addChild(createMenuItem("Copy", "", [m, s]() {
            vx_drum_sequencer_ui::memoryToClipboard(m->liveBank().memories[s]);
        }));

        // Shown disabled rather than hidden when the clipboard holds nothing
        // usable, so the menu keeps its shape and the reason is visible
        // (memory-slots.js:84-88). The clipboard is read again at the click:
        // it may have changed while the menu was open.
        vx_drum_sequencer::Memory probe;
        const bool can_paste = vx_drum_sequencer_ui::memoryFromClipboard(probe);

        menu->addChild(createMenuItem("Paste", "", [m, s]() {
            vx_drum_sequencer::Memory clip;
            if (!vx_drum_sequencer_ui::memoryFromClipboard(clip)) return;

            vx_drum_sequencer::Bank b = m->bankCopy();
            b.memories[s] = clip;
            vx_drum_sequencer_ui::commitBankEdit(m, "paste memory", b, m->mute);
        }, !can_paste));

        // An empty memory: masks 0, no ratchets, length 16 (memory-slots.js:74-78).
        menu->addChild(createMenuItem("Clear", "", [m, s]() {
            vx_drum_sequencer::Bank b = m->bankCopy();
            b.memories[s] = vx_drum_sequencer::Memory();
            vx_drum_sequencer_ui::commitBankEdit(m, "clear memory", b, m->mute);
        }));
    }
};
