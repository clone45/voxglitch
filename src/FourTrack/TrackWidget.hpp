#pragma once
#include <rack.hpp>
#include <vector>

struct TrackWidget : TransparentWidget 
{
    TrackModel *track_model = nullptr;

    // New properties to manage resizing interactions
    bool resizing = false;
    bool dragging_start = false;
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

    // Helper function to check if the mouse is near the right edge of a clip
    int getClipIndexAtRightEdge(const Vec &mouse_pos, float resize_threshold = 5.0f)
    {
        for (size_t i = 0; i < track_model->clips.size(); ++i) 
        {
            Clip &clip = track_model->clips[i];
            float clip_start_x = box.pos.x + clip.track_position;
            float clip_end_x = clip_start_x + clip.clip_width_px;

            if (std::abs(mouse_pos.x - clip_end_x) <= resize_threshold &&
                mouse_pos.y >= box.pos.y && mouse_pos.y <= (box.pos.y + box.size.y)) 
            {
                return i; // Return the index of the clip
            }
        }
        return -1; // Return -1 if no clip is found near the right edge
    }

    // Helper function to check if the mouse is near the left edge of a clip
    int getClipIndexAtLeftEdge(const Vec &mouse_pos, float resize_threshold = 5.0f)
    {
        for (size_t i = 0; i < track_model->clips.size(); ++i) 
        {
            Clip &clip = track_model->clips[i];
            float clip_start_x = box.pos.x + clip.track_position;

            if (std::abs(mouse_pos.x - clip_start_x) <= resize_threshold &&
                mouse_pos.y >= box.pos.y && mouse_pos.y <= (box.pos.y + box.size.y)) 
            {
                return i; // Return the index of the clip
            }
        }
        return -1; // Return -1 if no clip is found near the left edge
    }

    // Handling mouse button presses
    void onButton(const event::Button &e) override
    {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) 
        {
            e.consume(this);
            drag_start_position = e.pos;

            // Detect if click is near the right boundary of any clip
            int clip_index = getClipIndexAtRightEdge(e.pos);
            if (clip_index != -1) 
            {
                // Start resizing this clip from the right edge
                resizing = true;
                resizing_clip_index = clip_index;
                dragging_start = false;
                return;
            }

            // Detect if click is near the left boundary of any clip
            clip_index = getClipIndexAtLeftEdge(e.pos);
            if (clip_index != -1) 
            {
                // Start dragging the start of this clip
                resizing = true;
                resizing_clip_index = clip_index;
                dragging_start = true;
                return;
            }
        }
        else if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE) 
        {
            // Stop resizing
            resizing = false;
            resizing_clip_index = -1;
            dragging_start = false;
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

            if (dragging_start) 
            {
                // Dragging the start of the clip
                float new_start_position = drag_start_position.x - box.pos.x;
                new_start_position = std::max(new_start_position, 0.0f); // Ensure it doesn't go negative
                float max_start_position = clip.track_position + clip.clip_width_px - 10.0f; // Ensure there's a minimum width
                new_start_position = std::min(new_start_position, max_start_position);

                // Update clip's start position and width accordingly
                clip.updateClipWidthFromLeft(new_start_position);
            } 
            else 
            {
                // Resizing the clip from the right edge
                float new_width = drag_start_position.x - (box.pos.x + clip.track_position);
                new_width = std::max(new_width, 10.0f); // Set a minimum width to avoid collapsing

                // Update clip width and recalculate sample window
                clip.clip_width_px = new_width;
                clip.updateSampleWindowEnd(); // Recalculate sample_window_end based on new width
            }
        }
    }

    void onHover(const event::Hover &e) override
    {
        TransparentWidget::onHover(e);
        e.consume(this);

        // Check if the mouse is near the right edge of any clip
        int clip_index = getClipIndexAtRightEdge(e.pos);
        if (clip_index != -1) 
        {
            // Change the mouse pointer to indicate resizing from the right edge
            glfwSetCursor(APP->window->win, glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));
            return;
        }

        // Check if the mouse is near the left edge of any clip
        clip_index = getClipIndexAtLeftEdge(e.pos);
        if (clip_index != -1) 
        {
            // Change the mouse pointer to indicate resizing from the left edge
            glfwSetCursor(APP->window->win, glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));
            return;
        }

        // Reset cursor if not near any edge
        glfwSetCursor(APP->window->win, NULL);
    }

    void onLeave(const event::Leave &e) override
    {
        TransparentWidget::onLeave(e);
        glfwSetCursor(APP->window->win, NULL);
    }
};
