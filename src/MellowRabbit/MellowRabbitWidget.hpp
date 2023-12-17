struct MellowRabbitWidget : VoxglitchModuleWidget
{
    MellowRabbitWidget(MellowRabbit *module)
    {
        setModule(module);

        // Load and apply theme
        theme.load("mellow_rabbit");
        applyTheme();
    }
};
