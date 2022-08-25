struct ImageWidget : TransparentWidget
{
  std::string image_file_path;
  float width;
  float height;
  float alpha = 1.0;

  ImageWidget(std::string image_file_path, float width, float height, float alpha = 1.0)
  {
    this->image_file_path = image_file_path;
    this->width = width; // 2154
    this->height = height; // 1525
    this->alpha = alpha;
  }

  void draw(const DrawArgs &args) override
  {
    std::shared_ptr<Image> img = APP->window->loadImage(asset::plugin(pluginInstance, this->image_file_path));

    int temp_width, temp_height;

    // Get the image size and store it in the width and height variables
    nvgImageSize(args.vg, img->handle, &temp_width, &temp_height);

    // Set the bounding box of the widget
    box.size = mm2px(Vec(width, height));

    // Paint the .png background
    NVGpaint paint = nvgImagePattern(args.vg, 0.0, 0.0, box.size.x, box.size.y, 0.0, img->handle, this->alpha);
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
    nvgFillPaint(args.vg, paint);
    nvgFill(args.vg);

    Widget::draw(args);
  }
};
