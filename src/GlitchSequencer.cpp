// TODO: add ability to clear a pattern completely
// TODO: pattern shift using arrow keys

#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include "cmath"

#define MAX_SEQUENCE_LENGTH 64
#define SEQUENCER_ROWS 16
#define SEQUENCER_COLUMNS 16
#define NUMBER_OF_TRIGGER_GROUPS 5

#define DRAW_AREA_WIDTH 277.4
#define DRAW_AREA_HEIGHT 277.4
#define DRAW_AREA_POSITION_X 3.800
#define DRAW_AREA_POSITION_Y 5.9

#define CELL_WIDTH 16.95
#define CELL_HEIGHT 16.95
#define CELL_PADDING 0.4

#define PLAY_MODE 0
#define EDIT_SEED_MODE 1
#define EDIT_TRIGGERS_MODE 2
#define EDIT_LENGTH_MODE 3

using namespace std;

struct CellularAutomatonSequencer
{
  unsigned int position = 0;
  unsigned int length = 0;

  bool seed[SEQUENCER_ROWS][SEQUENCER_COLUMNS] = {
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,1,1, 1,1,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,1, 1,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
  };

  bool state[SEQUENCER_ROWS][SEQUENCER_COLUMNS];
  bool next[SEQUENCER_ROWS][SEQUENCER_COLUMNS];
  bool triggers[NUMBER_OF_TRIGGER_GROUPS][SEQUENCER_ROWS][SEQUENCER_COLUMNS] =
  {
    {
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
    },
    {
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,1,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
    },
    {
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
    },
    {
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
      { 0,0,1,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
    },
    {
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 1,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,1,0, 0,1,0,0, 1,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,1,1, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,0,0 },
      { 0,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
    }
  };

  // constructor
  CellularAutomatonSequencer()
  {
    clearPattern(&state);
    clearPattern(&next);

    copyPattern(&state, &seed);
  }

  // Step the sequencer and return, as separate booleans, if any of the five
  // trigger groups have been triggered.

  void step(bool *trigger_results)
  {
    position ++;

    if(position >= length)
    {
      restart_sequence();
    }
    else
    {
      calculate_next_state(trigger_results);
    }
  }

  void reset()
  {
    restart_sequence();
  }

  void restart_sequence()
  {
    position = 0;
    copyPattern(&state, &seed);  // dst < src
    clearPattern(&next);
  }

  void calculate_next_state(bool *trigger_results)
  {
    for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      trigger_results[i] = false;
    }

    for(unsigned int row = 1; row < SEQUENCER_ROWS - 1; row++)
    {
      for(unsigned int column = 1; column < SEQUENCER_COLUMNS - 1; column++)
      {
        unsigned int neighbors = 0;

        // Calculate number of neighbors
        if(state[row-1][column-1]) neighbors++;
        if(state[row-1][column]) neighbors++;
        if(state[row-1][column+1]) neighbors++;
        if(state[row][column-1]) neighbors++;
        if(state[row][column+1]) neighbors++;
        if(state[row+1][column-1]) neighbors++;
        if(state[row+1][column]) neighbors++;
        if(state[row+1][column+1]) neighbors++;

        // Apply game of life rules
        if(state[row][column] && neighbors < 2) next[row][column] = 0;
        else if(state[row][column] && neighbors > 3) next[row][column] = 0;
        else if(state[row][column] && (neighbors == 2 || neighbors == 3)) next[row][column] = 1;
        else if(state[row][column] == 0 && neighbors == 3) next[row][column] = 1;

        // Detect when to output a trigger!
        if(state[row][column] == 0 && next[row][column] == 1)
        {
          for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
          {
            if(triggers[i][row][column]) trigger_results[i] = true;
          }
        }
      }
    }

    copyPattern(&state, &next); // dst < src
  }

  void copyPattern(bool (*dst)[SEQUENCER_ROWS][SEQUENCER_COLUMNS], bool (*src)[SEQUENCER_ROWS][SEQUENCER_COLUMNS])
  {
    for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
    {
      for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
      {
        (*dst)[row][column] = (*src)[row][column];
      }
    }
  }

  void clearPattern(bool (*pattern)[SEQUENCER_ROWS][SEQUENCER_COLUMNS])
  {
    for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
    {
      for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
      {
        (*pattern)[row][column] = 0;
      }
    }
  }

  void setLength(unsigned int length)
  {
    this->length = length;
  }

  //
  // unsigned int packPattern(pointer to a pattern)
  //
  // Used to compress the boolean array into an integer for saving to a patch

  std::string packPattern(bool (*pattern)[SEQUENCER_ROWS][SEQUENCER_COLUMNS])
  {
    std::string packed_pattern_data = "";

    for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
    {
      for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
      {
        if((*pattern)[row][column] == 1)
        {
          packed_pattern_data = packed_pattern_data + '1';
        }
        else
        {
          packed_pattern_data = packed_pattern_data + '0';
        }
      }
    }

    return(packed_pattern_data);
  }

  //
  // void unpackPattern(pointer to a pattern)
  //
  // Used to uncompress the integer created from packPattern when loading
  // a patch.

  void unpackPattern(std::string packed_pattern_data, bool (*pattern)[SEQUENCER_ROWS][SEQUENCER_COLUMNS])
  {
    unsigned int string_index = 0;

    for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
    {
      for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
      {
        if(packed_pattern_data[string_index] == '0')
        {
          (*pattern)[row][column] = 0;
        }
        else
        {
          (*pattern)[row][column] = 1;
        }

        string_index++;
      }
    }
  }
};


struct GlitchSequencer : Module
{
  CellularAutomatonSequencer sequencer;

  dsp::SchmittTrigger stepTrigger;
  dsp::SchmittTrigger resetTrigger;
  dsp::SchmittTrigger trigger_group_button_schmitt_trigger[5];
  dsp::PulseGenerator gateOutputPulseGenerators[NUMBER_OF_TRIGGER_GROUPS];

  unsigned int mode = PLAY_MODE;
  bool trigger_button_is_triggered[NUMBER_OF_TRIGGER_GROUPS];
  unsigned int trigger_group_buttons[NUMBER_OF_TRIGGER_GROUPS];
  unsigned int gate_outputs[NUMBER_OF_TRIGGER_GROUPS];
  int selected_trigger_group_index = -1; // -1 means "none selected"
  long clock_ignore_on_reset = 0;

  bool trigger_results[NUMBER_OF_TRIGGER_GROUPS];

  enum ParamIds {
    LENGTH_KNOB,
    TRIGGER_GROUP_1_BUTTON,
    TRIGGER_GROUP_2_BUTTON,
    TRIGGER_GROUP_3_BUTTON,
    TRIGGER_GROUP_4_BUTTON,
    TRIGGER_GROUP_5_BUTTON,
    NUM_PARAMS
  };
  enum InputIds {
    STEP_INPUT,
    RESET_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    GATE_OUTPUT_1,
    GATE_OUTPUT_2,
    GATE_OUTPUT_3,
    GATE_OUTPUT_4,
    GATE_OUTPUT_5,
    NUM_OUTPUTS
  };
  enum LightIds {
    TRIGGER_GROUP_1_LIGHT,
    TRIGGER_GROUP_2_LIGHT,
    TRIGGER_GROUP_3_LIGHT,
    TRIGGER_GROUP_4_LIGHT,
    TRIGGER_GROUP_5_LIGHT,
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  GlitchSequencer()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(LENGTH_KNOB, 1, MAX_SEQUENCE_LENGTH, 16, "LengthKnob");
    configParam(TRIGGER_GROUP_1_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup1Button");
    configParam(TRIGGER_GROUP_2_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup2Button");
    configParam(TRIGGER_GROUP_3_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup3Button");
    configParam(TRIGGER_GROUP_4_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup4Button");
    configParam(TRIGGER_GROUP_5_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup5Button");

    trigger_group_buttons[0] = TRIGGER_GROUP_1_BUTTON;
    trigger_group_buttons[1] = TRIGGER_GROUP_2_BUTTON;
    trigger_group_buttons[2] = TRIGGER_GROUP_3_BUTTON;
    trigger_group_buttons[3] = TRIGGER_GROUP_4_BUTTON;
    trigger_group_buttons[4] = TRIGGER_GROUP_5_BUTTON;

    gate_outputs[0] = GATE_OUTPUT_1;
    gate_outputs[1] = GATE_OUTPUT_2;
    gate_outputs[2] = GATE_OUTPUT_3;
    gate_outputs[3] = GATE_OUTPUT_4;
    gate_outputs[4] = GATE_OUTPUT_5;
  }

  json_t *dataToJson() override
  {
    json_t *root = json_object();

    //
    // Prepare seed and trigger patterns for saving
    //

    std::string packed_seed_pattern = sequencer.packPattern(&sequencer.seed);

    std::string packed_trigger_patterns[5];
    for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      packed_trigger_patterns[i] = sequencer.packPattern(&sequencer.triggers[i]);
    }

    //
    // Save seed pattern
    //
    json_object_set_new(root, "seed_pattern", json_string(packed_seed_pattern.c_str()));


    //
    // Save trigger patterns
    //

    json_t *trigger_groups_json_array = json_array();

    for(int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      json_array_append_new(trigger_groups_json_array, json_string(packed_trigger_patterns[i].c_str()));
    }

    json_object_set(root, "trigger_group_patterns", trigger_groups_json_array);
    json_decref(trigger_groups_json_array);

    // Done
    return root;
  }

  void dataFromJson(json_t *root) override
  {
    // Load seed_pattern
    json_t *loaded_seed_pattern_json = json_object_get(root, ("seed_pattern"));
    if(loaded_seed_pattern_json) sequencer.unpackPattern(json_string_value(loaded_seed_pattern_json), &sequencer.seed);

    // It's necessary to restart the sequence because it copies the seed
    // into the current state.  Otherwise, the old default seed would still
    // be used for the first loop of the sequence.
    sequencer.restart_sequence();

    //
    // load trigger group patterns
    //

    json_t *trigger_group_json_array = json_object_get(root, "trigger_group_patterns");

    if(trigger_group_json_array)
    {
      size_t i;
      json_t *loaded_trigger_pattern_json;

      json_array_foreach(trigger_group_json_array, i, loaded_trigger_pattern_json)
      {
        sequencer.unpackPattern(json_string_value(loaded_trigger_pattern_json), &sequencer.triggers[i]);
      }
    }
  }

  void toggleTriggerGroup(int index)
  {
    if(selected_trigger_group_index == index)
    {
      selected_trigger_group_index = -1;
      mode = PLAY_MODE;
    }
    else
    {
      selected_trigger_group_index = index;
      mode = EDIT_TRIGGERS_MODE;
    }
  }

  void process(const ProcessArgs &args) override
  {
    bool trigger_output_pulse = false;

    // Set sequencer length based on LEN knob
    sequencer.setLength(params[LENGTH_KNOB].getValue());

    // Process Reset input
    if(resetTrigger.process(rescale(inputs[RESET_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      // Set up a (reverse) counter so that the clock input will ignore
      // incoming clock pulses for 1 millisecond after a reset input. This
      // is to comply with VCV Rack's standards.  See section "Timing" at
      // https://vcvrack.com/manual/VoltageStandards

      clock_ignore_on_reset = (long) (args.sampleRate / 100);
      stepTrigger.reset();
      sequencer.reset();
    }

    // Process when the user presses one of the 5 buttons above the trigger outputs
    for(unsigned int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      trigger_button_is_triggered[i] = trigger_group_button_schmitt_trigger[i].process(params[trigger_group_buttons[i]].getValue());
      if(trigger_button_is_triggered[i]) toggleTriggerGroup(i);
    }

    // Process Step Input
    if((clock_ignore_on_reset == 0) && stepTrigger.process(rescale(inputs[STEP_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      sequencer.step(trigger_results);

      for(unsigned int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
      {
        if(trigger_results[i]) gateOutputPulseGenerators[i].trigger(0.01f);
      }
    }

    // Output gates
    for(unsigned int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
      outputs[gate_outputs[i]].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
    }

    // Trigger group selection lights
    lights[TRIGGER_GROUP_1_LIGHT].setBrightness(selected_trigger_group_index == 0);
    lights[TRIGGER_GROUP_2_LIGHT].setBrightness(selected_trigger_group_index == 1);
    lights[TRIGGER_GROUP_3_LIGHT].setBrightness(selected_trigger_group_index == 2);
    lights[TRIGGER_GROUP_4_LIGHT].setBrightness(selected_trigger_group_index == 3);
    lights[TRIGGER_GROUP_5_LIGHT].setBrightness(selected_trigger_group_index == 4);

    if (clock_ignore_on_reset > 0) clock_ignore_on_reset--;
  }
};

struct CellularAutomatonDisplay : TransparentWidget
{
  GlitchSequencer *module;
  Vec drag_position;
  bool mouse_lock = false;
  bool cell_edit_value = true;
  int old_row = -1;
  int old_column = -1;

  CellularAutomatonDisplay()
  {
    box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    if(module)
    {
      // testing draw area
      /*
      nvgBeginPath(vg);
      nvgRect(vg, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
      nvgFillColor(vg, nvgRGBA(120, 20, 20, 100));
      nvgFill(vg);
      */

      for(unsigned int row=0; row < SEQUENCER_ROWS; row++)
      {
        for(unsigned int column=0; column < SEQUENCER_COLUMNS; column++)
        {
          nvgBeginPath(vg);
          nvgRect(vg, (column * CELL_WIDTH) + (column * CELL_PADDING), (row * CELL_HEIGHT) + (row * CELL_PADDING), CELL_WIDTH, CELL_HEIGHT);

          // bool cell_is_alive = (module->edit_mode) ? module->sequencer.pattern[row][column] : module->sequencer.state[row][column];

          // Default color for inactive square
          nvgFillColor(vg, nvgRGB(55, 55, 55));

          // When in edit mode, the pattern that's being edited will be bright white
          // and the underlying animation will continue to be shown but at a dim gray
          switch(module->mode)
          {
            case PLAY_MODE:
            if(module->sequencer.seed[row][column]) nvgFillColor(vg, nvgRGB(80, 80, 80));
            if(module->sequencer.state[row][column]) nvgFillColor(vg, nvgRGB(255, 255, 255));
            break;

            case EDIT_SEED_MODE:
            if(module->sequencer.state[row][column]) nvgFillColor(vg, nvgRGB(65, 65, 65));
            if(module->sequencer.seed[row][column]) nvgFillColor(vg, nvgRGB(255, 255, 255));
            break;

            case EDIT_TRIGGERS_MODE:
            if(module->selected_trigger_group_index >= 0)
            {
              if(module->sequencer.state[row][column]) nvgFillColor(vg, nvgRGB(65, 65, 65));
              bool cell_contains_trigger = module->sequencer.triggers[module->selected_trigger_group_index][row][column];
              bool is_triggered = module->sequencer.state[row][column];
              if(cell_contains_trigger) nvgFillColor(vg, nvgRGB(140, 140, 140));
              if(cell_contains_trigger && is_triggered) nvgFillColor(vg, nvgRGB(255, 255, 255));
            }
            break;
          }

          nvgFill(vg);
        }
      }
    }
    // Paint static content for library display
    else
    {
      CellularAutomatonSequencer ca;

      for(unsigned int row=0; row < SEQUENCER_ROWS; row++)
      {
        for(unsigned int column=0; column < SEQUENCER_COLUMNS; column++)
        {
          nvgBeginPath(vg);
          nvgRect(vg, (column * CELL_WIDTH) + (column * CELL_PADDING), (row * CELL_HEIGHT) + (row * CELL_PADDING), CELL_WIDTH, CELL_HEIGHT);

          nvgFillColor(vg, nvgRGB(55, 55, 55)); // Default color for inactive square
          if(ca.seed[row][column]) nvgFillColor(vg, nvgRGB(255, 255, 255));

          nvgFill(vg);
        }
      }
    }

    nvgRestore(vg);
  }


  std::pair<int, int> getRowAndColumnFromVec(Vec position)
  {
    int row = position.y / (CELL_HEIGHT + CELL_PADDING);
    int column = position.x / (CELL_WIDTH + CELL_PADDING);

    return {row, column};
  }

  void setSequencerCell(unsigned int row, unsigned int column, bool value)
  {
    module->sequencer.seed[row][column] = this->cell_edit_value;

    // If the sequencer is at the first step, also update the current "state"
    // The first "state" of the sequencer should always mirror the pattern
    if(module->sequencer.position == 0) module->sequencer.state[row][column] = this->cell_edit_value;
  }

  void onButton(const event::Button &e) override
  {
    e.consume(this);

    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      if(this->mouse_lock == false)
      {
        this->mouse_lock = true;

        int row, column;
        tie(row, column) = getRowAndColumnFromVec(e.pos);

        if(module->mode == EDIT_SEED_MODE)
        {
          // Store the value that's being set for later in case the user
          // drags to set ("paints") additional triggers
          this->cell_edit_value = ! module->sequencer.seed[row][column];

          // Set the cell value in the sequencer
          this->setSequencerCell(row, column, this->cell_edit_value);
        }

        if(module->mode == EDIT_TRIGGERS_MODE && module->selected_trigger_group_index >= 0)
        {
          // Store the value that's being set for later in case the user
          // drags to set ("paints") additional triggers
          this->cell_edit_value = ! module->sequencer.triggers[module->selected_trigger_group_index][row][column];
          module->sequencer.triggers[module->selected_trigger_group_index][row][column] = this->cell_edit_value;
        }

        // Store the initial drag position
        drag_position = e.pos;
      }
    }
    else if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
    {
      this->mouse_lock = false;
    }
  }

  void onDragMove(const event::DragMove &e) override
  {
    TransparentWidget::onDragMove(e);

    double zoom = std::pow(2.f, settings::zoom);
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));

    int row, column;
    tie(row, column)  = getRowAndColumnFromVec(drag_position);

    if((row != old_row) || (column != old_column))
    {
      if(module->mode == EDIT_SEED_MODE)
      {
        this->setSequencerCell(row, column, this->cell_edit_value);
      }

      if(module->mode == EDIT_TRIGGERS_MODE && module->selected_trigger_group_index >= 0)
      {
        module->sequencer.triggers[module->selected_trigger_group_index][row][column] = this->cell_edit_value;
      }

      old_row = row;
      old_column = column;
    }
  }

  void onEnter(const event::Enter &e) override
  {
    TransparentWidget::onEnter(e);
    if(this->module->mode == PLAY_MODE) this->module->mode = EDIT_SEED_MODE;
  }

  void onLeave(const event::Leave &e) override
  {
    TransparentWidget::onLeave(e);
    if(this->module->mode == EDIT_SEED_MODE) this->module->mode = PLAY_MODE;
  }

  void onHover(const event::Hover& e) override {
    TransparentWidget::onHover(e);
    e.consume(this);
  }
};

struct LengthReadoutDisplay : TransparentWidget
{
  GlitchSequencer *module;
  std::shared_ptr<Font> font;

  LengthReadoutDisplay()
  {
    font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    nvgSave(vg);

    std::string text_to_display = "16";

    if(module)
    {
      text_to_display = std::to_string(module->sequencer.length);
    }

    nvgFontSize(vg, 12);
    nvgFontFaceId(vg, font->handle);
    nvgTextAlign(vg, NVG_ALIGN_CENTER);
    nvgTextLetterSpacing(vg, -1);
    nvgFillColor(vg, nvgRGB(3, 3, 3));
    nvgText(vg, 5, 5, text_to_display.c_str(), NULL);

    nvgRestore(vg);

  }
};

struct GlitchSequencerWidget : ModuleWidget
{
  GlitchSequencerWidget(GlitchSequencer* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/glitch_sequencer_front_panel.svg")));

    float button_spacing = 9.6; // 9.1
    float button_group_x = 53.0;
    float button_group_y = 109.0;

    float inputs_y = 116.0;

    // Step
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, inputs_y)), module, GlitchSequencer::STEP_INPUT));

    // Reset
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 13.544, inputs_y)), module, GlitchSequencer::RESET_INPUT));

    // Length
    addParam(createParamCentered<Trimpot>(mm2px(Vec(10 + (13.544 * 2), inputs_y)), module, GlitchSequencer::LENGTH_KNOB));

    // Sequence 1 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x, button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_1_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x, button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_1_LIGHT));
    // Sequence 2 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_2_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_2_LIGHT));
    // Sequence 3 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_3_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_3_LIGHT));
    // Sequence 4 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_4_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_4_LIGHT));
    // Sequence 5 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_5_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_5_LIGHT));

    float y = button_group_y + 10;
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x, y)), module, GlitchSequencer::GATE_OUTPUT_1));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 1.0), y)), module, GlitchSequencer::GATE_OUTPUT_2));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 2.0), y)), module, GlitchSequencer::GATE_OUTPUT_3));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 3.0), y)), module, GlitchSequencer::GATE_OUTPUT_4));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 4.0), y)), module, GlitchSequencer::GATE_OUTPUT_5));

    CellularAutomatonDisplay *ca_display = new CellularAutomatonDisplay();
    ca_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    ca_display->module = module;
    addChild(ca_display);

    LengthReadoutDisplay *length_readout_display = new LengthReadoutDisplay();
    length_readout_display->box.pos = mm2px(Vec(30.0, 122.0));
    length_readout_display->module = module;
    addChild(length_readout_display);
  }

  void appendContextMenu(Menu *menu) override
  {
    GlitchSequencer *module = dynamic_cast<GlitchSequencer*>(this->module);
    assert(module);
  }

};

Model* modelGlitchSequencer = createModel<GlitchSequencer, GlitchSequencerWidget>("glitchsequencer");
