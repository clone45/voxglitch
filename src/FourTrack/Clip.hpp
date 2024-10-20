#pragma once
#include <rack.hpp>
#include <vector>

struct Clip 
{
    // Model properties
    unsigned int samples_per_average = 1000; // Number of samples to average per chunk
    unsigned int sample_window_start = 0; // Start portion of the sample to display
    unsigned int sample_window_end = 0; // End portion of the sample to display
    float track_position = 0.0f; // Starting location in the track
    std::vector<float> averages; // Store waveform averages
    float max_average = 0.0;
    float clip_width_px = 60.0f; // Hardcoded width for testing
    float samples_per_pixel = 1000.0f; // Number of samples per pixel for consistent density

    Sample *sample;

    // Constructor
    Clip() {}

    void setSample(Sample *sample) 
    {
        this->sample = sample;
        if (sample && sample->isLoaded()) 
        {
            sample_window_start = 0.0f; // Start at the beginning of the sample
            updateSampleWindowEnd(); // Set end dynamically based on the clip width and density
        }
        computeAverages(); // Compute averages when sample is set
        normalizeAverages(); // Normalize them after computation
    }

    // Update Sample Window End based on clip width and density
    void updateSampleWindowEnd()
    {
        if (sample)
        {
            unsigned int calculated_window_end = sample_window_start + static_cast<unsigned int>(clip_width_px * samples_per_pixel);
            unsigned int sample_size = sample->size();
            sample_window_end = rack::math::clamp((int) calculated_window_end, (int) sample_window_start, (int) sample_size);
        }
    }

    // New Setter for Sample Window End
    void setSampleWindow(unsigned int start, unsigned int end)
    {
        if (sample) {
            // Clamp window to the valid sample size
            sample_window_start = rack::math::clamp((int) start, 0, (int) sample->size());
            updateSampleWindowEnd(); // Recalculate sample_window_end based on new start and clip width
            computeAverages(); // Recalculate waveform averages after adjusting window
            normalizeAverages(); // Normalize them after recomputation
        }
    }

    // Draw method
    // ============================================================================================================
    // This method is called by the TrackWidget to draw the clip
    //
    // vg is the NanoVG context
    // track_panel_x, track_panel_y, and track_height are the position and dimensions of the track on the panel

    void draw(NVGcontext *vg, float track_panel_x, float track_panel_y, float track_height) 
    {
        nvgSave(vg);

        // Calculate clip position and dimensions
        float clip_start_x = track_panel_x + track_position;
        float clip_width_px = 60.0f; // Hardcoded width for testing, ignoring zoom for now

        // Draw the clip rectangle
        nvgBeginPath(vg);
        nvgRect(vg, clip_start_x, track_panel_y, clip_width_px, track_height);
        nvgFillColor(vg, nvgRGB(0xFF, 0x00, 0x00)); // Bright red
        nvgFill(vg);

        // Draw the waveform inside the clip
        drawWaveform(vg, clip_start_x, track_panel_y, clip_width_px, track_height);

        nvgRestore(vg);
    }

    // Compute waveform averages for the clip
    /*
    void computeAverages()
    {
        if (!sample || !sample->isLoaded())
            return;

        float sample_size = sample_window_end - sample_window_start;
        float chunk_size = sample_size / 200.0f; // Hardcoded width for testing
        averages.clear();
        averages.reserve(200); // Predefine size to match the clip width

        for (unsigned int x = 0; x < 200; x++) // Hardcoded width as the number of averages
        {
            float left_sum = 0.0;
            float right_sum = 0.0;
            unsigned int count = 0;

            unsigned int chunk_start = static_cast<unsigned int>(sample_window_start + (x * chunk_size));
            unsigned int chunk_end = chunk_start + static_cast<unsigned int>(chunk_size);

            for (unsigned int i = chunk_start; i < chunk_end && i < sample->size(); i++)
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
    } */

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
            average = clamp((1.0f / max_average) * average, 0.0f, 1.0f);
        }
    }

    // Draw the waveform using the computed averages
    void drawWaveform(NVGcontext *vg, float clip_start_x, float track_panel_y, float clip_width_px, float track_height)
    {
        nvgSave(vg);
        nvgBeginPath(vg);

        // Determine the start and end indices in the averages vector that correspond to the visible sample window
        unsigned int visible_average_start_index = sample_window_start / samples_per_average;
        unsigned int visible_average_end_index = sample_window_end / samples_per_average;

        // Clamp the indices to ensure they are within bounds of the averages vector
        visible_average_start_index = std::min(visible_average_start_index, static_cast<unsigned int>(averages.size() - 1));
        visible_average_end_index = std::min(visible_average_end_index, static_cast<unsigned int>(averages.size()));

        // Ensure the indices are valid
        if (visible_average_start_index >= visible_average_end_index)
        {
            nvgRestore(vg); // Nothing to draw, just return
            return;
        }

        // Calculate the number of visible averages
        unsigned int num_visible_averages = visible_average_end_index - visible_average_start_index;
        float line_width = clip_width_px / static_cast<float>(num_visible_averages);

        // Draw the visible portion of the waveform
        for (unsigned int x = visible_average_start_index; x < visible_average_end_index; x++)
        {
            float x_position = clip_start_x + ((x - visible_average_start_index) * line_width);
            float average_height = averages[x] * track_height;
            float y_position = track_panel_y + (track_height - average_height) / 2.0f;

            nvgRect(vg, x_position, y_position, line_width, average_height);
        }

        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200)); // Light grey color for the waveform
        nvgFill(vg);
        nvgRestore(vg);
    }
};
