struct ScatterWidget : ModuleWidget
{
  ScatterWidget(Scatter* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/scatter_front_panel.svg")));
  }


  void appendContextMenu(Menu *menu) override
  {
    Scatter *module = dynamic_cast<Scatter*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Samples"));

    //
    // Add the sample slots to the right-click context menu
    //
    
    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
    {
      ScatterLoadSample *menu_item_load_sample = new ScatterLoadSample();
      menu_item_load_sample->sample_number = i;
      menu_item_load_sample->text = std::to_string(i+1) + ": " + module->loaded_filenames[i];
      menu_item_load_sample->module = module;
      menu->addChild(menu_item_load_sample);
    }
  }
};
