#pragma once
#include <rack.hpp>
#include <vector>

#include "Marker.hpp"

struct WaveformChunk {
    float average_height;
    float x_position;
    float y_position;
    float width;
    bool valid;
};

struct TrackModel
{
    Sample *sample;

    unsigned int samples_per_average = 1000; // Number of samples to average per chunk
    std::vector<float> averages; // Store waveform averages
    float max_average = 0.0;

    // Zoom properties
    float zoom_factor = 1.0f; // Zoom level, default is full view
    unsigned int visible_window_start = 0; // Start index of the visible window
    unsigned int visible_window_end = 0;   // End index of the visible window
    
    // Scrubbing-related members
    // float playback_percentage = 0.0f;
    // bool scrubber_dragging = false;
    unsigned int playhead_position = 0;

    // Marker properties
    std::map<unsigned int, std::vector<Marker>>* markers = nullptr;
    int active_marker = 0;

    // Options
    bool *enable_vertical_drag_zoom = nullptr;
    bool *lock_markers = nullptr;
  
    // Keeping these temporarily while testing
    bool scrubber_dragging = false;
    std::function<void(unsigned int)> onDragPlayhead;
    // end temporary

    // Callbacks
    std::function<void(int)> onMarkerSelected = nullptr; // Callback for when a marker is selected
    std::function<void()> onSyncMarkers = nullptr; // Callback for syncing markers with waveform
    std::function<void()> lockMarkers = nullptr; // Callback for setting lock markers value
    std::function<void(unsigned int)> onPlayheadChanged = nullptr;

    // Cache-related members
    std::vector<WaveformChunk> chunk_cache;
    bool cache_valid = false;
    unsigned int cached_visible_start = 0;
    unsigned int cached_visible_end = 0;
    float cached_width = 0;
    float cached_height = 0;
    static const unsigned int NUM_CHUNKS = 1000;

    void setSample(Sample *sample) 
    {
        this->sample = sample;
        invalidateCache();
        initialize();
    }

    void initialize() 
    {
        if (sample && sample->isLoaded()) 
        {
            visible_window_end = sample->size(); // Initially, set the visible window to the full sample length
            computeAverages(); // Compute averages when sample is set
            normalizeAverages(); // Normalize them after computation
        }
    }

    void invalidateCache() {
        cache_valid = false;
    }

    void setVerticalDragZoomEnabled(bool *enabled) 
    {
        enable_vertical_drag_zoom = enabled;
    }

    void setLockMarkers(bool *locked) 
    {
        lock_markers = locked;
    }

    bool isLocked() 
    {
        return lock_markers ? *lock_markers : false;
    }

    void selectMarker(int output_number) {
        active_marker = output_number;
        if (onMarkerSelected) {
            onMarkerSelected(output_number);
        }
    }

    void setMarkers(std::map<unsigned int, std::vector<Marker>>* markers_map) {
        this->markers = markers_map;
        onSyncMarkers();
    }

    void addMarker(unsigned int position) {
        if (markers) {
            (*markers)[position].push_back(Marker(active_marker));
            onSyncMarkers();
        }
    }
    
    void insertMarkers(unsigned int position, const std::vector<Marker>& new_markers) {
        // Check if the markers pointer is valid
        if (markers && !new_markers.empty()) {
            // Insert or update the markers at the given position
            (*markers)[position] = new_markers;
            // Sync markers with any listeners
            if (onSyncMarkers) {
                onSyncMarkers();
            }
        }
    }

    void removeMarkers(unsigned int position) {
        // Check if the markers pointer is valid
        if (markers && markers->find(position) != markers->end()) {
            markers->erase(position);
            // Sync markers with any listeners
            if (onSyncMarkers) {
                onSyncMarkers();
            }
        }
    }

    void clearMarkers() {
        if (markers) {
            markers->clear();
            if (onSyncMarkers) {
                onSyncMarkers();
            }
        }
    }

    void setActiveMarker(int marker) {
        active_marker = marker;
    }

    // Add new methods
    void updatePlayheadPosition(unsigned int position) {
        if (position != playhead_position) {
            playhead_position = position;
            // Tell the widget to update its display
            if (onPlayheadChanged) {
                onPlayheadChanged(position);
            }
        }
    }

    // Register call back for playhead position changes
    void registerDragPlayheadObserver(std::function<void(unsigned int)> callback) {
        onDragPlayhead = callback;
    }

    // Method to adjust zoom level
    void setZoomFactor(float new_zoom_factor)
    {
        zoom_factor = rack::math::clamp(new_zoom_factor, 0.1f, 10.0f); // Clamp zoom factor to a reasonable range
        updateVisibleWindow();
    }

    void updateWaveformCache(float track_width, float track_height, float padding_left, float padding_right, 
                           float padding_top, float padding_bottom) {
        if (!sample || !sample->isLoaded()) return;

        unsigned int num_visible_samples = visible_window_end - visible_window_start;
        unsigned int chunk_size = std::max(1u, num_visible_samples / NUM_CHUNKS);
        float drawable_width = track_width - (padding_left + padding_right);
        float chunk_width = drawable_width / static_cast<float>(NUM_CHUNKS);
        float rect_overlap = chunk_width / 4.0f;

        // Resize cache if needed
        if (chunk_cache.size() != NUM_CHUNKS) {
            chunk_cache.resize(NUM_CHUNKS);
        }

        // Update each chunk
        for (unsigned int chunk_index = 0; chunk_index < NUM_CHUNKS; ++chunk_index) {
            WaveformChunk& chunk = chunk_cache[chunk_index];
            
            unsigned int chunk_start = visible_window_start + chunk_index * chunk_size;
            unsigned int chunk_end = std::min(chunk_start + chunk_size, visible_window_end);

            float left_sum = 0.0f;
            float right_sum = 0.0f;
            unsigned int count = 0;

            for (unsigned int i = chunk_start; i < chunk_end; ++i) {
                float left, right;
                sample->read(i, &left, &right);
                left_sum += std::abs(left);
                right_sum += std::abs(right);
                count++;
            }

            if (count > 0) {
                float average_height = (left_sum + right_sum) / (2.0f * count);
                average_height *= (track_height - (padding_top + padding_bottom));
                
                chunk.average_height = average_height;
                chunk.x_position = padding_left + (chunk_index * chunk_width);
                chunk.y_position = padding_top + 
                    ((track_height - padding_top - padding_bottom - average_height) / 2.0f);
                chunk.width = chunk_width + rect_overlap;
                chunk.valid = true;
            } else {
                chunk.valid = false;
            }
        }

        // Update cache state
        cache_valid = true;
        cached_visible_start = visible_window_start;
        cached_visible_end = visible_window_end;
        cached_width = track_width;
        cached_height = track_height;
    }

    // Update the visible window based on the current zoom factor
    void updateVisibleWindow()
    {
        if (!sample || !sample->isLoaded())
            return;

        unsigned int sample_size = sample->size();
        unsigned int visible_span = static_cast<unsigned int>(sample_size / zoom_factor);
        unsigned int visible_center = (visible_window_start + visible_window_end) / 2;

        // Update the visible window start and end, clamped to valid ranges
        visible_window_start = std::max(0, static_cast<int>(visible_center) - static_cast<int>(visible_span / 2));
        visible_window_end = std::min(sample_size, visible_window_start + visible_span);

        invalidateCache();
    }

    // Compute waveform averages for the entire sample
    void computeAverages()
    {
        if (!sample || !sample->isLoaded())
            return;

        unsigned int total_samples = sample->size();
        unsigned int num_chunks = (total_samples + samples_per_average - 1) / samples_per_average; // Calculate the number of chunks, rounding up
        averages.clear();
        averages.reserve(num_chunks); // Predefine size to match the number of chunks

        for (unsigned int chunk_index = 0; chunk_index < num_chunks; ++chunk_index) 
        {
            float left_sum = 0.0;
            float right_sum = 0.0;
            unsigned int count = 0;

            unsigned int chunk_start = chunk_index * samples_per_average;
            unsigned int chunk_end = std::min(chunk_start + samples_per_average, total_samples);

            for (unsigned int i = chunk_start; i < chunk_end; i++)
            {
                float left, right;
                sample->read(i, &left, &right);
                left_sum += std::abs(left);
                right_sum += std::abs(right);
                count++;
            }

            if (count > 0)
            {
                float average = (left_sum + right_sum) / (2.0f * count);
                averages.push_back(average);
                if (average > max_average)
                    max_average = average;
            }
            else
            {
                averages.push_back(0.0f);
            }
        }
    }

    // Normalize waveform averages
    void normalizeAverages()
    {
        if (max_average == 0.0f) return;
        for (auto &average : averages)
        {
            average = rack::math::clamp((1.0f / max_average) * average, 0.0f, 1.0f);
        }
    }
};
