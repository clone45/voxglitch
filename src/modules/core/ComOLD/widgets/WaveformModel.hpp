struct WaveformModel
{
    Sample *sample;
    bool visible = false;

    // Vertical line to show playback position
    bool draw_position_indicator = true;
    float playback_percentage = 0.0;

    // Overlay to highlight a section of the waveform
    bool highlight_section = false;
    float highlight_section_x = 0.0;
    float highlight_section_width = 0.0;
};