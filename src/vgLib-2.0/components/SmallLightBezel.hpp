#pragma once
#include <rack.hpp>

using namespace rack;

struct SmallLightBase : app::ParamWidget {
    widget::FramebufferWidget* fb;
    app::ModuleLightWidget* light;
    
    SmallLightBase() {
        fb = new widget::FramebufferWidget;
        addChild(fb);
        box.size = mm2px(math::Vec(3.0, 3.0));
    }

    void onButton(const event::Button& e) override {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
            ParamQuantity* pq = getParamQuantity();
            if (pq) {
                pq->setValue(1.f);
            }
            e.consume(this);
        }
    }

    void draw(const DrawArgs& args) override {
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, box.size.x / 2.0, box.size.y / 2.0, box.size.x / 2.0);
        nvgFillColor(args.vg, nvgRGBA(40, 40, 40, 255));
        nvgFill(args.vg);
        
        nvgStrokeWidth(args.vg, 0.5);
        nvgStrokeColor(args.vg, nvgRGBA(0, 0, 0, 60));
        nvgStroke(args.vg);

        Widget::draw(args);
    }
};

template <typename TLightBase = componentlibrary::WhiteLight>
struct SmallLightButton : SmallLightBase {
    SmallLightButton() {
        light = new TLightBase;
        light->box.size = mm2px(math::Vec(2.0, 2.0));
        light->box.pos = box.size.div(2).minus(light->box.size.div(2));
        addChild(light);
    }

    app::ModuleLightWidget* getLight() {
        return light;
    }
};