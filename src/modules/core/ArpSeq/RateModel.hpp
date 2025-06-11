struct RateModel
{
    int rate_index = 0;
    int MAX_RATE = 8;

    RateModel()
    {
    }

    int update(float cv_input, float knob_value, float attenuator_value, float attenuverter_range)
    {
        // knob_value ranges from -5 to +5
        // cv_input ranges from -10 to +10 and is multiplied by the attenuator_value

        // float rate = knob_value + (cv_input * attenuator_value);

        float rate = (knob_value / MAX_RATE) + ((cv_input / attenuverter_range) * attenuator_value);

        // rate_index should be an integer from -8 to +8
        // rate_index = static_cast<int>(std::round(rate));

        rate_index = (int) (rate * MAX_RATE);

        // Adjust for the skip over -1
        if (rate_index <= -1) 
        {
            rate_index--;
        }

        // Clamp the rate_index to the range -8 to +7
        rate_index = clamp(rate_index, MAX_RATE * -1, MAX_RATE - 1);

        return rate_index;
    }

    int getRateIndex()
    {
        return rate_index;
    }

    std::string toString()
    {
        // The string should be up to 3
        // Positive rate_indexes should be displayed as x1, x2, x3, etc.
        // Negative rate_indexes should be displayed as /1, /2, /3, etc.

        std::string rate_string = "";

        if (rate_index > 0)
        {
            rate_string = "x" + std::to_string(rate_index + 1);
        }
        else if (rate_index < 0)
        {
            rate_string = "/" + std::to_string(std::abs(rate_index));
        }
        else
        {
            rate_string = "x1";
        }

        return rate_string;
    }
};