#pragma once
#include <cstring>

struct PCMBug : VoxglitchModule
{
    static const int BUFFER_SIZE = 4096;
    
    enum ParamIds {
        BIT_OFFSET_PARAM,           // 0-7 bit offset
        DRIFT_PARAM,                // Drift rate
        CHUNK_SIZE_PARAM,           // How often realignment occurs
        BIT_DEPTH_MISMATCH_PARAM,   // Bit depth manipulation
        ENDIAN_FLIP_PARAM,          // Endianness corruption
        CORRUPTION_PROBABILITY_PARAM, // Percentage chance of corruption
        SAMPLE_RATE_CORRUPTION_PARAM, // Sample rate lies (0.5x to 4x)
        BUFFER_SCRAMBLE_PARAM,      // Buffer chunk scrambling intensity
        FILTER_PARAM,               // Anti-aliasing filter cutoff
        CORRUPTION_MIX_PARAM,       // Dry/wet mix
        NUM_PARAMS
    };
    
    enum InputIds {
        AUDIO_L_INPUT,
        AUDIO_R_INPUT,
        BIT_OFFSET_CV_INPUT,
        DRIFT_CV_INPUT,
        CHUNK_SIZE_CV_INPUT,
        CORRUPTION_PROBABILITY_CV_INPUT,
        SAMPLE_RATE_CV_INPUT,
        BUFFER_SCRAMBLE_CV_INPUT,
        FILTER_CV_INPUT,
        TRIGGER_INPUT,              // Trigger for realignment
        NUM_INPUTS
    };
    
    enum OutputIds {
        AUDIO_L_OUTPUT,
        AUDIO_R_OUTPUT,
        NUM_OUTPUTS
    };
    
    enum LightIds {
        CORRUPTION_LIGHT,
        NUM_LIGHTS
    };

    // PCM corruption state
    int16_t pcm_buffer_l[BUFFER_SIZE];
    int16_t pcm_buffer_r[BUFFER_SIZE];
    uint8_t* byte_buffer_l;
    uint8_t* byte_buffer_r;
    
    int buffer_write_pos = 0;
    int bit_offset = 0;
    float drift_accumulator = 0.0f;
    int chunk_counter = 0;
    bool endian_flipped = false;
    
    // Sample rate corruption state
    float sample_rate_phase = 0.0f;
    float sample_rate_multiplier = 1.0f;
    
    // Buffer scrambling state
    int scramble_counter = 0;
    
    // Anti-aliasing filters to reduce harsh digital noise
    dsp::RCFilter lowpass_l;
    dsp::RCFilter lowpass_r;
    
    dsp::SchmittTrigger trigger;
    dsp::PulseGenerator corruption_pulse;

    // Constructor
    PCMBug() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        
        // Configure parameters
        configParam(BIT_OFFSET_PARAM, 0.0f, 7.0f, 0.0f, "Bit Offset", " bits");
        configParam(DRIFT_PARAM, -1.0f, 1.0f, 0.0f, "Drift Rate", " %");
        configParam(CHUNK_SIZE_PARAM, 1.0f, 1024.0f, 64.0f, "Chunk Size", " samples");
        configParam(BIT_DEPTH_MISMATCH_PARAM, 0.0f, 1.0f, 0.0f, "Bit Depth Corruption");
        configParam(ENDIAN_FLIP_PARAM, 0.0f, 1.0f, 0.0f, "Endian Corruption");
        configParam(CORRUPTION_PROBABILITY_PARAM, 0.0f, 1.0f, 0.0f, "Corruption Probability", " %");
        configParam(SAMPLE_RATE_CORRUPTION_PARAM, 0.5f, 4.0f, 1.0f, "Sample Rate Lies", "x");
        configParam(BUFFER_SCRAMBLE_PARAM, 0.0f, 1.0f, 0.0f, "Buffer Scramble");
        configParam(FILTER_PARAM, 0.0f, 1.0f, 0.5f, "Anti-Alias Filter");
        configParam(CORRUPTION_MIX_PARAM, 0.0f, 1.0f, 0.0f, "Corruption Mix", " %");
        
        // Configure inputs/outputs
        configInput(AUDIO_L_INPUT, "Audio Left");
        configInput(AUDIO_R_INPUT, "Audio Right");
        configInput(BIT_OFFSET_CV_INPUT, "Bit Offset CV");
        configInput(DRIFT_CV_INPUT, "Drift CV");
        configInput(CHUNK_SIZE_CV_INPUT, "Chunk Size CV");
        configInput(CORRUPTION_PROBABILITY_CV_INPUT, "Corruption Probability CV");
        configInput(SAMPLE_RATE_CV_INPUT, "Sample Rate CV");
        configInput(BUFFER_SCRAMBLE_CV_INPUT, "Buffer Scramble CV");
        configInput(FILTER_CV_INPUT, "Filter CV");
        configInput(TRIGGER_INPUT, "Trigger");
        
        configOutput(AUDIO_L_OUTPUT, "Audio Left");
        configOutput(AUDIO_R_OUTPUT, "Audio Right");
        
        // Initialize byte buffer pointers
        byte_buffer_l = reinterpret_cast<uint8_t*>(pcm_buffer_l);
        byte_buffer_r = reinterpret_cast<uint8_t*>(pcm_buffer_r);
        
        // Clear buffers
        std::memset(pcm_buffer_l, 0, sizeof(pcm_buffer_l));
        std::memset(pcm_buffer_r, 0, sizeof(pcm_buffer_r));
    }

    void process(const ProcessArgs &args) override {
        // Get filter parameter first for filter setup
        float filter_amount = params[FILTER_PARAM].getValue();
        if (inputs[FILTER_CV_INPUT].isConnected()) {
            filter_amount += inputs[FILTER_CV_INPUT].getVoltage() * 0.1f;
        }
        filter_amount = clamp(filter_amount, 0.0f, 1.0f);
        
        // Update filter cutoff frequency based on parameter
        // Range: 20kHz (no filtering) down to 500Hz (heavy filtering)
        float cutoff_freq = 500.0f + (20000.0f - 500.0f) * filter_amount;
        lowpass_l.setCutoff(cutoff_freq / args.sampleRate);
        lowpass_r.setCutoff(cutoff_freq / args.sampleRate);
        
        // Get parameter values with CV modulation
        float bit_offset_param = params[BIT_OFFSET_PARAM].getValue();
        if (inputs[BIT_OFFSET_CV_INPUT].isConnected()) {
            bit_offset_param += inputs[BIT_OFFSET_CV_INPUT].getVoltage() * 0.7f; // 7 bits max from 10V
        }
        bit_offset_param = clamp(bit_offset_param, 0.0f, 7.0f);
        
        float drift_rate = params[DRIFT_PARAM].getValue();
        if (inputs[DRIFT_CV_INPUT].isConnected()) {
            drift_rate += inputs[DRIFT_CV_INPUT].getVoltage() * 0.1f;
        }
        drift_rate = clamp(drift_rate, -1.0f, 1.0f);
        
        float chunk_size_param = params[CHUNK_SIZE_PARAM].getValue();
        if (inputs[CHUNK_SIZE_CV_INPUT].isConnected()) {
            chunk_size_param += inputs[CHUNK_SIZE_CV_INPUT].getVoltage() * 100.0f;
        }
        chunk_size_param = clamp(chunk_size_param, 1.0f, 1024.0f);
        
        float corruption_probability = params[CORRUPTION_PROBABILITY_PARAM].getValue();
        if (inputs[CORRUPTION_PROBABILITY_CV_INPUT].isConnected()) {
            corruption_probability += inputs[CORRUPTION_PROBABILITY_CV_INPUT].getVoltage() * 0.1f;
        }
        corruption_probability = clamp(corruption_probability, 0.0f, 1.0f);
        
        float sample_rate_corruption = params[SAMPLE_RATE_CORRUPTION_PARAM].getValue();
        if (inputs[SAMPLE_RATE_CV_INPUT].isConnected()) {
            sample_rate_corruption += inputs[SAMPLE_RATE_CV_INPUT].getVoltage() * 0.35f; // 3.5x range from 10V
        }
        sample_rate_corruption = clamp(sample_rate_corruption, 0.5f, 4.0f);
        
        float buffer_scramble = params[BUFFER_SCRAMBLE_PARAM].getValue();
        if (inputs[BUFFER_SCRAMBLE_CV_INPUT].isConnected()) {
            buffer_scramble += inputs[BUFFER_SCRAMBLE_CV_INPUT].getVoltage() * 0.1f;
        }
        buffer_scramble = clamp(buffer_scramble, 0.0f, 1.0f);
        
        float corruption_mix = params[CORRUPTION_MIX_PARAM].getValue();
        
        // Get input audio for early bypass check
        float input_l = inputs[AUDIO_L_INPUT].getVoltage();
        float input_r = inputs[AUDIO_R_INPUT].isConnected() ? 
                       inputs[AUDIO_R_INPUT].getVoltage() : input_l;
        
        // Early bypass if no corruption mix - pass audio through cleanly
        if (corruption_mix <= 0.0f) {
            outputs[AUDIO_L_OUTPUT].setVoltage(input_l);
            outputs[AUDIO_R_OUTPUT].setVoltage(input_r);
            lights[CORRUPTION_LIGHT].setBrightness(0.0f);
            return;
        }
        
        // Handle trigger input for realignment
        if (trigger.process(inputs[TRIGGER_INPUT].getVoltage())) {
            buffer_write_pos = 0;
            chunk_counter = 0;
            corruption_pulse.trigger(0.1f);
        }
        
        // Apply drift to bit offset
        drift_accumulator += drift_rate * args.sampleTime * 10.0f; // Scale drift rate
        if (drift_accumulator >= 1.0f) {
            bit_offset = (bit_offset + 1) % 8;
            drift_accumulator -= 1.0f;
        } else if (drift_accumulator <= -1.0f) {
            bit_offset = (bit_offset - 1 + 8) % 8;
            drift_accumulator += 1.0f;
        }
        
        // Update bit offset from parameter if not drifting
        if (std::abs(drift_rate) < 0.01f) {
            bit_offset = static_cast<int>(bit_offset_param);
        }
        
        // Apply sample rate corruption by changing read position
        sample_rate_multiplier = sample_rate_corruption;
        sample_rate_phase += sample_rate_multiplier;
        
        // Get samples using corrupted sample rate
        int16_t pcm_l, pcm_r;
        if (sample_rate_multiplier != 1.0f) {
            // Sample rate corruption: read from buffer at different rate
            int read_pos = static_cast<int>(sample_rate_phase) % BUFFER_SIZE;
            pcm_l = pcm_buffer_l[read_pos];
            pcm_r = pcm_buffer_r[read_pos];
            
            // Reset phase if it gets too large
            if (sample_rate_phase >= BUFFER_SIZE) {
                sample_rate_phase -= BUFFER_SIZE;
            }
        } else {
            // Normal operation: use current input
            pcm_l = static_cast<int16_t>(clamp(input_l * 3276.7f, -32767.0f, 32767.0f));
            pcm_r = static_cast<int16_t>(clamp(input_r * 3276.7f, -32767.0f, 32767.0f));
        }
        
        // Store current input in buffer for future sample rate corruption
        int16_t current_pcm_l = static_cast<int16_t>(clamp(input_l * 3276.7f, -32767.0f, 32767.0f));
        int16_t current_pcm_r = static_cast<int16_t>(clamp(input_r * 3276.7f, -32767.0f, 32767.0f));
        pcm_buffer_l[buffer_write_pos] = current_pcm_l;
        pcm_buffer_r[buffer_write_pos] = current_pcm_r;
        
        // Apply corruptions
        int16_t corrupted_l = applyCorruptions(pcm_l, buffer_write_pos, true, corruption_probability);
        int16_t corrupted_r = applyCorruptions(pcm_r, buffer_write_pos, false, corruption_probability);
        
        // Apply buffer scrambling
        if (buffer_scramble > 0.0f) {
            corrupted_l = applyBufferScrambling(corrupted_l, buffer_write_pos, true, buffer_scramble);
            corrupted_r = applyBufferScrambling(corrupted_r, buffer_write_pos, false, buffer_scramble);
        }
        
        // Convert back to voltage
        float output_l = static_cast<float>(corrupted_l) / 3276.7f;
        float output_r = static_cast<float>(corrupted_r) / 3276.7f;
        
        // Apply anti-aliasing filter to reduce harsh digital noise
        lowpass_l.process(output_l);
        output_l = lowpass_l.lowpass();
        lowpass_r.process(output_r);
        output_r = lowpass_r.lowpass();
        
        // Apply mix
        output_l = input_l * (1.0f - corruption_mix) + output_l * corruption_mix;
        output_r = input_r * (1.0f - corruption_mix) + output_r * corruption_mix;
        
        // Output
        outputs[AUDIO_L_OUTPUT].setVoltage(output_l);
        outputs[AUDIO_R_OUTPUT].setVoltage(output_r);
        
        // Update lights
        lights[CORRUPTION_LIGHT].setBrightness(corruption_pulse.process(args.sampleTime) ? 1.0f : 
                                               (corruption_mix > 0.1f ? 0.3f : 0.0f));
        
        // Advance buffer position
        buffer_write_pos = (buffer_write_pos + 1) % BUFFER_SIZE;
        chunk_counter++;
        
        // Check for chunk-based realignment
        if (chunk_counter >= static_cast<int>(chunk_size_param)) {
            chunk_counter = 0;
            // Potentially trigger endian flip
            if (params[ENDIAN_FLIP_PARAM].getValue() > 0.5f && 
                random::uniform() < 0.1f) { // 10% chance per chunk
                endian_flipped = !endian_flipped;
                corruption_pulse.trigger(0.05f);
            }
        }
    }
    
    int16_t applyCorruptions(int16_t sample, int pos, bool is_left, float probability) {
        // Check if corruption should occur based on probability
        if (random::uniform() > probability) {
            return sample; // No corruption - return original sample
        }
        
        // Get bytes
        uint8_t low_byte = static_cast<uint8_t>(sample & 0xFF);
        uint8_t high_byte = static_cast<uint8_t>((sample >> 8) & 0xFF);
        
        // Apply endian flip
        if (endian_flipped) {
            std::swap(low_byte, high_byte);
        }
        
        // Apply bit offset corruption
        if (bit_offset > 0) {
            // Shift the bytes and create misalignment
            uint16_t shifted = (static_cast<uint16_t>(high_byte) << 8) | low_byte;
            shifted = (shifted >> bit_offset) | (shifted << (16 - bit_offset));
            low_byte = static_cast<uint8_t>(shifted & 0xFF);
            high_byte = static_cast<uint8_t>((shifted >> 8) & 0xFF);
        }
        
        // Apply bit depth mismatch
        if (params[BIT_DEPTH_MISMATCH_PARAM].getValue() > 0.5f) {
            // Treat 16-bit as 8-bit then expand back
            int8_t as_8bit = static_cast<int8_t>(high_byte);
            return static_cast<int16_t>(as_8bit) << 8;
        }
        
        // Reconstruct sample
        return static_cast<int16_t>((static_cast<uint16_t>(high_byte) << 8) | low_byte);
    }
    
    int16_t applyBufferScrambling(int16_t sample, int pos, bool is_left, float intensity) {
        // Buffer scrambling: randomly swap with other positions in buffer
        if (random::uniform() < intensity) {
            // Calculate scramble distance based on intensity
            int max_distance = static_cast<int>(intensity * BUFFER_SIZE * 0.25f); // Up to 25% of buffer
            int scramble_distance = static_cast<int>(random::uniform() * max_distance);
            
            // Choose random direction
            if (random::uniform() < 0.5f) {
                scramble_distance = -scramble_distance;
            }
            
            // Calculate scramble position with wraparound
            int scramble_pos = (pos + scramble_distance + BUFFER_SIZE) % BUFFER_SIZE;
            
            // Get sample from scrambled position
            if (is_left) {
                return pcm_buffer_l[scramble_pos];
            } else {
                return pcm_buffer_r[scramble_pos];
            }
        }
        
        return sample; // No scrambling
    }
    
    // Save/load state
    json_t *dataToJson() override {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "bit_offset", json_integer(bit_offset));
        json_object_set_new(rootJ, "endian_flipped", json_boolean(endian_flipped));
        json_object_set_new(rootJ, "sample_rate_phase", json_real(sample_rate_phase));
        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override {
        json_t *bit_offset_j = json_object_get(rootJ, "bit_offset");
        if (bit_offset_j) bit_offset = json_integer_value(bit_offset_j);
        
        json_t *endian_j = json_object_get(rootJ, "endian_flipped");
        if (endian_j) endian_flipped = json_boolean_value(endian_j);
        
        json_t *phase_j = json_object_get(rootJ, "sample_rate_phase");
        if (phase_j) sample_rate_phase = json_real_value(phase_j);
    }
};