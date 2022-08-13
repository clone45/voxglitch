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
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/digital_sequencer_xp_front_panel.svg")));

    // Cosmetic rack screws
    addChild(createWidget<ScrewSilver>(Vec(15, 365)));
    addChild(createWidget<ScrewSilver>(mm2px(Vec(171.5, 0))));

    // Main voltage sequencer display
    dsxp::VoltageSequencerDisplayXP *voltage_sequencer_display_xp = new dsxp::VoltageSequencerDisplayXP();
    voltage_sequencer_display_xp->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    voltage_sequencer_display_xp->module = module;

    dsxp::GateSequencerDisplayXP *gates_display = new dsxp::GateSequencerDisplayXP();
    gates_display->box.pos = mm2px(Vec(GATES_DRAW_AREA_POSITION_X, GATES_DRAW_AREA_POSITION_Y));
    gates_display->module = module;

    addChild(voltage_sequencer_display_xp);
    addChild(gates_display);

    double button_spacing = 9.6; // 9.1
    double vertical_button_spacing = 11.085;
    double button_group_x = 72.0;
    double button_group_y = 107.568;


    // Draw sequence buttons
    int half_number_of_sequencers = NUMBER_OF_SEQUENCERS/2;

    for(unsigned int i=0; i<NUMBER_OF_SEQUENCERS; i++)
    {
      float x_position = (button_group_x + (button_spacing * (i % half_number_of_sequencers)));
      float y_position = button_group_y + (i/half_number_of_sequencers * vertical_button_spacing);
      addParam(createParamCentered<LEDButton>(mm2px(Vec(x_position, y_position)), module, DigitalSequencerXP::SEQUENCER_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(x_position, y_position)), module, DigitalSequencerXP::SEQUENCER_LIGHTS + i));
    }

    // Inputs
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 114.893)), module, DigitalSequencerXP::POLY_STEP_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 14.544, 114.893)), module, DigitalSequencerXP::POLY_LENGTH_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 29.088, 114.893)), module, DigitalSequencerXP::RESET_INPUT));
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 43.632, 114.893)), module, DigitalSequencerXP::POLY_MOD_INPUT));

    // Poly CV and Gate outputs
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(170.321, 107.568)), module, DigitalSequencerXP::POLY_CV_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(170.321, 118.653)), module, DigitalSequencerXP::POLY_GATE_OUTPUT));
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
