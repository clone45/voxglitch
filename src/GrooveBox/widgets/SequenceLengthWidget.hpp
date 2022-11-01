//
// SequenceLengthWidget
//
// This is the grey horizontal bar that shows the sequence length of the
// selected track.

struct SequenceLengthWidget : TransparentWidget
{
  GrooveBox *module;
  // how far past the LEDs does this strip appear?
  float overhang = 16.25;

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    nvgSave(vg);
    nvgBeginPath(vg);

    if(module) {
      // Draw horizontal rectangle for track indictor with pretty rounded corners
      float length = button_positions[module->selected_track->range_end][0] - button_positions[module->selected_track->range_start][0] + (overhang*2);
      //nvgRoundedRect(vg, button_positions[module->selected_track->range_start][0] - 20, 0, length, 12, 5);
      nvgRect(vg, button_positions[module->selected_track->range_start][0] - 19 - overhang, 0, length, 12);
    }
    else {
      // Paint static content for library display
      nvgRoundedRect(vg, 0, 0, mm2px(186.51), 12, 5);
    }

    nvgFillColor(vg, nvgRGB(83, 92, 91));
    nvgFill(vg);

    nvgRestore(vg);
  }

};
