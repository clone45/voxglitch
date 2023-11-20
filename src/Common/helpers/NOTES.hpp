class NOTES {
public:

    static std::string getNoteName(int note_selection, int octave_selection)
    {
        std::string note_name = "A4";

        switch (note_selection)
        {
            case 0:
                note_name = "C";
                break;
            case 1:
                note_name = "C#";
                break;
            case 2:
                note_name = "D";
                break;
            case 3:
                note_name = "D#";
                break;
            case 4:
                note_name = "E";
                break;
            case 5:
                note_name = "F";
                break;
            case 6:
                note_name = "F#";
                break;
            case 7:
                note_name = "G";
                break;
            case 8:
                note_name = "G#";
                break;
            case 9:
                note_name = "A";
                break;
            case 10:
                note_name = "A#";
                break;
            case 11:
                note_name = "B";
                break;
            default:
                note_name = "A4";
                break;
        }

        if(octave_selection >= 0) 
        {
            note_name += std::to_string(octave_selection);
        }

        return note_name;
    }
};