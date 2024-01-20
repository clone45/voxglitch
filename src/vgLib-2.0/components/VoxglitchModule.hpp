struct VoxglitchModule : Module
{
    template <typename T = rack::SwitchQuantity>

    T *configOnOff(int paramId, float defaultValue, std::string name)
    {
        return configSwitch<T>(paramId, 0, 1, defaultValue, name, {"Off", "On"});
    }

    VoxglitchModule()
    {
    }
};
