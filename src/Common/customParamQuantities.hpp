#pragma once

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
        std::string note_name = NOTES::getNoteName(note_selection, octave_selection);

        return note_name;
    }
};