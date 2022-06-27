struct UpdatesVisualizerWidget : TransparentWidget
{
  GrooveBox *module;
  ScrollWidget* scroll;
  std::string release_notes;

  float x = 0;
  float y = 0;
  float width = 133.741 * 2.952756;
  float height = 49.672 * 2.952756;
  float mid_height = height / 2.0;

  UpdatesVisualizerWidget()
  {
    std::string path = asset::plugin(pluginInstance, "res/release_notes/groovebox.txt");
    std::ifstream ifs(path);

    // If the release notes cannot be found, this means that the file was already
    // viewed and renamed, so don't view it again.
    if(ifs.fail())
    {
      module->show_updates_visualizer = false;
    }
    // Show the release notes
    else
    {
      // Read the release notes from the file.  The release notes will be
      // displayed in the drawLayer(...) function
      release_notes.assign((std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()) );

      // Rename updates file so it isn't shown next time the module is loaded
      std::string destination = asset::plugin(pluginInstance, "res/release_notes/groovebox_viewed.txt");
      if(std::rename(path.c_str(), destination.c_str()) != 0)
      {
        DEBUG("unable to rename file to ");
        DEBUG(destination.c_str());
      }
    }
  }

  void drawLayer(const DrawArgs& args, int layer) override
  {
  	if (layer == 1)
    {
      TransparentWidget::draw(args);
      TransparentWidget::drawLayer(args, layer);

      const auto vg = args.vg;

      if(module)
      {
        nvgSave(vg);

        if(module->show_updates_visualizer)
        {
          //
          // Draw black background
          //
          nvgBeginPath(vg);
          nvgRect(vg, x, y, width, height);
          nvgFillColor(vg, nvgRGB(0, 0, 0));
          nvgFill(vg);

          //
          // Set up the font styling
          //

          std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
          if (font) {
            nvgFontSize(args.vg, 10);
            nvgFontFaceId(args.vg, font->handle);
            nvgTextLetterSpacing(args.vg, 0);
            nvgFillColor(args.vg, nvgRGBA(255, 215, 20, 0xff));
          }

          //
          // Iterate over the release notes and display them
          //

          std::string line;
          unsigned int row = 0;
          std::stringstream string_stream(release_notes);

          while(std::getline(string_stream, line, '\n')){
            nvgText(vg, 10, 20 + (12 * row), line.c_str(), NULL);
            row++;
          }


        }

        nvgRestore(vg);
      }
    }
  }

  void onEnter(const event::Enter &e) override
  {
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    TransparentWidget::onLeave(e);
  }

  void onHover(const event::Hover& e) override {
    e.consume(this);
  }

  void step() override {
    TransparentWidget::step();
  }
};
