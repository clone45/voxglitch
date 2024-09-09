#pragma once
#include <rack.hpp>

struct VoxglitchRoundShadow : TransparentWidget {
    float diameter;
    float opacity;
    NVGcolor color;

    VoxglitchRoundShadow(float diameter, float opacity = 0.5) {
        this->diameter = diameter;
        this->opacity = opacity;
        this->color = nvgRGBA(0, 0, 0, 255);
        box.size = Vec(diameter * 1.5, diameter * 1.5);
    }

    void draw(const DrawArgs& args) override {
        nvgBeginPath(args.vg);
        nvgFillColor(args.vg, nvgRGBAf(color.r, color.g, color.b, opacity));
        
        float centerX = box.size.x / 2.0f;
        float centerY = box.size.y / 2.0f;
        float radius = diameter / 2.0f;
        
        nvgEllipse(args.vg, centerX, centerY, radius, radius);
        
        // Create gradient for softer edge
        NVGpaint paint = nvgRadialGradient(args.vg, centerX, centerY, radius * 0.9f, radius,
                                           nvgRGBAf(color.r, color.g, color.b, opacity),
                                           nvgRGBAf(color.r, color.g, color.b, 0.0f));
        nvgFillPaint(args.vg, paint);
        nvgFill(args.vg);
    }
};