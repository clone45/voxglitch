#pragma once
#include <rack.hpp>
#include <vector>

// ============================================================================
// TrackWidget - Zoomable / pannable waveform display with markers
// ============================================================================
//
// Originally built for CueResearch and reused by Looper. Supports two
// orientations:
//
//   TrackOrientation::HORIZONTAL  (default)
//     The sample axis runs left-to-right; amplitude is drawn vertically.
//     Existing CueResearch behavior. No change to caller code -- the
//     orientation parameter has a default value, so old constructor
//     callsites keep working.
//
//   TrackOrientation::VERTICAL
//     The sample axis runs top-to-bottom; amplitude is drawn horizontally.
//     Used by the Looper's vertical waveform redesign.
//
// Internally the code reasons in terms of MAIN (the sample axis, the long
// edge) and CROSS (perpendicular, amplitude direction). Helpers map those
// to screen (x, y) at the boundary -- the geometry of every element
// (waveform chunks, playhead indicator, marker triangles+lines, mouse
// hit-tests, scroll/drag axes) flips together.
// ============================================================================

struct TrackWidget : TransparentWidget
{
    TrackModel *track_model = nullptr;
    // Used to watch for updates to the sample
    std::string sample_filename = "";

    // Layout orientation. Default keeps the historical behavior intact.
    TrackOrientation orientation = TrackOrientation::HORIZONTAL;

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
    float drag_start_main = 0.0f;     // main-axis screen coord at drag start
    float drag_current_main = 0.0f;   // main-axis screen coord during drag

    // Properties for scrubber interaction
    bool scrubber_dragging = false;
    float scrubber_hit_zone = 5.0f;

    // Shared properties that can be used by both scrubbing and view dragging
    Vec mouse_click_position;
    float cumulative_drag_offset = 0.0f;

    // Padding properties (in CSS-style top/right/bottom/left).
    // These keep their conventional names even in vertical mode -- the
    // orientation helpers below pick the right pair as "main" vs "cross".
    float container_padding_top = 2.0f;
    float container_padding_right = 2.0f;
    float container_padding_bottom = 2.0f;
    float container_padding_left = 2.0f;

    // Original constructor (horizontal default). Existing callers don't
    // change.
    TrackWidget(float x, float y, float width, float height, TrackModel *track_model)
        : TrackWidget(x, y, width, height, track_model, TrackOrientation::HORIZONTAL) {}

    // Constructor variant that takes orientation.
    TrackWidget(float x, float y, float width, float height, TrackModel *track_model,
                TrackOrientation orientation_arg)
    {
        box.size = Vec(width, height);
        box.pos = Vec(x, y);
        this->track_model = track_model;
        this->orientation = orientation_arg;

        // Register for playhead updates
        track_model->onPlayheadChanged = [this](unsigned int position) {
            // Force redraw when playhead position changes
            // This will trigger drawPlaybackIndicator with new position
            // Widget::dirty = true;
        };
    }

    // -----------------------------------------------------------------------
    // Orientation helpers - map MAIN (sample axis) / CROSS (amplitude) to
    // screen (x, y).
    // -----------------------------------------------------------------------

    bool isHorizontal() const { return orientation == TrackOrientation::HORIZONTAL; }

    float mainAxisSize()  const { return isHorizontal() ? box.size.x : box.size.y; }
    float crossAxisSize() const { return isHorizontal() ? box.size.y : box.size.x; }

    float mainAxisPaddingStart() const { return isHorizontal() ? container_padding_left  : container_padding_top; }
    float mainAxisPaddingEnd()   const { return isHorizontal() ? container_padding_right : container_padding_bottom; }
    float crossAxisPaddingStart() const { return isHorizontal() ? container_padding_top  : container_padding_left; }
    float crossAxisPaddingEnd()   const { return isHorizontal() ? container_padding_bottom : container_padding_right; }

    float drawableMain()  const { return mainAxisSize()  - (mainAxisPaddingStart()  + mainAxisPaddingEnd()); }
    float drawableCross() const { return crossAxisSize() - (crossAxisPaddingStart() + crossAxisPaddingEnd()); }

    float mouseMainAxis(const Vec& p)  const { return isHorizontal() ? p.x : p.y; }
    float mouseCrossAxis(const Vec& p) const { return isHorizontal() ? p.y : p.x; }

    // Draw a rect in axis-neutral coordinates: position / size are
    // expressed as (main, cross) pairs and translated to screen here.
    void rectAxis(NVGcontext* vg, float main_pos, float cross_pos,
                  float main_size, float cross_size)
    {
        if (isHorizontal())
            nvgRect(vg, main_pos, cross_pos, main_size, cross_size);
        else
            nvgRect(vg, cross_pos, main_pos, cross_size, main_size);
    }

    // Map sample position to main-axis screen coordinate (was sampleToScreenX).
    float sampleToScreenMain(unsigned int sample_pos) const {
        if (!track_model || !track_model->sample) return 0.0f;

        float drawable = drawableMain();
        float relative_pos = 0.0f;

        if (track_model->visible_window_end > track_model->visible_window_start) {
            relative_pos = float(sample_pos - track_model->visible_window_start) /
                float(track_model->visible_window_end - track_model->visible_window_start);
        }

        relative_pos = rack::math::clamp(relative_pos, 0.0f, 1.0f);
        return mainAxisPaddingStart() + (relative_pos * drawable);
    }

    // Reverse mapping: main-axis screen coord -> sample position
    // (was screenToSample).
    unsigned int screenMainToSample(float main_screen) const {
        if (!track_model || !track_model->sample) return 0;

        float drawable = drawableMain();
        float relative_pos = (main_screen - mainAxisPaddingStart()) / drawable;
        relative_pos = rack::math::clamp(relative_pos, 0.0f, 1.0f);

        return track_model->visible_window_start +
            (relative_pos * (track_model->visible_window_end - track_model->visible_window_start));
    }

    // Backward-compat aliases. CueResearch and other existing callers expect
    // these names; they delegate to the orientation-aware versions.
    float sampleToScreenX(unsigned int sample_pos) const { return sampleToScreenMain(sample_pos); }
    unsigned int screenToSample(float screen_x) const { return screenMainToSample(screen_x); }

    // -----------------------------------------------------------------------
    // Drawing
    // -----------------------------------------------------------------------

    void draw(const DrawArgs &args) override
    {
        const auto vg = args.vg;
        nvgSave(vg);

        // Draw the track background (full box, axis-agnostic).
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(vg, nvgRGB(0x10, 0x20, 0x20));
        nvgFill(vg);

        // Draw the waveform
        if (track_model && track_model->sample && track_model->sample->isLoaded())
        {
            drawWaveform(vg);
            drawMarkers(args);
            drawPlaybackIndicator(args);
        }

        nvgRestore(vg);
    }

    void drawWaveform(NVGcontext *vg) {
        if (!track_model || !track_model->sample) return;

        nvgSave(vg);
        nvgBeginPath(vg);

        // Clamp visible window to sample bounds
        unsigned int visible_sample_start = std::min(
            static_cast<unsigned int>(track_model->visible_window_start),
            track_model->sample->size() - 1
        );
        unsigned int visible_sample_end = std::min(
            static_cast<unsigned int>(track_model->visible_window_end),
            track_model->sample->size()
        );

        float main_size  = mainAxisSize();
        float cross_size = crossAxisSize();

        // Check if we need to update the cache
        bool needs_update = !track_model->cache_valid ||
            visible_sample_start != track_model->cached_visible_start ||
            visible_sample_end != track_model->cached_visible_end ||
            main_size != track_model->cached_width ||
            cross_size != track_model->cached_height;

        if (needs_update) {
            track_model->updateWaveformCache(
                main_size, cross_size,
                mainAxisPaddingStart(), mainAxisPaddingEnd(),
                crossAxisPaddingStart(), crossAxisPaddingEnd()
            );
        }

        // Draw from cache. Chunks are stored in axis-neutral coords; rectAxis
        // routes them to (x, y) per orientation.
        for (const auto& chunk : track_model->chunk_cache) {
            if (chunk.valid) {
                rectAxis(vg, chunk.main_pos, chunk.cross_pos,
                         chunk.main_size + 0.2f, chunk.amplitude);
            }
        }

        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200));
        nvgFill(vg);
        nvgRestore(vg);
    }

    void drawPlaybackIndicator(const DrawArgs &args)
    {
        if (!track_model || !track_model->sample)
            return;

        const auto vg = args.vg;
        nvgSave(vg);

        unsigned int position = track_model->playhead_position;

        if (position >= track_model->visible_window_start &&
            position <= track_model->visible_window_end)
        {
            float main_pos = sampleToScreenMain(position);

            // Center the indicator stripe on the sample position
            if (playback_indicator_width > 1.0f) {
                main_pos -= (playback_indicator_width / 2.0f);
            }

            // Stripe spans the full cross axis (between cross paddings).
            nvgBeginPath(vg);
            rectAxis(vg, main_pos, crossAxisPaddingStart(),
                     playback_indicator_width, drawableCross());
            nvgFillColor(vg, playback_indicator_color);
            nvgFill(vg);
        }

        nvgRestore(vg);
    }

    void drawMarkers(const DrawArgs &args)
    {
        if (!track_model || !track_model->sample || !track_model->markers)
            return;

        const auto vg = args.vg;
        nvgSave(vg);

        unsigned int visible_sample_start = track_model->visible_window_start;
        unsigned int visible_sample_end = track_model->visible_window_end;

        float triangle_height = 10.0f;

        // Define our colors
        NVGcolor inactive_color = nvgRGBA(100, 100, 100, 180);
        NVGcolor active_color = nvgRGBA(32, 178, 170, 255);

        for (const auto &marker_pair : *(track_model->markers))
        {
            if (!isValidMarker(marker_pair.first)) {
                continue;
            }

            unsigned int pos = marker_pair.first;

            if (pos < visible_sample_start || pos > visible_sample_end)
                continue;

            float main = sampleToScreenMain(pos);
            float cross_start = crossAxisPaddingStart();
            float cross_end   = crossAxisSize() - crossAxisPaddingEnd();

            for (const auto &marker : marker_pair.second)
            {
                NVGcolor marker_color = (marker.output_number == track_model->active_marker)
                                            ? active_color
                                            : inactive_color;

                // Triangle at the cross-start edge (top in horizontal, left
                // in vertical). Apex points into the track along the cross
                // axis; base sits on the cross-start edge spanning +/- 5
                // along the main axis.
                nvgBeginPath(vg);
                if (isHorizontal()) {
                    // base along x at y=cross_start; apex pointing down
                    nvgMoveTo(vg, main,     cross_start + triangle_height);
                    nvgLineTo(vg, main - 5, cross_start);
                    nvgLineTo(vg, main + 5, cross_start);
                } else {
                    // base along y at x=cross_start; apex pointing right
                    nvgMoveTo(vg, cross_start + triangle_height, main);
                    nvgLineTo(vg, cross_start, main - 5);
                    nvgLineTo(vg, cross_start, main + 5);
                }
                nvgClosePath(vg);
                nvgFillColor(vg, marker_color);
                nvgFill(vg);

                // Stripe spanning the cross axis at the marker's main pos.
                nvgBeginPath(vg);
                if (isHorizontal()) {
                    nvgMoveTo(vg, main, cross_start);
                    nvgLineTo(vg, main, cross_end);
                } else {
                    nvgMoveTo(vg, cross_start, main);
                    nvgLineTo(vg, cross_end,   main);
                }
                nvgStrokeColor(vg, marker_color);
                nvgStrokeWidth(vg, 1.0f);
                nvgStroke(vg);
            }
        }

        nvgRestore(vg);
    }

    std::vector<unsigned int> findMarkersNearPosition(const Vec &click_position)
    {
        std::vector<unsigned int> found_markers;

        if (track_model && track_model->markers && !track_model->isLockedMarkers())
        {
            float marker_distance = 5.0f;
            float click_main = mouseMainAxis(click_position);

            for (const auto &marker_pair : *(track_model->markers))
            {
                if (!isValidMarker(marker_pair.first)) {
                    continue;
                }

                if (marker_pair.first >= track_model->visible_window_start &&
                    marker_pair.first <= track_model->visible_window_end) {

                    float marker_main = sampleToScreenMain(marker_pair.first);

                    if (std::abs(click_main - marker_main) < marker_distance)
                    {
                        found_markers.push_back(marker_pair.first);
                    }
                }
            }
        }

        return found_markers;
    }

    // Convert a mouse position to a sample index using the main axis.
    float mouseToSamplePosition(Vec mouse_pos)
    {
        return (float)screenMainToSample(mouseMainAxis(mouse_pos));
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
        ui::Menu* menu = createMenu();

        menu->addChild(createMenuItem("Clear All Markers", "", [=]() {
            track_model->clearMarkers();
        }));

        menu->addChild(createMenuItem("Add Marker", "", [=]() {
            float sample_position = mouseToSamplePosition(context_menu_target_position);
            track_model->addMarker(sample_position);
        }));
    }

    void createMarkerContextMenu()
    {
        ui::Menu* menu = createMenu();

        menu->addChild(createMenuItem("Delete Marker", "", [=]() {
            std::vector<unsigned int> nearby_markers = findMarkersNearPosition(context_menu_target_position);
            if (!nearby_markers.empty())
            {
                track_model->removeMarkers(nearby_markers.front());
            }
        }));
    }

    void setIndicatorWidth(float width) { playback_indicator_width = width; }
    void setIndicatorColor(NVGcolor color) { playback_indicator_color = color; }

    bool isValidMarker(unsigned int position) const {
        if (!track_model || !track_model->markers) return false;
        return track_model->markers->find(position) != track_model->markers->end();
    }

    // Padding setters (matching WaveformWidget's interface).
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

    //
    //
    //  █▀▀ █░█ █▀▀ █▄░█ ▀█▀   █░█ ▄▀█ █▄░█ █▀▄ █░░ █ █▄░█ █▀▀
    //  ██▄ ▀▄▀ ██▄ █░▀█ ░█░   █▀█ █▀█ █░▀█ █▄▀ █▄▄ █ █░▀█ █▄█    // Event handling
    //
    //

    void onHoverScroll(const event::HoverScroll &e) override
    {
        if (track_model->areInteractionsLocked()) return;

        e.consume(this);

        if (track_model && track_model->sample && track_model->sample->isLoaded())
        {
            float zoom_factor = 1.1f;

            // Mouse position along the sample axis, normalized 0..1 across
            // the widget's main extent.
            float mouse_relative_main = (mouseMainAxis(e.pos) - mouseMainAxis(box.pos)) / mainAxisSize();

            float focus_point = track_model->visible_window_start +
                mouse_relative_main * (track_model->visible_window_end - track_model->visible_window_start);

            // Scroll-wheel y still drives zoom in/out regardless of orientation.
            if (e.scrollDelta.y > 0)
            {
                float window_size = track_model->visible_window_end - track_model->visible_window_start;
                float new_window_size = window_size / zoom_factor;
                track_model->visible_window_start = std::max(0.0f, focus_point - (mouse_relative_main * new_window_size));
                track_model->visible_window_end   = std::min(static_cast<float>(track_model->sample->size()),
                                                             track_model->visible_window_start + new_window_size);
            }
            else if (e.scrollDelta.y < 0)
            {
                float window_size = track_model->visible_window_end - track_model->visible_window_start;
                float new_window_size = window_size * zoom_factor;
                track_model->visible_window_start = std::max(0.0f, focus_point - (mouse_relative_main * new_window_size));
                track_model->visible_window_end   = std::min(static_cast<float>(track_model->sample->size()),
                                                             track_model->visible_window_start + new_window_size);
            }
            track_model->invalidateCache();
        }
    }

    void onHover(const event::Hover &e) override
    {
        if (track_model->areInteractionsLocked()) return;

        e.consume(this);

        // Check for scrubber hover first
        if (track_model && track_model->sample) {
            float relative_playback_pos = float(track_model->playhead_position -
                track_model->visible_window_start) /
                (track_model->visible_window_end - track_model->visible_window_start);

            float scrubber_main = mainAxisPaddingStart() + (relative_playback_pos * drawableMain());
            float mouse_main    = mouseMainAxis(e.pos);

            if (std::abs(mouse_main - scrubber_main) < scrubber_hit_zone) {
                int cursor_kind = isHorizontal() ? GLFW_HRESIZE_CURSOR : GLFW_VRESIZE_CURSOR;
                glfwSetCursor(APP->window->win, glfwCreateStandardCursor(cursor_kind));
                return;
            }
        }

        // Check for marker hover
        if (track_model && track_model->markers && !track_model->isLockedMarkers())
        {
            float marker_distance = 5.0f;
            float mouse_main = mouseMainAxis(e.pos);

            for (const auto &marker_pair : *(track_model->markers))
            {
                if (!isValidMarker(marker_pair.first)) continue;

                if (marker_pair.first >= track_model->visible_window_start &&
                    marker_pair.first <= track_model->visible_window_end) {

                    float marker_main = sampleToScreenMain(marker_pair.first);

                    if (std::abs(mouse_main - marker_main) < marker_distance)
                    {
                        int cursor_kind = isHorizontal() ? GLFW_HRESIZE_CURSOR : GLFW_VRESIZE_CURSOR;
                        glfwSetCursor(APP->window->win, glfwCreateStandardCursor(cursor_kind));
                        return;
                    }
                }
            }
        }

        glfwSetCursor(APP->window->win, NULL);
    }

    void onLeave(const event::Leave &e) override
    {
        TransparentWidget::onLeave(e);

        glfwSetCursor(APP->window->win, NULL);

        if (scrubber_dragging) {
            scrubber_dragging = false;
            track_model->scrubber_dragging = false;
        }
        dragging_marker = false;
        markers_being_dragged = nullptr;
        dragging = false;
    }

    void onButton(const event::Button &e) override
    {
        TransparentWidget::onButton(e);

        if (track_model->areInteractionsLocked()) return;

        e.consume(this);

        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            mouse_click_position = e.pos;
            drag_start_position = e.pos;

            float mouse_main = mouseMainAxis(e.pos);

            // Scrubber hit?
            float relative_playback_pos = float(track_model->playhead_position -
                track_model->visible_window_start) /
                (track_model->visible_window_end - track_model->visible_window_start);
            float scrubber_main = mainAxisPaddingStart() + (relative_playback_pos * drawableMain());

            if (std::abs(mouse_main - scrubber_main) < scrubber_hit_zone) {
                scrubber_dragging = true;
                track_model->scrubber_dragging = true;
                drag_start_main = mouse_main;
                cumulative_drag_offset = 0.0f;
                return;
            }

            // Marker hit?
            if (track_model && track_model->markers && !track_model->isLockedMarkers())
            {
                float marker_distance = 5.0f;

                for (auto &marker_pair : *(track_model->markers))
                {
                    if (!isValidMarker(marker_pair.first)) continue;

                    if (marker_pair.first >= track_model->visible_window_start &&
                        marker_pair.first <= track_model->visible_window_end) {

                        float marker_main = sampleToScreenMain(marker_pair.first);

                        if (std::abs(mouse_main - marker_main) < marker_distance)
                        {
                            dragging_marker = true;
                            drag_source_position = marker_pair.first;
                            markers_being_dragged = &marker_pair.second;
                            drag_start_main = mouse_main;
                            drag_current_main = mouse_main;
                            if (!marker_pair.second.empty())
                            {
                                track_model->selectMarker(marker_pair.second[0].output_number);
                            }
                            return;
                        }
                    }
                }
            }

            // Otherwise: pan dragging.
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
            if (scrubber_dragging)
            {
                scrubber_dragging = false;
                track_model->scrubber_dragging = false;
            }
            glfwSetCursor(APP->window->win, NULL);
        }
        // Right-click for marker context menu
        else if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0)
        {
            bool marker_hit = false;
            if (track_model && track_model->markers && !track_model->isLockedMarkers())
            {
                float marker_distance = 5.0f;
                float mouse_main = mouseMainAxis(e.pos);
                for (auto &marker_pair : *(track_model->markers))
                {
                    float marker_main = sampleToScreenMain(marker_pair.first);
                    if (std::abs(mouse_main - marker_main) < marker_distance)
                    {
                        marker_hit = true;
                        break;
                    }
                }
            }
            context_menu_target_position = e.pos;
            if (marker_hit && !track_model->isLockedMarkers()) createMarkerContextMenu();
            if (!marker_hit && !track_model->isLockedMarkers()) createContextMenu();
        }
    }

    void onDragMove(const event::DragMove &e) override
    {
        if (track_model->areInteractionsLocked()) return;

        e.consume(this);

        if (scrubber_dragging)
        {
            float zoom = getAbsoluteZoom();
            float delta_main = mouseMainAxis(e.mouseDelta) / zoom;
            float current_main = drag_start_main + delta_main;

            float relative = (current_main - mainAxisPaddingStart()) / drawableMain();
            relative = rack::math::clamp(relative, 0.0f, 1.0f);

            unsigned int new_position = track_model->visible_window_start +
                static_cast<unsigned int>(relative *
                    (track_model->visible_window_end - track_model->visible_window_start));

            track_model->onDragPlayhead(new_position);
            drag_start_main = current_main;
            return;
        }

        float final_start, final_end;

        if (dragging && track_model && track_model->sample && track_model->sample->isLoaded())
        {
            // Pan along the main axis.
            cumulative_drag_offset += mouseMainAxis(e.mouseDelta);
            float window_width = initial_visible_window_end - initial_visible_window_start;
            float main_size = mainAxisSize();
            float sample_delta = (cumulative_drag_offset / main_size) * window_width;

            float new_start = initial_visible_window_start - sample_delta;
            float new_end   = new_start + window_width;

            if (*(track_model->enable_vertical_drag_zoom))
            {
                // Zoom on the cross axis (the perpendicular drag axis).
                float zoom_sensitivity = 0.005f;
                cumulative_zoom_offset += mouseCrossAxis(e.mouseDelta);

                float mouse_relative_main =
                    (mouseMainAxis(drag_start_position) - mouseMainAxis(box.pos)) / main_size;
                float focus_point = initial_visible_window_start +
                    mouse_relative_main * (initial_visible_window_end - initial_visible_window_start);
                float zoom_multiplier = std::exp(cumulative_zoom_offset * zoom_sensitivity);
                float new_window_size = (initial_visible_window_end - initial_visible_window_start) * zoom_multiplier;
                float min_window_size = 2.0f;
                new_window_size = std::max(min_window_size, new_window_size);

                new_start = focus_point - (mouse_relative_main * new_window_size) - sample_delta;
                new_end   = new_start + new_window_size;
            }

            // Edge handling
            if (new_start <= 0.0f)
            {
                final_start = 0.0f;
                final_end = new_end - new_start;
            }
            else if (new_end >= track_model->sample->size())
            {
                final_end = track_model->sample->size();
                final_start = final_end - (new_end - new_start);
            }
            else
            {
                final_start = new_start;
                final_end = new_end;
            }

            track_model->visible_window_start = std::max(0.0f, final_start);
            track_model->visible_window_end = std::min(
                static_cast<float>(track_model->sample->size()), final_end);

            track_model->invalidateCache();
        }
        else if (dragging_marker && markers_being_dragged && track_model && track_model->markers && !track_model->isLockedMarkers())
        {
            if (!isValidMarker(drag_source_position)) {
                dragging_marker = false;
                markers_being_dragged = nullptr;
                return;
            }

            float zoom = getAbsoluteZoom();
            float current_main = drag_start_main + mouseMainAxis(e.mouseDelta) / zoom;

            float relative = (current_main - mainAxisPaddingStart()) / drawableMain();
            relative = rack::math::clamp(relative, 0.0f, 1.0f);

            unsigned int new_position = track_model->visible_window_start +
                static_cast<unsigned int>(relative *
                    (track_model->visible_window_end - track_model->visible_window_start));

            if (new_position != drag_source_position)
            {
                std::vector<Marker> markers_copy = *markers_being_dragged;
                track_model->insertMarkers(new_position, markers_copy);
                track_model->removeMarkers(drag_source_position);
                drag_source_position = new_position;
                markers_being_dragged = &(track_model->markers->at(new_position));
                drag_start_main = current_main;
            }
        }
    }

    void onDoubleClick(const event::DoubleClick &e) override
    {
        e.consume(this);

        if (track_model->areInteractionsLocked()) return;

        if (track_model && track_model->markers && !track_model->isLockedMarkers())
        {
            std::vector<unsigned int> nearby_markers = findMarkersNearPosition(mouse_click_position);

            if (!nearby_markers.empty())
            {
                if (dragging_marker && nearby_markers.front() == drag_source_position)
                {
                    dragging_marker = false;
                    markers_being_dragged = nullptr;
                    glfwSetCursor(APP->window->win, NULL);
                }

                track_model->removeMarkers(nearby_markers.front());
            }
            else
            {
                float sample_position = mouseToSamplePosition(mouse_click_position);
                track_model->addMarker(sample_position);
            }
        }
    }
};
