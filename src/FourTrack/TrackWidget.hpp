#pragma once
#include <rack.hpp>
#include <vector>

struct TrackWidget : TransparentWidget
{
    TrackModel *track_model = nullptr;

    // Used to watch for updates to the sample
    std::string sample_filename = "";

    // New properties to manage sample dragging interactions
    bool dragging = false;
    bool dragging_zoom = false;
    bool shift_key_held = false;
    bool right_button_held = false;

    Vec drag_start_position;
    Vec drag_position;
    float initial_visible_window_start;
    float initial_visible_window_end;
    float cumulative_drag_offset = 0.0f; // Accumulates the drag offset
    float cumulative_zoom_offset = 0.0f;

    // New properties to manage marker dragging interactions
    bool dragging_marker = false;
    unsigned int drag_source_position = 0; // Original position
    std::vector<Marker> *markers_being_dragged = nullptr;
    float drag_start_x = 0.0f;   // Where the drag began
    float drag_current_x = 0.0f; // Current dragging position

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
        nvgFillColor(vg, nvgRGB(0x10, 0x20, 0x20));
        nvgFill(vg);

        // Draw the waveform
        if (track_model && track_model->sample && track_model->sample->isLoaded())
        {
            drawWaveform(vg, box.size.x, box.size.y);
            drawMarkers(args, box.size.x, box.size.y);
        }

        nvgRestore(vg);
    }

    void drawWaveform(NVGcontext *vg, float track_width, float track_height)
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
                float x_position = chunk_index * chunk_width;
                float y_position = (track_height - average_height) / 2.0f;

                // Draw the rectangle representing the chunk
                float rect_overlap = chunk_width / 4.0f;
                nvgRect(vg, x_position, y_position, (chunk_width + rect_overlap), average_height);
            }
        }

        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200)); // Light grey color for the waveform
        nvgFill(vg);
        nvgRestore(vg);
    }

    // Add to TrackWidget::draw() after drawing the waveform
    void drawMarkers(const DrawArgs &args, float track_width, float track_height)
    {
        if (!track_model || !track_model->sample || !track_model->markers)
            return;

        const auto vg = args.vg;
        nvgSave(vg);

        unsigned int visible_sample_start = track_model->visible_window_start;
        unsigned int visible_sample_end = track_model->visible_window_end;
        float samples_per_pixel = (visible_sample_end - visible_sample_start) / track_width;

        float line_extension = track_height * 1.0f;
        float triangle_height = 10.0f;

        // Define our colors
        NVGcolor inactive_color = nvgRGBA(100, 100, 100, 180); // Gray for inactive
        NVGcolor active_color = nvgRGBA(32, 178, 170, 255);    // Bright teal for active

        for (const auto &marker_pair : *(track_model->markers))
        {
            unsigned int pos = marker_pair.first;

            if (pos < visible_sample_start || pos > visible_sample_end)
                continue;

            float x = ((pos - visible_sample_start) / samples_per_pixel);

            for (const auto &marker : marker_pair.second)
            {

                // Choose color based on whether this marker matches the active output
                NVGcolor marker_color = (marker.output_number == track_model->active_marker)
                                            ? active_color
                                            : inactive_color;

                // Draw triangle at top
                nvgBeginPath(vg);
                nvgMoveTo(vg, x, triangle_height); // Point at bottom
                nvgLineTo(vg, x - 5, 0);           // Left point at top
                nvgLineTo(vg, x + 5, 0);           // Right point at top
                nvgClosePath(vg);
                nvgFillColor(vg, marker_color); // Back to teal
                nvgFill(vg);

                // Draw vertical line
                nvgBeginPath(vg);
                nvgMoveTo(vg, x, 0);
                nvgLineTo(vg, x, line_extension);
                nvgStrokeColor(vg, marker_color);
                nvgStrokeWidth(vg, 1.0f);
                nvgStroke(vg);
            }
        }

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

    void onHover(const event::Hover &e) override
    {
        if (track_model && track_model->markers)
        {
            float marker_distance = 5.0f; // How close you need to be to a marker to trigger the hover effect

            for (const auto &marker_pair : *(track_model->markers))
            {
                float marker_x = ((marker_pair.first - track_model->visible_window_start) * box.size.x / (track_model->visible_window_end - track_model->visible_window_start));

                if (std::abs(e.pos.x - marker_x) < marker_distance)
                {
                    // Set the cursor to a drag hand when hovering over a marker
                    glfwSetCursor(APP->window->win, glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));
                    return;
                }
            }
        }

        // If not hovering over a marker, set the cursor to default
        glfwSetCursor(APP->window->win, NULL);
    }

    void onButton(const event::Button &e) override
    {
        TransparentWidget::onButton(e);
        e.consume(this);

        // Handle left-click dragging
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
            drag_position = e.pos;
            drag_start_position = e.pos;
            
            // Check for marker dragging
            if (track_model && track_model->markers) {
                float marker_distance = 5.0f;
                for (auto &marker_pair : *(track_model->markers)) {
                    float marker_x = ((marker_pair.first - track_model->visible_window_start) *
                                box.size.x / (track_model->visible_window_end - track_model->visible_window_start));
                    
                    if (std::abs(e.pos.x - marker_x) < marker_distance) {
                        dragging_marker = true;
                        drag_source_position = marker_pair.first;
                        markers_being_dragged = &marker_pair.second;
                        drag_start_x = e.pos.x;
                        drag_current_x = e.pos.x;
                        
                        if (!marker_pair.second.empty()) {
                            track_model->selectMarker(marker_pair.second[0].output_number);
                        }
                        return;
                    }
                }
            }

            // If no marker hit, start normal dragging
            dragging = true;
            if (track_model) {
                initial_visible_window_start = track_model->visible_window_start;
                initial_visible_window_end = track_model->visible_window_end;
                cumulative_drag_offset = 0.0f;
                cumulative_zoom_offset = 0.0f;
            }
        }
        else if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE) {
            dragging = false;
            dragging_marker = false;
            markers_being_dragged = nullptr;
        }
        // Handle right-click for marker deletion
        else if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
            if (track_model && track_model->markers) {
                float marker_distance = 5.0f;
                for (auto &marker_pair : *(track_model->markers)) {
                    float marker_x = ((marker_pair.first - track_model->visible_window_start) *
                                box.size.x / (track_model->visible_window_end - track_model->visible_window_start));
                    
                    if (std::abs(e.pos.x - marker_x) < marker_distance) {
                        track_model->removeMarkers(marker_pair.first);
                        break;
                    }
                }
            }
        }
    }

    void onDragMove(const event::DragMove &e) override
    {
        e.consume(this);
        if (dragging && track_model && track_model->sample && track_model->sample->isLoaded()) {
            // Calculate pan offset
            cumulative_drag_offset += e.mouseDelta.x;
            float window_width = initial_visible_window_end - initial_visible_window_start;
            float track_width = box.size.x;
            float sample_delta = (cumulative_drag_offset / track_width) * window_width;

            // Calculate zoom
            float zoom_sensitivity = 0.005f;
            cumulative_zoom_offset += e.mouseDelta.y;
            float mouse_relative_x = drag_start_position.x / box.size.x;
            float focus_point = initial_visible_window_start +
                                mouse_relative_x * (initial_visible_window_end - initial_visible_window_start);
            float zoom_multiplier = std::exp(cumulative_zoom_offset * zoom_sensitivity);
            float new_window_size = (initial_visible_window_end - initial_visible_window_start) * zoom_multiplier;
            float min_window_size = 2.0f;
            new_window_size = std::max(min_window_size, new_window_size);

            // Apply both pan and zoom
            float new_start = focus_point - (mouse_relative_x * new_window_size) - sample_delta;
            float new_end = new_start + new_window_size;
            
            // Clamp values
            track_model->visible_window_start = std::max(0.0f, new_start);
            track_model->visible_window_end = std::min(
                static_cast<float>(track_model->sample->size()),
                new_end);
        }
        else if (dragging_marker && markers_being_dragged && track_model && track_model->markers)
        {
            float zoom = getAbsoluteZoom();
            float current_x = drag_start_x + e.mouseDelta.x / zoom;
            float relative_x = current_x / box.size.x;
            relative_x = rack::math::clamp(relative_x, 0.0f, 1.0f);
            
            unsigned int new_position = track_model->visible_window_start + relative_x * (track_model->visible_window_end - track_model->visible_window_start);
            
            if (new_position != drag_source_position)
            {
                std::vector<Marker> markers_copy = *markers_being_dragged;
                track_model->insertMarkers(new_position, markers_copy);
                track_model->removeMarkers(drag_source_position);
                drag_source_position = new_position;
                markers_being_dragged = &(track_model->markers->at(new_position));
                drag_start_x = current_x; // Update for next move
            }
        }
    }

    void onDoubleClick(const event::DoubleClick &e) override
    {
        if (track_model && track_model->markers)
        {
            float relative_x = drag_position.x / box.size.x;
            unsigned int click_position = track_model->visible_window_start +
                                          relative_x * (track_model->visible_window_end - track_model->visible_window_start);

            // Add marker at click position
            track_model->addMarker(click_position);
        }
        e.consume(this);
    }

    void step() override
    {
        TransparentWidget::step();

        // Sample has changed
        if (sample_filename != track_model->sample->filename)
        {
            sample_filename = track_model->sample->filename;
            track_model->initialize();
        }
    }
};
