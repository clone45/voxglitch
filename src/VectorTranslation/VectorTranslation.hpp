struct VectorTranslation : Module {
    enum ParamIds {
        TRANS_X_PARAM,  // X translation
        TRANS_Y_PARAM,  // Y translation
        BLANK_TIME_PARAM,
        NUM_PARAMS
    };
   
    enum InputIds {
        TRANS_X_INPUT,  // CV for X translation
        TRANS_Y_INPUT,  // CV for Y translation
        X_INPUT,        // X coordinate input
        Y_INPUT,        // Y coordinate input
        NUM_INPUTS
    };
   
    enum OutputIds {
        X_OUTPUT,
        Y_OUTPUT,
        BLANK_OUTPUT,
        NUM_OUTPUTS
    };

    float prevX = 0.f;
    float prevY = 0.f;
    dsp::PulseGenerator blankPulse;
    const float BLANK_TIME = 0.0001f;

    const float DEFAULT_BLANK_TIME = 0.0001f;  // 100 microseconds
    const float MIN_BLANK_TIME = 0.00001f;     // 10 microseconds
    const float MAX_BLANK_TIME = 0.0005f;      // 500 microseconds

    VectorTranslation() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        configParam(TRANS_X_PARAM, -5.f, 5.f, 0.f, "X Translation");
        configParam(TRANS_Y_PARAM, -5.f, 5.f, 0.f, "Y Translation");
        configParam(BLANK_TIME_PARAM, 0.f, 1.f, 0.5f, "Blank Time", "μs", 
                   MIN_BLANK_TIME * 1e6,   // Convert to microseconds for display
                   MAX_BLANK_TIME * 1e6,
                   DEFAULT_BLANK_TIME * 1e6);
        
        configInput(X_INPUT, "X Coordinate");
        configInput(Y_INPUT, "Y Coordinate");
    }

    bool isWrapping(float prev, float current) {
        return std::abs(current - prev) > 2.f;
    }

    float wrapCoordinate(float coord) {
        if (coord > 2.5f) return coord - 5.f;
        if (coord < -2.5f) return coord + 5.f;
        return coord;
    }

    void process(const ProcessArgs& args) override {
        // Get translation amounts
        float transX = params[TRANS_X_PARAM].getValue() + inputs[TRANS_X_INPUT].getVoltage();
        float transY = params[TRANS_Y_PARAM].getValue() + inputs[TRANS_Y_INPUT].getVoltage();
        
        // Get and translate input coordinates
        float newX = inputs[X_INPUT].getVoltage() + transX;
        float newY = inputs[Y_INPUT].getVoltage() + transY;
        
        // Always wrap the coordinates
        newX = wrapCoordinate(newX);
        newY = wrapCoordinate(newY);
        
        // Check if we wrapped and trigger pulse if so
        bool wrapping = isWrapping(prevX, newX) || isWrapping(prevY, newY);

        // Calculate blank time from knob position (exponential scaling)
        float knobValue = params[BLANK_TIME_PARAM].getValue();
        float blankTime = std::pow(2.f, (knobValue * 2.f - 1.f)) * DEFAULT_BLANK_TIME;
        blankTime = clamp(blankTime, MIN_BLANK_TIME, MAX_BLANK_TIME);

        if (wrapping) {
            blankPulse.trigger(blankTime);
        }
        
        // Process pulse and set output (note inverted logic: 5V normal, 0V during blank)
        outputs[BLANK_OUTPUT].setVoltage(blankPulse.process(args.sampleTime) ? 0.f : 5.f);
        
        // Store current coordinates for next frame
        prevX = newX;
        prevY = newY;
        
        // Output
        outputs[X_OUTPUT].setVoltage(newX);
        outputs[Y_OUTPUT].setVoltage(newY);
    }
};