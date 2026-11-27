#pragma once
// TimelineVoltSnap.hpp — the V SNAP display at the top right of the panel
// (2026-11-26). It shows and sets the SELECTED lane's voltage snap: how many
// values, over which range (both ends included). The snapping itself is VoltSnap
// (TimelineEngine.hpp) and the editor applies it.
//
//   click (either button)  the menu: Off, a typed value count, common
//                          counts, the range, copy to all lanes
//   mouse wheel            count +/-1 (off <-> 2: one value would snap
//                          everything to the bottom of the range)
//
// Code-drawn like the tabs and the readout, so it needs no panel-art change.

#include "Timeline.hpp"
#include "TimelineEditor.hpp"
#include <cstdio>
#include <cstdlib>

namespace timeline_ui
{

// The menu's range choices. Each is identified by its VOLTS: the check mark
// matches on lo/hi, and the patch stores lo/hi, so this table can be
// reordered or extended freely.
struct VoltSnapRange { float lo, hi; const char* label; const char* shortLabel; };
static const VoltSnapRange VSNAP_RANGES[4] = {
    {   0.f, 10.f, "0 to 10 V",   "0-10V" },
    {   0.f,  5.f, "0 to 5 V",    "0-5V"  },
    {  -5.f,  5.f, "-5 to 5 V",   "+-5V"  },
    { -10.f, 10.f, "-10 to 10 V", "+-10V" },
};
static const int VSNAP_COMMON[10] = { 2, 3, 4, 5, 6, 8, 10, 11, 12, 16 };

inline void vsnapRangeLabel(const timeline_dsp::VoltSnap& vs, char* buf, size_t n)
{
    for (int i = 0; i < 4; i++)
        if (VSNAP_RANGES[i].lo == vs.lo && VSNAP_RANGES[i].hi == vs.hi)
        {
            std::snprintf(buf, n, "%s", VSNAP_RANGES[i].shortLabel);
            return;
        }
    std::snprintf(buf, n, "%g..%gV", (double)vs.lo, (double)vs.hi);   // a hand-edited patch
}

// Type a value count, press Enter. 0 (or empty) turns the snap off.
struct VoltSnapCountField : ui::TextField
{
    Timeline* module = NULL;
    int lane = 0;
    bool focused = false;

    void step() override
    {
        // Take the keyboard once, when the menu opens, so the user can just
        // type. Doing it every frame would steal focus forever.
        if (!focused)
        {
            APP->event->setSelectedWidget(this);
            selectAll();
            focused = true;
        }
        ui::TextField::step();
    }

    void onSelectKey(const SelectKeyEvent& e) override
    {
        if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER))
        {
            if (module)
            {
                timeline_dsp::VoltSnap& vs = module->voltSnap[lane];
                vs.count = std::atoi(text.c_str());
                vs.sanitize();
            }
            ui::MenuOverlay* overlay = getAncestorOfType<ui::MenuOverlay>();
            if (overlay) overlay->requestDelete();
            e.consume(this);
            return;
        }
        ui::TextField::onSelectKey(e);
    }
};

struct TimelineVoltSnapDisplay : OpaqueWidget
{
    Timeline* module = NULL;

    void openMenu()
    {
        Timeline* m = module;
        if (!m) return;
        const int L = m->currentLane();
        ui::Menu* menu = createMenu();
        menu->addChild(createMenuLabel(string::f("Voltage snap - lane %d", L + 1)));

        menu->addChild(createCheckMenuItem("Off", "",
            [m, L]() { return m->voltSnap[L].count == 0; },
            [m, L]() { m->voltSnap[L].count = 0; }));

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Values (type a number, then Enter)"));
        VoltSnapCountField* field = new VoltSnapCountField;
        field->module = m;
        field->lane = L;
        field->box.size.x = 160.f;
        field->multiline = false;
        field->placeholder = "e.g. 39";
        if (m->voltSnap[L].count > 0) field->text = string::f("%d", m->voltSnap[L].count);
        menu->addChild(field);

        menu->addChild(createSubmenuItem("Common counts", "", [m, L](ui::Menu* sub) {
            for (int i = 0; i < 10; i++)
            {
                int n = VSNAP_COMMON[i];
                sub->addChild(createCheckMenuItem(string::f("%d", n), "",
                    [m, L, n]() { return m->voltSnap[L].count == n; },
                    [m, L, n]() { m->voltSnap[L].count = n; }));
            }
        }));

        menu->addChild(createSubmenuItem("Range", "", [m, L](ui::Menu* sub) {
            for (int i = 0; i < 4; i++)
            {
                const VoltSnapRange* r = &VSNAP_RANGES[i];
                sub->addChild(createCheckMenuItem(r->label, "",
                    [m, L, r]() { return m->voltSnap[L].lo == r->lo && m->voltSnap[L].hi == r->hi; },
                    [m, L, r]() { m->voltSnap[L].lo = r->lo; m->voltSnap[L].hi = r->hi; }));
            }
        }));

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Copy to all lanes", "", [m, L]() {
            for (int k = 0; k < timeline_dsp::TL_LANES; k++) m->voltSnap[k] = m->voltSnap[L];
        }));
    }

    void onButton(const ButtonEvent& e) override
    {
        if (e.action == GLFW_PRESS
            && (e.button == GLFW_MOUSE_BUTTON_LEFT || e.button == GLFW_MOUSE_BUTTON_RIGHT))
        {
            openMenu();
            e.consume(this);
            return;
        }
        OpaqueWidget::onButton(e);
    }

    void onHoverScroll(const HoverScrollEvent& e) override
    {
        Timeline* m = module;
        if (!m || e.scrollDelta.y == 0.f) return;
        if (getAbsoluteZoom() < 0.95f) return;      // the editor's rule: let Rack scroll
        timeline_dsp::VoltSnap& vs = m->voltSnap[m->currentLane()];
        if (e.scrollDelta.y > 0.f) vs.count = (vs.count < 2) ? 2 : vs.count + 1;
        else                       vs.count = (vs.count <= 2) ? 0 : vs.count - 1;
        vs.sanitize();
        e.consume(this);
    }

    void draw(const DrawArgs& args) override
    {
        NVGcontext* vg = args.vg;
        float w = box.size.x, h = box.size.y;
        nvgBeginPath(vg);
        nvgRect(vg, 0.5f, 0.5f, w - 1.f, h - 1.f);
        nvgFillColor(vg, tcol(0x1b222b));
        nvgFill(vg);
        nvgStrokeColor(vg, tcol(0x2b333d));
        nvgStrokeWidth(vg, 1.f);
        nvgStroke(vg);

        std::shared_ptr<Font> font =
            APP->window->loadFont(asset::system("res/fonts/ShareTechMono-Regular.ttf"));
        if (!font) return;
        nvgFontFaceId(vg, font->handle);
        nvgFontSize(vg, 8.f);
        nvgTextLetterSpacing(vg, 0.5f);
        nvgFillColor(vg, tcol(0x8b98a5));
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgText(vg, 4.f, h * 0.5f, "V SNAP", NULL);

        char buf[64];
        bool on = false;
        if (module)
        {
            const timeline_dsp::VoltSnap& vs = module->voltSnap[module->currentLane()];
            on = vs.on();
            if (on)
            {
                char range[40];
                vsnapRangeLabel(vs, range, sizeof(range));
                std::snprintf(buf, sizeof(buf), "%d  %s", vs.count, range);
            }
        }
        if (!on) std::snprintf(buf, sizeof(buf), "OFF");
        nvgFillColor(vg, on ? tcol(0x39ff14) : tcol(0x5a6470));
        nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        nvgText(vg, w - 4.f, h * 0.5f, buf, NULL);
    }
};

} // namespace timeline_ui
