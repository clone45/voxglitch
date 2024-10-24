#pragma once
#include <rack.hpp>
#include <vector>

#include "Marker.hpp"

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
    
    std::map<unsigned int, std::vector<Marker>>* markers = nullptr;
    int active_marker = 0;

    // Callback for when a marker is selected
    std::function<void(int)> onMarkerSelected = nullptr;

    void setSample(Sample *sample) 
    {
        this->sample = sample;
        if (sample && sample->isLoaded()) 
        {
            visible_window_end = sample->size(); // Initially, set the visible window to the full sample length
            computeAverages(); // Compute averages when sample is set
            normalizeAverages(); // Normalize them after computation
        }
    }

    void selectMarker(int output_number) {
        active_marker = output_number;
        if (onMarkerSelected) {
            onMarkerSelected(output_number);
        }
    }

    void setMarkers(std::map<unsigned int, std::vector<Marker>>* markers_map) {
        this->markers = markers_map;
    }

    void addMarker(unsigned int position) {
        if (markers) {
            (*markers)[position].push_back(Marker(active_marker));
        }
    }

    void removeMarker(unsigned int position) {
        if (markers) {
            markers->erase(position);
        }
    }

    void setActiveMarker(int marker) {
        active_marker = marker;
    }

    // Method to adjust zoom level
    void setZoomFactor(float new_zoom_factor)
    {
        zoom_factor = rack::math::clamp(new_zoom_factor, 0.1f, 10.0f); // Clamp zoom factor to a reasonable range
        updateVisibleWindow();
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
