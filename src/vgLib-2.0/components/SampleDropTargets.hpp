#pragma once
// SampleDropTargets — paints a module's sample slots as drop targets while a
// file is being dragged over it from stoermelder SIREN, the one under the
// cursor brighter, and tells the owner which slot a point falls in.
//
// SIREN (stoermelder PackOne) is a sample browser. A file dragged out of it
// is an ordinary in-Rack mouse drag that ends in Rack's standard file-drop
// event at the release point (SirenDropHandler.hpp in PackOne), the same
// event a file dragged in from the OS produces. The owning ModuleWidget
// handles the drop itself in onPathDrop(), asking targetAt(e.pos) which slot
// took it; this widget only does the feedback. An OS drag gives Rack no hover
// events, so it lands in the right slot but without the highlight.
//
// The drag is watched from step(), not from the DragHover/DragEnter events:
// DragHover only arrives while the mouse moves, and every knob is an
// OpaqueWidget that would take DragEnter/DragLeave as the cursor crosses it.
// APP->event->getDraggedWidget() says what is being dragged every frame, and
// the rack's mouse position says where (the idiom SIREN itself uses).
//
// Usage (VXDrumsWidget.hpp, SamplerX8Widget.hpp): construct with the module
// and the panel size, push one panel-local Rect per slot onto `targets`, add
// it as the LAST child so it paints over the knobs. TransparentWidget, so it
// takes no events away from them. Inert without a module (the browser).

#include <cstring>
#include <typeinfo>
#include <vector>

namespace vgLib_v2
{
    struct SampleDropTargets : TransparentWidget
    {
        engine::Module* module = NULL;
        std::vector<rack::Rect> targets;   // panel-local, one per slot
        bool active = false;               // a SIREN drag is over the panel this frame
        int hovered = -1;                  // the slot under the cursor, or -1
        NVGcolor color = nvgRGB(0xd8, 0xc9, 0x4a);

        // The dragged widget last examined and the verdict, so the type lookup
        // runs once per drag rather than once per frame.
        widget::Widget* checked_origin = NULL;
        bool checked_is_siren = false;

        SampleDropTargets(engine::Module* module, Vec panel_size)
        {
            this->module = module;
            box.pos = Vec(0.f, 0.f);
            box.size = panel_size;
        }

        // The slot under a panel-local point, or -1.
        int targetAt(Vec pos) const
        {
            for (size_t i = 0; i < targets.size(); i++)
                if (targets[i].contains(pos)) return (int)i;
            return -1;
        }

        // A drag from SIREN's file list or waveform pane starts inside its
        // ModuleWidget, so the plugin and module slugs identify it. SIREN's
        // "Ctrl+Shift-click anywhere" drag starts from an overlay it adds to
        // the rack itself, with no module above it, so that one is recognised
        // by its type name. Moving a module, or turning a knob, is never a
        // sample drag.
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
            hovered = targetAt(mouse);
        }

        // Layer 1: on top of the knobs and lamps, and not dimmed by the room
        // brightness.
        void drawLayer(const DrawArgs& args, int layer) override
        {
            if (layer == 1 && active)
            {
                NVGcontext* vg = args.vg;
                nvgSave(vg);
                for (size_t i = 0; i < targets.size(); i++)
                {
                    const bool hot = ((int)i == hovered);
                    const rack::Rect& r = targets[i];
                    nvgBeginPath(vg);
                    nvgRoundedRect(vg, r.pos.x, r.pos.y, r.size.x, r.size.y, 6.f);
                    nvgFillColor(vg, nvgTransRGBA(color, hot ? 0x60 : 0x22));
                    nvgFill(vg);
                    nvgStrokeWidth(vg, hot ? 2.f : 1.2f);
                    nvgStrokeColor(vg, nvgTransRGBA(color, hot ? 0xff : 0x90));
                    nvgStroke(vg);
                }
                nvgRestore(vg);
            }
            TransparentWidget::drawLayer(args, layer);
        }
    };
}
