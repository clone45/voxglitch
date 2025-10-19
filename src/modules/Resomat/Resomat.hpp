#pragma once

#include "DelayLine.hpp"
#include "DampingFilter.hpp"
#include "DCBlocker.hpp"
#include "excitation/NoiseExcitation.hpp"
#include "excitation/BytebeatExcitation.hpp"

struct Resomat : Module
{
    enum ParamIds {
        PITCH_PARAM,
        DAMPING_PARAM,
        DECAY_PARAM,
        EQUATION_SELECT_PARAM,
        EQUATION_OFFSET_PARAM,
        EQUATION_RATE_PARAM,
        RATE_SYNC_BUTTON,
        BLEED_PARAM,
        INJECTION_PARAM,
        P1_PARAM,
        P2_PARAM,
        P3_PARAM,
        SOURCE_BUTTON,
        TRIGGER_BUTTON,
        DAMPING_ATTEN_PARAM,
        DECAY_ATTEN_PARAM,
        EQUATION_ATTEN_PARAM,
        OFFSET_ATTEN_PARAM,
        RATE_ATTEN_PARAM,
        BLEED_ATTEN_PARAM,
        INJECTION_ATTEN_PARAM,
        P1_ATTEN_PARAM,
        P2_ATTEN_PARAM,
        P3_ATTEN_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        VOCT_INPUT,
        TRIGGER_INPUT,
        DAMPING_INPUT,
        DECAY_INPUT,
        EQUATION_INPUT,
        OFFSET_INPUT,
        RATE_INPUT,
        BLEED_INPUT,
        INJECTION_INPUT,
        P1_INPUT,
        P2_INPUT,
        P3_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        TRIGGER_LIGHT,
        NOISE_LIGHT,
        EQUATION_LIGHT,
        EQUATION_CONTINUOUS_LIGHT,
        RATE_SYNC_LIGHT,
        NUM_LIGHTS
    };

    // DSP components
    DelayLine delayLine;
    DampingFilter dampingFilter;
    DCBlocker dcBlocker;
    NoiseExcitation noiseSource;
    BytebeatExcitation bytebeatSource;

    // Trigger detection
    dsp::SchmittTrigger triggerDetector;
    dsp::SchmittTrigger sourceButtonTrigger;
    dsp::SchmittTrigger rateSyncButtonTrigger;
    dsp::PulseGenerator triggerLight;

    // State
    float prevSample = 0.0;
    bool noteActive = false;
    float sampleRate = 44100.0;
    int sourceMode = 0; // 0=Noise, 1=Equation, 2=Equation Continuous
    bool rateSyncEnabled = false; // Toggle state for rate sync
    uint32_t continuousTime = 0; // Continuously running counter for continuous mode

    // Injection state
    bool isInjecting = false;
    int injectionSamplesRemaining = 0;
    uint32_t injectionTime = 0; // Time counter for bytebeat injection
    float injectionRate = 1.0;

    Resomat()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // Configure parameters
        configParam(PITCH_PARAM, -5.f, 5.f, 0.f, "Pitch", " Hz", 2.f, dsp::FREQ_C4);
        configParam(DAMPING_PARAM, 0.0f, 1.0f, 0.5f, "Damping", "%", 0.f, 100.f);
        configParam(DECAY_PARAM, 0.1f, 10.0f, 1.0f, "Decay", " s");
        configParam(EQUATION_SELECT_PARAM, 0.f, 21.f, 0.f, "Equation");
        paramQuantities[EQUATION_SELECT_PARAM]->snapEnabled = true;
        configParam(EQUATION_OFFSET_PARAM, 0.f, 4095.f, 0.f, "Equation Offset");
        configParam(EQUATION_RATE_PARAM, 0.01f, 8.0f, 1.0f, "Equation Rate", "x");
        configParam(BLEED_PARAM, 0.0f, 0.5f, 0.0f, "Bleed", "%", 0.f, 100.f);
        configParam(INJECTION_PARAM, 0.0f, 10.0f, 0.0f, "Injection", "x");
        configParam(P1_PARAM, 0.f, 255.f, 128.f, "P1");
        configParam(P2_PARAM, 0.f, 255.f, 64.f, "P2");
        configParam(P3_PARAM, 0.f, 255.f, 32.f, "P3");
        configButton(SOURCE_BUTTON, "Source Select");
        configButton(TRIGGER_BUTTON, "Trigger");
        configButton(RATE_SYNC_BUTTON, "Rate Sync");

        // Configure attenuverters
        configParam(DAMPING_ATTEN_PARAM, 0.f, 1.f, 0.f, "Damping CV Amount", "%", 0.f, 100.f);
        configParam(DECAY_ATTEN_PARAM, 0.f, 1.f, 0.f, "Decay CV Amount", "%", 0.f, 100.f);
        configParam(EQUATION_ATTEN_PARAM, 0.f, 1.f, 0.f, "Equation CV Amount", "%", 0.f, 100.f);
        configParam(OFFSET_ATTEN_PARAM, 0.f, 1.f, 0.f, "Offset CV Amount", "%", 0.f, 100.f);
        configParam(RATE_ATTEN_PARAM, -1.f, 1.f, 0.f, "Rate CV Amount", "%", 0.f, 100.f);
        configParam(BLEED_ATTEN_PARAM, 0.f, 1.f, 0.f, "Bleed CV Amount", "%", 0.f, 100.f);
        configParam(INJECTION_ATTEN_PARAM, 0.f, 1.f, 0.f, "Injection CV Amount", "%", 0.f, 100.f);
        configParam(P1_ATTEN_PARAM, 0.f, 1.f, 0.f, "P1 CV Amount", "%", 0.f, 100.f);
        configParam(P2_ATTEN_PARAM, 0.f, 1.f, 0.f, "P2 CV Amount", "%", 0.f, 100.f);
        configParam(P3_ATTEN_PARAM, 0.f, 1.f, 0.f, "P3 CV Amount", "%", 0.f, 100.f);

        // Configure inputs
        configInput(VOCT_INPUT, "V/Oct");
        configInput(TRIGGER_INPUT, "Trigger");
        configInput(DAMPING_INPUT, "Damping CV");
        configInput(DECAY_INPUT, "Decay CV");
        configInput(EQUATION_INPUT, "Equation CV");
        configInput(OFFSET_INPUT, "Offset CV");
        configInput(RATE_INPUT, "Rate CV");
        configInput(BLEED_INPUT, "Bleed CV");
        configInput(INJECTION_INPUT, "Injection CV");
        configInput(P1_INPUT, "P1 CV");
        configInput(P2_INPUT, "P2 CV");
        configInput(P3_INPUT, "P3 CV");

        // Configure outputs
        configOutput(AUDIO_OUTPUT, "Audio");
    }

    void onSampleRateChange() override
    {
        sampleRate = APP->engine->getSampleRate();
    }

    void process(const ProcessArgs &args) override
    {
        // Get pitch with V/Oct tracking
        float pitch = params[PITCH_PARAM].getValue();
        if (inputs[VOCT_INPUT].isConnected())
        {
            pitch += inputs[VOCT_INPUT].getVoltage();
        }

        // Calculate frequency
        float frequency = dsp::FREQ_C4 * pow(2.0f, pitch);
        frequency = clamp(frequency, 20.0f, args.sampleRate * 0.45f);

        // Calculate equation rate with CV - either synced to pitch or from knob
        float rateKnob = params[EQUATION_RATE_PARAM].getValue();
        if (inputs[RATE_INPUT].isConnected())
        {
            float cv = inputs[RATE_INPUT].getVoltage() / 10.f; // Normalize 0-10V to 0-1
            rateKnob += cv * params[RATE_ATTEN_PARAM].getValue() * 8.0f; // Scale by max range
        }
        rateKnob = clamp(rateKnob, 0.01f, 8.0f);

        float rate;
        if (rateSyncEnabled)
        {
            // Sync to pitch, but multiply by rate knob for harmonic/subharmonic control
            rate = (frequency / dsp::FREQ_C4) * rateKnob;
        }
        else
        {
            rate = rateKnob;
        }
        lights[RATE_SYNC_LIGHT].setBrightness(rateSyncEnabled ? 1.0f : 0.0f);

        // Handle source button - cycle through source modes
        if (sourceButtonTrigger.process(params[SOURCE_BUTTON].getValue(), 0.1f, 1.0f))
        {
            sourceMode = (sourceMode + 1) % 3; // Cycle: 0 -> 1 -> 2 -> 0
        }

        // Handle rate sync button - toggle sync state
        if (rateSyncButtonTrigger.process(params[RATE_SYNC_BUTTON].getValue(), 0.1f, 1.0f))
        {
            rateSyncEnabled = !rateSyncEnabled;
        }

        // Update source LEDs
        lights[NOISE_LIGHT].setBrightness(sourceMode == 0 ? 1.0f : 0.0f);
        lights[EQUATION_LIGHT].setBrightness(sourceMode == 1 ? 1.0f : 0.0f);
        lights[EQUATION_CONTINUOUS_LIGHT].setBrightness(sourceMode == 2 ? 1.0f : 0.0f);

        // Update equation and parameters every frame for immediate response
        // This ensures equation knob, P1/P2/P3 knobs respond in real-time
        if (sourceMode == 1 || sourceMode == 2)
        {
            bytebeatSource.setEquation(getEquationIndex());
            bytebeatSource.setParameters(getP1(), getP2(), getP3());
        }

        // Detect trigger
        bool triggered = triggerDetector.process(
            inputs[TRIGGER_INPUT].getVoltage() + params[TRIGGER_BUTTON].getValue(),
            0.1f,
            1.0f
        );

        if (triggered)
        {
            exciteString(frequency, args.sampleRate, rate);
            triggerLight.trigger(0.001f);
            noteActive = true;
        }

        // Process audio
        float output = 0.0;
        if (noteActive)
        {
            // Read from delay line
            output = delayLine.read();

            // Apply damping filter
            float dampingParam = params[DAMPING_PARAM].getValue();
            if (inputs[DAMPING_INPUT].isConnected())
            {
                float cv = inputs[DAMPING_INPUT].getVoltage() / 10.f; // Normalize 0-10V to 0-1
                dampingParam += cv * params[DAMPING_ATTEN_PARAM].getValue() * 1.0f; // Scale by max range (0-1)
            }
            dampingParam = clamp(dampingParam, 0.0f, 1.0f);
            float cutoff = 500.0f + dampingParam * 4500.0f;
            dampingFilter.setCutoff(cutoff, args.sampleRate);
            output = dampingFilter.process(output);

            // Apply bytebeat bleed (only in bytebeat modes)
            float bleed = params[BLEED_PARAM].getValue();
            if (inputs[BLEED_INPUT].isConnected())
            {
                float cv = inputs[BLEED_INPUT].getVoltage() / 10.f; // Normalize 0-10V to 0-1
                bleed += cv * params[BLEED_ATTEN_PARAM].getValue() * 0.5f; // Scale by max range (0.5)
            }
            bleed = clamp(bleed, 0.0f, 0.5f);

            if ((sourceMode == 1 || sourceMode == 2) && bleed > 0.0f)
            {
                // Equation and parameters already set earlier in process() every frame
                // Scale continuousTime by sample rate normalization and rate parameter
                uint32_t scaledTime = (uint32_t)(continuousTime * (44100.0f / args.sampleRate) * rate);

                float bytebeatSample = bytebeatSource.generateSingle(scaledTime);
                output = output * (1.0f - bleed) + bytebeatSample * bleed;
            }

            // Add injection if active
            if (isInjecting)
            {
                float injectionSample = 0.0f;

                if (sourceMode == 0)
                {
                    // Noise injection - generate fresh random sample
                    injectionSample = (random::uniform() * 2.0f - 1.0f);
                }
                else if (sourceMode == 1 || sourceMode == 2)
                {
                    // Bytebeat injection - uses current real-time equation/parameters (already set at line 209)
                    // Scale time by sample rate normalization and stored rate
                    uint32_t scaledTime = (uint32_t)(injectionTime * (44100.0f / args.sampleRate) * injectionRate);
                    injectionSample = bytebeatSource.generateSingle(scaledTime);

                    injectionTime++;
                }

                // Add injection to output
                output += injectionSample;

                // Decrement injection counter
                injectionSamplesRemaining--;
                if (injectionSamplesRemaining <= 0)
                {
                    isInjecting = false;
                }
            }

            // Calculate feedback gain for decay
            float decayTime = params[DECAY_PARAM].getValue();
            if (inputs[DECAY_INPUT].isConnected())
            {
                float cv = inputs[DECAY_INPUT].getVoltage() / 10.f; // Normalize 0-10V to 0-1
                decayTime += cv * params[DECAY_ATTEN_PARAM].getValue() * 9.9f; // Scale by max range (10.0 - 0.1)
            }
            decayTime = clamp(decayTime, 0.1f, 10.0f);
            float feedback = exp(-6.91f / (frequency * decayTime)) / cos(M_PI * frequency / args.sampleRate);
            feedback = clamp(feedback, 0.9f, 0.9999f);

            // Apply DC blocker and feedback
            output = dcBlocker.process(output);
            output *= feedback;

            // Push back to delay line
            delayLine.push(output);

            // Scale to ±5V range
            output *= 5.0f;
        }

        // Set output
        outputs[AUDIO_OUTPUT].setVoltage(output);

        // Update lights
        lights[TRIGGER_LIGHT].setBrightness(triggerLight.process(args.sampleTime));

        // Increment continuous time counter for continuous bytebeat mode
        continuousTime++;
    }

    void exciteString(float frequency, float sampleRate, float rate)
    {
        // Calculate delay line length
        int length = (int)(sampleRate / frequency);
        length = clamp(length, 10, MAX_DELAY_SIZE);

        // Set delay line length
        delayLine.setLength(length);

        // Generate excitation using the selected source
        float excitation[MAX_DELAY_SIZE];

        if (sourceMode == 0)
        {
            // Noise excitation
            noiseSource.generate(excitation, length);
        }
        else if (sourceMode == 1)
        {
            // Bytebeat equation excitation with CV
            bytebeatSource.setEquation(getEquationIndex());
            bytebeatSource.setParameters(getP1(), getP2(), getP3());
            bytebeatSource.generate(excitation, length, getOffset(), sampleRate, rate);
        }
        else if (sourceMode == 2)
        {
            // Continuous bytebeat equation - uses continuously running time counter
            bytebeatSource.setEquation(getEquationIndex());
            bytebeatSource.setParameters(getP1(), getP2(), getP3());
            bytebeatSource.generate(excitation, length, continuousTime + getOffset(), sampleRate, rate);
        }

        // Fill delay line with excitation
        delayLine.fill(excitation, length);

        // Reset filters
        dampingFilter.reset();
        dcBlocker.reset();
        prevSample = excitation[length - 1];

        // Initialize injection if injection parameter > 0
        float injection = params[INJECTION_PARAM].getValue();
        if (inputs[INJECTION_INPUT].isConnected())
        {
            float cv = inputs[INJECTION_INPUT].getVoltage() / 10.f; // Normalize 0-10V to 0-1
            injection += cv * params[INJECTION_ATTEN_PARAM].getValue() * 10.0f; // Scale by max range
        }
        injection = clamp(injection, 0.0f, 10.0f);

        if (injection > 0.0f)
        {
            isInjecting = true;
            injectionSamplesRemaining = (int)(length * injection);
            injectionTime = length; // Start injection after the initial excitation period

            // Initialize injection timing based on source mode
            if (sourceMode == 1)
            {
                // For triggered equation mode, add offset to injection start time
                injectionTime += getOffset();
            }
            else if (sourceMode == 2)
            {
                // For continuous mode, use current continuous time
                injectionTime = continuousTime + length;
            }

            injectionRate = rate;
        }
        else
        {
            isInjecting = false;
        }
    }

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "sourceMode", json_integer(sourceMode));
        json_object_set_new(rootJ, "rateSyncEnabled", json_boolean(rateSyncEnabled));
        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {
        json_t *sourceModeJ = json_object_get(rootJ, "sourceMode");
        if (sourceModeJ)
            sourceMode = json_integer_value(sourceModeJ);

        json_t *rateSyncEnabledJ = json_object_get(rootJ, "rateSyncEnabled");
        if (rateSyncEnabledJ)
            rateSyncEnabled = json_boolean_value(rateSyncEnabledJ);
    }

private:
    // Helper function to get parameter value with CV and attenuverter
    float getParameterWithCV(int paramId, int inputId, int attenId, float maxRange, float min, float max)
    {
        float value = params[paramId].getValue();
        if (inputs[inputId].isConnected())
        {
            float cv = inputs[inputId].getVoltage() / 10.f;
            value += cv * params[attenId].getValue() * maxRange;
        }
        return clamp(value, min, max);
    }

    // Specific parameter getters
    int getEquationIndex()
    {
        return (int)getParameterWithCV(EQUATION_SELECT_PARAM, EQUATION_INPUT, EQUATION_ATTEN_PARAM, 21.0f, 0.0f, 21.0f);
    }

    uint32_t getP1()
    {
        return (uint32_t)getParameterWithCV(P1_PARAM, P1_INPUT, P1_ATTEN_PARAM, 255.0f, 0.0f, 255.0f);
    }

    uint32_t getP2()
    {
        return (uint32_t)getParameterWithCV(P2_PARAM, P2_INPUT, P2_ATTEN_PARAM, 255.0f, 0.0f, 255.0f);
    }

    uint32_t getP3()
    {
        return (uint32_t)getParameterWithCV(P3_PARAM, P3_INPUT, P3_ATTEN_PARAM, 255.0f, 0.0f, 255.0f);
    }

    uint32_t getOffset()
    {
        return (uint32_t)getParameterWithCV(EQUATION_OFFSET_PARAM, OFFSET_INPUT, OFFSET_ATTEN_PARAM, 4095.0f, 0.0f, 4095.0f);
    }
};
