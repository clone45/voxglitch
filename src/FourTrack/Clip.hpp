#pragma once
#include <rack.hpp>

struct Clip 
{
    // Model properties
    float sample_offset_start = 0.0f;
    float sample_offset_end = 44100.0f; // around 1 second default
    float track_position = 0.0f; // starting location in the track
    
    Sample *sample;

    // Constructor
    Clip() {}


    void setSample(Sample *sample) 
    {
        this->sample = sample;
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
        // float clip_width = sample_offset_end - sample_offset_start;
        float clip_width_px = 20;

        // Draw the clip
        nvgBeginPath(vg);
        nvgRect(vg, clip_start_x, track_panel_y, clip_width_px, track_height);
        nvgFillColor(vg, nvgRGB(0xFF, 0x00, 0x00)); // Bright red
        nvgFill(vg);

        // Add more drawing logic here (e.g., waveform, clip name)

        nvgRestore(vg);
    }
};