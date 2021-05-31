#ifdef _TIME_DRAWING
static DrawTimer drawTimer("Satanonaut");
#endif

struct SatanonautWidget : ModuleWidget
{
  SatanonautWidget(Satanonaut* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/satanonaut_front_panel.svg")));


    //
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_2)), module, Satanonaut::BUFFER_SIZE_INPUT));
    // addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_7 + 8)), module, Satanonaut::EFFECT_KNOB));
    add_snapping_parameter_knob(COLUMN_15, ROW_7 + 8, Satanonaut::EFFECT_KNOB);
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_9 + 4)), module, Satanonaut::BUFFER_SIZE_KNOB));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_11)), module, Satanonaut::FEEDBACK_KNOB));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_13 - 4)), module, Satanonaut::PARAM_1_KNOB));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_15 - 8)), module, Satanonaut::PARAM_2_KNOB));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_7 + 8)), module, Satanonaut::EFFECT_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_9 + 4)), module, Satanonaut::BUFFER_SIZE_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_11)), module, Satanonaut::FEEDBACK_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_13 - 4)), module, Satanonaut::PARAM_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_15 - 8)), module, Satanonaut::PARAM_2_INPUT));

    // Purge button  (removed.  This doesn't seem to be needed anymore)
    // addParam(createParamCentered<LEDButton>(mm2px(Vec(COLUMN_15, ROW_1)), module, Satanonaut::PURGE_BUTTON));

    // Drive knob
    addParam(createParamCentered<Trimpot>(mm2px(Vec(COLUMN_3 + 2, ROW_15 + 1)), module, Satanonaut::DRIVE_KNOB));

    // Inputs and outputs
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_1 - 1, ROW_13 AND_A_HALF_ROW + 1)), module, Satanonaut::AUDIO_INPUT_LEFT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_1 - 1, ROW_15 + 1)), module, Satanonaut::AUDIO_INPUT_RIGHT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_3 - 9, ROW_13 AND_A_HALF_ROW + 1)), module, Satanonaut::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_3 - 9, ROW_15 + 1)), module, Satanonaut::AUDIO_OUTPUT_RIGHT));

    /*
    SatanonautEffectReadout *effect_readout = new SatanonautEffectReadout();
		effect_readout->box.pos = mm2px(Vec(8, 64.3));
		effect_readout->box.size = Vec(200, 30); // bounding box of the widget
		effect_readout->module = module;
		addChild(effect_readout);
    */
  }

  void add_snapping_parameter_knob(float column, float row, int index)
  {
    auto P = createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(column, row)), module, index);
    dynamic_cast<Knob*>(P)->snap = true;
    addParam(P);
  }

  #ifdef _TIME_DRAWING
      // Seq: avg = 399.650112, stddev = 78.684572 (us) Quota frac=2.397901
      void draw(const DrawArgs &args) override
      {
          DrawLocker l(drawTimer);
          ModuleWidget::draw(args);
      }
  #endif
};
