#pragma once
#include <rack.hpp>

struct TempestVS1 : Module
{
    enum ParamIds {
        // Main knobs (M1-M4)
        M1_KNOB_PARAM,
        M2_KNOB_PARAM,
        M3_KNOB_PARAM,
        M4_KNOB_PARAM,

        // Small knobs (SM1-SM4)
        SM_KNOB1_PARAM,
        SM2_KNOB_PARAM,
        SM_3_KNOB_PARAM,
        SM4_KNOB_PARAM,

        // Central controls
        EPIC_KNOB_PARAM,
        CENTER_KNOB_PARAM,
        ROTARY_KNOB_PARAM,  // Visual only, no output

        // Buttons
        MODE_BUTTON_PARAM,
        PRESET_BUTTON_PARAM,
        CLEAR_BUTTON_PARAM,

        // Switch
        MODE_SWITCH_PARAM,

        // MIDI button
        MIDI_BUTTON_PARAM,

        NUM_PARAMS
    };
    
    enum InputIds {
        NUM_INPUTS
    };
    
    enum OutputIds {
        // Main knobs (M1-M4)
        M1_KNOB_OUTPUT,
        M2_KNOB_OUTPUT,
        M3_KNOB_OUTPUT,
        M4_KNOB_OUTPUT,
        
        // Small knobs (SM1-SM4)
        SM_KNOB1_OUTPUT,
        SM2_KNOB_OUTPUT,
        SM_3_KNOB_OUTPUT,
        SM4_KNOB_OUTPUT,
        
        // Central controls
        EPIC_KNOB_OUTPUT,
        CENTER_KNOB_OUTPUT,

        // Encoder triggers
        ENCODER_LEFT_OUTPUT,
        ENCODER_RIGHT_OUTPUT,

        // Buttons
        MODE_BUTTON_OUTPUT,
        PRESET_BUTTON_OUTPUT,
        CLEAR_BUTTON_OUTPUT,
        
        // Switches
        MODE_SWITCH_OUTPUT,
        
        NUM_OUTPUTS
    };
    
    enum LightIds {
        MIDI_ACTIVITY_LIGHT,
        NUM_LIGHTS
    };

    midi::InputQueue midiInput;
    
    // Legacy MIDI note outputs
    float currentCV = 0.0f;
    float currentGate = 0.0f;
    float currentVelocity = 0.0f;
    
    // Pot values (0-10V) - 10 pots mapped to the knob outputs

    // Knob mapping structure for flexible pot-to-output assignment
    struct KnobMapping {
        int potIndex;        // Which pot value to use
        const char* label;   // For debugging/display
    };

    // Mapping configuration: easily change which pot controls which output
    KnobMapping knobMappings[10] = {
        {8, "M1"},     // M1_KNOB_OUTPUT gets pot 8 (top left)
        {9, "M2"},     // M2_KNOB_OUTPUT gets pot 9 (bottom left)
        {5, "M3"},     // M3_KNOB_OUTPUT gets pot 5 (top right)
        {2, "M4"},     // M4_KNOB_OUTPUT gets pot 2
        {4, "SM1"},    // SM_KNOB1_OUTPUT gets pot 4
        {1, "SM2"},    // SM2_KNOB_OUTPUT gets pot 1
        {3, "SM3"},    // SM_3_KNOB_OUTPUT gets pot 3
        {0, "SM4"},    // SM4_KNOB_OUTPUT gets pot 0
        {7, "Epic"},   // EPIC_KNOB_OUTPUT gets pot 7 (center large)
        {6, "Center"}  // CENTER_KNOB_OUTPUT gets pot 6 (center right)
    };

    // Map pot indices to parameter IDs for MIDI control
    ParamIds potToParam[10] = {
        SM4_KNOB_PARAM,    // Pot 0 → SM4
        SM2_KNOB_PARAM,    // Pot 1 → SM2
        M4_KNOB_PARAM,     // Pot 2 → M4
        SM_3_KNOB_PARAM,   // Pot 3 → SM3
        SM_KNOB1_PARAM,    // Pot 4 → SM1
        M3_KNOB_PARAM,     // Pot 5 → M3
        CENTER_KNOB_PARAM, // Pot 6 → Center
        EPIC_KNOB_PARAM,   // Pot 7 → Epic
        M1_KNOB_PARAM,     // Pot 8 → M1
        M2_KNOB_PARAM      // Pot 9 → M2
    };
    
    // Encoder state
    dsp::PulseGenerator encoderLeftPulse;
    dsp::PulseGenerator encoderRightPulse;
    float lastRotaryKnobValue = 0.0f;  // Track previous value to detect changes

    // Button pulse generators
    dsp::PulseGenerator modeButtonPulse;
    dsp::PulseGenerator presetButtonPulse;
    dsp::PulseGenerator clearButtonPulse;

    // Button triggers for detecting clicks
    dsp::SchmittTrigger modeButtonTrigger;
    dsp::SchmittTrigger presetButtonTrigger;
    dsp::SchmittTrigger clearButtonTrigger;

    // Bank state
    enum Bank { BANK_A, BANK_B };
    Bank currentBank = BANK_A;
    
    dsp::PulseGenerator activityPulse;
    
    TempestVS1() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // Configure knob parameters (0V to 10V range like MIDI CC values)
        configParam(M1_KNOB_PARAM, 0.0f, 10.0f, 0.0f, "M1 Knob", "V");
        configParam(M2_KNOB_PARAM, 0.0f, 10.0f, 0.0f, "M2 Knob", "V");
        configParam(M3_KNOB_PARAM, 0.0f, 10.0f, 0.0f, "M3 Knob", "V");
        configParam(M4_KNOB_PARAM, 0.0f, 10.0f, 0.0f, "M4 Knob", "V");
        configParam(SM_KNOB1_PARAM, 0.0f, 10.0f, 0.0f, "SM1 Knob", "V");
        configParam(SM2_KNOB_PARAM, 0.0f, 10.0f, 0.0f, "SM2 Knob", "V");
        configParam(SM_3_KNOB_PARAM, 0.0f, 10.0f, 0.0f, "SM3 Knob", "V");
        configParam(SM4_KNOB_PARAM, 0.0f, 10.0f, 0.0f, "SM4 Knob", "V");
        configParam(EPIC_KNOB_PARAM, 0.0f, 10.0f, 0.0f, "Epic Knob", "V");
        configParam(CENTER_KNOB_PARAM, 0.0f, 10.0f, 0.0f, "Center Knob", "V");
        configParam(ROTARY_KNOB_PARAM, -10000.0f, 10000.0f, 0.0f, "Rotary Encoder");
        paramQuantities[ROTARY_KNOB_PARAM]->snapEnabled = true;

        configParam(MODE_BUTTON_PARAM, 0.0f, 1.0f, 0.0f, "Mode Button");
        configParam(PRESET_BUTTON_PARAM, 0.0f, 1.0f, 0.0f, "Preset Button");
        configParam(CLEAR_BUTTON_PARAM, 0.0f, 1.0f, 0.0f, "Clear Button");

        configParam(MODE_SWITCH_PARAM, 0.0f, 1.0f, 0.0f, "Mode Switch");

        configParam(MIDI_BUTTON_PARAM, 0.0f, 1.0f, 0.0f, "MIDI Config");

        // Main knobs (M1-M4)
        configOutput(M1_KNOB_OUTPUT, "M1 Knob");
        configOutput(M2_KNOB_OUTPUT, "M2 Knob");
        configOutput(M3_KNOB_OUTPUT, "M3 Knob");
        configOutput(M4_KNOB_OUTPUT, "M4 Knob");
        
        // Small knobs (SM1-SM4)
        configOutput(SM_KNOB1_OUTPUT, "SM Knob 1");
        configOutput(SM2_KNOB_OUTPUT, "SM Knob 2");
        configOutput(SM_3_KNOB_OUTPUT, "SM Knob 3");
        configOutput(SM4_KNOB_OUTPUT, "SM Knob 4");
        
        // Central controls
        configOutput(EPIC_KNOB_OUTPUT, "Epic Knob");
        configOutput(CENTER_KNOB_OUTPUT, "Center Knob");

        // Encoder triggers
        configOutput(ENCODER_LEFT_OUTPUT, "Encoder Left Step");
        configOutput(ENCODER_RIGHT_OUTPUT, "Encoder Right Step");

        // Buttons
        configOutput(MODE_BUTTON_OUTPUT, "Mode Button");
        configOutput(PRESET_BUTTON_OUTPUT, "Preset Button");
        configOutput(CLEAR_BUTTON_OUTPUT, "Clear Button");
        
        // Switches
        configOutput(MODE_SWITCH_OUTPUT, "Mode Switch");
        
        configLight(MIDI_ACTIVITY_LIGHT, "MIDI Activity");
    }

    void process(const ProcessArgs &args) override {
        midi::Message msg;
        while (midiInput.tryPop(&msg, args.frame)) {
            processMidiMessage(msg);
        }

        // Output parameter values directly
        outputs[M1_KNOB_OUTPUT].setVoltage(params[M1_KNOB_PARAM].getValue());
        outputs[M2_KNOB_OUTPUT].setVoltage(params[M2_KNOB_PARAM].getValue());
        outputs[M3_KNOB_OUTPUT].setVoltage(params[M3_KNOB_PARAM].getValue());
        outputs[M4_KNOB_OUTPUT].setVoltage(params[M4_KNOB_PARAM].getValue());
        outputs[SM_KNOB1_OUTPUT].setVoltage(params[SM_KNOB1_PARAM].getValue());
        outputs[SM2_KNOB_OUTPUT].setVoltage(params[SM2_KNOB_PARAM].getValue());
        outputs[SM_3_KNOB_OUTPUT].setVoltage(params[SM_3_KNOB_PARAM].getValue());
        outputs[SM4_KNOB_OUTPUT].setVoltage(params[SM4_KNOB_PARAM].getValue());
        outputs[EPIC_KNOB_OUTPUT].setVoltage(params[EPIC_KNOB_PARAM].getValue());
        outputs[CENTER_KNOB_OUTPUT].setVoltage(params[CENTER_KNOB_PARAM].getValue());

        // Detect rotary knob changes and trigger encoder outputs
        float currentRotaryValue = params[ROTARY_KNOB_PARAM].getValue();
        if (currentRotaryValue != lastRotaryKnobValue) {
            if (currentRotaryValue > lastRotaryKnobValue) {
                // Turned right
                encoderRightPulse.trigger(0.01f);  // 10ms trigger
            } else {
                // Turned left
                encoderLeftPulse.trigger(0.01f);  // 10ms trigger
            }
            lastRotaryKnobValue = currentRotaryValue;
        }

        // Encoder trigger outputs
        outputs[ENCODER_LEFT_OUTPUT].setVoltage(encoderLeftPulse.process(args.sampleTime) ? 10.0f : 0.0f);
        outputs[ENCODER_RIGHT_OUTPUT].setVoltage(encoderRightPulse.process(args.sampleTime) ? 10.0f : 0.0f);

        // Detect button clicks from UI
        if (modeButtonTrigger.process(params[MODE_BUTTON_PARAM].getValue())) {
            modeButtonPulse.trigger(0.1f); // 100ms trigger
        }
        if (presetButtonTrigger.process(params[PRESET_BUTTON_PARAM].getValue())) {
            presetButtonPulse.trigger(0.1f); // 100ms trigger
        }
        if (clearButtonTrigger.process(params[CLEAR_BUTTON_PARAM].getValue())) {
            clearButtonPulse.trigger(0.1f); // 100ms trigger
        }

        // Button outputs - output 10V gates while buttons are held
        outputs[MODE_BUTTON_OUTPUT].setVoltage(modeButtonPulse.process(args.sampleTime) ? 10.0f : 0.0f);
        outputs[PRESET_BUTTON_OUTPUT].setVoltage(presetButtonPulse.process(args.sampleTime) ? 10.0f : 0.0f);
        outputs[CLEAR_BUTTON_OUTPUT].setVoltage(clearButtonPulse.process(args.sampleTime) ? 10.0f : 0.0f);

        // Switch output - 0V (down) or 10V (up)
        outputs[MODE_SWITCH_OUTPUT].setVoltage(params[MODE_SWITCH_PARAM].getValue() * 10.0f);
        
        bool activityLight = activityPulse.process(args.sampleTime);
        lights[MIDI_ACTIVITY_LIGHT].setBrightness(activityLight ? 1.0f : 0.0f);
    }
    
    void processMidiMessage(const midi::Message& msg) {
        uint8_t status = msg.bytes[0] & 0xF0;
        uint8_t data1 = msg.bytes.size() > 1 ? msg.bytes[1] : 0;
        uint8_t data2 = msg.bytes.size() > 2 ? msg.bytes[2] : 0;

        switch (status) {
            case 0x90: // Note On (legacy support)
                if (data2 > 0) {
                    currentCV = (data1 - 60) / 12.0f;
                    currentGate = 10.0f;
                    currentVelocity = data2 / 127.0f * 10.0f;
                    activityPulse.trigger(0.1f);
                } else {
                    currentGate = 0.0f;
                }
                break;

            case 0x80: // Note Off (legacy support)
                currentGate = 0.0f;
                break;
                
            case 0xB0: // Control Change (VS-1 main functionality)
                {
                    uint8_t cc = data1;
                    uint8_t value = data2;
                    
                    // Button handling (CC#22-24)
                    if (cc == 22) { // Preset button
                        presetButtonPulse.trigger(0.1f); // 100ms trigger
                        activityPulse.trigger(0.1f);
                        break;
                    }
                    if (cc == 23) { // Mode button
                        modeButtonPulse.trigger(0.1f); // 100ms trigger
                        activityPulse.trigger(0.1f);
                        break;
                    }
                    if (cc == 24) { // Clear button
                        clearButtonPulse.trigger(0.1f); // 100ms trigger
                        activityPulse.trigger(0.1f);
                        break;
                    }

                    // A/B Switch (CC#25: 0=Bank A, 127=Bank B)
                    if (cc == 25) {
                        currentBank = (value >= 64) ? BANK_B : BANK_A;
                        params[MODE_SWITCH_PARAM].setValue((currentBank == BANK_B) ? 1.0f : 0.0f);
                        activityPulse.trigger(0.1f);
                        break;
                    }
                    
                    // Pots: Map LaunchControl XL MK2 CCs for testing
                    // Top row: CC#13-19 → Pots 0-6 (CC20 reserved for encoder)
                    // Bottom row: CC#29-30 → Pots 8-9 (Epic, Center)
                    int potIndex = -1;

                    if (cc >= 13 && cc <= 19) {
                        potIndex = cc - 13; // CC#13→Pot0, CC#14→Pot1, etc.
                    } else if (cc == 29) {
                        potIndex = 8; // Epic knob
                    } else if (cc == 30) {
                        potIndex = 9; // Center knob
                    }
                    
                    if (potIndex >= 0) {
                        float newValue = (value / 127.0f) * 10.0f; // Convert to 0-10V

                        // Set parameter directly - this will automatically update the knob visual
                        params[potToParam[potIndex]].setValue(newValue);

                        activityPulse.trigger(0.1f);
                        break;
                    }

                    // VS-1 Hardware Encoder (CC#20: 127=step right, 0=step left)
                    if (cc == 20) {
                        float step = (value >= 64) ? 1.0f : -1.0f;
                        float newEncoderValue = params[ROTARY_KNOB_PARAM].getValue() + step;
                        params[ROTARY_KNOB_PARAM].setValue(newEncoderValue);

                        activityPulse.trigger(0.1f);
                        break;
                    }
                    // Original VS-1 hardware mapping (keep for future)
                    // Pots: Bank A (CC#0-9) or Bank B (CC#10-19)
                    if ((cc <= 9) || (cc >= 10 && cc <= 19)) {
                        potIndex = (cc <= 9) ? cc : (cc - 10); // Map to 0-9
                        
                        // Only update if this CC matches current bank
                        bool isCorrectBank = (currentBank == BANK_A && cc <= 9) || 
                                           (currentBank == BANK_B && cc >= 10);
                        
                        if (isCorrectBank) {
                            float newValue = (value / 127.0f) * 10.0f; // Convert to 0-10V

                            // Set parameter directly - this will automatically update the knob visual
                            params[potToParam[potIndex]].setValue(newValue);

                            activityPulse.trigger(0.1f);
                        }
                        break;
                    }
                }
                break;

            default:
                break;
        }
    }
    
    json_t *dataToJson() override {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "midi", midiInput.toJson());
        json_object_set_new(rootJ, "currentBank", json_integer(currentBank));
        // Parameters (including encoderValue) are automatically saved by VCV Rack
        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override {
        json_t *midiJ = json_object_get(rootJ, "midi");
        if (midiJ)
            midiInput.fromJson(midiJ);

        json_t *bankJ = json_object_get(rootJ, "currentBank");
        if (bankJ)
            currentBank = (Bank)json_integer_value(bankJ);
        // Parameters (including encoderValue) are automatically loaded by VCV Rack
    }
};