struct LCDColorScheme
{
    unsigned int selected_color_scheme = 0;

    NVGcolor color_scheme[3][5] = {
        // Vampire Red
        {
            nvgRGB(60,110,100),
            nvgRGBA(146, 42, 43, 140),    // dark color
            nvgRGBA(245, 141, 138, 140),  // light color
            nvgRGBA(255, 255, 255, 50),   // normal highlight overlay
            nvgRGBA(255, 255, 255, 100),   // strong highlight overlay
            
        },
        // Theme #2
        {
            nvgRGB(60,80,120),
            nvgRGBA(46, 142, 43, 140),    // dark color
            nvgRGBA(145, 141, 138, 140),  // light color
            nvgRGBA(255, 255, 255, 50),   // normal highlight overlay
            nvgRGBA(255, 255, 255, 100)   // strong highlight overlay
        },
        // Theme #3
        {
            nvgRGB(60,0,1),
            nvgRGBA(146, 42, 43, 140),    // dark color
            nvgRGBA(245, 141, 138, 140),  // light color
            nvgRGBA(255, 255, 255, 50),   // normal highlight overlay
            nvgRGBA(255, 255, 255, 100)   // strong highlight overlay
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
};