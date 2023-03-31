#include <fstream>
#include <string>

constexpr int NUM_WAVETABLES = 28;
constexpr int WAVETABLE_SIZE = 512;

// Read the wavetables from a file
bool readWavetables(const std::string& filename, float* data) {
    std::ifstream file(filename);
    if (file.is_open()) {
        for (int i = 0; i < NUM_WAVETABLES; ++i) {
            for (int j = 0; j < WAVETABLE_SIZE; ++j) {
                std::string value;
                std::getline(file, value, ','); // Ignore the comma character
                file >> data[i * WAVETABLE_SIZE + j];
                // DEBUG(std::to_string(data[i * WAVETABLE_SIZE + j]).c_str());
            }
        }
        file.close();

        return(true);
    }
    else
    {
        DEBUG("Unable to open wavetable file: ");
        DEBUG(filename.c_str());

        return(false);
    }

}