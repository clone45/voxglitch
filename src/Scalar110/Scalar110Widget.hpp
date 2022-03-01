#include <componentlibrary.hpp>

struct Scalar110Widget : ModuleWidget
{
  Scalar110Widget(Scalar110* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/scalar110_front_panel.svg")));

    float button_group_x  = 10.0;
    float button_group_y  = 100.0;
    float button_spacing  = 10.0;

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 40)), module, Scalar110::STEP_INPUT));

    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * i), button_group_y + 10)), module, Scalar110::STEP_SELECT_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * i), button_group_y + 10)), module, Scalar110::STEP_SELECT_BUTTON_LIGHTS + i));

      addParam(createLightParamCentered<VCVLightBezel<>>(mm2px(Vec(button_group_x + (button_spacing * i), button_group_y)), module, Scalar110::DRUM_PADS + i, Scalar110::DRUM_PAD_LIGHTS + i));
      addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(button_group_x + (button_spacing * i), button_group_y - 6)), module, Scalar110::STEP_LOCATION_LIGHTS + i));
    }

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(50,50)), module, Scalar110::TRACK_SELECT_KNOB));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(50,80)), module, Scalar110::ENGINE_SELECT_KNOB));


    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(100,50)), module, Scalar110::ENGINE_PARAMS));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(120,50)), module, Scalar110::ENGINE_PARAMS + 1));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(140,50)), module, Scalar110::ENGINE_PARAMS + 2));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(160,50)), module, Scalar110::ENGINE_PARAMS + 3));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(100,70)), module, Scalar110::ENGINE_PARAMS + 4));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(120,70)), module, Scalar110::ENGINE_PARAMS + 5));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(140,70)), module, Scalar110::ENGINE_PARAMS + 6));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(160,70)), module, Scalar110::ENGINE_PARAMS + 7));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(210, 114.702)), module, Scalar110::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(220, 114.702)), module, Scalar110::AUDIO_OUTPUT_RIGHT));

    for(unsigned int i=0; i<NUMBER_OF_PARAMETERS; i++)
    {
      float x_position = 90.2 + ((i%4) * 20);
      float y_position = 37 + ((i/4) * 20);

      LabelDisplay *label_display = new LabelDisplay(i);
      label_display->box.pos = mm2px(Vec(x_position, y_position));;
      label_display->module = module;
      addChild(label_display);
    }

    /*
    if(module->selected_track->engine_index == 2 && module->selected_parameter == 0)
    {
      FileSelectWidget *file_select_widget = new FileSelectWidget();
      file_select_widget->module = module;
      file_select_widget->box.pos = mm2px(Vec(LCD_DISPLAY_X, LCD_DISPLAY_Y));
      addChild(file_select_widget);
    }
    else // show parameter edit display
    {
      ParamEditorDisplay *param_editor_display = new ParamEditorDisplay();
      param_editor_display->module = module;
      param_editor_display->box.pos = mm2px(Vec(LCD_DISPLAY_X, LCD_DISPLAY_Y));
      addChild(param_editor_display);
    }
    */

    FileSelectWidget *file_select_widget = new FileSelectWidget();
    file_select_widget->module = module;
    file_select_widget->box.pos = mm2px(Vec(LCD_DISPLAY_X + 2, LCD_DISPLAY_Y + 5));
    addChild(file_select_widget);

  }

  void appendContextMenu(Menu *menu) override
  {
    Scalar110 *module = dynamic_cast<Scalar110*>(this->module);
    assert(module);

    FolderSelect *folder_select = new FolderSelect();
    folder_select->text = "Load samples from folder";
    folder_select->module = module;
    menu->addChild(folder_select);
  }


};
