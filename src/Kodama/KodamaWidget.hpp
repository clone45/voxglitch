struct KodamaWidget : VoxglitchModuleWidget
{
  KodamaWidget(Kodama *module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("kodama");
    applyTheme();

    NotesDisplay *notesDisplay = createWidget<NotesDisplay>(mm2px(Vec(0.0, 12.869)));
    notesDisplay->box.size = mm2px(Vec(60.0, 105.059));
    notesDisplay->setModule(module);
    addChild(notesDisplay);
  }

  struct NotesTextField : LedDisplayTextField
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

  struct NotesDisplay : LedDisplay
  {
    void setModule(Kodama *module)
    {
      NotesTextField *textField = createWidget<NotesTextField>(Vec(0, 0));
      textField->box.size = box.size;
      textField->multiline = true;
      textField->module = module;
      addChild(textField);
    }
  };

};
