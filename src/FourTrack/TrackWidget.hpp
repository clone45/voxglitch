#pragma once
#include <rack.hpp>

struct TrackWidget : TransparentWidget 
{

    TrackModel *track_model = nullptr;

    TrackWidget(float x, float y, float width, float height, TrackModel *track_model) 
    {
        box.size = Vec(width, height);
        box.pos = Vec(x, y);
        this->track_model = track_model;
    }

    void draw(const DrawArgs &args) override 
    {
        const auto vg = args.vg;

        nvgSave(vg);

        // Draw the track background
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(vg, nvgRGB(0x20, 0x20, 0x20));
        nvgFill(vg);

        // Draw all clips
        for (size_t i = 0; i < this->track_model->clips.size(); ++i) 
        {
            this->track_model->clips[i].draw(vg, box.pos.x, box.pos.y, box.size.y);
        }

        nvgRestore(vg);
    }
};