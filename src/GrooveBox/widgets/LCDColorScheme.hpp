struct LCDColorScheme
{
    unsigned int selected_color_scheme = 0;

    NVGcolor color_scheme[1][2] = {
        // Vampire Red
        {
            nvgRGBA(146, 42, 43, 140),   // dark color
            nvgRGBA(245, 141, 138, 140)  // light color
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

};