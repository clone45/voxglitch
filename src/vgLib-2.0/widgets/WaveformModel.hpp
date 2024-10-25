struct WaveformModel
{
    Sample *sample;
    bool visible = false;

    std::vector<float> marker_positions;

    // Vertical line to show playback position
    bool draw_position_indicator = true;
    float playback_percentage = 0.0;

    // Overlay to highlight a section of the waveform
    bool highlight_section = false;
    float highlight_section_x = 0.0;
    float highlight_section_width = 0.0;

    void addMarker(float sample_position)
    {
        // Ensure the marker position is within the valid range
        if (sample_position >= 0 && sample_position <= sample->size())
        {
            marker_positions.push_back(sample_position);
            DEBUG("Added marker at position %f", sample_position);
        }
    }

    void clearMarkers()
    {
        marker_positions.clear();
    }

};