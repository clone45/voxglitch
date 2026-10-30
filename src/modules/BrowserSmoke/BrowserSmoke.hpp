#pragma once
// BrowserSmoke — the module-browser smoke test. TEST BUILDS ONLY.
//
// Rack's module browser builds a preview of every module with module == NULL
// (src/app/Browser.cpp, ModelBox::createPreview), steps it once and draws it.
// A ModuleWidget that reaches through the null pointer crashes Rack the
// moment the browser opens — and only on some compilers: `&module->page`
// before the `if (module)` is undefined behaviour that a recent clang turned
// into a dropped null check (ArpSeq, the nightly of 2026-10-30, macOS),
// while the library's older toolchain kept the check. No static tool we run
// reports the pattern. So this module runs the browser's own procedure over
// every Voxglitch model inside a real Rack, in a build instrumented with
// UBSan (`make BROWSER_SMOKE=1`), which reports the null member access at
// its exact line on every compiler, crash or no crash.
//
// Compiled only with -DVOXGLITCH_BROWSER_SMOKE, and the manifest entry that
// lets Rack load it is injected into a scratch copy of plugin.json by
// scripts/browser_smoke.py, so the shipped plugin never contains it. The
// script puts one BrowserSmoke in an autosave patch; when Rack creates the
// widget (UI thread, window up) it builds a preview of every other model the
// way Browser.cpp does — TransparentWidget > ZoomWidget > FramebufferWidget >
// container that draws layers 0 and 1 > ModuleWidget(NULL) — adds them to
// the scene, lets Rack step and draw them for a few frames, tears them down,
// writes a report, and closes the window. The report's last line is
// "OK <n> models"; anything else, a missing report (a crash), or a UBSan
// log is a failure.

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <widget/FramebufferWidget.hpp>
#include <widget/ZoomWidget.hpp>

struct BrowserSmoke : Module
{
    BrowserSmoke() { config(0, 0, 0, 0); }
};

struct BrowserSmokeWidget : ModuleWidget
{
    // Browser.cpp's ModuleWidgetContainer: lights are layer 1, which a
    // FramebufferWidget does not draw by itself.
    struct ModuleWidgetContainer : widget::Widget
    {
        void draw(const DrawArgs& args) override
        {
            Widget::draw(args);
            Widget::drawLayer(args, 1);
        }
    };

    // Holds every preview, on top of the scene. Its own draw() is the stock
    // one: each preview sits at (0, 0) inside it, so none is culled.
    struct Holder : widget::TransparentWidget {};

    enum Phase { START, DRAWING, DONE };
    Phase phase = START;
    int frames_drawn = 0;
    static const int FRAMES_TO_DRAW = 5;

    Holder* holder = NULL;
    FILE* report = NULL;
    size_t previews = 0;

    BrowserSmokeWidget(BrowserSmoke* module)
    {
        setModule(module);
        box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    }

    // The work happens from step(), on the UI thread, once the widget is in
    // the rack: START builds the previews, DRAWING lets Rack step and draw
    // them for a few frames, then everything is torn down and Rack closed.
    void step() override
    {
        ModuleWidget::step();
        if (!module) return;   // hidden from the browser, but be safe

        switch (phase)
        {
            case START:
                begin();
                phase = DRAWING;
                break;
            case DRAWING:
                if (++frames_drawn >= FRAMES_TO_DRAW)
                {
                    finish();
                    phase = DONE;
                }
                break;
            case DONE:
                break;
        }
    }

    void say(const char* fmt, ...)
    {
        if (!report) return;
        va_list args;
        va_start(args, fmt);
        std::vfprintf(report, fmt, args);
        va_end(args);
        std::fputc('\n', report);
        std::fflush(report);   // the last line names the culprit if Rack dies
    }

    void begin()
    {
        const char* report_path = std::getenv("VOXGLITCH_BROWSER_SMOKE_REPORT");
        std::string path = report_path ? report_path : asset::user("browser_smoke_report.txt");
        report = std::fopen(path.c_str(), "w");
        if (!report) WARN("BrowserSmoke: cannot write report %s", path.c_str());
        say("rack %s", APP_VERSION.c_str());
        say("plugin %s %s", pluginInstance->slug.c_str(), pluginInstance->version.c_str());

        holder = new Holder;
        holder->box.pos = Vec(0.f, 0.f);
        holder->box.size = APP->scene->box.size;
        APP->scene->addChild(holder);

        for (plugin::Model* model : pluginInstance->models)
        {
            if (model == module->model) continue;
            say("creating %s", model->slug.c_str());
            INFO("BrowserSmoke: creating module widget %s", model->getFullName().c_str());
            addPreview(model);
            say("created %s", model->slug.c_str());
            previews++;
        }
    }

    // ModelBox::createPreview, verbatim in structure.
    void addPreview(plugin::Model* model)
    {
        widget::Widget* previewWidget = new widget::TransparentWidget;
        holder->addChild(previewWidget);

        widget::ZoomWidget* zoomWidget = new widget::ZoomWidget;
        previewWidget->addChild(zoomWidget);

        widget::FramebufferWidget* fb = new widget::FramebufferWidget;
        if (APP->window->pixelRatio < 2.0) fb->oversample = 2.0;
        zoomWidget->addChild(fb);

        ModuleWidgetContainer* mwc = new ModuleWidgetContainer;
        fb->addChild(mwc);

        ModuleWidget* moduleWidget = model->createModuleWidget(NULL);
        mwc->addChild(moduleWidget);
        mwc->box.size = moduleWidget->box.size;

        // "Step ModuleWidget so it can set its default appearance."
        moduleWidget->step();

        const float zoom = 0.5f;
        zoomWidget->setZoom(zoom);
        previewWidget->box.size = moduleWidget->box.size.mult(zoom);
    }

    void finish()
    {
        say("drew %d frames", frames_drawn);

        // The browser destroys previews too (on every refresh of its list).
        APP->scene->removeChild(holder);
        delete holder;
        holder = NULL;
        say("destroyed");

        say("OK %zu models", previews);
        if (report) std::fclose(report);
        report = NULL;

        APP->window->close();
    }
};
