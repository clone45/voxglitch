struct KodamaWidget : VoxglitchModuleWidget
{
  KodamaWidget(Kodama *module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("kodama");
    applyTheme();

    // addInput(createInputCentered<VoxglitchInputPort>(themePos("STEP_INPUT"), module, Kodama::STEP_INPUT));
    // addInput(createInputCentered<VoxglitchInputPort>(themePos("RESET_INPUT"), module, Kodama::RESET_INPUT));

    addInput(createInputCentered<VoxglitchInputPort>(Vec(150.0, 15.0), module, Kodama::STEP_INPUT));

    CodeDisplay *code_display = createWidget<CodeDisplay>(mm2px(Vec(0.0, 12.869)));
    code_display->box.size = mm2px(Vec(60.0, 105.059));
    code_display->setModule(module);
    addChild(code_display);

    KodamaGridWidget *kodama_grid_widget = new KodamaGridWidget();
    kodama_grid_widget->box.pos = Vec(200, 20);
    kodama_grid_widget->module = module;
    addChild(kodama_grid_widget);

  }

  struct CodeTextField : LedDisplayTextField
  {
    Kodama *module;

    void step() override
    {
      LedDisplayTextField::step();

      if (module && module->dirty)
      {
        setText(module->text);
        module->dirty = false;
      }
    }

    void onChange(const ChangeEvent &e) override
    {
      if (module)
        module->text = getText();
    }
  };

  struct CodeDisplay : LedDisplay
  {
    void setModule(Kodama *module)
    {
      CodeTextField *code_text_field = createWidget<CodeTextField>(Vec(0, 0));
      code_text_field->box.size = box.size;
      code_text_field->multiline = true;
      code_text_field->module = module;
      addChild(code_text_field);
    }
  };

};
