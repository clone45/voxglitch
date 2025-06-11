struct VectorRotation : Module {
    enum ParamIds {
        ALPHA_PARAM,  // X rotation
        BETA_PARAM,   // Y rotation
        GAMMA_PARAM,  // Z rotation
        NUM_PARAMS
    };
   
    enum InputIds {
        ALPHA_INPUT,
        BETA_INPUT,
        GAMMA_INPUT,
        X_INPUT,      // X coordinate input
        Y_INPUT,      // Y coordinate input
        NUM_INPUTS
    };
   
    enum OutputIds {
        X_OUTPUT,
        Y_OUTPUT,
        Z_OUTPUT,
        NUM_OUTPUTS
    };

    const float twoPi = 2.f * M_PI;
   
    VectorRotation() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        configParam(ALPHA_PARAM, 0.f, twoPi, 0.f, "X Rotation");
        configParam(BETA_PARAM, 0.f, twoPi, 0.f, "Y Rotation");
        configParam(GAMMA_PARAM, 0.f, twoPi, 0.f, "Z Rotation");
        
        configInput(X_INPUT, "X Coordinate");
        configInput(Y_INPUT, "Y Coordinate");
    }

    void rotatePoint(float* x, float* y, float* z, float alpha, float beta, float gamma) {
        // Rotate around X
        float y1 = *y * std::cos(alpha) - *z * std::sin(alpha);
        float z1 = *y * std::sin(alpha) + *z * std::cos(alpha);
        *y = y1;
        *z = z1;
        
        // Rotate around Y
        float x1 = *x * std::cos(beta) + *z * std::sin(beta);
        float z2 = -*x * std::sin(beta) + *z * std::cos(beta);
        *x = x1;
        *z = z2;
        
        // Rotate around Z
        float x2 = *x * std::cos(gamma) - *y * std::sin(gamma);
        float y2 = *x * std::sin(gamma) + *y * std::cos(gamma);
        *x = x2;
        *y = y2;
    }

    void process(const ProcessArgs& args) override
    {
        // Get rotation angles (in radians)
        float alpha = params[ALPHA_PARAM].getValue() + inputs[ALPHA_INPUT].getVoltage() * 0.1f * twoPi;
        float beta = params[BETA_PARAM].getValue() + inputs[BETA_INPUT].getVoltage() * 0.1f * twoPi;
        float gamma = params[GAMMA_PARAM].getValue() + inputs[GAMMA_INPUT].getVoltage() * 0.1f * twoPi;
       
        // Get input coordinates
        float x = inputs[X_INPUT].getVoltage() / 5.f; // Scale from ±5V to ±1 range
        float y = inputs[Y_INPUT].getVoltage() / 5.f;
        float z = 0.f; // Default Z depth for input signals
       
        // Apply rotation
        rotatePoint(&x, &y, &z, alpha, beta, gamma);
       
        // Output the rotated coordinates as voltages (-5V to +5V range)
        outputs[X_OUTPUT].setVoltage(5.f * x);
        outputs[Y_OUTPUT].setVoltage(5.f * y);
        outputs[Z_OUTPUT].setVoltage(5.f * z);
    }
};