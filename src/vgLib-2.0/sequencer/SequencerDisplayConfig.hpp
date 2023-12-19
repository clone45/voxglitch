struct SequencerDisplayConfig 
{
    bool visible = false;
    
    float x = 0.0;
    float y = 0.0;
    float y_axis = 0.0;

    float overlay_width = 359.0;
    float overlay_height = 230.0;
    float draw_area_width = 359.0;
    float draw_area_height = 230.0;    

    float bar_width = 8.0;
    float bar_height = 0.0;
    float bar_horizontal_padding = 0.8;

    int max_sequencer_steps = 16;
};