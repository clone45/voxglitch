# WavBank Auto-Increment Feature Implementation

This document explains how to add a context menu option to the WavBank module that automatically increments to the next sample when the current sample finishes playing.

## Overview

The feature will:
- Be **disabled by default** to maintain backward compatibility
- Automatically advance to the next sample when playback completes
- Update the WAV_KNOB parameter to reflect the new position
- Wrap around to the first sample after the last one
- **Automatically disable when CV input is connected** to prevent conflicts with external sequencing

## Implementation Steps

### 1. Add Member Variable to WavBank Module

In `src/modules/WavBank/WavBank.hpp`, add a boolean flag to track the feature state:

```cpp
struct WavBank : VoxglitchSamplerModule
{
    // ... existing members ...

    // Auto-increment feature
    bool auto_increment_on_completion = false;  // Disabled by default

    // ... rest of the class ...
```

### 2. Update JSON Persistence

Modify the save/load methods to persist the setting:

```cpp
// In dataToJson():
json_t *dataToJson() override
{
    json_t *json_root = json_object();
    json_object_set_new(json_root, "path", json_string(this->path.c_str()));
    json_object_set_new(json_root, "trig_input_response_mode", json_integer(trig_input_response_mode));
    json_object_set_new(json_root, "auto_increment_on_completion", json_boolean(auto_increment_on_completion));
    return json_root;
}

// In dataFromJson():
void dataFromJson(json_t *json_root) override
{
    // ... existing code ...

    // Load auto-increment setting
    json_t *auto_increment_json = json_object_get(json_root, "auto_increment_on_completion");
    if (auto_increment_json)
        auto_increment_on_completion = json_boolean_value(auto_increment_json);
}
```

### 3. Add Context Menu Option

In `src/modules/WavBank/WavBankWidget.hpp`, update the `appendContextMenu` method:

```cpp
void appendContextMenu(Menu *menu) override
{
    WavBank *module = dynamic_cast<WavBank *>(this->module);
    assert(module);

    // For spacing only
    menu->addChild(new MenuEntry);

    TriggerModeMenu *trigger_mode_menu = createMenuItem<TriggerModeMenu>("Trigger Mode", RIGHT_ARROW);
    trigger_mode_menu->module = module;
    menu->addChild(trigger_mode_menu);

    // Add auto-increment option
    menu->addChild(createBoolPtrMenuItem("Auto-Increment on Completion", "", &module->auto_increment_on_completion));

    // ... rest of existing menu items ...
}
```

### 4. Implement Auto-Increment Logic

In the `process()` method of `WavBank.hpp`, add the increment logic when end-of-sample is detected:

```cpp
void process(const ProcessArgs &args) override
{
    // ... existing code up to end-of-sample detection ...

    // Detect end of sample (works in both loop and non-loop modes)
    if (selected_sample_player->sample.loaded)
    {
        unsigned int sample_size = selected_sample_player->sample.size() * SAMPLE_END_POSITION;

        if (loop)
        {
            // In loop mode, detect when playback position wraps around
            if (previousPlaybackPosition >= (sample_size - 1) &&
                selected_sample_player->playback_position < previousPlaybackPosition)
            {
                // Sample reached end and looped back
                endOfSamplePulse.trigger(0.01f);

                // Auto-increment if enabled AND no CV control
                // Don't auto-increment when CV is controlling sample selection
                if (auto_increment_on_completion && !inputs[WAV_INPUT].isConnected())
                {
                    incrementSampleSelection();
                }
            }
        }
        else
        {
            // In non-loop mode, detect when sample stops playing
            if (!selected_sample_player->playing && previousPlaybackState)
            {
                // Sample just ended - trigger the pulse
                endOfSamplePulse.trigger(0.01f);

                // Auto-increment if enabled AND no CV control
                // Don't auto-increment when CV is controlling sample selection
                if (auto_increment_on_completion && !inputs[WAV_INPUT].isConnected())
                {
                    incrementSampleSelection();
                }
            }
        }
    }

    // ... rest of process method ...
}
```

### 5. Add Helper Method for Incrementing

Add a new private method to handle the increment logic:

```cpp
private:
    void incrementSampleSelection()
    {
        unsigned int number_of_samples = sample_players.size();
        if (number_of_samples == 0) return;

        // Calculate next sample index with wrap-around
        unsigned int next_sample = (selected_sample_slot + 1) % number_of_samples;

        // Update the WAV_KNOB parameter value
        // The knob ranges from 0.0 to 1.0, so we need to scale appropriately
        float knob_value = (float)next_sample / (float)(number_of_samples - 1);

        // Use setValue to programmatically change the parameter
        // This will update the knob position visually
        params[WAV_KNOB].setValue(knob_value);

        // Note: The actual sample change will happen in the next process() call
        // when wav_input_value is recalculated from the new knob position
    }
```

## Key Technical Details

### Parameter Automation

The critical aspect is using `params[WAV_KNOB].setValue(value)` to programmatically change the knob position. This:
- Updates the visual position of the knob
- Changes the parameter value that will be read on the next `process()` call
- Maintains consistency with manual knob adjustments

### Value Calculation

The WAV_KNOB parameter ranges from 0.0 to 1.0, but represents selection across N samples:
- For N samples, indices range from 0 to N-1
- Knob value = sample_index / (number_of_samples - 1)
- This ensures the last sample is reachable at knob position 1.0

### Timing Considerations

The increment happens when:
- **Non-loop mode**: When `playing` transitions from true to false
- **Loop mode**: When the playback position wraps from end to beginning
- **Never when CV is connected**: The condition `!inputs[WAV_INPUT].isConnected()` prevents auto-increment when external CV is controlling sample selection

### Edge Cases Handled

1. **Empty sample bank**: Check `number_of_samples == 0` before incrementing
2. **Wrap-around**: Use modulo operator to cycle back to first sample
3. **Single sample**: Will stay on the same sample (0 % 1 = 0)
4. **Division by zero**: Avoided by checking sample count first
5. **CV control conflict**: Auto-increment is bypassed when `WAV_INPUT` is connected

## Testing Checklist

- [ ] Verify feature is disabled by default
- [ ] Test with various sample counts (0, 1, 2, many)
- [ ] Confirm knob visually updates when sample increments
- [ ] Test in both loop and non-loop modes
- [ ] Verify wrap-around from last to first sample
- [ ] Check that manual knob adjustments still work
- [ ] Confirm setting persists when saving/loading patches
- [ ] Test with trigger and gate input modes
- [ ] Verify END_OF_SAMPLE_OUTPUT still fires correctly
- [ ] **Verify auto-increment is disabled when CV cable is connected to WAV_INPUT**
- [ ] **Test that disconnecting CV cable allows auto-increment to resume**
- [ ] **Confirm CV control takes precedence over auto-increment**

## Alternative Approaches Considered

1. **Using ParamQuantity**: Could use `getParamQuantity(WAV_KNOB)->setValue()` for additional control
2. **Direct sample selection**: Could bypass the knob and directly set `selected_sample_slot`, but this would break the visual feedback
3. **CV output for sample position**: Could add an output jack for the current sample position for external sequencing

## Future Enhancements

- Add option to increment by more than 1 (skip samples)
- Add reverse/random increment modes
- Add CV input to enable/disable auto-increment
- Add option to stop at last sample instead of wrapping