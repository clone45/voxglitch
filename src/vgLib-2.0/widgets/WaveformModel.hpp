struct WaveformModel
{
    Sample *sample;
    bool visible = false;
    std::vector<float> marker_positions;
    bool *lock_interactions = nullptr;

    // Display properties
    bool draw_position_indicator = true;
    float playback_percentage = 0.0f;  // Keep for compatibility. Remove after updating autobreak.
    unsigned int playhead_position = 0;  // Changed from percentage to position
    bool scrubber_dragging = false;
   
    // Optional section highlighting
    bool highlight_section = false;
    unsigned int highlight_section_start = 0;  // Changed from x to sample position
    unsigned int highlight_section_length = 0;  // Changed from width to sample count
   
    unsigned int highlight_section_x = 0;  // Temporary, for backwards compatibility
    unsigned int highlight_section_width = 0;  // Temporary, for backwards compatibility

    // Callbacks
    std::function<void(unsigned int)> onPlayheadChanged = nullptr;
    std::function<void(unsigned int)> onDragPlayhead = nullptr;
   

    // Marker management
    void addMarker(unsigned int sample_position) {
        if (sample && sample_position < sample->size()) {
            marker_positions.push_back(sample_position);
        }
    }
   
    void clearMarkers() {
        marker_positions.clear();
    }
   
    void updatePlayheadPosition(unsigned int position) {
        if (position != playhead_position) {
            playhead_position = position;
            playback_percentage = sample ? 
                static_cast<float>(position) / static_cast<float>(sample->size()) : 0.0f;
            if (onPlayheadChanged) {
                onPlayheadChanged(position);
            }
        }
    }

    // Register observers
    void registerDragPlayheadObserver(std::function<void(unsigned int)> callback) {
        onDragPlayhead = callback;
    }

    void setLockInteractions(bool *lock_interactions_ptr)
    {
        lock_interactions = lock_interactions_ptr;
    }
};