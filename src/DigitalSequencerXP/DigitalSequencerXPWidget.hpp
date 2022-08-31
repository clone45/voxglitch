/*
#include "menus/AllSequencersItem.hpp"
#include "menus/InputSnapItem.hpp"
#include "menus/OutputRangeValueItem.hpp"
#include "menus/OutputRangeItem.hpp"
#include "menus/QuickKeyMenu.hpp"
#include "menus/ResetMenu.hpp"
#include "menus/SampleAndHoldItem.hpp"
#include "menus/LabelTextField.hpp"
#include "menus/SequencerItem.hpp"
*/

struct DigitalSequencerXPWidget : VoxglitchModuleWidget
{
  DigitalSequencerXP *module;
  int copy_sequencer_index = -1;

  DigitalSequencerXPWidget(DigitalSequencerXP *module)
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
    VoltageSequencerDisplayXP *voltage_sequencer_display_xp = new VoltageSequencerDisplayXP();
    voltage_sequencer_display_xp->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    voltage_sequencer_display_xp->module = module;

    GateSequencerDisplayXP *gates_display = new GateSequencerDisplayXP();
    gates_display->box.pos = mm2px(Vec(GATES_DRAW_AREA_POSITION_X, GATES_DRAW_AREA_POSITION_Y));

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
    DigitalSequencerXP *module = dynamic_cast<DigitalSequencerXP *>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Sequencer Settings"));

    // Add "all" sequencer settings
    AllSequencersItem *all_sequencer_items;
    all_sequencer_items = createMenuItem<AllSequencersItem>("All Sequencers", RIGHT_ARROW);
    all_sequencer_items->module = module;
    menu->addChild(all_sequencer_items);

    // Add individual sequencer settings
    SequencerItem *sequencer_items[NUMBER_OF_SEQUENCERS];

    for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
    {
      sequencer_items[i] = createMenuItem<SequencerItem>("Sequencer #" + std::to_string(i + 1), RIGHT_ARROW);
      sequencer_items[i]->module = module;
      sequencer_items[i]->sequencer_number = i;
      menu->addChild(sequencer_items[i]);
    }

    ResetModeItem *reset_mode_item = createMenuItem<ResetModeItem>("Reset Mode", RIGHT_ARROW);
    reset_mode_item->module = module;
    menu->addChild(reset_mode_item);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuItem<QuickKeyMenu>("Quick Key Reference", RIGHT_ARROW));
  }

  void step() override
  {
    ModuleWidget::step();
  }

  //
  // Handler for keypresses that affect the entire module
  //
  void onHoverKey(const event::HoverKey &e) override
  {
    if ((e.key == GLFW_KEY_F) && ((e.mods & RACK_MOD_MASK) != GLFW_MOD_CONTROL)) // F (no ctrl)
    {
      if (e.action == GLFW_PRESS)
      {
        module->frozen = !module->frozen;
        e.consume(this);
      }
    }

    if ((e.key == GLFW_KEY_C) && ((e.mods & RACK_MOD_MASK) == GLFW_MOD_CONTROL)) // Control-C
    {
      if (e.action == GLFW_PRESS)
      {
        copy_sequencer_index = module->selected_sequencer_index;
        e.consume(this);
      }
    }

    if ((e.key == GLFW_KEY_V) && ((e.mods & RACK_MOD_MASK) == GLFW_MOD_CONTROL)) // Control-V
    {
      if (e.action == GLFW_PRESS)
      {
        if (copy_sequencer_index > -1)
        {
          module->copy(copy_sequencer_index, module->selected_sequencer_index);
          e.consume(this);
        }
      }
    }

    // Switch between seuences using the number keys 1-8
    if (e.key >= GLFW_KEY_1 && e.key <= GLFW_KEY_8) // quick-select
    {
      if (e.action == GLFW_PRESS)
      {
        unsigned int sequencer_number = e.key - 49;
        if ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT)
          sequencer_number += 8;
        sequencer_number = clamp(sequencer_number, 0, NUMBER_OF_SEQUENCERS - 1);
        module->selected_sequencer_index = sequencer_number;
        e.consume(this);
      }
    }

    ModuleWidget::onHoverKey(e);
  }




  //
  // ==========================================================================================
  // MENUS
  // ==========================================================================================
  //

  struct AllInputSnapsValueItem : MenuItem {
    DigitalSequencerXP *module;
    int snap_division_index = 0;

    void onAction(const event::Action &e) override {
      for(unsigned int i=0; i<NUMBER_OF_SEQUENCERS; i++)
      {
        module->voltage_sequencers[i].snap_division_index = snap_division_index;
      }
    }
  };

  struct AllInputSnapsItem : MenuItem
  {
    DigitalSequencerXP *module;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      for (unsigned int i = 0; i < NUMBER_OF_SNAP_DIVISIONS; i++)
      {
        AllInputSnapsValueItem *all_input_snaps_value_item = createMenuItem<AllInputSnapsValueItem>(module->snap_division_names[i]);
        all_input_snaps_value_item->module = module;
        all_input_snaps_value_item->snap_division_index = i;
        menu->addChild(all_input_snaps_value_item);
      }

      return menu;
    }
  };

  struct AllOutputRangesValueItem : MenuItem {
    DigitalSequencerXP *module;
    int range_index = 0;

    void onAction(const event::Action &e) override {
      for(unsigned int i=0; i<NUMBER_OF_SEQUENCERS; i++)
      {
        module->voltage_sequencers[i].voltage_range_index = range_index;
      }
    }
  };

  struct AllOutputRangesItem : MenuItem {
    DigitalSequencerXP *module;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      for (unsigned int i=0; i < NUMBER_OF_VOLTAGE_RANGES; i++)
      {
        AllOutputRangesValueItem *all_output_ranges_value_item = createMenuItem<AllOutputRangesValueItem>(module->voltage_range_names[i]);
        all_output_ranges_value_item->module = module;
        all_output_ranges_value_item->range_index = i;
        menu->addChild(all_output_ranges_value_item);
      }

      return menu;
    }
  };

  struct AllSampleAndHoldsValueItem : MenuItem {
    DigitalSequencerXP *module;
    bool value = false;

    void onAction(const event::Action &e) override {
      for(unsigned int i=0; i<NUMBER_OF_SEQUENCERS; i++)
      {
        module->voltage_sequencers[i].sample_and_hold = value;
      }
    }
  };

  struct AllSampleAndHoldsItem : MenuItem {
    DigitalSequencerXP *module;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      AllSampleAndHoldsValueItem *sample_and_hold_off = createMenuItem<AllSampleAndHoldsValueItem>("Off");
      sample_and_hold_off->module = module;
      sample_and_hold_off->value = false;
      menu->addChild(sample_and_hold_off);

      AllSampleAndHoldsValueItem *sample_and_hold_on = createMenuItem<AllSampleAndHoldsValueItem>("On");
      sample_and_hold_on->module = module;
      sample_and_hold_on->value = true;
      menu->addChild(sample_and_hold_on);

      return menu;
    }
  };

  struct AllSequencersItem : MenuItem {
    DigitalSequencerXP *module;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      AllOutputRangesItem *all_output_ranges_item = createMenuItem<AllOutputRangesItem>("Output Range", RIGHT_ARROW);
      all_output_ranges_item->module = module;
      menu->addChild(all_output_ranges_item);

      AllInputSnapsItem *all_input_snaps_item = createMenuItem<AllInputSnapsItem>("Snap", RIGHT_ARROW);
      all_input_snaps_item->module = module;
      menu->addChild(all_input_snaps_item);

      AllSampleAndHoldsItem *all_sample_and_holds_items = createMenuItem<AllSampleAndHoldsItem>("Sample & Hold", RIGHT_ARROW);
      all_sample_and_holds_items->module = module;
      menu->addChild(all_sample_and_holds_items);

      return menu;
    }
  };

  struct InputSnapValueItem : MenuItem {
    DigitalSequencerXP *module;
    int snap_division_index = 0;
    int sequencer_number = 0;

    void onAction(const event::Action &e) override {
      module->voltage_sequencers[sequencer_number].snap_division_index = snap_division_index;
    }
  };

  struct InputSnapItem : MenuItem {
    DigitalSequencerXP *module;
    int sequencer_number = 0;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      for (unsigned int i=0; i < NUMBER_OF_SNAP_DIVISIONS; i++)
      {
        InputSnapValueItem *input_snap_value_item = createMenuItem<InputSnapValueItem>(module->snap_division_names[i], CHECKMARK(module->voltage_sequencers[sequencer_number].snap_division_index == i));
        input_snap_value_item->module = module;
        input_snap_value_item->snap_division_index = i;
        input_snap_value_item->sequencer_number = this->sequencer_number;
        menu->addChild(input_snap_value_item);
      }

      return menu;
    }
  };

  struct LabelTextField : TextField {

    DigitalSequencerXP *module;
    unsigned int index = 0;

    LabelTextField(unsigned int index)
    {
      this->index = index;
      this->box.pos.x = 50;
      this->box.size.x = 160;
      this->multiline = false;
    }

    void onChange(const event::Change& e) override {
      module->labels[index] = text;
    }

  };

  struct OutputRangeValueItem : MenuItem {

    DigitalSequencerXP *module;
    int range_index = 0;
    int sequencer_number = 0;

    void onAction(const event::Action &e) override {
      module->voltage_sequencers[sequencer_number].voltage_range_index = range_index;
    }
  };

  struct OutputRangeItem : MenuItem {
    DigitalSequencerXP *module;
    int sequencer_number = 0;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      for (unsigned int i=0; i < NUMBER_OF_VOLTAGE_RANGES; i++)
      {
        OutputRangeValueItem *output_range_value_menu_item = createMenuItem<OutputRangeValueItem>(module->voltage_range_names[i], CHECKMARK(module->voltage_sequencers[sequencer_number].voltage_range_index == i));
        output_range_value_menu_item->module = module;
        output_range_value_menu_item->range_index = i;
        output_range_value_menu_item->sequencer_number = this->sequencer_number;
        menu->addChild(output_range_value_menu_item);
      }

      return menu;
    }
  };

  struct QuickKeyMenu : MenuItem {
    Menu *createChildMenu() override {
      Menu *menu = new Menu;
      menu->addChild(createMenuLabel("f: Toggle Freeze Mode (for easy editing)"));
      menu->addChild(createMenuLabel("g: When frozen, press 'g' to send gate out"));
      menu->addChild(createMenuLabel("r: Randomize current howevered sequencer"));
      menu->addChild(createMenuLabel("shift-r: Randomize both active sequencers (CV/Gate)"));
      menu->addChild(createMenuLabel("shift+drag : Shift hovered sequence left or right"));
      return menu;
    }
  };

  struct ResetOnNextOption : MenuItem {
    DigitalSequencerXP *module;

    void onAction(const event::Action &e) override {
      module->legacy_reset = false;
    }
  };

  struct ResetInstantOption : MenuItem {
    DigitalSequencerXP *module;

    void onAction(const event::Action &e) override {
      module->legacy_reset = true;
    }
  };

  struct ResetModeItem : MenuItem {
    DigitalSequencerXP *module;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      ResetOnNextOption *reset_on_next = createMenuItem<ResetOnNextOption>("Next clock input.", CHECKMARK(! module->legacy_reset));
      reset_on_next->module = module;
      menu->addChild(reset_on_next);

      ResetInstantOption *reset_instant = createMenuItem<ResetInstantOption>("Instant", CHECKMARK(module->legacy_reset));
      reset_instant->module = module;
      menu->addChild(reset_instant);

      return menu;
    }
  };

  struct SampleAndHoldItem : MenuItem {
    DigitalSequencerXP *module;
    int sequencer_number = 0;

    void onAction(const event::Action &e) override {
      module->voltage_sequencers[sequencer_number].sample_and_hold ^= true; // flip the value
    }
  };

  struct SequencerItem : MenuItem {
    DigitalSequencerXP *module;
    unsigned int sequencer_number = 0;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      OutputRangeItem *output_range_item = createMenuItem<OutputRangeItem>("Output Range", RIGHT_ARROW);
      output_range_item->sequencer_number = this->sequencer_number;
      output_range_item->module = module;
      menu->addChild(output_range_item);

      InputSnapItem *input_snap_item = createMenuItem<InputSnapItem>("Snap", RIGHT_ARROW);
      input_snap_item->sequencer_number = this->sequencer_number;
      input_snap_item->module = module;
      menu->addChild(input_snap_item);

      SampleAndHoldItem *sample_and_hold_item = createMenuItem<SampleAndHoldItem>("Sample & Hold", CHECKMARK(module->voltage_sequencers[sequencer_number].sample_and_hold));
      sample_and_hold_item->sequencer_number = this->sequencer_number;
      sample_and_hold_item->module = module;
      menu->addChild(sample_and_hold_item);

      // Add label input
      auto holder = new rack::Widget;
      holder->box.size.x = 220; // This value will determine the width of the menu
      holder->box.size.y = 20;

      auto lab = new rack::Label;
      lab->text = "Label: ";
      lab->box.size = 50; // label box size determins the bounding box around #1, #2, #3 etc.
      holder->addChild(lab);

      auto textfield = new LabelTextField(sequencer_number);
      textfield->module = module;
      textfield->text = module->labels[sequencer_number];
      holder->addChild(textfield);

      menu->addChild(holder);

      return menu;
    }
  };

};
