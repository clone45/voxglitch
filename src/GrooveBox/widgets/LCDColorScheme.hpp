struct LCDColorScheme
{
    unsigned int selected_color_scheme = 0;

    NVGcolor color_scheme[1][3] = {
        // Vampire Red
        {
            nvgRGBA(146, 42, 43, 140),    // dark color
            nvgRGBA(245, 141, 138, 140),  // light color
            nvgRGBA(255, 255, 255, 50)    // highlight overlay
        }
    };

    NVGcolor getDarkColor()
    {
        return(color_scheme[selected_color_scheme][0]);
    }

    NVGcolor getLightColor()
    {
        return(color_scheme[selected_color_scheme][1]);
    }

    NVGcolor getHighlightOverlay()
    {
        return(color_scheme[selected_color_scheme][2]);
    }

};