struct PanelBackground : TransparentWidget
{
  std::string image_file_path;
  float module_width_px;
  float module_height_px;
  float src_panel_offset_x_px = 0.0;
  float src_panel_offset_y_px = 0.0;
  float alpha = 1.0;
  float zoom = 0.1;
  bool image_exists = true;

  PanelBackground(std::string image_file_path, float module_width_mm, float module_height_mm, float src_panel_offset_x_px, float src_panel_offset_y_px, float zoom = 0.1)
  {
    this->image_file_path = image_file_path;
    this->module_width_px = mm2px(module_width_mm);
    this->module_height_px = mm2px(module_height_mm);
    this->src_panel_offset_x_px = src_panel_offset_x_px;
    this->src_panel_offset_y_px = src_panel_offset_y_px;
    this->zoom = zoom;

    this->image_exists = rack::system::exists(asset::plugin(pluginInstance, this->image_file_path));
  }

  void draw(const DrawArgs &args) override
  {
    if(! this->image_exists) return;

    std::shared_ptr<Image> img = APP->window->loadImage(asset::plugin(pluginInstance, this->image_file_path));

  	int background_width_px, background_height_px;

  	nvgImageSize(args.vg, img->handle, &background_width_px, &background_height_px);

    box.size = Vec(module_width_px, module_height_px);

    // Creates and returns an image pattern. Parameters (ox,oy) specify the left-top location of the image pattern,
    // (ex,ey) the size of one image, angle rotation around the top-left corner, image is handle to the image to render.
    // The gradient is transformed by the current transform when it is passed to nvgFillPaint() or nvgStrokePaint().
    //
    //NVGpaint nvgImagePattern(NVGcontext* ctx, float ox, float oy, float ex, float ey,
    //						 float angle, int image, float alpha);

    float aspect_ratio_width = background_width_px / module_width_px;
    float aspect_ratio_height = background_height_px / module_height_px;

  	NVGpaint nvg_paint = nvgImagePattern(args.vg, src_panel_offset_x_px, src_panel_offset_y_px, module_width_px * aspect_ratio_width * zoom, module_height_px * aspect_ratio_height * zoom, 0.0, img->handle, this->alpha);
  	nvgBeginPath(args.vg);
  	nvgRect(args.vg, 0.0, 0.0, module_width_px, module_height_px);
  	nvgFillPaint(args.vg, nvg_paint);
  	nvgFill(args.vg);

    Widget::draw(args);
  }
};
