struct GrimeLayer : VoxglitchWidget
{

  bool grimey = true;

  GrimeLayer()
  {
  }

  void drawLayer(const DrawArgs& args, int layer) override
  {
  	if (grimey && (layer == 1))
    {
      const auto vg = args.vg;

      // Save the drawing context to restore later
      nvgSave(vg);

      //
      // draw dirt overlay
      //

      std::shared_ptr<Image> img = APP->window->loadImage(asset::plugin(pluginInstance, "res/digital_sequencer/dust_layer.png"));

      int temp_width, temp_height;

      // Get the image size and store it in the width and height variables
      nvgImageSize(vg, img->handle, &temp_width, &temp_height);

      // Set the bounding box of the widget
      box.size = Vec(500.5, 231.6);

      // Paint the .png background
      NVGpaint paint = nvgImagePattern(vg, 0.0, 0.0, box.size.x, box.size.y, 0.0, img->handle, .5);
      nvgBeginPath(vg);
      nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);
      nvgFillPaint(vg, paint);
      nvgFill(vg);

      nvgRestore(vg);
    }
  }

};
