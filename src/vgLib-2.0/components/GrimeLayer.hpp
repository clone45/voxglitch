/*
struct GrimeLayer : VoxglitchWidget
{

  bool grimey = true;

  float offset_x_px;
  float offset_y_px;
  float width_px;
  float height_px;

  GrimeLayer(float offset_x, float offset_y, float width, float height)
  {
    this->offset_x_px = offset_x;
    this->offset_y_px = offset_y;
    this->width_px = width;
    this->height_px = height;

    box.size = Vec(width, height);
  }

  void drawLayer(const DrawArgs& args, int layer) override
  {
  	if (grimey && (layer == 1))
    {
      const auto vg = args.vg;

      // Save the drawing context to restore later
      nvgSave(vg);
      std::shared_ptr<Image> img = APP->window->loadImage(asset::plugin(pluginInstance, "res/components/dust_layer_3.png"));

      // Get the image size and store it in the width and height variables
      int png_width_px, png_height_px;
      nvgImageSize(vg, img->handle, &png_width_px, &png_height_px);

      // Rescale the .png file
      float scale =  mm2px(128.5) / png_height_px;

      // Paint the .png background
      NVGpaint paint = nvgImagePattern(vg, offset_x_px, offset_y_px, (float) png_width_px * scale, (float) png_height_px * scale, 0.0, img->handle, .25);
      nvgBeginPath(vg);
      nvgRect(vg, 0.0, 0.0, width_px, height_px);
      nvgFillPaint(vg, paint);
      nvgFill(vg);

      nvgRestore(vg);
    }
  }

};
*/
