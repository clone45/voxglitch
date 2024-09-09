#include <cmath>
#include <vector>

struct Quantizer 
{

    public:

    enum Note {
        C, CSharp, D, DSharp, E, F, FSharp, G, GSharp, A, ASharp, B
    };

    enum ScaleType {
        CHROMATIC_SCALE = 0,  // All notes
        MAJOR_SCALE,
        MINOR_SCALE,
        PENTATONIC_SCALE,
        DORIAN_SCALE,
        PHRYGIAN_SCALE,
        LYDIAN_SCALE,
        MIXOLYDIAN_SCALE,
        HARMONIC_MINOR_SCALE,
        MELODIC_MINOR_SCALE,
        BLUES_SCALE,
        WHOLE_TONE_SCALE,
        DIMINISHED_SCALE
    };

    // Note rootNote = C;  // Default root note

    static const unsigned int NUM_SCALES = 13;

    // Define some scales as arrays. true means the note is in the scale.
    // The indices correspond to semitones in an octave from 0 to 11
    static const bool chromaticScale[12];
    static const bool majorScale[12];
    static const bool minorScale[12];
    static const bool pentatonicScale[12];
    static const bool dorianScale[12];
    static const bool phrygianScale[12];
    static const bool lydianScale[12];
    static const bool mixolydianScale[12];
    static const bool harmonicMinorScale[12];
    static const bool melodicMinorScale[12];
    static const bool bluesScale[12];
    static const bool wholeToneScale[12];
    static const bool diminishedScale[12];
    
    // Array of pointers to scales for easy selection
    static const bool* scales[NUM_SCALES];
    
    unsigned int selectedScale = 0; // default to first scale (major)
    unsigned int rootNote = 0; // default to C
    
    Quantizer() {
        // Initialize here if needed
    }
    
    float quantize(float pitch) 
    {
        int octave = static_cast<int>(std::floor(pitch));
        int semitone = static_cast<int>(std::round((pitch - octave) * 12.0));

        if (semitone < 0) {
            semitone += 12;
            octave -= 1;
        }

        int originalSemitone = semitone;
        int lowerSemitone = semitone;
        int upperSemitone = semitone;

        // Find the closest lower note in the scale
        while (!scales[selectedScale][(lowerSemitone + rootNote) % 12]) {
            lowerSemitone--;
        }

        // Find the closest upper note in the scale
        while (!scales[selectedScale][(upperSemitone + rootNote) % 12]) {
            upperSemitone++;
        }

        // Choose the closest note between the lower and upper
        if (std::abs(upperSemitone - originalSemitone) < std::abs(lowerSemitone - originalSemitone)) {
            semitone = upperSemitone;
        } else {
            semitone = lowerSemitone;
        }

        // Reassemble the pitch from the octave and quantized semitone
        return octave + static_cast<float>(semitone) / 12.0;
    }

    std::string getScaleName(unsigned int scale_index) {
        switch (scale_index) {
            case CHROMATIC_SCALE:
                return "Chromatic";
            case MAJOR_SCALE:
                return "Major";
            case MINOR_SCALE:
                return "Minor";
            case PENTATONIC_SCALE:
                return "Pentatonic";
            case DORIAN_SCALE:
                return "Dorian";
            case PHRYGIAN_SCALE:
                return "Phrygian";
            case LYDIAN_SCALE:
                return "Lydian";
            case MIXOLYDIAN_SCALE:
                return "Mixolydian";
            case HARMONIC_MINOR_SCALE:
                return "Harmonic Minor";
            case MELODIC_MINOR_SCALE:
                return "Melodic Minor";
            case BLUES_SCALE:
                return "Blues";
            case WHOLE_TONE_SCALE:
                return "Whole Tone";
            case DIMINISHED_SCALE:
                return "Diminished";
            default:
                return "Unknown";
        }
    }

    std::vector<std::string> getScaleNames() {
        std::vector<std::string> scale_names;
        for (unsigned int i = 0; i < NUM_SCALES; i++) {
            scale_names.push_back(getScaleName(i));
        }
        return scale_names;
    }
    
    std::string getNoteName(unsigned int note_index) {
        switch (note_index) {
            case C:
                return "C";
            case CSharp:
                return "C#";
            case D:
                return "D";
            case DSharp:
                return "D#";
            case E:
                return "E";
            case F:
                return "F";
            case FSharp:
                return "F#";
            case G:
                return "G";
            case GSharp:
                return "G#";
            case A:
                return "A";
            case ASharp:
                return "A#";
            case B:
                return "B";
            default:
                return "Unknown";
        }
    }

    std::vector<std::string> getNoteNames() {
        std::vector<std::string> note_names;
        for (unsigned int i = 0; i < 12; i++) {
            note_names.push_back(getNoteName(i));
        }
        return note_names;
    }

    // Function to set the scale
    void setScale(unsigned int scaleIndex) {
        if (scaleIndex < NUM_SCALES) {
            selectedScale = scaleIndex;
        }
    }

    void setRoot(unsigned int newRoot) {
        rootNote = newRoot;
    }
};

// Definitions of scale arrays
const bool Quantizer::chromaticScale[12] = { true, true, true, true, true, true, true, true, true, true, true, true };
const bool Quantizer::majorScale[12] = { true, false, true, false, true, true, false, true, false, true, false, true };
const bool Quantizer::minorScale[12] = { true, false, true, true, false, true, false, true, true, false, true, false };
const bool Quantizer::pentatonicScale[12] = { true, false, true, false, false, true, false, true, false, false, true, false };
const bool Quantizer::dorianScale[12] = { true, false, true, true, false, true, false, true, false, true, true, false };
const bool Quantizer::phrygianScale[12] = { true, true, false, true, false, true, false, true, true, false, true, false };
const bool Quantizer::lydianScale[12] = { true, false, true, false, true, false, true, true, false, true, false, true };
const bool Quantizer::mixolydianScale[12] = { true, false, true, false, true, true, false, true, false, true, true, false };
const bool Quantizer::harmonicMinorScale[12] = { true, false, true, true, false, true, false, true, true, false, false, true };
const bool Quantizer::melodicMinorScale[12] = { true, false, true, true, false, true, false, true, false, true, false, true };
const bool Quantizer::bluesScale[12] = { true, false, true, true, true, true, false, true, false, true, true, false };
const bool Quantizer::wholeToneScale[12] = { true, false, true, false, true, false, true, false, true, false, true, false };
const bool Quantizer::diminishedScale[12] = { true, true, false, true, true, false, true, true, false, true, true, false };


// Definition of scales array
const bool* Quantizer::scales[NUM_SCALES] = { 
    chromaticScale,
    majorScale, 
    minorScale, 
    pentatonicScale,
    dorianScale,
    phrygianScale,
    lydianScale,
    mixolydianScale,
    harmonicMinorScale,
    melodicMinorScale,
    bluesScale,
    wholeToneScale,
    diminishedScale
};

