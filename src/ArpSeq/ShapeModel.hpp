struct ShapeModel
{
    int shape_mode = 0;
    bool note_repeat_mode = false;

    std::string playback_mode_strings[NUMBER_OF_PLAYBACK_MODES] = {
        "MAN",
        "FWD",
        "REV",
        "PEN",
        "RND",
        "MN2",
        "FW2",
        "RV2",
        "PN2",
        "RN2"};

    int update(float cv_input, float knob_value, float attenuator_value, float attenuverter_range)
    {
        float shape = (knob_value / NUMBER_OF_PLAYBACK_MODES) + ((cv_input / attenuverter_range) * attenuator_value);

        shape_mode = (int) (shape * NUMBER_OF_PLAYBACK_MODES);
        shape_mode = clamp(shape_mode, 0, NUMBER_OF_PLAYBACK_MODES - 1);

        if(shape_mode >= (NUMBER_OF_PLAYBACK_MODES / 2.0))
        {
            note_repeat_mode = true;
        }
        else
        {
            note_repeat_mode = false;
        }

        return shape_mode;
    }

    int getShapeMode()
    {
        return shape_mode;
    }

    bool isNoteRepeat()
    {
        return note_repeat_mode;
    }

    std::string toString()
    {
        if(shape_mode >= NUMBER_OF_PLAYBACK_MODES) return "ERR";

        std::string mode_string = playback_mode_strings[shape_mode];

        return mode_string;
    }
};