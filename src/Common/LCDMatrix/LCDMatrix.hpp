#include <vector>
#include <unordered_map>
#include <string>

// Define a simple 5x7 monospace font
const int FONT_WIDTH = 5;
const int FONT_HEIGHT = 7;

std::unordered_map<char, std::vector<std::vector<bool>>> font = {
    // Each character is a 5x7 grid (width x height) of on/off pixels
    // Here's an example for a few characters:
    {'A', {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1}
    }},
    // ... Add other characters ...
};

class LCDMatrix {
private:
    int width, height;
    std::vector<std::vector<bool>> pixels;

public:
    LCDMatrix(int w, int h) : width(w), height(h), pixels(h, std::vector<bool>(w, false)) {}

    void setPixel(int x, int y, bool on) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixels[y][x] = on;
        }
    }

    bool getPixel(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return pixels[y][x];
        }
        return false;
    }

    void drawText(int x, int y, const std::string& text) {
        for (char c : text) {
            if (font.find(c) != font.end()) {
                const auto& charMap = font[c];
                for (int i = 0; i < FONT_HEIGHT; ++i) {
                    for (int j = 0; j < FONT_WIDTH; ++j) {
                        if (charMap[i][j]) {
                            setPixel(x + j, y + i, true);
                        }
                    }
                }
            }
            x += FONT_WIDTH;
        }
    }
};
