struct VoxglitchWidget : TransparentWidget
{

  // brightness = 1; // full brightness
  // brightness = 0; // complete darkness
  NVGcolor brightness(NVGcolor rgba, float brightness)
  {
    NVGcolor new_color;

    new_color.r = brightness * (float(rgba.r));
    new_color.g = brightness * (float(rgba.g));
    new_color.b = brightness * (float(rgba.b));
    new_color.a = rgba.a;

    return(new_color);
  }

};
