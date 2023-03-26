#pragma once

#include <string>
#include <vector>

class Module {
public:
    virtual void update() = 0;
    virtual std::string getType() const = 0;
    virtual std::vector<std::string> getInputNames() const = 0;
    virtual std::vector<std::string> getOutputNames() const = 0;
    // virtual float getInputValue(const std::string& name) const = 0;
    // virtual void setOutputValue(const std::string& name, float value) = 0;
};