#pragma once
#include <rack.hpp>
#include <vector>

struct TrackWidget : TransparentWidget 
{
    TrackModel *track_model = nullptr;

    // New properties to manage dragging interactions
    bool dragging = false;
    Vec drag_start_position;
    float initial_visible_window_start;
    float initial_visible_window_end;
    float cumulative_drag_offset = 0.0f; // Accumulates the drag offset

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

        // Draw the waveform
        if (track_model && track_model->sample && track_model->sample->isLoaded())
        {
            drawWaveform(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
        }

        nvgRestore(vg);
    }

    void drawWaveform(NVGcontext *vg, float track_panel_x, float track_panel_y, float track_width, float track_height)
    {
        nvgSave(vg);
        nvgBeginPath(vg);

        // Determine the start and end indices in the sample that correspond to the visible window
        unsigned int visible_sample_start = static_cast<unsigned int>(track_model->visible_window_start);
        unsigned int visible_sample_end = static_cast<unsigned int>(track_model->visible_window_end);

        // Clamp the indices to ensure they are within the bounds of the sample size
        visible_sample_start = std::min(visible_sample_start, track_model->sample->size() - 1);
        visible_sample_end = std::min(visible_sample_end, track_model->sample->size());

        // Calculate the number of visible samples
        unsigned int num_visible_samples = visible_sample_end - visible_sample_start;

        // Decide on a target number of chunks (e.g., 1000)
        unsigned int num_chunks = 1000;
        unsigned int chunk_size = std::max(1u, num_visible_samples / num_chunks);

        // Calculate width of each chunk
        float chunk_width = track_width / static_cast<float>(num_chunks);

        // Iterate over chunks to draw the averaged waveform
        for (unsigned int chunk_index = 0; chunk_index < num_chunks; ++chunk_index)
        {
            // Determine the range for this chunk
            unsigned int chunk_start = visible_sample_start + chunk_index * chunk_size;
            unsigned int chunk_end = std::min(chunk_start + chunk_size, visible_sample_end);

            // Compute the average of this chunk
            float left_sum = 0.0f;
            float right_sum = 0.0f;
            unsigned int count = 0;

            for (unsigned int i = chunk_start; i < chunk_end; ++i)
            {
                float left, right;
                track_model->sample->read(i, &left, &right);
                left_sum += std::abs(left);
                right_sum += std::abs(right);
                count++;
            }

            // Avoid division by zero
            if (count > 0)
            {
                float average_height = (left_sum + right_sum) / (2.0f * count);
                average_height *= track_height;

                // Calculate the x and y position for drawing
                float x_position = track_panel_x + chunk_index * chunk_width;
                float y_position = track_panel_y + (track_height - average_height) / 2.0f;

                // Draw the rectangle representing the chunk
                nvgRect(vg, x_position, y_position, chunk_width, average_height);
            }
        }

        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200)); // Light grey color for the waveform
        nvgFill(vg);
        nvgRestore(vg);
    }


    void onHoverScroll(const event::HoverScroll &e) override
    {
        if (track_model && track_model->sample && track_model->sample->isLoaded())
        {
            // Zoom factor adjustment
            float zoom_factor = 1.1f;
            float mouse_relative_x = (e.pos.x - box.pos.x) / box.size.x; // Mouse position relative to the track (0.0 - 1.0)

            // Calculate the focus point in the sample based on the mouse position
            float focus_point = track_model->visible_window_start + mouse_relative_x * (track_model->visible_window_end - track_model->visible_window_start);

            if (e.scrollDelta.y > 0)
            {
                // Zoom in
                float window_size = track_model->visible_window_end - track_model->visible_window_start;
                float new_window_size = window_size / zoom_factor;
                track_model->visible_window_start = std::max(0.0f, focus_point - (mouse_relative_x * new_window_size));
                track_model->visible_window_end = std::min(static_cast<float>(track_model->sample->size()), track_model->visible_window_start + new_window_size);
            }
            else if (e.scrollDelta.y < 0)
            {
                // Zoom out
                float window_size = track_model->visible_window_end - track_model->visible_window_start;
                float new_window_size = window_size * zoom_factor;
                track_model->visible_window_start = std::max(0.0f, focus_point - (mouse_relative_x * new_window_size));
                track_model->visible_window_end = std::min(static_cast<float>(track_model->sample->size()), track_model->visible_window_start + new_window_size);
            }
            e.consume(this);
        }
    }

    void onButton(const event::Button &e) override
    {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) 
        {
            e.consume(this);
            dragging = true;
            drag_start_position = e.pos;

            // Record the initial window for reference while dragging
            if (track_model)
            {
                initial_visible_window_start = track_model->visible_window_start;
                initial_visible_window_end = track_model->visible_window_end;
                cumulative_drag_offset = 0.0f; // Reset cumulative drag offset
            }
        }
        else if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE) 
        {
            dragging = false;
        }
    }

    void onDragMove(const event::DragMove &e) override
    {
        if (dragging && track_model)
        {
            e.consume(this);
            // Accumulate the horizontal drag delta
            cumulative_drag_offset += e.mouseDelta.x;

            // Convert cumulative_drag_offset to a relative movement in the sample's time space
            float window_width = initial_visible_window_end - initial_visible_window_start;
            float track_width = box.size.x;
            float sample_delta = (cumulative_drag_offset / track_width) * window_width;

            // Update visible window based on drag
            track_model->visible_window_start = std::max(0.0f, initial_visible_window_start - sample_delta);
            track_model->visible_window_end = std::min(static_cast<float>(track_model->sample->size()), track_model->visible_window_start + window_width);
        }
    }
};
