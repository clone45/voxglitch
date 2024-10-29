#pragma once
#include <rack.hpp>
#include <vector>

struct TrackWidget : TransparentWidget
{
    TrackModel *track_model = nullptr;
    // Used to watch for updates to the sample
    std::string sample_filename = "";

    // Properties for sample view dragging/zooming
    Vec drag_start_position;
    Vec context_menu_target_position;

    float playback_indicator_width = 2.0f;
    NVGcolor playback_indicator_color = nvgRGBA(255, 215, 20, 200);

    bool dragging = false;
    bool dragging_zoom = false;
    bool shift_key_held = false;
    bool right_button_held = false;
    float initial_visible_window_start;
    float initial_visible_window_end;
    float cumulative_zoom_offset = 0.0f;

    // Properties for marker dragging
    bool dragging_marker = false;
    unsigned int drag_source_position = 0;
    std::vector<Marker> *markers_being_dragged = nullptr;
    float drag_start_x = 0.0f;
    float drag_current_x = 0.0f;

    // Properties for scrubber interaction
    bool scrubber_dragging = false;
    float scrubber_hit_zone = 5.0f;
    float initial_percentage = 0.0f;

    // Shared properties that can be used by both scrubbing and view dragging
    Vec mouse_click_position;
    float cumulative_drag_offset = 0.0f;

    // Padding properties
    float container_padding_top = 2.0f;
    float container_padding_right = 2.0f;
    float container_padding_bottom = 2.0f;
    float container_padding_left = 2.0f;

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
            drawPlaybackIndicator(args, box.size.x, box.size.y);
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

        // Calculate width of each chunk, accounting for padding
        float drawable_width = track_width - (container_padding_left + container_padding_right);
        float chunk_width = drawable_width / static_cast<float>(num_chunks);

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
                average_height *= (track_height - (container_padding_top + container_padding_bottom));

                // Calculate the x and y position for drawing, accounting for padding
                float x_position = container_padding_left + (chunk_index * chunk_width);
                float y_position = container_padding_top + ((track_height - container_padding_top - container_padding_bottom - average_height) / 2.0f);

                // Draw the rectangle representing the chunk
                float rect_overlap = chunk_width / 4.0f;
                nvgRect(vg, x_position, y_position, (chunk_width + rect_overlap), average_height);
            }
        }

        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200)); // Light grey color for the waveform
        nvgFill(vg);
        nvgRestore(vg);
    }

    void drawPlaybackIndicator(const DrawArgs &args, float track_width, float track_height)
    {
        if (!track_model || !track_model->sample)
            return;

        const auto vg = args.vg;
        nvgSave(vg);

        float sample_position = track_model->playback_percentage * track_model->sample->size();

        if (sample_position >= track_model->visible_window_start && 
            sample_position <= track_model->visible_window_end)
        {
            float relative_pos = (sample_position - track_model->visible_window_start) / 
                (track_model->visible_window_end - track_model->visible_window_start);
            
            float drawable_width = track_width - (container_padding_left + container_padding_right);
            float x_position = container_padding_left + (relative_pos * drawable_width);

            if (playback_indicator_width > 1.0f) {
                x_position -= (playback_indicator_width / 2.0f);
            }

            nvgBeginPath(vg);
            nvgRect(vg, x_position, container_padding_top, 
                    playback_indicator_width, 
                    track_height - (container_padding_top + container_padding_bottom));
            nvgFillColor(vg, playback_indicator_color);
            nvgFill(vg);
        }

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
        float drawable_width = track_width - (container_padding_left + container_padding_right);
        float samples_per_pixel = (visible_sample_end - visible_sample_start) / drawable_width;

        float line_extension = track_height - (container_padding_top + container_padding_bottom);
        float triangle_height = 10.0f;

        // Define our colors
        NVGcolor inactive_color = nvgRGBA(100, 100, 100, 180); // Gray for inactive
        NVGcolor active_color = nvgRGBA(32, 178, 170, 255);    // Bright teal for active

        for (const auto &marker_pair : *(track_model->markers))
        {
            unsigned int pos = marker_pair.first;

            if (pos < visible_sample_start || pos > visible_sample_end)
                continue;

            float x = container_padding_left + ((pos - visible_sample_start) / samples_per_pixel);

            for (const auto &marker : marker_pair.second)
            {
                // Choose color based on whether this marker matches the active output
                NVGcolor marker_color = (marker.output_number == track_model->active_marker)
                                            ? active_color
                                            : inactive_color;

                // Draw triangle at top, accounting for padding
                nvgBeginPath(vg);
                nvgMoveTo(vg, x, container_padding_top + triangle_height);
                nvgLineTo(vg, x - 5, container_padding_top);
                nvgLineTo(vg, x + 5, container_padding_top);
                nvgClosePath(vg);
                nvgFillColor(vg, marker_color);
                nvgFill(vg);

                // Draw vertical line, accounting for padding
                nvgBeginPath(vg);
                nvgMoveTo(vg, x, container_padding_top);
                nvgLineTo(vg, x, container_padding_top + line_extension);
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
        // Check for scrubber hover first
        if (track_model && track_model->sample) {
            // Convert playback percentage to visible window position, accounting for padding
            float drawable_width = box.size.x - (container_padding_left + container_padding_right);
            float relative_playback_pos = (track_model->playback_percentage * track_model->sample->size() - 
                track_model->visible_window_start) / (track_model->visible_window_end - track_model->visible_window_start);
            float scrubber_x = container_padding_left + (relative_playback_pos * drawable_width);
            
            if (std::abs(e.pos.x - scrubber_x) < scrubber_hit_zone) {
                glfwSetCursor(APP->window->win, glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));
                return;
            }
        }

        // Check for marker hover
        if (track_model && track_model->markers && !track_model->isLocked())
        {
            float marker_distance = 5.0f;
            float drawable_width = box.size.x - (container_padding_left + container_padding_right);
            
            for (const auto &marker_pair : *(track_model->markers))
            {
                if (marker_pair.first >= track_model->visible_window_start && 
                    marker_pair.first <= track_model->visible_window_end) {
                    
                    float relative_pos = float(marker_pair.first - track_model->visible_window_start) / 
                        float(track_model->visible_window_end - track_model->visible_window_start);
                    float marker_x = container_padding_left + (relative_pos * drawable_width);
                    
                    if (std::abs(e.pos.x - marker_x) < marker_distance)
                    {
                        glfwSetCursor(APP->window->win, glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));
                        return;
                    }
                }
            }
        }

        // If not hovering over scrubber or marker, set cursor to default
        glfwSetCursor(APP->window->win, NULL);
    }

    void onButton(const event::Button &e) override
    {
        TransparentWidget::onButton(e);
        e.consume(this);

        // Handle left-click dragging
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            mouse_click_position = e.pos;
            drag_start_position = e.pos;

            // Check for scrubber dragging first
            float drawable_width = box.size.x - (container_padding_left + container_padding_right);
            float relative_playback_pos = (track_model->playback_percentage * track_model->sample->size() - 
                track_model->visible_window_start) / (track_model->visible_window_end - track_model->visible_window_start);
            float scrubber_x = container_padding_left + (relative_playback_pos * drawable_width);
            
            if (std::abs(e.pos.x - scrubber_x) < scrubber_hit_zone) {
                scrubber_dragging = true;
                track_model->scrubber_dragging = true;
                drag_start_x = e.pos.x;
                cumulative_drag_offset = 0.0f;
                return;
            }

            // Check for marker dragging
            if (track_model && track_model->markers && !track_model->isLocked())
            {
                float marker_distance = 5.0f;
                
                for (auto &marker_pair : *(track_model->markers))
                {
                    if (marker_pair.first >= track_model->visible_window_start && 
                        marker_pair.first <= track_model->visible_window_end) {
                        
                        float relative_pos = float(marker_pair.first - track_model->visible_window_start) / 
                            float(track_model->visible_window_end - track_model->visible_window_start);
                        float marker_x = container_padding_left + (relative_pos * drawable_width);
                        
                        if (std::abs(e.pos.x - marker_x) < marker_distance)
                        {
                            dragging_marker = true;
                            drag_source_position = marker_pair.first;
                            markers_being_dragged = &marker_pair.second;
                            drag_start_x = e.pos.x;
                            drag_current_x = e.pos.x;
                            if (!marker_pair.second.empty())
                            {
                                track_model->selectMarker(marker_pair.second[0].output_number);
                            }
                            return;
                        }
                    }
                }
            }

            // If no marker or scrubber hit, start normal dragging
            dragging = true;
            if (track_model)
            {
                initial_visible_window_start = track_model->visible_window_start;
                initial_visible_window_end = track_model->visible_window_end;
                cumulative_drag_offset = 0.0f;
                cumulative_zoom_offset = 0.0f;
            }
        }
        else if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
        {
            dragging = false;
            dragging_marker = false;
            markers_being_dragged = nullptr;
            if (scrubber_dragging) {
                scrubber_dragging = false;
                track_model->scrubber_dragging = false;
            }
        }
        // Handle right-click for marker deletion
        else if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0)
        {
            bool marker_hit = false;
            if (track_model && track_model->markers && !track_model->isLocked())
            {
                float marker_distance = 5.0f;
                for (auto &marker_pair : *(track_model->markers))
                {
                    float marker_x = ((marker_pair.first - track_model->visible_window_start) *
                                      box.size.x / (track_model->visible_window_end - track_model->visible_window_start));
                    if (std::abs(e.pos.x - marker_x) < marker_distance)
                    {
                        marker_hit = true;
                        break;
                    }
                }
            }
            // Show context menu
            context_menu_target_position = e.pos; // Store the position for the context menu
            if (marker_hit && !track_model->isLocked()) createMarkerContextMenu();
            if (!marker_hit && !track_model->isLocked()) createContextMenu();
        }
    }

    void onDragMove(const event::DragMove &e) override
    {
        e.consume(this);

        if (scrubber_dragging) {
            float zoom = getAbsoluteZoom();
            float current_x = drag_start_x + e.mouseDelta.x / zoom;
            // Adjust relative_x calculation to account for padding
            float drawable_width = box.size.x - (container_padding_left + container_padding_right);
            float relative_x = (current_x - container_padding_left) / drawable_width;
            relative_x = rack::math::clamp(relative_x, 0.0f, 1.0f);
            
            unsigned int new_position = track_model->visible_window_start + 
                static_cast<unsigned int>(relative_x * (track_model->visible_window_end - track_model->visible_window_start));

            track_model->notifyScrubberPosition(new_position);
            drag_start_x = current_x;
            return;
        }

        float final_start, final_end;

        if (dragging && track_model && track_model->sample && track_model->sample->isLoaded())
        {
            // Calculate pan offset first
            cumulative_drag_offset += e.mouseDelta.x;
            float window_width = initial_visible_window_end - initial_visible_window_start;
            float track_width = box.size.x;
            float sample_delta = (cumulative_drag_offset / track_width) * window_width;

            // Apply pan
            float new_start = initial_visible_window_start - sample_delta;
            float new_end = new_start + window_width;

            if (*(track_model->enable_vertical_drag_zoom))
            {
                // Handle zooming (vertical)
                float zoom_sensitivity = 0.005f;
                cumulative_zoom_offset += e.mouseDelta.y;
                float mouse_relative_x = drag_start_position.x / box.size.x;
                float focus_point = initial_visible_window_start +
                                    mouse_relative_x * (initial_visible_window_end - initial_visible_window_start);
                float zoom_multiplier = std::exp(cumulative_zoom_offset * zoom_sensitivity);
                float new_window_size = (initial_visible_window_end - initial_visible_window_start) * zoom_multiplier;
                float min_window_size = 2.0f;
                new_window_size = std::max(min_window_size, new_window_size);

                // Update new_start and new_end with zoom
                new_start = focus_point - (mouse_relative_x * new_window_size) - sample_delta;
                new_end = new_start + new_window_size;
            }

            // Edge handling (applies to both zooming and non-zooming)
            if (new_start <= 0.0f)
            {
                final_start = 0.0f;
                final_end = new_end - new_start; // Maintain window size
            }
            else if (new_end >= track_model->sample->size())
            {
                final_end = track_model->sample->size();
                final_start = final_end - (new_end - new_start); // Maintain window size
            }
            else
            {
                final_start = new_start;
                final_end = new_end;
            }

            // Apply and clamp the final values
            track_model->visible_window_start = std::max(0.0f, final_start);
            track_model->visible_window_end = std::min(
                static_cast<float>(track_model->sample->size()),
                final_end);
        }
        else if (dragging_marker && markers_being_dragged && track_model && track_model->markers && !track_model->isLocked())
        {
            float zoom = getAbsoluteZoom();
            float current_x = drag_start_x + e.mouseDelta.x / zoom;
            float drawable_width = box.size.x - (container_padding_left + container_padding_right);
            
            // Calculate relative position accounting for padding
            float relative_x = (current_x - container_padding_left) / drawable_width;
            relative_x = rack::math::clamp(relative_x, 0.0f, 1.0f);

            unsigned int new_position = track_model->visible_window_start + 
                static_cast<unsigned int>(relative_x * (track_model->visible_window_end - track_model->visible_window_start));

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
        if (track_model && track_model->markers && !track_model->isLocked())
        {
            // Find markers near the click position
            std::vector<unsigned int> nearby_markers = findMarkersNearPosition(mouse_click_position);

            if (!nearby_markers.empty())
            {
                // Remove the first marker found (since we are returning all nearby markers, but removing only one)
                track_model->removeMarkers(nearby_markers.front());
            }
            else
            {
                // If no marker was found at the click position, add a new one
                float sample_position = mouseToSamplePosition(mouse_click_position);
                track_model->addMarker(sample_position);
            }
        }
        e.consume(this);
    }

    std::vector<unsigned int> findMarkersNearPosition(const Vec &click_position)
    {
        std::vector<unsigned int> found_markers;

        if (track_model && track_model->markers && !track_model->isLocked())
        {
            float marker_distance = 5.0f;
            float drawable_width = box.size.x - (container_padding_left + container_padding_right);

            for (const auto &marker_pair : *(track_model->markers))
            {
                if (marker_pair.first >= track_model->visible_window_start && 
                    marker_pair.first <= track_model->visible_window_end) {
                    
                    float relative_pos = float(marker_pair.first - track_model->visible_window_start) / 
                        float(track_model->visible_window_end - track_model->visible_window_start);
                    float marker_x = container_padding_left + (relative_pos * drawable_width);

                    if (std::abs(click_position.x - marker_x) < marker_distance)
                    {
                        found_markers.push_back(marker_pair.first);
                    }
                }
            }
        }

        return found_markers;
    }

    // Update mouseToSamplePosition to account for padding
    float mouseToSamplePosition(Vec mouse_pos)
    {
        float drawable_width = box.size.x - (container_padding_left + container_padding_right);
        float relative_x = (mouse_pos.x - container_padding_left) / drawable_width;
        relative_x = rack::math::clamp(relative_x, 0.0f, 1.0f);
        
        unsigned int sample_position = track_model->visible_window_start +
            relative_x * (track_model->visible_window_end - track_model->visible_window_start);
        return sample_position;
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

    void createContextMenu() 
    {
        // Create a new menu
        ui::Menu* menu = createMenu();

        menu->addChild(createMenuItem("Clear All Markers", "", [=]() 
        {
            track_model->clearMarkers();
        }));

        // Add marker
        menu->addChild(createMenuItem("Add Marker", "", [=]() 
        {
            float sample_position = mouseToSamplePosition(context_menu_target_position.x);
            track_model->addMarker(sample_position);
        }));
    }

    void createMarkerContextMenu() 
    {
        // Create a new menu
        ui::Menu* menu = createMenu();

        menu->addChild(createMenuItem("Delete Marker", "", [=]() 
        {
            std::vector<unsigned int> nearby_markers = findMarkersNearPosition(context_menu_target_position);
            if (!nearby_markers.empty())
            {
                track_model->removeMarkers(nearby_markers.front());
            }
        }));
    }

    void setIndicatorWidth(float width) {
        playback_indicator_width = width;
    }

    void setIndicatorColor(NVGcolor color) {
        playback_indicator_color = color;
    }

   // Add padding setter methods (matching WaveformWidget's interface)
    void setContainerPadding(float padding) {
        container_padding_top = padding;
        container_padding_right = padding;
        container_padding_bottom = padding;
        container_padding_left = padding;
    }

    void setContainerPadding(float vertical, float horizontal) {
        container_padding_top = vertical;
        container_padding_bottom = vertical;
        container_padding_left = horizontal;
        container_padding_right = horizontal;
    }

    void setContainerPadding(float top, float right, float bottom, float left) {
        container_padding_top = top;
        container_padding_right = right;
        container_padding_bottom = bottom;
        container_padding_left = left;
    }

};
