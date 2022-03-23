#include "kick.hpp"

#define KICK_SAMPLE_LENGTH 22820

struct Circadian : Module
{
  dsp::SchmittTrigger playback_trigger;
  float left_audio = 0;
  float right_audio = 0;
  bool playback = false;
  double playback_position = 0;

  enum ParamIds {
    PLAYBACK_BUTTON,
    NUM_PARAMS
  };
  enum InputIds {
    NUM_INPUTS
  };
  enum OutputIds {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
    NUM_OUTPUTS
  };

  Circadian()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
  }

  void process(const ProcessArgs &args) override
  {
    if(playback_trigger.process(params[PLAYBACK_BUTTON].getValue()))
    {
      // reset sample playback position
      playback_position = 0;

      // Set playback flag.  The sample will playback while this is true
      playback = true;
    }

    if(playback)
    {
      // 44100 is the sample rate of the recorded sample
      float step_amount = 44100 / args.sampleRate;

      // Step the playback position forward.
      playback_position = playback_position + step_amount;

      // convert float to integer
      unsigned int sample_position = playback_position;

      // If the playback position is past the playback length, end sample playback
      if(sample_position >= KICK_SAMPLE_LENGTH)
      {
        playback_position = 0;
        playback = false;
      }
      else
      {
        left_audio = kick_drum[sample_position][0];
        right_audio = kick_drum[sample_position][1];

        outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_audio);
        outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_audio);
      }
    }
  }
};

struct CircadianWidget : ModuleWidget
{
  CircadianWidget(Circadian* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/looper_front_panel.svg")));

    // Add output jacks
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.560, 35.0)), module, Circadian::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.560, 40.0)), module, Circadian::AUDIO_OUTPUT_RIGHT));

    // Add playback input
    addParam(createParamCentered<LEDButton>(mm2px(Vec(7.560, 5)), module, Circadian::PLAYBACK_BUTTON));
  }

  void appendContextMenu(Menu *menu) override
  {
    Circadian *module = dynamic_cast<Circadian*>(this->module);
    assert(module);
  }
};
