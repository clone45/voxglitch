# Open the input file
with open('wavetables.txt', 'r') as f:
    # Read the lines into a list
    lines = f.readlines()

# Initialize an empty list for the modified values
modified_lines = []

# Iterate over each line in the input file
for line in lines:
    # Split the line into individual values
    values = line.strip().split(',')

    # Convert each value and add it to the modified list
    modified_values = [(float(v) / 25.6) - 5.0 for v in values]
    modified_lines.append(','.join(str(v) for v in modified_values))

# Open the output file
with open('converted_wavetables.txt', 'w') as f:
    # Write the modified lines to the output file
    f.write('\n'.join(modified_lines))