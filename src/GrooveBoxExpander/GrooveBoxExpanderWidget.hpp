#include "widgets/GrooveboxExpanderSoftButton.hpp"
#include "widgets/GrooveboxExpanderSoftButtonGreen.hpp"
#include "widgets/GrooveboxExpanderSoloButton.hpp"
#include "widgets/GrooveboxExpanderMuteButton.hpp"

struct GrooveBoxExpanderWidget : VoxglitchModuleWidget
{
  // 1st column 20

  float columns[NUMBER_OF_TRACKS] = {
    10 * 2.952756,
    20 * 2.952756,
    30 * 2.952756,
    40 * 2.952756,
    50 * 2.952756,
    60 * 2.952756,
    70 * 2.952756,
    80 * 2.952756
  };

  float volume_knob_positions[NUMBER_OF_TRACKS][2] = {
    {25.411 + 6.0, 30 * 2.952756},
    {25.411 + 6.0, 40 * 2.952756},
    {25.411 + 6.0, 50 * 2.952756},
    {25.411 + 6.0, 60 * 2.952756},
    {25.411 + 6.0, 70 * 2.952756},
    {25.411 + 6.0, 80 * 2.952756},
    {25.411 + 6.0, 90 * 2.952756},
    {25.411 + 6.0, 100 * 2.952756}
  };

  float mute_buttons_row = 40.74;
  float solo_buttons_row = 118.11024;
  float gate_outputs_row = 345.898;
  float volume_knobs_row = 178.79;
  float pan_knobs_row = 233.69;
  float pitch_knobs_row = 288.57;

  struct ModdedCL1362 : SvgPort {
  	ModdedCL1362() {
  		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/modded_CL1362.svg")));
  	}
  };

  GrooveBoxExpanderWidget(GrooveBoxExpander* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox_expander/groove_box_expander_front_panel.svg")));

    for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
    {
      float x = columns[i];

      // Add volume, pan, and pitch knobs
      addParam(createParamCentered<Trimpot>(Vec(x, volume_knobs_row), module, GrooveBoxExpander::VOLUME_KNOBS + i));
      addParam(createParamCentered<Trimpot>(Vec(x, pan_knobs_row), module, GrooveBoxExpander::PAN_KNOBS + i));
      addParam(createParamCentered<Trimpot>(Vec(x, pitch_knobs_row), module, GrooveBoxExpander::PITCH_KNOBS + i));

      //
      // Add mute buttons and inputs
      //
      GrooveboxExpanderMuteButton *mute_button = createParamCentered<GrooveboxExpanderMuteButton>(Vec(x, mute_buttons_row), module, GrooveBoxExpander::MUTE_BUTTONS + i);
      mute_button->module = module;
      addParam(mute_button);
      addInput(createInputCentered<PJ301MPort>(Vec(x, mute_buttons_row + 29.52756), module, GrooveBoxExpander::MUTE_INPUTS + i));

      //
      // Add solo buttons
      //
      GrooveboxExpanderSoloButton *solo_button = createParamCentered<GrooveboxExpanderSoloButton>(Vec(x, solo_buttons_row), module, GrooveBoxExpander::SOLO_BUTTONS + i);
      solo_button->module = module;
      solo_button->track_index = i;
      addParam(solo_button);

      //
      // Add track trigger outputs
      //
      addOutput(createOutputCentered<ModdedCL1362>(Vec(x, gate_outputs_row), module, GrooveBoxExpander::TRIGGER_OUTPUTS + i));
      addChild(createLightCentered<SmallLight<RedLight>>(Vec(x, gate_outputs_row + 20), module, GrooveBoxExpander::GATE_OUTPUT_LIGHTS + i));

    }

    // Add "connected" light indicator
    addChild(createLightCentered<SmallLight<RedLight>>(Vec(75.0, 7.5), module, GrooveBoxExpander::CONNECTED_LIGHT));
    
  }

  void onHoverKey(const event::HoverKey &e) override
  {
    GrooveBoxExpander *module = dynamic_cast<GrooveBoxExpander*>(this->module);
    assert(module);

    // Read and store shift key status
    module->shift_key = ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT);

    ModuleWidget::onHoverKey(e);
  }

};
