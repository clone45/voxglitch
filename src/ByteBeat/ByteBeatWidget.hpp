struct ByteBeatWidget : ModuleWidget
{
  ByteBeatWidget(ByteBeat* module)
  {
    float group_y = 30;
    float row_padding = 10.2;

    float output_l_column = 29.08 + 15.24 + 10.16;
    float output_r_column = 39.78 + 15.24 + 10.16;

    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/bytebeat_front_panel.svg")));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, group_y + (row_padding * 0))), module, ByteBeat::AUDIO_OUTPUT_LEFT));

  }
};
