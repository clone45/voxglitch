struct DigitalProgrammerWidget : ModuleWidget
{
  DigitalProgrammer* module;

  float bank_button_positions[24][2] = {
    {189.56, 45.333}, {200.827, 45.333}, {212.093, 45.333}, {223.360, 45.333},
    {189.56, 57.133}, {200.827, 57.133}, {212.093, 57.133}, {223.360, 57.133},
    {189.56, 68.933}, {200.827, 68.933}, {212.093, 68.933}, {223.360, 68.933},
    {189.56, 80.733}, {200.827, 80.733}, {212.093, 80.733}, {223.360, 80.733},
    {189.56, 92.533}, {200.827, 92.533}, {212.093, 92.533}, {223.360, 92.533},
    {189.56, 104.333}, {200.827, 104.333}, {212.093, 104.333}, {223.360, 104.333}
  };

  DigitalProgrammerWidget(DigitalProgrammer* module)
  {
    this->module = module;
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/digital_programmer_front_panel.svg")));

    // panel_x_position and panel_y_position specify where a slider should
    // be displayed on the front panel relative to 0,0.
    float panel_x_position = 0;
    float panel_y_position = 0;
    // float margin_left = 7;  // offset from the far-lefthand side of the panel
    //float horizontal_padding = 11.0; // Offset from the top of the panel

    // Location of the inputs relative to the panel
    //float inputs_margin_left = 11.0;
    float outputs_vertical_position = 118.0;

    // Slider's natural position, without any padding between groups of 4, is
    // { 7, 18, 29, 40, 51, 62, 73, 84, 95, 106, 117, 128, 139, 150, 161, 172 };
    // However, I'm going to add some padding to set each group apart a little big

    float slider_column_x[NUMBER_OF_SLIDERS] = { 7, 18, 29, 40, 51, 62, 73, 84, 95, 106, 117, 128, 139, 150, 161, 172 };

    if(module)
    {

      //
      // Loop through each column and draw the slider and outputs
      //
      for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
      {
        // Calculate where to put the slider
        // panel_x_position = (column * horizontal_padding) + margin_left;
        panel_x_position = slider_column_x[column];
        panel_y_position = 7.0;

        // Add slider widget
        DPSliderDisplay *dp_slider_display = new DPSliderDisplay(column);
        dp_slider_display->setPosition(mm2px(Vec(panel_x_position, panel_y_position)));
        dp_slider_display->setSize(Vec(DRAW_AREA_WIDTH, SLIDER_HEIGHT));
        dp_slider_display->module = module;
        addChild(dp_slider_display);

        // Add corresponding output
        addOutput(createOutput<PJ301MPort>(mm2px(Vec(panel_x_position, outputs_vertical_position)), module, column));
      }

      //
      // bank buttons
      //
      for(unsigned int i = 0; i < NUMBER_OF_BANKS; i ++)
      {
        // calculation panel_x_position, panel_y_position
        panel_x_position = bank_button_positions[i][0];
        panel_y_position = bank_button_positions[i][1];

        // Add button widget
        DPBankButtonDisplay *dp_bank_button_display = new DPBankButtonDisplay(i);
        dp_bank_button_display->setPosition(mm2px(Vec(panel_x_position, panel_y_position)));
        dp_bank_button_display->setSize(Vec(BANK_BUTTON_WIDTH, BANK_BUTTON_HEIGHT));
        dp_bank_button_display->module = module;
        addChild(dp_bank_button_display);
      }

      // Poly add input
      addInput(createInputCentered<PJ301MPort>(mm2px(Vec(202.404, 11.366)), module, DigitalProgrammer::POLY_ADD_INPUT));

      // Poly output
      addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(225.683, 11.366)), module, DigitalProgrammer::POLY_OUTPUT));

      // bank controls
      addInput(createInputCentered<PJ301MPort>(mm2px(Vec(193.162, 36.593)), module, DigitalProgrammer::BANK_CV_INPUT));
      addInput(createInputCentered<PJ301MPort>(mm2px(Vec(205.383, 36.593)), module, DigitalProgrammer::BANK_NEXT_INPUT));
      addInput(createInputCentered<PJ301MPort>(mm2px(Vec(217.612, 36.593)), module, DigitalProgrammer::BANK_PREV_INPUT));
      addInput(createInputCentered<PJ301MPort>(mm2px(Vec(229.824, 36.593)), module, DigitalProgrammer::BANK_RESET_INPUT));

    }
  }

};
