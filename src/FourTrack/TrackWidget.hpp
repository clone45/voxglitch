#pragma once
#include <rack.hpp>
#include <vector>

struct TrackWidget : TransparentWidget 
{
    TrackModel *track_model = nullptr;

    // New properties to manage resizing interactions
    bool resizing = false;
    int resizing_clip_index = -1;
    Vec drag_start_position;

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
        nvgRect(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
        nvgFillColor(vg, nvgRGB(0x20, 0x20, 0x20));
        nvgFill(vg);

        // Draw all clips
        for (size_t i = 0; i < this->track_model->clips.size(); ++i) 
        {
            this->track_model->clips[i].draw(vg, box.pos.x, box.pos.y, box.size.y);
        }

        nvgRestore(vg);
    }

    // Handling mouse button presses
    void onButton(const event::Button &e) override
    {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) 
        {
            e.consume(this);
            drag_start_position = e.pos;

            // Detect if click is near the right boundary of any clip
            for (size_t i = 0; i < track_model->clips.size(); ++i) 
            {
                Clip &clip = track_model->clips[i];
                float clip_start_x = box.pos.x + clip.track_position;
                float clip_end_x = clip_start_x + (clip.sample_window_end - clip.sample_window_start);
                float resize_threshold = 5.0f; // Threshold for detecting resize near the right edge

                if (std::abs(e.pos.x - clip_end_x) <= resize_threshold &&
                    e.pos.y >= box.pos.y && e.pos.y <= (box.pos.y + box.size.y)) 
                {
                    // Start resizing this clip
                    resizing = true;
                    resizing_clip_index = i;
                    return;
                }
            }
        }
        else if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE) 
        {
            // Stop resizing
            resizing = false;
            resizing_clip_index = -1;
        }
    }

    // Handling mouse dragging for resizing
    void onDragMove(const event::DragMove &e) override
    {
        TransparentWidget::onDragMove(e);
        float zoom = getAbsoluteZoom();
        drag_start_position = drag_start_position.plus(e.mouseDelta.div(zoom));

        if (resizing && resizing_clip_index != -1) 
        {
            Clip &clip = track_model->clips[resizing_clip_index];
            float new_width = drag_start_position.x - (box.pos.x + clip.track_position);
            new_width = std::max(new_width, 10.0f); // Set a minimum width to avoid collapsing
            clip.setSampleWindow(clip.sample_window_start, clip.sample_window_start + new_width);
        }
    }
};
