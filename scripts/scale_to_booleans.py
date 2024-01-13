import re

def scale_to_booleans(scale_string):
    # Map of notes to their positions in a 12-note octave starting with C
    note_map = {
        'C': 0,
        'C#': 1, 'Db': 1,
        'D': 2,
        'D#': 3, 'Eb': 3,
        'E': 4,
        'F': 5,
        'F#': 6, 'Gb': 6,
        'G': 7,
        'G#': 8, 'Ab': 8,
        'A': 9,
        'A#': 10, 'Bb': 10,
        'B': 11
    }

    # Initialize an array of 12 booleans, all set to False
    notes = [False] * 12

    # Split the input string into individual notes
    scale_notes = re.split(r'[,\s]\s*', scale_string)

    # Process each note
    for note in scale_notes:
        if note in note_map:
            notes[note_map[note]] = True

    # Convert boolean values to 'true'/'false' strings
    bool_strings = ['true' if note else 'false' for note in notes]

    return '{ ' + ', '.join(bool_strings) + ' }'

# Example usage
scale_string = input("Enter a scale string (e.g., 'C D Eb E G A'): ")
print(scale_to_booleans(scale_string))
