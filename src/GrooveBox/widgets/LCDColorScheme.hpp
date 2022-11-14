struct LCDColorScheme
{
    unsigned int selected_color_scheme = 0;

    NVGcolor color_scheme[5][6] = {
        // Legacy Red
        {
            nvgRGB(50,0,0),               // panel background
            nvgRGBA(128, 35, 35, 140),    // dark blocks
            nvgRGBA(200, 50, 50, 140),  // light blocks
            nvgRGBA(255, 0, 0, 50),       // normal highlight overlay
            nvgRGBA(255, 30, 30, 100),       // strong highlight overlay
            nvgRGBA(255, 180, 180, 255)       // text and icons

        },
        // Soft Yellow
        {
            nvgRGB(40, 25, 0),             // panel background
            nvgRGBA(110, 90, 0, 140),    // dark blocks
            nvgRGBA(160, 140, 0, 140),  // light blocks
            nvgRGBA(220, 200, 0, 50),     // normal highlight overlay
            nvgRGBA(220, 200, 0, 100),     // strong highlight overlay
            nvgRGBA(220, 220, 180, 255)     // text and icons
        },
        // Cool White
        {
            nvgRGB(10, 10, 10),             // panel background
            nvgRGBA(70, 70, 70, 140),       // dark blocks
            nvgRGBA(135, 135, 135, 140),      // light blocks
            nvgRGBA(255, 255, 255, 50),   // normal highlight overlay
            nvgRGBA(255, 255, 255, 100),   // strong highlight overlay
            nvgRGBA(255, 255, 255, 255)   // text and icons
        },
        // Ice Blue
        {
            nvgRGB(0, 30, 45),             // panel bqackground
            nvgRGBA(35, 95, 125, 140),       // dark blocks
            nvgRGBA(50, 150, 210, 140),      // light blocks
            nvgRGBA(0, 180, 255, 50),   // normal highlight overlay
            nvgRGBA(0, 180, 255, 100),   // strong highlight overlay
            nvgRGBA(180, 230, 255, 255)   // text and icons
        },
        // Data Green
        {
            nvgRGB(0, 50, 0),               // panel background
            nvgRGBA(35, 128, 35, 140),    // dark blocks
            nvgRGBA(50, 185, 50, 140),   // light blocks
            nvgRGBA(0, 255, 0, 50),   // normal highlight overlay
            nvgRGBA(0, 255, 0, 100),   // strong highlight overlay
            nvgRGBA(180, 255, 180, 255)   // text and icons
        }
    };

    NVGcolor getBackgroundColor()
    {
        return(color_scheme[selected_color_scheme][0]);
    }

    NVGcolor getDarkColor()
    {
        return(color_scheme[selected_color_scheme][1]);
    }

    NVGcolor getLightColor()
    {
        return(color_scheme[selected_color_scheme][2]);
    }

    NVGcolor getHighlightOverlay()
    {
        return(color_scheme[selected_color_scheme][3]);
    }

    NVGcolor getStrongHighlightOverlay()
    {
        return(color_scheme[selected_color_scheme][4]);
    }

    NVGcolor getTextColor()
    {
        return(color_scheme[selected_color_scheme][5]);
    }
};
