struct LooperWaveformDisplay : TransparentWidget
{
  Looper *module;
  std::deque<float> waveform_array;

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    // Debugging code for draw area, which often has to be set experimentally
    /*
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
    nvgFillColor(vg, nvgRGBA(120, 20, 20, 100));
    nvgFill(vg);
    */

    if(module)
    {
      waveform_array.push_front(module->left_audio);

      if(waveform_array.size() > 40) waveform_array.pop_back();

      for (unsigned int i = 0; i < waveform_array.size(); i++)
      {
        nvgBeginPath(vg);
        nvgStrokeWidth(vg, 3);
        nvgStrokeColor(vg, nvgRGBA(97, 143, 170, 200));
        nvgMoveTo(vg, (DRAW_AREA_WIDTH / 2), i * 4.3);
        nvgLineTo(vg, (DRAW_AREA_WIDTH / 2) + (DRAW_AREA_WIDTH * waveform_array[i]), i * 4.3);
        nvgStroke(vg);
      }
    }

    nvgRestore(vg);
  }

};
