import wave
import csv

def process_wav_file(wav_file):
    output_file = wav_file[:-4] + ".csv"
    cutoff_min = -5.0
    cutoff_max = 5.0

    with wave.open(wav_file, 'rb') as wf:
        num_channels = wf.getnchannels()
        sample_width = wf.getsampwidth()
        sample_rate = wf.getframerate()
        num_frames = wf.getnframes()

        frames = wf.readframes(num_frames)

    # Calculate the maximum value based on the sample width
    if sample_width == 1:
        max_value = 255
    elif sample_width == 2:
        max_value = 32767
    else:
        raise ValueError("Unsupported sample width")

    # Convert the frames to a list of floating-point values
    samples = []
    for i in range(0, len(frames), sample_width):
        sample = int.from_bytes(frames[i:i + sample_width], byteorder='little', signed=True)
        sample /= max_value
        samples.append(sample)

    # Filter the samples within the specified range
    filtered_samples = [sample for sample in samples if cutoff_min <= sample <= cutoff_max]

    # Write the filtered samples to a CSV file
    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['Sample'])
        writer.writerows([[sample] for sample in filtered_samples])

    print(f"Filtered samples have been saved to {output_file}")


# Example usage
wav_path = input("Enter the path to the .wav file: ")
process_wav_file(wav_path)
