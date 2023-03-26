// OutModule.hpp

#pragma once

#include "../BaseModule.hpp"

class OutputModule : public BaseModule {
public:
    OutputModule() {
        // Add input port
        addInput("this.in");
        addOutput("this.out");
    }

    void process(unsigned int sample_rate) override {
        output_ports[0]->setValue(input_ports[0]->getValue());
    }
};