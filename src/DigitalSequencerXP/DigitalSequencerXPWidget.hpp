// namespace dsxp_menus {
  #include "menus/AllSequencersItem.hpp"
  #include "menus/InputSnapItem.hpp"
  #include "menus/OutputRangeValueItem.hpp"
  #include "menus/OutputRangeItem.hpp"
  #include "menus/QuickKeyMenu.hpp"
  #include "menus/ResetMenu.hpp"
  #include "menus/SampleAndHoldItem.hpp"
  #include "menus/LabelTextField.hpp"
  #include "menus/SequencerItem.hpp"
// }

struct DigitalSequencerXPWidget : VoxglitchModuleWidget
{
  DigitalSequencerXP* module;
  int copy_sequencer_index = -1;

  DigitalSequencerXPWidget(DigitalSequencerXP* module)
  {
    this->module = module;
    setModule(module);

    // Load and apply theme
    theme.load("digital_sequencer_xp");
    applyTheme();

    if(theme.showScrews())
    {
  		addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, 0)));
  		addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  		addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  		addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    // Main voltage sequencer display
    dsxp::VoltageSequencerDisplayXP *voltage_sequencer_display_xp = new dsxp::VoltageSequencerDisplayXP();
    voltage_sequencer_display_xp->box.pos = themePos("CV_SEQUENCER");
    voltage_sequencer_display_xp->module = module;

    dsxp::GateSequencerDisplayXP *gates_display = new dsxp::GateSequencerDisplayXP();
    gates_display->box.pos = themePos("GATE_SEQUENCER");
    gates_display->module = module;

    addChild(voltage_sequencer_display_xp);
    addChild(gates_display);

    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_1_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 0));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_2_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 1));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_3_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 2));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_4_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 3));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_5_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 4));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_6_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 5));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_7_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 6));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_8_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 7));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_9_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 8));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_10_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 9));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_11_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 10));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_12_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 11));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_13_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 12));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_14_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 13));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_15_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 14));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("SEQUENCER_16_BUTTON"), module, DigitalSequencerXP::SEQUENCER_BUTTONS + 15));

    // Inputs
    addInput(createInputCentered<VoxglitchPolyPort>(Vec(29.5276, 339.251), module, DigitalSequencerXP::POLY_STEP_INPUT));
    addInput(createInputCentered<VoxglitchPolyPort>(Vec(72.4724, 339.251), module, DigitalSequencerXP::POLY_LENGTH_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(115.4173, 339.251), module, DigitalSequencerXP::RESET_INPUT));
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 43.632, 114.893)), module, DigitalSequencerXP::POLY_MOD_INPUT));

    // Poly CV and Gate outputs
    addOutput(createOutputCentered<VoxglitchPolyPort>(themePos("CV_OUTPUT"), module, DigitalSequencerXP::POLY_CV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchPolyPort>(themePos("GATE_OUTPUT"), module, DigitalSequencerXP::POLY_GATE_OUTPUT));
  }

  void appendContextMenu(Menu *menu) override
  {
    DigitalSequencerXP *module = dynamic_cast<DigitalSequencerXP*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Sequencer Settings"));

    // Add "all" sequencer settings
    dsxp::AllSequencersItem *all_sequencer_items;
    all_sequencer_items = createMenuItem<dsxp::AllSequencersItem>("All Sequencers", RIGHT_ARROW);
    all_sequencer_items->module = module;
    menu->addChild(all_sequencer_items);

    // Add individual sequencer settings
    dsxp::SequencerItem *sequencer_items[NUMBER_OF_SEQUENCERS];

    for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
    {
      sequencer_items[i] = createMenuItem<dsxp::SequencerItem>("Sequencer #" + std::to_string(i + 1), RIGHT_ARROW);
      sequencer_items[i]->module = module;
      sequencer_items[i]->sequencer_number = i;
      menu->addChild(sequencer_items[i]);
    }

    dsxp::ResetModeItem *reset_mode_item = createMenuItem<dsxp::ResetModeItem>("Reset Mode", RIGHT_ARROW);
    reset_mode_item->module = module;
    menu->addChild(reset_mode_item);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuItem<dsxp::QuickKeyMenu>("Quick Key Reference", RIGHT_ARROW));
  }

  void step() override {
    ModuleWidget::step();
  }

  //
  // Handler for keypresses that affect the entire module
  //
  void onHoverKey(const event::HoverKey &e) override
  {
      if ((e.key == GLFW_KEY_F) && ((e.mods & RACK_MOD_MASK) != GLFW_MOD_CONTROL)) // F (no ctrl)
      {
        if(e.action == GLFW_PRESS)
        {
          module->frozen = ! module->frozen;
          e.consume(this);
        }
      }

      if ((e.key == GLFW_KEY_C) && ((e.mods & RACK_MOD_MASK) == GLFW_MOD_CONTROL)) // Control-C
      {
        if(e.action == GLFW_PRESS)
        {
          copy_sequencer_index = module->selected_sequencer_index;
          e.consume(this);
        }
      }

      if ((e.key == GLFW_KEY_V) && ((e.mods & RACK_MOD_MASK) == GLFW_MOD_CONTROL)) // Control-V
      {
        if(e.action == GLFW_PRESS)
        {
          if(copy_sequencer_index > -1)
          {
            module->copy(copy_sequencer_index, module->selected_sequencer_index);
            e.consume(this);
          }
        }
      }

      // Switch between seuences using the number keys 1-8
      if (e.key >= GLFW_KEY_1 && e.key <= GLFW_KEY_8) // quick-select
      {
        if(e.action == GLFW_PRESS)
        {
          unsigned int sequencer_number = e.key - 49;
          if ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) sequencer_number += 8;
          sequencer_number = clamp(sequencer_number,0,NUMBER_OF_SEQUENCERS-1);
          module->selected_sequencer_index = sequencer_number;
          e.consume(this);
        }
      }

      ModuleWidget::onHoverKey(e);

  }

};
