#pragma once

struct ClockDividerRateParamQuantity : ParamQuantity 
{
    std::string getDisplayValueString() override 
    {
        return getClockDividerRateName(getValue());
    }

    static std::string getClockDividerRateName(int value)
    {
        std::string division_string = "";

        switch (value)
        {
            case -7:
                division_string = "/8";
                break;
            case -6:
                division_string = "/7";
                break;
            case -5:
                division_string = "/6";
                break;
            case -4:
                division_string = "/5";
                break;
            case -3:
                division_string = "/4";
                break;
            case -2:    
                division_string = "/3";
                break;
            case -1:
                division_string = "/2";
                break;
            case 0:
                division_string = "x1";
                break;
            case 1:
                division_string = "x2";
                break;
            case 2:
                division_string = "x3";
                break;
            case 3:
                division_string = "x4";
                break;
            case 4:
                division_string = "x5";
                break;
            case 5:
                division_string = "x6";
                break;
            case 6:
                division_string = "x7";
                break;
            case 7:
                division_string = "x8";
                break;
            default:
                division_string = "ERROR";
                break;
        }

        return division_string;
    }
};

static ClockDividerRateParamQuantity* createClockDividerRateParamQuantity(Module* module, int paramId, float minValue, float maxValue, float defaultValue, float displayMultiplier, bool snapEnabled, const std::string& name) {
    ClockDividerRateParamQuantity* pq = new ClockDividerRateParamQuantity();
    pq->module = module;
    pq->paramId = paramId;
    pq->minValue = minValue;
    pq->maxValue = maxValue;
    pq->defaultValue = defaultValue;
    pq->displayMultiplier = displayMultiplier;
    pq->snapEnabled = snapEnabled;
    pq->name = name;
    return pq;
}

struct ShapeParamQuantity : ParamQuantity 
{
    std::string getDisplayValueString() override 
    {
        return getShapeName(getValue());
    }

    static std::string getShapeName(int value)
    {
        std::string tooltip_string = "";

        switch (value)
        {
            case 0:
                tooltip_string = "Manual";
                break;
            case 1:
                tooltip_string = "Forward";
                break;
            case 2:
                tooltip_string = "Reverse";
                break;
            case 3:
                tooltip_string = "Pendulum";
                break;
            case 4:
                tooltip_string = "Random";
                break;
            case 5:    
                tooltip_string = "Manual x2";
                break;
            case 6:
                tooltip_string = "Forward x2";
                break;
            case 7:
                tooltip_string = "Reverse x2";
                break;
            case 8:
                tooltip_string = "Pendulum x2";
                break;
            case 9:
                tooltip_string = "Random x2";
                break;
            default:
                tooltip_string = "ERROR";
                break;
        }

        return tooltip_string;
    }
};

static ShapeParamQuantity* createShapeParamQuantity(Module* module, int paramId, float minValue, float maxValue, float defaultValue, float displayMultiplier, bool snapEnabled, const std::string& name) {
    ShapeParamQuantity* pq = new ShapeParamQuantity();
    pq->module = module;
    pq->paramId = paramId;
    pq->minValue = minValue;
    pq->maxValue = maxValue;
    pq->defaultValue = defaultValue;
    pq->displayMultiplier = displayMultiplier;
    pq->snapEnabled = snapEnabled;
    pq->name = name;
    return pq;
}


struct OctaveParamQuantity : ParamQuantity {
    std::string getDisplayValueString() override {
        if (getValue() == -1.0f) {
            return "All";
        }
        return ParamQuantity::getDisplayValueString();
    }
};

struct NoteParamQuantity : ParamQuantity {
    std::string getDisplayValueString() override {
        float value = getValue();
        int note_selection = (int)roundf(value);
        int octave_selection = -1;
        std::string note_name = NOTES::getNoteName(note_selection, octave_selection, false);

        return note_name;
    }
};